/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der
 *  angewandten Forschung e.V. All rights reserved.
 *
 * BSD 3-Clause License
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1.  Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * We kindly request you to use one or more of the following phrases to refer
 * to foxBMS in your hardware, software, documentation or advertising
 * materials:
 *
 * &Prime;This product uses parts of foxBMS&reg;&Prime;
 *
 * &Prime;This product includes parts of foxBMS&reg;&Prime;
 *
 * &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/**
 * @file    adc.c
 * @author  foxBMS Team
 * @date    2019-01-07 (date of creation)
 * @updated 2021-03-24 (date of last update)
 * @ingroup DRIVERS
 * @prefix  ADC
 *
 * @brief   Driver for the ADC module.
 *
 */

/*========== Includes =======================================================*/
#include "adc.h"

#include "beta.h"
#include "database.h"
#include "epcos_b57251v5103j060.h"
#include "io.h"
#include "spi.h"

/*========== Macros and Definitions =========================================*/

/** scaling factor for the conversion of a measurement value to a temperature */
#define ADC_TEMPERATURE_FACTOR (10.0f)

/**
 * Type of temperature sensors
 */
typedef enum ADC_TEMPERATURE_SENSOR_TYPE {
    ADC0_TEMPERATURE_SENSOR, /*!< First ADC (ADC0) */
    ADC1_TEMPERATURE_SENSOR, /*!< Second ADC (ADC1) */
} ADC_TEMPERATURE_SENSOR_TYPE_e;

/*========== Static Constant and Variable Definitions =======================*/

/**
 * @brief   describes the current state of the conversion
 * @details This variable is used as a state-variable for switching through the
 *          steps of a conversion.
 */
static uint8_t adc_conversionState = ADC_INIT;

/** NULL command sent to initialize the ADC */
static uint16_t adc_txNull[SINGLE_MESSAGE_LENGTH] = {0x0000u, 0x0000u};

/**
 * Unlock command for the ADC
 * When ADC is locked, regsiters cannot be written
 */
static uint16_t adc_txUnlockCommand[SINGLE_MESSAGE_LENGTH] = {0x0655u, 0x0000u};

/**
 * Lock command the ADC
 * When ADC is locked, regsiters cannot be written
 */
static uint16_t adc_txLockCommand[SINGLE_MESSAGE_LENGTH] = {0x0555u, 0x0000u};

/** The wake-up command starts the conversions by the ADC */
static uint16_t adc_txWakeupCommand[SINGLE_MESSAGE_LENGTH] = {0x0033u, 0x0000u};

/** This command is used to read registers */
static uint16_t adc_txReadRegisterCommand[SINGLE_MESSAGE_LENGTH] = {0x2F00u, 0x0000u};

/** This command is used to write registers */
static uint16_t adc_txWriteRegisterCommand[SINGLE_MESSAGE_LENGTH] = {0x4F0Fu, 0x0000u};

/**
 * This is a generic purpose receive buffer.
 * It is used to read the answer of the ADC to the commands
 * defined above.
 */
static uint16_t adc_rxReadSingleMessage[SINGLE_MESSAGE_LENGTH] = {0x0000u, 0x0000u};

/**
 * This message is used to read conversion results
 * It consists of NULL commands. When receiving NULL commands,
 * the ADC outputs conversion data on the MasterInSlaveOut line.
 */
static uint16_t adc_txConvert[CONVERT_LENGTH] =
    {0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u};

/** This message is used to get the result of conversions */
static uint16_t adc_rxConvert[CONVERT_LENGTH] =
    {0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u};

/** Voltages measured by the 4 channels of ADC0 */
static float adc_adc0Voltage[4] = {0.0f, 0.0f, 0.0f, 0.0f};
/** Voltages measured by the 4 channels of ADC1 */
static float adc_adc1Voltage[4] = {0.0f, 0.0f, 0.0f, 0.0f};
/** LSB of ADC0, used to convert raw measurement to voltage in V */
static float adc_lsb1 = 0.0f;
/** LSB of ADC1, used to convert raw measurement to voltage in V */
static float adc_lsb2 = 0.0f;

/** local copy of the adc temperature table */
static DATA_BLOCK_ADC_TEMPERATURE_s adc_tableTemperature = {.header.uniqueId = DATA_BLOCK_ID_ADC_TEMPERATURE};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/** transmission wrapper for the SPI communication to the ADC */
static STD_RETURN_TYPE_e ADC_Transmit(
    uint32 blocksize,
    uint16 *pTxBuffer,
    uint16 *pRxBuffer,
    SPI_INTERFACE_CONFIG_s *pSpiInterface);

/**
 * @brief   converts a raw voltage from ADC to a temperature value in Celsius.
 *
 * The temperatures are read from NTC elements via voltage dividers.
 * This function implements the look-up table between voltage and temperature,
 * taking into account the NTC characteristics and the voltage divider.
 *
 * @param   v_adc_V     voltage read from ADC in V
 * @param   TsensorType sensor type, dependent on ADC used (ADC0 or ADC1)
 *
 * @return  temperature value in deci &deg;C
 */
static float ADC_ConvertVoltagesToTemperatures(float v_adc_V, ADC_TEMPERATURE_SENSOR_TYPE_e TsensorType);

/*========== Static Function Implementations ================================*/

static STD_RETURN_TYPE_e ADC_Transmit(
    uint32 blocksize,
    uint16 *pTxBuffer,
    uint16 *pRxBuffer,
    SPI_INTERFACE_CONFIG_s *pSpiInterface) {
    FAS_ASSERT(pTxBuffer != NULL_PTR);
    FAS_ASSERT(pRxBuffer != NULL_PTR);
    FAS_ASSERT(pSpiInterface != NULL_PTR);

    return SPI_TransmitReceiveData(pSpiInterface, pTxBuffer, pRxBuffer, blocksize);
}

static float ADC_ConvertVoltagesToTemperatures(float v_adc_V, ADC_TEMPERATURE_SENSOR_TYPE_e TsensorType) {
    float temperature_degC = 0.0f;
    if (TsensorType == ADC0_TEMPERATURE_SENSOR) {
        temperature_degC = ADC_TEMPERATURE_FACTOR * TS_Epc00GetTemperatureFromLut((uint16_t)(v_adc_V * 1000.0f));
    } else if (TsensorType == ADC1_TEMPERATURE_SENSOR) {
        temperature_degC = ADC_TEMPERATURE_FACTOR * BETA_GetTemperatureFromBeta((uint16_t)(v_adc_V * 1000.0f));
    } else {
        temperature_degC = ADC_TEMPERATURE_FACTOR * v_adc_V;
    }

    return (10.0f * temperature_degC); /* Convert to deci &deg;C */
}

/*========== Extern Function Implementations ================================*/

void ADC_Initialize(void) {
    /* set reset pin to output */
    ADC_HET1_GIO->DIR |= (uint32)((uint32)1u << ADC_HET1_RESET_PIN);

    /* first set reset pin to 0 */
    IO_PinReset((uint32_t *)&ADC_HET1_GIO->DOUT, ADC_HET1_RESET_PIN);
    /* wait after pin toggle */
    for (uint8_t i = 0u; i < 20u; i++) {
    }
    /* set reset pin to 1 to go out of reset */
    IO_PinSet((uint32_t *)&ADC_HET1_GIO->DOUT, ADC_HET1_RESET_PIN);

    /* LSB computation, datasheet equation 9 page 38 */
    adc_lsb1 = (2 * ADC_VREF_1 / ADC_GAIN) / (16777216.0f);
    adc_lsb2 = (2 * ADC_VREF_2 / ADC_GAIN) / (16777216.0f);
}

void ADC_Control(void) {
    switch (adc_conversionState) {
        case ADC_INIT:
            /* set reset pin to output */
            ADC_HET1_GIO->DIR |= (uint32)((uint32)1u << ADC_HET1_RESET_PIN);
            /* first set reset pin to 0 */
            IO_PinReset((uint32_t *)&ADC_HET1_GIO->DOUT, ADC_HET1_RESET_PIN);
            adc_conversionState = ADC_ENDINIT;
            break;

        case ADC_ENDINIT:
            /* set reset pin to 1 to go out of reset */
            IO_PinSet((uint32_t *)&ADC_HET1_GIO->DOUT, ADC_HET1_RESET_PIN);
            /* LSB computation, datasheet equation 9 page 38 */
            adc_lsb1            = (2 * ADC_VREF_1 / ADC_GAIN) / (16777216.0f);
            adc_lsb2            = (2 * ADC_VREF_2 / ADC_GAIN) / (16777216.0f);
            adc_conversionState = ADC_READY;
            break;

        /* Start initialization procedure, datasheet figure 106 page 79 */
        case ADC_READY:
            ADC_Transmit(SINGLE_MESSAGE_LENGTH, adc_txNull, adc_rxReadSingleMessage, &spi_adc0Interface);
            /* if device 1 is ready after startup */
            if (adc_rxReadSingleMessage[0] == 0xFF04u) {
                ADC_Transmit(SINGLE_MESSAGE_LENGTH, adc_txNull, adc_rxReadSingleMessage, &spi_adc1Interface);
                /* if device 2 is ready after startup */
                if (adc_rxReadSingleMessage[0] == 0xFF04u) {
                    adc_conversionState = ADC_UNLOCK;
                }
            }
            break;

        case ADC_UNLOCK:
            ADC_Transmit(SINGLE_MESSAGE_LENGTH, adc_txUnlockCommand, adc_rxReadSingleMessage, &spi_adc0Interface);
            ADC_Transmit(SINGLE_MESSAGE_LENGTH, adc_txUnlockCommand, adc_rxReadSingleMessage, &spi_adc1Interface);
            adc_conversionState = ADC_UNLOCKED;
            break;

        case ADC_UNLOCKED:
            ADC_Transmit(SINGLE_MESSAGE_LENGTH, adc_txNull, adc_rxReadSingleMessage, &spi_adc0Interface);
            /* if unlock message received by ADC 1*/
            if (adc_rxReadSingleMessage[0] == 0x0655u) {
                ADC_Transmit(SINGLE_MESSAGE_LENGTH, adc_txNull, adc_rxReadSingleMessage, &spi_adc1Interface);
                /* if unlock message received by ADC 2*/
                if (adc_rxReadSingleMessage[0] == 0x0655u) {
                    adc_conversionState = ADC_WRITE_ADC_ENA;
                }
            }
            break;

        case ADC_WRITE_ADC_ENA:
            ADC_Transmit(
                SINGLE_MESSAGE_LENGTH, adc_txWriteRegisterCommand, adc_rxReadSingleMessage, &spi_adc0Interface);
            ADC_Transmit(
                SINGLE_MESSAGE_LENGTH, adc_txWriteRegisterCommand, adc_rxReadSingleMessage, &spi_adc1Interface);
            adc_conversionState = ADC_READ_ADC_ENA;
            break;

        case ADC_READ_ADC_ENA:
            ADC_Transmit(SINGLE_MESSAGE_LENGTH, adc_txReadRegisterCommand, adc_rxReadSingleMessage, &spi_adc0Interface);
            ADC_Transmit(SINGLE_MESSAGE_LENGTH, adc_txReadRegisterCommand, adc_rxReadSingleMessage, &spi_adc1Interface);
            adc_conversionState = ADC_CHECK_ADC_ENA;
            break;

        case ADC_CHECK_ADC_ENA:
            /* If register not written successfully, retry */
            adc_conversionState = ADC_WRITE_ADC_ENA;
            ADC_Transmit(SINGLE_MESSAGE_LENGTH, adc_txNull, adc_rxReadSingleMessage, &spi_adc0Interface);
            /* if ADC 1 register written successfully */
            if ((adc_rxReadSingleMessage[0] & 0xFFu) == 0x0Fu) {
                ADC_Transmit(SINGLE_MESSAGE_LENGTH, adc_txNull, adc_rxReadSingleMessage, &spi_adc1Interface);
                /* if ADC 1 register written successfully */
                if ((adc_rxReadSingleMessage[0] & 0xFFu) == 0x0Fu) {
                    adc_conversionState = ADC_WAKEUP;
                }
            }
            break;

        case ADC_WAKEUP:
            ADC_Transmit(SINGLE_MESSAGE_LENGTH, adc_txWakeupCommand, adc_rxReadSingleMessage, &spi_adc0Interface);
            ADC_Transmit(SINGLE_MESSAGE_LENGTH, adc_txWakeupCommand, adc_rxReadSingleMessage, &spi_adc1Interface);
            adc_conversionState = ADC_LOCK;
            break;

        case ADC_LOCK:
            ADC_Transmit(SINGLE_MESSAGE_LENGTH, adc_txLockCommand, adc_rxReadSingleMessage, &spi_adc0Interface);
            ADC_Transmit(SINGLE_MESSAGE_LENGTH, adc_txLockCommand, adc_rxReadSingleMessage, &spi_adc1Interface);
            adc_conversionState = ADC_CONVERT_1;
            break;
        /* end initialization procedure, datasheet figure 106 page 79 */

        /* To read channel data (i.e., measured voltages), send null message */
        case ADC_CONVERT_1:
            /* The first 3 voltages are the NTCs located on the master board */
            ADC_Transmit(CONVERT_LENGTH, adc_txConvert, adc_rxConvert, &spi_adc0Interface);
            for (uint8_t i = 0u; i < ADC_NUMBER_OF_CHANNELS; i++) {
                /* datasheet SBAS590D -MARCH 2016-REVISED JANUARY 2018 */
                /* ADC in 32 bit mode (M1 tied to IOVDD via 1kOhm), 24 bits output by ADC, datasheet page 38 */
                adc_adc0Voltage[i] = (adc_rxConvert[2u + (2u * i)] << 8u) |
                                     ((adc_rxConvert[3u + (2u * i)] >> 8u) & 0xFFu);
                adc_adc0Voltage[i] *= adc_lsb1;
            }
            adc_conversionState = ADC_CONVERT_2;
            DATA_READ_DATA(&adc_tableTemperature);
            for (uint8_t i = 0u; i < BS_NR_OF_TEMP_SENSORS_ON_ADC0; i++) {
                adc_tableTemperature.temperatureAdc0_ddegC[i] =
                    ADC_ConvertVoltagesToTemperatures(adc_adc0Voltage[i], ADC0_TEMPERATURE_SENSOR);
            }
            DATA_WRITE_DATA(&adc_tableTemperature);
            break;

        case ADC_CONVERT_2:
            /* The first 3 voltages are the external NTCs connected to the master board */
            ADC_Transmit(CONVERT_LENGTH, adc_txConvert, adc_rxConvert, &spi_adc1Interface);
            for (uint8_t i = 0u; i < ADC_NUMBER_OF_CHANNELS; i++) {
                /* ADC in 32 bit mode (M1 tied to IOVDD via 1kOhm), 24 bits output by ADC, datasheet page 38 */
                adc_adc1Voltage[i] = (adc_rxConvert[2u + (2u * i)] << 8u) |
                                     ((adc_rxConvert[3u + (2u * i)] >> 8u) & 0xFFu);
                adc_adc1Voltage[i] *= adc_lsb2;
            }
            adc_conversionState = ADC_CONVERT_1;
            DATA_READ_DATA(&adc_tableTemperature);
            for (uint8_t i = 0u; i < BS_NR_OF_TEMP_SENSORS_ON_ADC1; i++) {
                adc_tableTemperature.temperatureAdc1_ddegC[i] =
                    ADC_ConvertVoltagesToTemperatures(adc_adc1Voltage[i], ADC1_TEMPERATURE_SENSOR);
            }
            DATA_WRITE_DATA(&adc_tableTemperature);
            break;

        default:
            /* invalid state */
            FAS_ASSERT(FAS_TRAP);
            break;
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
