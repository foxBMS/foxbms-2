/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * We kindly request you to use one or more of the following phrases to refer to
 * foxBMS in your hardware, software, documentation or advertising materials:
 *
 * - &Prime;This product uses parts of foxBMS&reg;&Prime;
 * - &Prime;This product includes parts of foxBMS&reg;&Prime;
 * - &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/**
 * @file    sps.c
 * @author  foxBMS Team
 * @date    2020-10-14 (date of creation)
 * @updated 2021-11-10 (date of last update)
 * @ingroup DRIVERS
 * @prefix  SPS
 *
 * @brief   Driver for the smart power switches.
 *
 */

/*========== Includes =======================================================*/
#include "sps.h"

#include "database.h"
#include "io.h"
#include "mcu.h"
#include "os.h"
#include "pex.h"
#include "spi.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
/** state indicator of the state-machine */
static SPS_STATE_e sps_state = SPS_START;

/** timer for the state-machine */
static uint8_t sps_timer = 0;

/** SPI buffer:
 *  Tx part used to issue a write or read register command
 *  Rx part is unused
 * @{
 */
static uint16_t sps_spiTxRegisterBuffer[SPS_SPI_BUFFERSIZE] = {0};
static uint16_t sps_spiRxRegisterBuffer[SPS_SPI_BUFFERSIZE] = {0};
/**@}*/

/** SPI buffer:
 *  Tx part used to write to contactor output control register
 *  Rx part used to read answer from former read register command
 *
 *  When a read register command was issued on the MOSI line,
 *  the answers comes during the next command (read or write)
 *  on the MISO line
 *  (data sheet figure 6 page 9 in data sheet Rev. 2 - 11 September 2019)
 *
 *  In the SPS driver, there are always two communications with the SPS IC
 *  each time the state machine is triggered:
 *  - first a register access (read or write)
 *  - then a write access to the channel output control register
 *  This way, the channel outputs are switched each time the state machine
 *  is triggered, and during the second access, if a read access was done
 *  before, the answer comes during the second access.
 * @{
 */
static uint16_t sps_spiTxWriteToChannelChannelControlRegister[SPS_SPI_BUFFERSIZE] = {0};
static uint16_t sps_spiRxReadAnswerDuringChannelControl[SPS_SPI_BUFFERSIZE]       = {0};
/**@}*/

/** amount of ticks to wait when toggling the reset */
#define SPS_RESET_WAIT_TIME_TICKS (5u)

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief   Assemble a general command to the SPS ICs
 * @details For each call of the #SPS_Ctrl() function two transactions are
 *          sent. For details please read the documentation of #SPS_Transmit().
 *
 *          This function has to be called before the transaction in order to
 *          update the TX buffer with a general command to the SPS chips in the
 *          daisy-chain. The actual command (and with that registers) that
 *          shall be accessed and sent is controlled by the parameter that is
 *          passed on with this function. *
 * @param[in]   action  decides which command should be used
 */
static void SPS_SetCommandTxBuffer(const SPS_ACTION_e action);

/**
 * @brief   Assemble an update to the channel state of the SPS ICs
 * @details For each call of the #SPS_Ctrl() function two transactions are
 *          sent. For details please read the documentation of #SPS_Transmit().
 *
 *          This function has to be called before the transaction in order to
 *          update the TX buffer with a channel configuration to the SPS chips
 *          in the daisy-chain. The actual contactor or channel state is taken
 *          from the current state of the array #sps_channelStatus.
 */
static void SPS_SetContactorsTxBuffer(void);

/**
 * @brief   Transmit all assembled messages for one timeframe
 * @details This function leverages the fact that the SPS are connected in a
 *          "daisy-chain". This way the chain acts as a large shift register.
 *          When sending a message into the chain it is clocked in until every
 *          chip in the chain as its message. Then chip-select is released and
 *          the messages parsed. The answer is then assembled by the chip in
 *          its register and clocked out and received by the master with the
 *          next transmission.
 *
 *          This function sends out a general command to each chip that has
 *          been assembled with #SPS_SetCommandTxBuffer(). The clocked out
 *          information that is received of this transaction is stored in a
 *          separate buffer and is the answer from the last transaction of the
 *          timeframe before the current one.
 *
 *          After this transaction the answer to the general command is
 *          retrieved and the transmission is used to clock in an update of the
 *          state of the contactor channels. This update has been assembled
 *          before the call of this function with the function
 *          #SPS_SetContactorsTxBuffer().
 * @returns returns STD_OK if both transactions have been successful
 */
static STD_RETURN_TYPE_e SPS_Transmit(void);

/**
 * @brief   Read the current measurements for a specific output
 *          for all SPS ICs of the daisy-chain.
 * @details A read command must be issued for all SPS ICs in the daisy chain
 *          first. There is one register to read for each output.
 *          This funcion i used to retrieve the answer on the MISO line
 *          after the write command to the contactors output registers
 *          was made.
 *          The result is written to SPS_CoilCurrent[].
 * @param[in]   outputAllDevices    Output (1 to 4) to be read. Value between
 *                                  1-4 instead of 0-3 to match numbering
 *                                  in data sheet.
 * @return  TODO
 */
static void SPS_GlobalReadCurrent(const uint8_t outputAllDevices);

/** initialize the IO ports for the contactor driver */
static void SPS_InitializeIo(void);

/** Init TX buffers with 0 */
static void SPS_InitializeBuffers(void);
/**
 * @brief   Sets the SPI Tx buffer for a register read access
 *          for all SPS ICs of the daisy-chain.
 * @details The same command is issued for all SPS ICs in the daisy chain.
 *          This means that the same register is read
 *          to all SPS ICs in the daisy chain.
 * @param[in]   address             address of register accessed
 * @param[in]   controlOrDiagnostic type of read access,
 *                                  #SPS_READ_DIAGNOSTIC_REGISTER for diagnostic
 *                                  register, #SPS_READ_CONTROL_REGISTER for
 *                                  control register.
 * @param[out]  pSpiTxBuffer        Tx buffer used for the transmission
 */
static void SPS_GlobalRegisterRead(
    const uint16_t address,
    const SPS_READ_TYPE_e controlOrDiagnostic,
    uint16_t *pSpiTxBuffer);

/**
 * @brief   Sets the SPI Tx buffer for a register write access
 *          for all SPS ICs of the daisy-chain.
 * @details The same command is issued for all SPS ICs in the daisy chain.
 *          This means that the same value is written
 *          to all SPS ICs in the daisy chain.
 *          The current value in register is overwritten by writeData.
 *
 * @param[in]   address         address of register accessed
 * @param[in]   writeData       data to write
 * @param[out]  pSpiTxBuffer    Tx buffer used for the transmission
 */
static void SPS_GlobalRegisterWrite(const uint16_t address, uint8_t writeData, uint16_t *pSpiTxBuffer);

/**
 * @brief   Sets the SPI Tx buffer for a register write access
 *          for a specific single SPS ICs in the daisy-chain.
 * @details The same command is issued for all SPS ICs in the daisy chain.
 *          This means that the same value is written
 *          to all SPS ICs in the daisy chain.
 *          This function does not set the Tx buffer to 0.
 *          This must be done manually.
 * @param[in]   device          device number in the daisy-chain to be written to.
 * @param[in]   address         address of register accessed
 * @param[in]   writeData       data to write. In case of read access, this
 *                              field is ignored and can be set to 0u.
 * @param[in]   writeType       defines whether the value in register should be
 *                              replaced by writeData, or or-ed with writeType,
 *                              or and-ed with writeData
 * @param[out]  pSpiTxBuffer    Tx buffer used for the transmission
 */
static void SPS_SingleDeviceRegisterWrite(
    uint8_t device,
    const uint16_t address,
    uint8_t writeData,
    SPS_WRITE_TYPE_e writeType,
    uint16_t *pSpiTxBuffer);

/**
 * @brief   Request the functional state of a channel
 * @details Pass a value of #SPS_CHANNEL_FUNCTION_e and a channel index number
 *          in order to request this state.
 * @param[in]   channelIndex    number of the channel that should be accessed;
 *                              do not confuse with contactor channel!
 * @param[in]   channelFunction requested functional state of the channel
 */
static void SPS_RequestChannelState(SPS_CHANNEL_INDEX channelIndex, SPS_CHANNEL_FUNCTION_e channelFunction);

/*========== Static Function Implementations ================================*/
static void SPS_InitializeIo(void) {
    /** Pin to drive reset line of SPS ICs */
    SPS_RESET_GIO_PORT_DIR |= (uint32)((uint32)1u << SPS_RESET_PIN);
    /** Pin to drive Chip Select line of SPS ICs */
    SPS_SPI_CS_GIOPORT_DIR |= (uint32)((uint32)1u << SPS_SPI_CS_PIN);

    /* set feedback enable as output */
    SETBIT(SPS_FEEDBACK_GIO_PORT_DIR, SPS_FEEDBACK_PIN);
    /* enable output on feedback enable */
    IO_PinSet(&SPS_FEEDBACK_GIO_PORT, SPS_FEEDBACK_PIN);
}

static void SPS_GlobalRegisterRead(
    const uint16_t address,
    const SPS_READ_TYPE_e controlOrDiagnostic,
    uint16_t *pSpiTxBuffer) {
    FAS_ASSERT(pSpiTxBuffer != NULL_PTR);
    if (controlOrDiagnostic == SPS_READ_DIAGNOSTIC_REGISTER) {
        for (uint8_t i = 0u; i < SPS_SPI_BUFFERSIZE; i++) {
            /* R/W bit = 0 to read */
            pSpiTxBuffer[i] = (address << SPS_ADDRESS_BIT_START); /* Register address */
        }
    } else if (controlOrDiagnostic == SPS_READ_CONTROL_REGISTER) {
        for (uint8_t i = 0u; i < SPS_SPI_BUFFERSIZE; i++) {
            pSpiTxBuffer[i] = (address << SPS_ADDRESS_BIT_START) |          /* Register address */
                              ((uint16_t)1u << SPS_DIAG_CTRL_BIT_POSITION); /* Select diagnostic register */
        }
    } else {
        /* Invalid access type */
        FAS_ASSERT(FAS_TRAP);
    }
}

static void SPS_GlobalRegisterWrite(const uint16_t address, uint8_t writeData, uint16_t *pSpiTxBuffer) {
    FAS_ASSERT(pSpiTxBuffer != NULL_PTR);
    for (uint8_t i = 0u; i < SPS_SPI_BUFFERSIZE; i++) {
        pSpiTxBuffer[i] = ((uint16_t)1u << SPS_RW_BIT_POSITION) | /* R/W bit = 1 to write */
                          (address << SPS_ADDRESS_BIT_START) |    /* Register address */
                          (writeData);                            /* Data to write */
    }
}

static void SPS_SingleDeviceRegisterWrite(
    uint8_t device,
    const uint16_t address,
    uint8_t writeData,
    SPS_WRITE_TYPE_e writeType,
    uint16_t *pSpiTxBuffer) {
    FAS_ASSERT(pSpiTxBuffer != NULL_PTR);
    /* The index (SPS_SPI_BUFFERSIZE - 1 - device) is used because the devices
       in the daisy-chain are accessed like a shift register. The first message
       sent to the daisy-chain will arrive to the last device and the last
       message will arrive to the first device. To communicate with
       devices 0-1-2 in the daisy-chain, the sequence 2-1-0 has to be sent.
     */
    /* Keep the previous data which lies in the lower 8 bits */
    uint8_t preceedingWriteData = (uint8_t)((pSpiTxBuffer[(SPS_SPI_BUFFERSIZE - 1u - device)]) & 0xFFu);
    /* Clear write data which will be replaced */
    pSpiTxBuffer[(SPS_SPI_BUFFERSIZE - 1u - device)] &= 0xFF00u;
    /* Write R/W bit and address in the higher 8 bits */
    pSpiTxBuffer[(SPS_SPI_BUFFERSIZE - 1u - device)] = (uint16_t)(
        ((uint16_t)1u << SPS_RW_BIT_POSITION) | /* R/W bit = 1 to write */
        (address << SPS_ADDRESS_BIT_START));    /* Register address */

    if (writeType == SPS_replaceCurrentValue) {
        pSpiTxBuffer[(SPS_SPI_BUFFERSIZE - 1u - device)] |= (uint16_t)writeData; /* Data to write */
    } else if (writeType == SPS_orWithCurrentValue) {
        pSpiTxBuffer[(SPS_SPI_BUFFERSIZE - 1u - device)] |=
            ((uint16_t)preceedingWriteData | (uint16_t)writeData); /* Data to write */
    } else if (writeType == SPS_andWithCurrentValue) {
        pSpiTxBuffer[(SPS_SPI_BUFFERSIZE - 1u - device)] |=
            ((uint16_t)preceedingWriteData & (uint16_t)writeData); /* Data to write */
    } else {
        /* Invalid write type */
        FAS_ASSERT(FAS_TRAP);
    }
}

static void SPS_SetCommandTxBuffer(const SPS_ACTION_e action) {
    switch (action) {
        case SPS_ACTION_CONFIGURE_CONTROL_REGISTER:
            SPS_GlobalRegisterWrite(
                SPS_GLOBAL_CONTROL_REGISTER_ADDRESS,
                /* transition to Normal mode and set drive strength to strong (This is required for high speed!) */
                (SPS_NORMAL_MODE << SPS_MODE_BIT_START) | (SPS_STRONG_DRIVE << SPS_DRIVE_STRENGTH_BIT_START),
                sps_spiTxRegisterBuffer);
            break;

        case SPS_ACTION_TRIGGER_CURRENT_MEASUREMENT:
            SPS_GlobalRegisterWrite(
                SPS_C_CONTROL_REGISTER_ADDRESS,
                0x000F, /* Measure all four outputs for current and voltage on demand*/
                sps_spiTxRegisterBuffer);
            break;

        case SPS_ACTION_READ_CURRENT_MEASUREMENT1:
            SPS_GlobalRegisterRead(
                SPS_OD_IOUT1_DIAG_REGISTER_ADDRESS, SPS_READ_DIAGNOSTIC_REGISTER, sps_spiTxRegisterBuffer);
            break;

        case SPS_ACTION_READ_CURRENT_MEASUREMENT2:
            SPS_GlobalRegisterRead(
                SPS_OD_IOUT2_DIAG_REGISTER_ADDRESS, SPS_READ_DIAGNOSTIC_REGISTER, sps_spiTxRegisterBuffer);
            break;

        case SPS_ACTION_READ_CURRENT_MEASUREMENT3:
            SPS_GlobalRegisterRead(
                SPS_OD_IOUT3_DIAG_REGISTER_ADDRESS, SPS_READ_DIAGNOSTIC_REGISTER, sps_spiTxRegisterBuffer);
            break;

        case SPS_ACTION_READ_CURRENT_MEASUREMENT4:
            SPS_GlobalRegisterRead(
                SPS_OD_IOUT4_DIAG_REGISTER_ADDRESS, SPS_READ_DIAGNOSTIC_REGISTER, sps_spiTxRegisterBuffer);
            break;

        case SPS_ACTION_READ_EN_IRQ_PIN:
            SPS_GlobalRegisterRead(
                SPS_EN_IRQ_PIN_DIAG_REGISTER_ADDRESS, SPS_READ_DIAGNOSTIC_REGISTER, sps_spiTxRegisterBuffer);
            break;

        default:
            for (uint8_t i = 0u; i < SPS_SPI_BUFFERSIZE; i++) {
                sps_spiTxRegisterBuffer[i] = 0x00;
            }
            break;
    }
}

static void SPS_InitializeBuffers(void) {
    for (uint8_t i = 0u; i < SPS_SPI_BUFFERSIZE; i++) {
        sps_spiTxRegisterBuffer[i] = 0u;
        SPS_SingleDeviceRegisterWrite(
            i,
            SPS_OUTPUT_CONTROL_REGISTER_ADDRESS,
            0u,                      /* data to write, output control */
            SPS_replaceCurrentValue, /* replace because we want to initialize with 0 */
            sps_spiTxWriteToChannelChannelControlRegister);
    }
}

static void SPS_SetContactorsTxBuffer(void) {
    for (SPS_CHANNEL_INDEX channel = 0u; channel < SPS_NR_OF_AVAILABLE_SPS_CHANNELS; channel++) {
        SPS_CHANNEL_STATE_s *pChannel = &sps_channelStatus[channel];
        /* If channel state is changed by request, process request */
        if (pChannel->channelRequested != pChannel->channel) {
            pChannel->channel = pChannel->channelRequested;

            /**
             * With this computation, the contactor number is linked to the SPS IC
             * position in the daisy-chain which is driving this contactor.
             * Contactor 0-3: SPS IC 0
             * Contactor 4-7: SPS IC 1
             * Contactor 8-11: SPS IC 2
             * ...
             */
            uint8_t spsDevicePositionInDaisyChain = channel / SPS_NR_CONTACTOR_PER_IC;

            if (pChannel->channelRequested == SPS_CHANNEL_ON) {
                SPS_SingleDeviceRegisterWrite(
                    spsDevicePositionInDaisyChain,
                    SPS_OUTPUT_CONTROL_REGISTER_ADDRESS,
                    1u << (channel % SPS_NR_CONTACTOR_PER_IC), /* data to write, output control */
                    SPS_orWithCurrentValue,                    /* OR because we want to set additional bits to 1 */
                    sps_spiTxWriteToChannelChannelControlRegister);
            } else if (pChannel->channelRequested == SPS_CHANNEL_OFF) {
                uint8_t writeData = (~(1u << (channel % SPS_NR_CONTACTOR_PER_IC)) & 0xFFu);
                SPS_SingleDeviceRegisterWrite(
                    spsDevicePositionInDaisyChain,
                    SPS_OUTPUT_CONTROL_REGISTER_ADDRESS,
                    writeData,               /* data to write, output control */
                    SPS_andWithCurrentValue, /* AND because we want to set additional bits to 0 */
                    sps_spiTxWriteToChannelChannelControlRegister);
            } else {
                /* invalid entry in sps_channelStatus! */
                FAS_ASSERT(FAS_TRAP);
            }
        }
    }
}

static STD_RETURN_TYPE_e SPS_Transmit(void) {
    STD_RETURN_TYPE_e retVal1 = SPI_TransmitReceiveData(
        &spi_spsInterface, sps_spiTxRegisterBuffer, sps_spiRxRegisterBuffer, SPS_SPI_BUFFERSIZE);

    /* The chip select has to be high for at least 300ns according to data sheet. This code delays
       for the smallest time available in #MCU_delay_us() which is 1us. After this time we can
       be sure that the SPI interface is able to receive again. */
    MCU_delay_us(1u);

    STD_RETURN_TYPE_e retVal2 = SPI_TransmitReceiveData(
        &spi_spsInterface,
        sps_spiTxWriteToChannelChannelControlRegister,
        sps_spiRxReadAnswerDuringChannelControl,
        SPS_SPI_BUFFERSIZE);

    STD_RETURN_TYPE_e retVal = STD_NOT_OK;
    if ((STD_OK == retVal1) && (STD_OK == retVal2)) {
        retVal = STD_OK;
    }
    return retVal;
}

static void SPS_GlobalReadCurrent(const uint8_t outputAllDevices) {
    /* outputAllDevices has to be 1, 2, 3 or 4 */
    FAS_ASSERT((outputAllDevices > 0u) && (outputAllDevices <= SPS_NR_CONTACTOR_PER_IC));
    for (SPS_CHANNEL_INDEX channel = 0u; channel < SPS_NR_OF_AVAILABLE_SPS_CHANNELS; channel++) {
        if ((channel % SPS_NR_CONTACTOR_PER_IC) == (outputAllDevices - 1u)) {
            uint8_t buffer_position = channel / SPS_NR_CONTACTOR_PER_IC;
            uint16_t buffer = sps_spiRxReadAnswerDuringChannelControl[(SPS_SPI_BUFFERSIZE - 1u - buffer_position)] &
                              SPS_BITMASK_DIAGNOSTIC_ONDEMAND_OUTPUT_CURRENT;
            sps_channelStatus[channel].current_mA = (float)buffer * SPS_I_MEASUREMENT_LSB_mA;
        }
    }
}

static void SPS_RequestChannelState(SPS_CHANNEL_INDEX channelIndex, SPS_CHANNEL_FUNCTION_e channelFunction) {
    FAS_ASSERT(channelIndex < SPS_NR_OF_AVAILABLE_SPS_CHANNELS);
    FAS_ASSERT((SPS_CHANNEL_OFF == channelFunction) || (SPS_CHANNEL_ON == channelFunction));

    OS_EnterTaskCritical();
    sps_channelStatus[channelIndex].channelRequested = channelFunction;
    OS_ExitTaskCritical();
}

/*========== Extern Function Implementations ================================*/
extern void SPS_Ctrl(void) {
    if (sps_timer > 0u) {
        sps_timer--;
    }
    STD_RETURN_TYPE_e transmitRetval = STD_NOT_OK;

    if (sps_timer == 0u) {
        switch (sps_state) {
            case SPS_START:
                /* set SPI speed to low speed in order to safely reach the all SPS */
                SPI_SpsInterfaceSwitchToLowSpeed(&spi_spsInterface);
                SPS_InitializeBuffers();
                sps_state = SPS_RESET_LOW;
                break;

            case SPS_RESET_LOW:
                IO_PinReset(&SPS_RESET_GIO_PORT, SPS_RESET_PIN);
                sps_state = SPS_RESET_HIGH;
                sps_timer = SPS_RESET_WAIT_TIME_TICKS;
                break;

            case SPS_RESET_HIGH:
                IO_PinSet(&SPS_RESET_GIO_PORT, SPS_RESET_PIN);
                sps_state = SPS_CONFIGURE_CONTROL_REGISTER;
                sps_timer = SPS_RESET_WAIT_TIME_TICKS;
                break;

            case SPS_CONFIGURE_CONTROL_REGISTER:
                SPS_SetCommandTxBuffer(SPS_ACTION_CONFIGURE_CONTROL_REGISTER);
                SPS_SetContactorsTxBuffer();
                transmitRetval = SPS_Transmit();
                if (STD_OK == transmitRetval) {
                    sps_state = SPS_TRIGGER_CURRENT_MEASUREMENT;
                    /* we have successfully configured the control registers to
                       strong drive mode and can now go to high speed communication */
                    SPI_SpsInterfaceSwitchToHighSpeed(&spi_spsInterface);
                } else {
                    sps_state = SPS_START;
                }
                break;

            case SPS_TRIGGER_CURRENT_MEASUREMENT:
                SPS_SetCommandTxBuffer(SPS_ACTION_TRIGGER_CURRENT_MEASUREMENT);
                SPS_SetContactorsTxBuffer();
                transmitRetval = SPS_Transmit();
                if (STD_OK == transmitRetval) {
                    sps_state = SPS_READ_EN_IRQ_PIN;
                } else {
                    sps_state = SPS_START;
                }
                break;

            case SPS_READ_EN_IRQ_PIN:
                SPS_SetCommandTxBuffer(SPS_ACTION_READ_EN_IRQ_PIN);
                SPS_SetContactorsTxBuffer();
                transmitRetval = SPS_Transmit();
                if (STD_OK == transmitRetval) {
                    sps_state = SPS_READ_MEASURED_CURRENT1;
                } else {
                    sps_state = SPS_START;
                }
                break;

            case SPS_READ_MEASURED_CURRENT1:
                SPS_SetCommandTxBuffer(SPS_ACTION_READ_CURRENT_MEASUREMENT1);
                SPS_SetContactorsTxBuffer();
                transmitRetval = SPS_Transmit();
                if (STD_OK == transmitRetval) {
                    sps_state = SPS_READ_MEASURED_CURRENT2;
                } else {
                    sps_state = SPS_START;
                }
                SPS_GlobalReadCurrent(1u);
                break;

            case SPS_READ_MEASURED_CURRENT2:
                SPS_SetCommandTxBuffer(SPS_ACTION_READ_CURRENT_MEASUREMENT2);
                SPS_SetContactorsTxBuffer();
                transmitRetval = SPS_Transmit();
                if (STD_OK == transmitRetval) {
                    sps_state = SPS_READ_MEASURED_CURRENT3;
                } else {
                    sps_state = SPS_START;
                }
                SPS_GlobalReadCurrent(2u);
                break;

            case SPS_READ_MEASURED_CURRENT3:
                SPS_SetCommandTxBuffer(SPS_ACTION_READ_CURRENT_MEASUREMENT3);
                SPS_SetContactorsTxBuffer();
                transmitRetval = SPS_Transmit();
                if (STD_OK == transmitRetval) {
                    sps_state = SPS_READ_MEASURED_CURRENT4;
                } else {
                    sps_state = SPS_START;
                }
                SPS_GlobalReadCurrent(3u);
                break;

            case SPS_READ_MEASURED_CURRENT4:
                SPS_SetCommandTxBuffer(SPS_ACTION_READ_CURRENT_MEASUREMENT4);
                SPS_SetContactorsTxBuffer();
                transmitRetval = SPS_Transmit();
                if (STD_OK == transmitRetval) {
                    sps_state = SPS_TRIGGER_CURRENT_MEASUREMENT;
                } else {
                    sps_state = SPS_START;
                }
                SPS_GlobalReadCurrent(4u);
                break;
            default:
                FAS_ASSERT(FAS_TRAP);
                break;
        }
    }
}

extern void SPS_Initialize(void) {
    SPS_InitializeIo();
}

extern void SPS_RequestContactorState(SPS_CHANNEL_INDEX channelIndex, SPS_CHANNEL_FUNCTION_e channelFunction) {
    FAS_ASSERT(SPS_AFF_CONTACTOR == SPS_GetChannelAffiliation(channelIndex));
    SPS_RequestChannelState(channelIndex, channelFunction);
}

extern void SPS_RequestGeneralIOState(SPS_CHANNEL_INDEX channelIndex, SPS_CHANNEL_FUNCTION_e channelFunction) {
    FAS_ASSERT(SPS_AFF_GENERAL_IO == SPS_GetChannelAffiliation(channelIndex));
    SPS_RequestChannelState(channelIndex, channelFunction);
}

extern CONT_ELECTRICAL_STATE_TYPE_e SPS_GetChannelCurrentFeedback(const SPS_CHANNEL_INDEX channelIndex) {
    FAS_ASSERT(channelIndex < SPS_NR_OF_AVAILABLE_SPS_CHANNELS);

    CONT_ELECTRICAL_STATE_TYPE_e channelFeedback = CONT_SWITCH_OFF;

    OS_EnterTaskCritical();
    float channelCurrent_mA = sps_channelStatus[channelIndex].current_mA;
    OS_ExitTaskCritical();

    if (channelCurrent_mA > sps_channelStatus[channelIndex].thresholdFeedbackOn_mA) {
        channelFeedback = CONT_SWITCH_ON;
    }

    return channelFeedback;
}

extern CONT_ELECTRICAL_STATE_TYPE_e SPS_GetChannelPexFeedback(const SPS_CHANNEL_INDEX channelIndex, bool normallyOpen) {
    FAS_ASSERT(channelIndex < SPS_NR_OF_AVAILABLE_SPS_CHANNELS);
    FAS_ASSERT((normallyOpen == true) || (normallyOpen == false));

    SPS_CHANNEL_FEEDBACK_MAPPING_s channelMapping = sps_kChannelFeedbackMapping[channelIndex];
    uint8_t pinState                              = PEX_GetPin(channelMapping.pexDevice, channelMapping.pexChannel);

    CONT_ELECTRICAL_STATE_TYPE_e channelFeedback = CONT_SWITCH_OFF;
    if (normallyOpen == true) {
        /* contactor is on if pinstate is high */
        if (pinState != PEX_PIN_LOW) {
            channelFeedback = CONT_SWITCH_ON;
        }
    } else {
        /* contactor is on if pinstate is low */
        if (pinState != PEX_PIN_HIGH) {
            channelFeedback = CONT_SWITCH_ON;
        }
    }

    return channelFeedback;
}

extern SPS_CHANNEL_AFFILIATION_e SPS_GetChannelAffiliation(SPS_CHANNEL_INDEX channelIndex) {
    FAS_ASSERT(channelIndex < SPS_NR_OF_AVAILABLE_SPS_CHANNELS);
    return sps_channelStatus[channelIndex].affiliation;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_SPS_RequestChannelState(SPS_CHANNEL_INDEX channelIndex, SPS_CHANNEL_FUNCTION_e channelFunction) {
    SPS_RequestChannelState(channelIndex, channelFunction);
}
extern SPS_STATE_e TEST_SPS_GetSpsState(void) {
    return sps_state;
}
extern void TEST_SPS_SetSpsState(const SPS_STATE_e newState) {
    sps_state = newState;
}
extern uint8_t TEST_SPS_GetSpsTimer(void) {
    return sps_timer;
}
extern void TEST_SPS_SetSpsTimer(const uint8_t newTimer) {
    sps_timer = newTimer;
}
#endif /* UNITY_UNIT_TEST */
