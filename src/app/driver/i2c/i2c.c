/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    i2c.c
 * @author  foxBMS Team
 * @date    2021-07-22 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  I2C
 *
 * @brief   Driver for the I2C module
 *
 */

/*========== Includes =======================================================*/
#include "i2c.h"

#include "HL_system.h"

#include "database.h"
#include "diag.h"
#include "dma.h"
#include "fstd_types.h"
#include "fsystem.h"
#include "mcu.h"
#include "os.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
/* Pointer to the last byte of the table where received bytes are written, pI2cInterface */
uint8_t i2c_rxLastByteInterface1 = 0u;
/* Pointer to the last byte of the table where received bytes are written, i2cREG2 */
uint8_t i2c_rxLastByteInterface2 = 0u;

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Return transmit time of a word in microseconds.
 * @details The function uses the clock settings of the interface
 *          to determine the time needed to transmit one word.
 *          Word means one byte + the ACK bit.
 * @param   pI2cInterface I2C interface to use
 * @return  time in microseconds needed to transmit a byte on the I2C interface
 */
static uint32_t I2C_GetWordTransmitTime(i2cBASE_t *pI2cInterface);
/**
 * @brief   Waits for the I2C Tx buffer to be empty.
 * @details When the buffer is empty, the next byte can be sent.
 *          If the buffer is empty before timeout_us microseconds are elapsed,
 *          the function returns true, false otherwise.
 *          The function also returns false if a NACK condition
 *          is detected.
 * @param   pI2cInterface I2C interface to use
 * @param   timeout_us    time in microseconds to wait until the buffer is empty
 * @return  true if buffer is empty within timeout, false otherwise
 */
static bool I2C_WaitTransmit(i2cBASE_t *pI2cInterface, uint32_t timeout_us);
/**
 * @brief   Waits for a stop condition to be detected.
 * @details When a stop condition is issued, this function waits until
 *          the stop condition is detected on the bus. This means that
 *          that transmission is finished. If stop is detected before
 *          timeout_us microseconds are elapsed, the function returns true,
 *          false otherwise.
 * @param   pI2cInterface I2C interface to use
 * @param   timeout_us    time in microseconds to wait until stop is detected
 * @return  true if stop is detected within timeout, false otherwise
 */
static bool I2C_WaitStop(i2cBASE_t *pI2cInterface, uint32_t timeout_us);

/**
 * @brief   Wait for the I2C transmit communication to complete, using notifications
 *
 * @return  I2C_TX_NOTIFIED_VALUE if notification received,
 *          I2C_NO_NOTIFIED_VALUE if timeout reached
 */
static uint32_t I2C_WaitForTxCompletedNotification(void);
/**
 * @brief   Wait for the I2C receive communication to complete, using notifications
 *
 * @return  I2C_RX_NOTIFIED_VALUE if notification received,
 *          I2C_NO_NOTIFIED_VALUE if timeout reached
 */
static uint32_t I2C_WaitForRxCompletedNotification(void);
/**
 * @brief   Clear pending notifications
 *
 */
static void I2C_ClearNotifications(void);

/*========== Static Function Implementations ================================*/
static uint32_t I2C_GetWordTransmitTime(i2cBASE_t *pI2cInterface) {
    FAS_ASSERT(pI2cInterface != NULL_PTR);
    uint32_t i2cClock_khz        = 0;
    uint32_t prescaler           = 0;
    uint32_t wordTransmitTime_us = 0u;
    uint8_t dFactor              = 0u;

    /* Get prescaler */
    prescaler = pI2cInterface->PSC & I2C_PRESCALER_MASK;
    if (prescaler == 0u) {
        dFactor = I2C_DFACTOR_VALUE_PRESCALER_0;
    } else if (prescaler == 1u) {
        dFactor = I2C_DFACTOR_VALUE_PRESCALER_1;
    } else {
        dFactor = I2C_DFACTOR_VALUE_PRESCALER_OTHER;
    }
    /* This is the equation used in the HAL; seems to differ from Technical Reference Manual
        (p.1769 eq.65, SPNU563A - March 2018) */
    i2cClock_khz = (uint32_t)(AVCLK1_FREQ * I2C_FACTOR_MHZ_TO_HZ) /
                   (2u * (prescaler + 1u) * (pI2cInterface->CKH + dFactor));
    wordTransmitTime_us = (I2C_FACTOR_WORD_TO_BITS * I2C_FACTOR_S_TO_US) / i2cClock_khz;
    return wordTransmitTime_us;
}

static bool I2C_WaitTransmit(i2cBASE_t *pI2cInterface, uint32_t timeout_us) {
    FAS_ASSERT(pI2cInterface != NULL_PTR);
    bool success          = true;
    bool timeElapsed      = false;
    uint32_t startCounter = MCU_GetFreeRunningCount();

    while (((pI2cInterface->STR & (uint32_t)I2C_NACK_INT) == 0u) &&
           ((pI2cInterface->STR & (uint32_t)I2C_TX_INT) == 0u) && (timeElapsed == false)) {
        timeElapsed = MCU_IsTimeElapsed(startCounter, timeout_us);
    }

    if (timeElapsed == true) {
        success = false;
    }

    return success;
}

static bool I2C_WaitStop(i2cBASE_t *pI2cInterface, uint32_t timeout_us) {
    FAS_ASSERT(pI2cInterface != NULL_PTR);
    bool success          = true;
    bool timeElapsed      = false;
    uint32_t startCounter = MCU_GetFreeRunningCount();

    while ((i2cIsStopDetected(pI2cInterface) == 0u) && (timeElapsed == false)) {
        timeElapsed = MCU_IsTimeElapsed(startCounter, timeout_us);
    }

    if (timeElapsed == true) {
        success = false;
    }

    return success;
}

static uint32_t I2C_WaitForTxCompletedNotification(void) {
    uint32_t notifiedValueTx = I2C_NO_NOTIFIED_VALUE;
    /**
     * Suspend task and wait for I2C DMA TX finished notification,
     * clear notification value on entry and exit
     */
    OS_WaitForNotificationIndexed(I2C_NOTIFICATION_TX_INDEX, &notifiedValueTx, I2C_NOTIFICATION_TIMEOUT_ms);
    return notifiedValueTx;
}

static uint32_t I2C_WaitForRxCompletedNotification(void) {
    uint32_t notifiedValueRx = I2C_NO_NOTIFIED_VALUE;
    /**
     * Suspend task and wait for I2C DMA RX finished notification,
     * clear notification value on entry and exit
     */
    OS_WaitForNotificationIndexed(I2C_NOTIFICATION_RX_INDEX, &notifiedValueRx, I2C_NOTIFICATION_TIMEOUT_ms);
    return notifiedValueRx;
}

static void I2C_ClearNotifications(void) {
    OS_ClearNotificationIndexed(I2C_NOTIFICATION_TX_INDEX);
    OS_ClearNotificationIndexed(I2C_NOTIFICATION_RX_INDEX);
}

/*========== Extern Function Implementations ================================*/
extern void I2C_Initialize(void) {
    i2cInit();
}

extern STD_RETURN_TYPE_e I2C_Read(
    i2cBASE_t *pI2cInterface,
    uint32_t slaveAddress,
    uint32_t nrBytes,
    uint8_t *readData) {
    FAS_ASSERT(pI2cInterface != NULL_PTR);
    FAS_ASSERT(readData != NULL_PTR);
    FAS_ASSERT(nrBytes > 0u);
    FAS_ASSERT(slaveAddress < 128u);
    STD_RETURN_TYPE_e retVal = STD_OK;

    if ((pI2cInterface->STR & (uint32_t)I2C_BUSBUSY) == 0u) {
        /* Clear bits */
        pI2cInterface->MDR &= ~((uint32_t)I2C_STOP_COND);
        pI2cInterface->MDR &= ~((uint32_t)I2C_START_COND);
        pI2cInterface->MDR &= ~((uint32_t)I2C_REPEATMODE);
        pI2cInterface->STR |= (uint32_t)I2C_TX_INT;
        pI2cInterface->STR |= (uint32_t)I2C_RX_INT;

        pI2cInterface->MDR |= (uint32_t)I2C_REPEATMODE;

        i2cSetMode(pI2cInterface, (uint32_t)I2C_MASTER);        /* Set as master */
        i2cSetDirection(pI2cInterface, (uint32_t)I2C_RECEIVER); /* Set as transmitter */
        i2cSetSlaveAdd(pI2cInterface, slaveAddress);            /* Set slave address */
        i2cSetStart(pI2cInterface);                             /* Start receive */
        if (nrBytes == 1u) {
            i2cSetStop(pI2cInterface); /* generate a STOP condition */
        }
        /* Receive nrBytes bytes in polling mode */
        for (uint16_t i = 0u; i < nrBytes; i++) {
            bool success = I2C_WaitReceive(pI2cInterface, I2C_TIMEOUT_us);
            if (((pI2cInterface->STR & (uint32_t)I2C_NACK_INT) == (uint32_t)I2C_NACK_INT) || (success == false)) {
                pI2cInterface->MDR |= (uint32_t)I2C_REPEATMODE;
                i2cSetStop(pI2cInterface);
                retVal = STD_NOT_OK;
                break;
            }
            readData[i] = (uint8)(pI2cInterface->DRR & I2C_DDR_REGISTER_DATA_MASK);
            if (i == (nrBytes - 2u)) {
                i2cSetStop(pI2cInterface); /* generate a STOP condition */
            }
        }

        bool success = I2C_WaitStop(pI2cInterface, I2C_TIMEOUT_us);
        if (success == false) {
            /* Set Stop condition */
            pI2cInterface->MDR |= (uint32_t)I2C_REPEATMODE;
            i2cSetStop(pI2cInterface);
            retVal = STD_NOT_OK;
        }
    } else {
        retVal = STD_NOT_OK;
    }

    return retVal;
}

extern STD_RETURN_TYPE_e I2C_Write(
    i2cBASE_t *pI2cInterface,
    uint32_t slaveAddress,
    uint32_t nrBytes,
    uint8_t *writeData) {
    FAS_ASSERT(pI2cInterface != NULL_PTR);
    FAS_ASSERT(writeData != NULL_PTR);
    FAS_ASSERT(nrBytes > 0u);
    FAS_ASSERT(slaveAddress < 128u);
    STD_RETURN_TYPE_e retVal = STD_OK;

    if ((pI2cInterface->STR & (uint32_t)I2C_BUSBUSY) == 0u) {
        /* Clear bits */
        pI2cInterface->MDR &= ~((uint32_t)I2C_STOP_COND);
        pI2cInterface->MDR &= ~((uint32_t)I2C_START_COND);
        pI2cInterface->MDR &= ~((uint32_t)I2C_REPEATMODE);
        pI2cInterface->STR |= (uint32_t)I2C_TX_INT;
        pI2cInterface->STR |= (uint32_t)I2C_RX_INT;

        i2cSetMode(pI2cInterface, (uint32_t)I2C_MASTER);           /* Set as master */
        i2cSetDirection(pI2cInterface, (uint32_t)I2C_TRANSMITTER); /* Set as transmitter */
        i2cSetSlaveAdd(pI2cInterface, slaveAddress);               /* Set slave address */
        i2cSetStop(pI2cInterface);                                 /* Stop condition after sending nrBytes bytes */
        i2cSetCount(pI2cInterface, nrBytes);                       /* Send nrBytes bytes before STOP condition */
        i2cSetStart(pI2cInterface);                                /* Start transmit */

        /* Send nrBytes bytes in polling mode */
        for (uint16_t i = 0u; i < nrBytes; i++) {
            pI2cInterface->DXR = (uint32_t)writeData[i];
            bool success       = I2C_WaitTransmit(pI2cInterface, I2C_TIMEOUT_us);
            if (((pI2cInterface->STR & (uint32_t)I2C_NACK_INT) == (uint32_t)I2C_NACK_INT) || (success == false)) {
                pI2cInterface->MDR |= (uint32_t)I2C_REPEATMODE;
                i2cSetStop(pI2cInterface);
                retVal = STD_NOT_OK;
                break;
            }
        }

        bool success = I2C_WaitStop(pI2cInterface, I2C_TIMEOUT_us);
        if (success == false) {
            /* Set Stop condition */
            pI2cInterface->MDR |= (uint32_t)I2C_REPEATMODE;
            i2cSetStop(pI2cInterface);
            retVal = STD_NOT_OK;
        }
    } else {
        retVal = STD_NOT_OK;
    }

    return retVal;
}

extern STD_RETURN_TYPE_e I2C_WriteRead(
    i2cBASE_t *pI2cInterface,
    uint32_t slaveAddress,
    uint32_t nrBytesWrite,
    uint8_t *writeData,
    uint32_t nrBytesRead,
    uint8_t *readData) {
    FAS_ASSERT(pI2cInterface != NULL_PTR);
    FAS_ASSERT(writeData != NULL_PTR);
    FAS_ASSERT(nrBytesWrite > 0u);
    FAS_ASSERT(readData != NULL_PTR);
    FAS_ASSERT(nrBytesRead > 0u);
    FAS_ASSERT(slaveAddress < 128u);
    STD_RETURN_TYPE_e retVal = STD_OK;

    if ((pI2cInterface->STR & (uint32_t)I2C_BUSBUSY) == 0u) {
        /* Clear bits */
        pI2cInterface->MDR &= ~((uint32_t)I2C_STOP_COND);
        pI2cInterface->MDR &= ~((uint32_t)I2C_START_COND);
        pI2cInterface->MDR &= ~((uint32_t)I2C_REPEATMODE);
        pI2cInterface->STR |= (uint32_t)I2C_TX_INT;
        pI2cInterface->STR |= (uint32_t)I2C_RX_INT;

        i2cSetMode(pI2cInterface, (uint32_t)I2C_MASTER);           /* Set as master */
        i2cSetDirection(pI2cInterface, (uint32_t)I2C_TRANSMITTER); /* Set as transmitter */
        i2cSetSlaveAdd(pI2cInterface, slaveAddress);               /* Set slave address */
        i2cSetStart(pI2cInterface);                                /* Start transmit */

        bool success = true;

        /* Send nrBytesWrite bytes in polling mode */
        for (uint16_t i = 0u; i < nrBytesWrite; i++) {
            pI2cInterface->DXR = (uint32_t)writeData[i];
            success            = I2C_WaitTransmit(pI2cInterface, I2C_TIMEOUT_us);
            if (((pI2cInterface->STR & (uint32_t)I2C_NACK_INT) == (uint32_t)I2C_NACK_INT) || (success == false)) {
                pI2cInterface->MDR |= (uint32_t)I2C_REPEATMODE;
                i2cSetStop(pI2cInterface);
                retVal = STD_NOT_OK;
                break;
            }
        }

        if (!(((pI2cInterface->STR & (uint32_t)I2C_NACK_INT) == (uint32_t)I2C_NACK_INT) || (success == false))) {
            pI2cInterface->MDR |= (uint32_t)I2C_REPEATMODE;
            i2cSetMode(pI2cInterface, (uint32_t)I2C_MASTER);        /* Set as master */
            i2cSetDirection(pI2cInterface, (uint32_t)I2C_RECEIVER); /* Set as receiver */
            i2cSetStart(pI2cInterface);                             /* Start receive */
            if (nrBytesRead == 1u) {
                i2cSetStop(pI2cInterface); /* generate a STOP condition */
            }
            /* Receive nrBytes bytes in polling mode */
            for (uint16_t i = 0u; i < nrBytesRead; i++) {
                success = I2C_WaitReceive(pI2cInterface, I2C_TIMEOUT_us);
                if (((pI2cInterface->STR & (uint32_t)I2C_NACK_INT) == (uint32_t)I2C_NACK_INT) || (success == false)) {
                    pI2cInterface->MDR |= (uint32_t)I2C_REPEATMODE;
                    i2cSetStop(pI2cInterface);
                    retVal = STD_NOT_OK;
                    break;
                }
                readData[i] = (uint8)(pI2cInterface->DRR & I2C_DDR_REGISTER_DATA_MASK);
                if (i == (nrBytesRead - 2u)) {
                    i2cSetStop(pI2cInterface); /* generate a STOP condition */
                }
            }
        }

        success = I2C_WaitStop(pI2cInterface, I2C_TIMEOUT_us);
        if (success == false) {
            /* Set Stop condition */
            pI2cInterface->MDR |= (uint32_t)I2C_REPEATMODE;
            i2cSetStop(pI2cInterface);
            retVal = STD_NOT_OK;
        }
    } else {
        retVal = STD_NOT_OK;
    }

    return retVal;
}

extern STD_RETURN_TYPE_e I2C_ReadDma(
    i2cBASE_t *pI2cInterface,
    uint32_t slaveAddress,
    uint32_t nrBytes,
    uint8_t *readData) {
    FAS_ASSERT(pI2cInterface != NULL_PTR);
    FAS_ASSERT(readData != NULL_PTR);
    FAS_ASSERT(nrBytes > 1u);
    FAS_ASSERT(slaveAddress < 128u);
    STD_RETURN_TYPE_e retVal = STD_OK;

    I2C_ClearNotifications();

    if ((pI2cInterface->STR & (uint32_t)I2C_BUSBUSY) == 0u) {
        /* Clear bits */
        pI2cInterface->MDR &= ~((uint32_t)I2C_STOP_COND);
        pI2cInterface->MDR &= ~((uint32_t)I2C_START_COND);
        pI2cInterface->MDR &= ~((uint32_t)I2C_REPEATMODE);
        pI2cInterface->STR |= (uint32_t)I2C_TX_INT;
        pI2cInterface->STR |= (uint32_t)I2C_RX_INT;

        /* DMA config */
        dmaChannel_t channelRx = DMA_CH0;
        if (pI2cInterface == i2cREG1) {
            channelRx = DMA_CHANNEL_I2C1_RX;
        } else if (pI2cInterface == i2cREG2) {
            channelRx = DMA_CHANNEL_I2C2_RX;
        } else {
            /* invalid I2C interface */
            FAS_ASSERT(FAS_TRAP);
        }

        OS_EnterTaskCritical();

        /* Go to privileged mode to write DMA config registers */
        const int32_t raisePrivilegeResult = FSYS_RaisePrivilege();
        FAS_ASSERT(raisePrivilegeResult == 0);

        /* Set Tx buffer address */
        /* AXIVION Disable Style MisraC2012-1.1: Cast necessary for DMA configuration */
        dmaRAMREG->PCP[(dmaChannel_t)channelRx].IDADDR = (uint32_t)readData;
        /* AXIVION Enable Style MisraC2012-1.1: */
        /* Set number of Rx bytes to receive, (nrBytes-1) over DMA */
        dmaRAMREG->PCP[(dmaChannel_t)channelRx].ITCOUNT = ((nrBytes - 1u) << DMA_INITIAL_FRAME_COUNTER_POSITION) | 1u;

        dmaSetChEnable((dmaChannel_t)channelRx, (dmaTriggerType_t)DMA_HW);

        FSYS_SwitchToUserMode(); /* DMA config registers written, leave privileged mode */
        OS_ExitTaskCritical();
        /* End DMA config */

        pI2cInterface->DMACR |= (uint32_t)I2C_RXDMAEN; /* Activate I2C DMA RX */

        pI2cInterface->MDR &= ~((uint32_t)I2C_STOP_COND);
        pI2cInterface->MDR &= ~((uint32_t)I2C_START_COND);
        pI2cInterface->MDR |= (uint32_t)I2C_REPEATMODE;

        i2cSetMode(pI2cInterface, (uint32_t)I2C_MASTER);        /* Set as master */
        i2cSetDirection(pI2cInterface, (uint32_t)I2C_RECEIVER); /* Set as transmitter */
        i2cSetSlaveAdd(pI2cInterface, slaveAddress);            /* Set slave address */
        i2cSetStart(pI2cInterface);                             /* Start receive */

        uint32_t notificationRx = I2C_WaitForRxCompletedNotification();
        if (notificationRx != I2C_RX_NOTIFIED_VALUE) {
            /* Rx not happened, deactivate DMA */
            pI2cInterface->DMACR &= ~((uint32_t)I2C_RXDMAEN);
            /* Set Stop condition */
            pI2cInterface->MDR |= (uint32_t)I2C_REPEATMODE;
            i2cSetStop(pI2cInterface);
            retVal = STD_NOT_OK;
        } else {
            /* Rx happened */
            if (pI2cInterface == i2cREG1) {
                readData[nrBytes - 1u] = i2c_rxLastByteInterface1;
            } else if (pI2cInterface == i2cREG2) {
                readData[nrBytes - 1u] = i2c_rxLastByteInterface2;
            } else {
                /* invalid I2C interface */
                FAS_ASSERT(FAS_TRAP);
            }
            bool success = I2C_WaitStop(pI2cInterface, I2C_TIMEOUT_us);
            if (success == false) {
                /* Set Stop condition */
                pI2cInterface->MDR |= (uint32_t)I2C_REPEATMODE;
                i2cSetStop(pI2cInterface);
                retVal = STD_NOT_OK;
            }
        }
    } else {
        retVal = STD_NOT_OK;
    }

    return retVal;
}

extern STD_RETURN_TYPE_e I2C_WriteDma(
    i2cBASE_t *pI2cInterface,
    uint32_t slaveAddress,
    uint32_t nrBytes,
    uint8_t *writeData) {
    FAS_ASSERT(pI2cInterface != NULL_PTR);
    FAS_ASSERT(writeData != NULL_PTR);
    FAS_ASSERT(nrBytes > 0u);
    FAS_ASSERT(slaveAddress < 128u);
    STD_RETURN_TYPE_e retVal = STD_OK;

    I2C_ClearNotifications();

    if ((pI2cInterface->STR & (uint32_t)I2C_BUSBUSY) == 0u) {
        pI2cInterface->MDR &= ~((uint32_t)I2C_STOP_COND);
        pI2cInterface->MDR &= ~((uint32_t)I2C_START_COND);
        pI2cInterface->MDR &= ~((uint32_t)I2C_REPEATMODE);
        pI2cInterface->STR |= (uint32_t)I2C_TX_INT;
        pI2cInterface->STR |= (uint32_t)I2C_RX_INT;

        /* DMA config */
        dmaChannel_t channelTx = DMA_CH0;
        if (pI2cInterface == i2cREG1) {
            channelTx = DMA_CHANNEL_I2C1_TX;
        } else if (pI2cInterface == i2cREG2) {
            channelTx = DMA_CHANNEL_I2C2_TX;
        } else {
            /* invalid I2C interface */
            FAS_ASSERT(FAS_TRAP);
        }

        OS_EnterTaskCritical();

        /* Go to privileged mode to write DMA config registers */
        const int32_t raisePrivilegeResult = FSYS_RaisePrivilege();
        FAS_ASSERT(raisePrivilegeResult == 0);

        /* Set Tx buffer address */
        /* AXIVION Disable Style MisraC2012-1.1: Cast necessary for DMA configuration */
        dmaRAMREG->PCP[(dmaChannel_t)channelTx].ISADDR = (uint32_t)writeData;
        /* AXIVION Enable Style MisraC2012-1.1: */
        /* Set number of Tx bytes to transmit */
        dmaRAMREG->PCP[(dmaChannel_t)channelTx].ITCOUNT = (nrBytes << DMA_INITIAL_FRAME_COUNTER_POSITION) | 1u;

        dmaSetChEnable((dmaChannel_t)channelTx, (dmaTriggerType_t)DMA_HW);

        FSYS_SwitchToUserMode(); /* DMA config registers written, leave privileged mode */
        OS_ExitTaskCritical();
        /* end DMA config */

        /* Clear bits */
        pI2cInterface->MDR &= ~((uint32_t)I2C_STOP_COND);
        pI2cInterface->MDR &= ~((uint32_t)I2C_START_COND);
        pI2cInterface->MDR &= ~((uint32_t)I2C_REPEATMODE);
        pI2cInterface->STR |= (uint32_t)I2C_TX_INT;
        pI2cInterface->STR |= (uint32_t)I2C_RX_INT;

        i2cSetMode(pI2cInterface, (uint32_t)I2C_MASTER);           /* Set as master */
        i2cSetDirection(pI2cInterface, (uint32_t)I2C_TRANSMITTER); /* Set as transmitter */
        i2cSetSlaveAdd(pI2cInterface, slaveAddress);               /* Set slave address */
        i2cSetStop(pI2cInterface);                                 /* Stop condition after sending nrBytes bytes */
        i2cSetCount(pI2cInterface, nrBytes);                       /* Send nrBytes bytes before STOP condition */
        pI2cInterface->DMACR |= (uint32_t)I2C_TXDMAEN;             /* Activate I2C DMA TX */
        i2cSetStart(pI2cInterface);                                /* Start transmit */

        uint32_t notificationTx = I2C_WaitForTxCompletedNotification();
        if (notificationTx != I2C_TX_NOTIFIED_VALUE) {
            /* Tx not happened, deactivate DMA */
            pI2cInterface->DMACR &= ~((uint32_t)I2C_TXDMAEN);
            /* Set Stop condition */
            pI2cInterface->MDR |= (uint32_t)I2C_REPEATMODE;
            i2cSetStop(pI2cInterface);
            retVal = STD_NOT_OK;
        } else {
            bool success = I2C_WaitStop(pI2cInterface, I2C_TIMEOUT_us);
            if (success == false) {
                /* Set Stop condition */
                pI2cInterface->MDR |= (uint32_t)I2C_REPEATMODE;
                i2cSetStop(pI2cInterface);
                retVal = STD_NOT_OK;
            }
        }
    } else {
        retVal = STD_NOT_OK;
    }

    return retVal;
}

extern STD_RETURN_TYPE_e I2C_WriteReadDma(
    i2cBASE_t *pI2cInterface,
    uint32_t slaveAddress,
    uint32_t nrBytesWrite,
    uint8_t *writeData,
    uint32_t nrBytesRead,
    uint8_t *readData) {
    FAS_ASSERT(pI2cInterface != NULL_PTR);
    FAS_ASSERT(writeData != NULL_PTR);
    FAS_ASSERT(nrBytesWrite > 0u);
    FAS_ASSERT(readData != NULL_PTR);
    FAS_ASSERT(nrBytesRead > 1u);
    FAS_ASSERT(slaveAddress < 128u);
    STD_RETURN_TYPE_e retVal = STD_OK;
    dmaChannel_t channelRx   = DMA_CH0;
    dmaChannel_t channelTx   = DMA_CH0;

    I2C_ClearNotifications();

    if ((pI2cInterface->STR & (uint32_t)I2C_BUSBUSY) == 0u) {
        /* Firt write bytes */

        /* Clear bits */
        pI2cInterface->MDR &= ~((uint32_t)I2C_STOP_COND);
        pI2cInterface->MDR &= ~((uint32_t)I2C_START_COND);
        pI2cInterface->MDR &= ~((uint32_t)I2C_REPEATMODE);
        pI2cInterface->STR |= (uint32_t)I2C_TX_INT;
        pI2cInterface->STR |= (uint32_t)I2C_RX_INT;

        /* DMA config */
        if (pI2cInterface == i2cREG1) {
            channelTx = DMA_CHANNEL_I2C1_TX;
        } else if (pI2cInterface == i2cREG2) {
            channelTx = DMA_CHANNEL_I2C2_TX;
        } else {
            /* invalid I2C interface */
            FAS_ASSERT(FAS_TRAP);
        }

        OS_EnterTaskCritical();

        /* Go to privileged mode to write DMA config registers */
        const int32_t raisePrivilegeResultWrite = FSYS_RaisePrivilege();
        FAS_ASSERT(raisePrivilegeResultWrite == 0);

        /* Set Tx buffer address */
        /* AXIVION Disable Style MisraC2012-1.1: Cast necessary for DMA configuration */
        dmaRAMREG->PCP[(dmaChannel_t)channelTx].ISADDR = (uint32_t)writeData;
        /* AXIVION Enable Style MisraC2012-1.1: */
        /* Set number of Tx bytes to transmit */
        dmaRAMREG->PCP[(dmaChannel_t)channelTx].ITCOUNT = (nrBytesWrite << DMA_INITIAL_FRAME_COUNTER_POSITION) | 1u;

        dmaSetChEnable((dmaChannel_t)channelTx, (dmaTriggerType_t)DMA_HW);

        FSYS_SwitchToUserMode(); /* DMA config registers written, leave privileged mode */
        OS_ExitTaskCritical();
        /* end DMA config */

        /* Clear bits */
        pI2cInterface->MDR &= ~((uint32_t)I2C_STOP_COND);
        pI2cInterface->MDR &= ~((uint32_t)I2C_START_COND);
        pI2cInterface->MDR |= (uint32_t)I2C_REPEATMODE;
        pI2cInterface->STR |= (uint32_t)I2C_TX_INT;
        pI2cInterface->STR |= (uint32_t)I2C_RX_INT;

        i2cSetMode(pI2cInterface, (uint32_t)I2C_MASTER);           /* Set as master */
        i2cSetDirection(pI2cInterface, (uint32_t)I2C_TRANSMITTER); /* Set as transmitter */
        i2cSetSlaveAdd(pI2cInterface, slaveAddress);               /* Set slave address */
        pI2cInterface->DMACR |= (uint32_t)I2C_TXDMAEN;             /* Activate I2C DMA TX */
        i2cSetStart(pI2cInterface);                                /* Start transmit */

        uint32_t notificationTx = I2C_WaitForTxCompletedNotification();
        if (notificationTx != I2C_TX_NOTIFIED_VALUE) {
            /* Tx not happened, deactivate DMA */
            pI2cInterface->DMACR &= ~((uint32_t)I2C_TXDMAEN);
            /* Set Stop condition */
            pI2cInterface->MDR |= (uint32_t)I2C_REPEATMODE;
            i2cSetStop(pI2cInterface);
            retVal = STD_NOT_OK;
        } else {
            /* Write successful, now read */

            /* DMA config */
            if (pI2cInterface == i2cREG1) {
                channelRx = DMA_CHANNEL_I2C1_RX;
            } else if (pI2cInterface == i2cREG2) {
                channelRx = DMA_CHANNEL_I2C2_RX;
            } else {
                /* invalid I2C interface */
                FAS_ASSERT(FAS_TRAP);
            }

            OS_EnterTaskCritical();

            /* Go to privileged mode to write DMA config registers */
            const int32_t raisePrivilegeResultRead = FSYS_RaisePrivilege();
            FAS_ASSERT(raisePrivilegeResultRead == 0);

            /* Set Rx buffer address */
            /* AXIVION Disable Style MisraC2012-1.1: Cast necessary for DMA configuration */
            dmaRAMREG->PCP[(dmaChannel_t)channelRx].IDADDR = (uint32_t)readData;
            /* AXIVION Enable Style MisraC2012-1.1: */
            /* Set number of Rx bytes to receive */
            dmaRAMREG->PCP[(dmaChannel_t)channelRx].ITCOUNT =
                ((nrBytesRead - 1u) << DMA_INITIAL_FRAME_COUNTER_POSITION) | 1u;

            dmaSetChEnable((dmaChannel_t)channelRx, (dmaTriggerType_t)DMA_HW);

            FSYS_SwitchToUserMode(); /* DMA config registers written, leave privileged mode */
            OS_ExitTaskCritical();
            /* end DMA config */

            /* As we cannot wait on stop condition to be sure that all bytes have been sent on the bus,
               wait until transmission is finished */
            uint32_t wordTransmitTime_us = I2C_GetWordTransmitTime(pI2cInterface) + I2C_TX_TIME_MARGIN_us;

            MCU_Delay_us(wordTransmitTime_us);

            /* Clear bits */
            pI2cInterface->MDR &= ~((uint32_t)I2C_STOP_COND);
            pI2cInterface->MDR &= ~((uint32_t)I2C_START_COND);
            pI2cInterface->MDR &= ~((uint32_t)I2C_REPEATMODE);
            pI2cInterface->STR |= (uint32_t)I2C_TX_INT;
            pI2cInterface->STR |= (uint32_t)I2C_RX_INT;

            pI2cInterface->DMACR |= (uint32_t)I2C_RXDMAEN; /* Activate I2C DMA RX */

            pI2cInterface->MDR |= (uint32_t)I2C_REPEATMODE;
            i2cSetMode(pI2cInterface, (uint32_t)I2C_MASTER);        /* Set as master */
            i2cSetDirection(pI2cInterface, (uint32_t)I2C_RECEIVER); /* Set as transmitter */
            i2cSetStart(pI2cInterface);                             /* Start receive */
                                                                    /* Receive nrBytes bytes with DMA */

            uint32_t notificationRx = I2C_WaitForRxCompletedNotification();
            if (notificationRx != I2C_RX_NOTIFIED_VALUE) {
                /* Rx not happened, deactivate DMA */
                pI2cInterface->DMACR &= ~((uint32_t)I2C_RXDMAEN);
                /* Set Stop condition */
                pI2cInterface->MDR |= (uint32_t)I2C_REPEATMODE;
                i2cSetStop(pI2cInterface);
                retVal = STD_NOT_OK;
            } else {
                /* Rx happened */
                if (pI2cInterface == i2cREG1) {
                    readData[nrBytesRead - 1u] = i2c_rxLastByteInterface1;
                } else if (pI2cInterface == i2cREG2) {
                    readData[nrBytesRead - 1u] = i2c_rxLastByteInterface2;
                } else {
                    /* invalid I2C interface */
                    FAS_ASSERT(FAS_TRAP);
                }
                bool success = I2C_WaitStop(pI2cInterface, I2C_TIMEOUT_us);
                if (success == false) {
                    /* Set Stop condition */
                    pI2cInterface->MDR |= (uint32_t)I2C_REPEATMODE;
                    i2cSetStop(pI2cInterface);
                    retVal = STD_NOT_OK;
                }
            }
        }
    } else {
        retVal = STD_NOT_OK;
    }

    return retVal;
}

extern uint8_t I2C_ReadLastRxByte(i2cBASE_t *pI2cInterface) {
    FAS_ASSERT(pI2cInterface != NULL_PTR);
    uint8_t lastReadByte = (uint8)(pI2cInterface->DRR & I2C_DDR_REGISTER_DATA_MASK);
    return lastReadByte;
}

extern bool I2C_WaitReceive(i2cBASE_t *pI2cInterface, uint32_t timeout_us) {
    FAS_ASSERT(pI2cInterface != NULL_PTR);
    bool success          = true;
    bool timeElapsed      = false;
    uint32_t startCounter = MCU_GetFreeRunningCount();

    while (((pI2cInterface->STR & (uint32_t)I2C_RX_INT) == 0u) && (timeElapsed == false)) {
        timeElapsed = MCU_IsTimeElapsed(startCounter, timeout_us);
    }

    if (timeElapsed == true) {
        success = false;
    }

    return success;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
