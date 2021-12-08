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
 * @file    i2c.c
 * @author  foxBMS Team
 * @date    2021-07-22 (date of creation)
 * @updated 2021-12-08 (date of last update)
 * @ingroup DRIVERS
 * @prefix  I2C
 *
 * @brief   Driver for the I2C module
 *
 */

/*========== Includes =======================================================*/
#include "i2c.h"

#include "database.h"
#include "diag.h"
#include "dma.h"
#include "fsystem.h"
#include "mcu.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern void I2C_Initialize(void) {
    i2cInit();
}

extern STD_RETURN_TYPE_e I2C_Read(uint32_t slaveAddress, uint8_t readAddress, uint32_t nrBytes, uint8_t *readData) {
    FAS_ASSERT(readData != NULL_PTR);

    STD_RETURN_TYPE_e retVal = STD_OK;
    uint16_t timeout         = I2C_TIMEOUT_ITERATIONS;
    bool nack                = false;
    uint8_t *data            = readData;
    uint32_t count           = nrBytes;

    if ((i2cREG1->STR & (uint32_t)I2C_BUSBUSY) == 0u) {
        /* Clear bits */
        i2cREG1->MDR &= ~((uint32_t)I2C_STOP_COND);
        i2cREG1->MDR &= ~((uint32_t)I2C_START_COND);
        i2cREG1->MDR &= ~((uint32_t)I2C_REPEATMODE);
        i2cREG1->STR |= (uint32_t)I2C_TX_INT;
        i2cREG1->STR |= (uint32_t)I2C_RX_INT;

        i2cSetMode(i2cREG1, (uint32_t)I2C_MASTER);           /* Set as master */
        i2cSetDirection(i2cREG1, (uint32_t)I2C_TRANSMITTER); /* Set as transmitter */
        i2cSetSlaveAdd(i2cREG1, slaveAddress);               /* Set slave address */
        i2cSetCount(i2cREG1, 1u);                            /* Send 1 byte */
        i2cSetStart(i2cREG1);                                /* Start write */
        i2cREG1->DXR = (uint32_t)readAddress;                /* Send register address */

        /* Wait until Tx buffer was copied to shift buffer */
        timeout = I2C_TIMEOUT_ITERATIONS;
        while (((i2cREG1->STR & (uint32_t)I2C_TX_INT) == 0u) && (timeout > 0u)) {
            if ((i2cREG1->STR & (uint32_t)I2C_NACK) != 0u) {
                nack = true;
                break;
            }
            timeout--;
        }

        if (timeout == 0u) {
            /* Set repeat flag */
            i2cREG1->MDR |= (uint32_t)I2C_REPEATMODE;
            /* Set Stop condition */
            i2cSetStop(i2cREG1);
            timeout = I2C_TIMEOUT_ITERATIONS;
            while ((i2cIsStopDetected(i2cREG1) == 0u) && (timeout > 0u)) {
                timeout--;
            }
            retVal = STD_NOT_OK;
        } else {
            /* If slave ACK received, receive data */
            if ((i2cREG1->STR & (uint32_t)I2C_NACK) == 0u) {
                i2cSetMode(i2cREG1, (uint32_t)I2C_MASTER);        /* Set as master */
                i2cSetDirection(i2cREG1, (uint32_t)I2C_RECEIVER); /* Set as transmitter */
                i2cSetCount(i2cREG1, nrBytes);                    /* Send count bytes before STOP condition */
                i2cSetStart(i2cREG1);                             /* Start write */
                i2cSetStop(i2cREG1);                              /* Stop condition after sending nrBytes bytes */

                /* Receive nrBytes bytes in polling mode */
                while (count > 0u) {
                    timeout = I2C_TIMEOUT_ITERATIONS;
                    while (((i2cREG1->STR & (uint32_t)I2C_RX_INT) == 0u) && (timeout > 0u)) {
                        if ((i2cREG1->STR & (uint32_t)I2C_NACK_INT) != 0u) {
                            nack = true;
                            break;
                        }
                        timeout--;
                    }
                    if ((nack == true) || (timeout == 0u)) {
                        break;
                    }
                    *data = ((uint8)i2cREG1->DRR);
                    data++;
                    count--;
                }
                if ((nack == true) || (timeout == 0u)) {
                    /* Set repeat flag */
                    i2cREG1->MDR |= (uint32_t)I2C_REPEATMODE;
                    /* Set Stop condition */
                    i2cSetStop(i2cREG1);
                    timeout = I2C_TIMEOUT_ITERATIONS;
                    while ((i2cIsStopDetected(i2cREG1) == 0u) && (timeout > 0u)) {
                        timeout--;
                    }
                    retVal = STD_NOT_OK;
                } else {
                    /* Wait until Stop is detected */
                    timeout = I2C_TIMEOUT_ITERATIONS;
                    while ((i2cIsStopDetected(i2cREG1) == 0u) && (timeout > 0u)) {
                        timeout--;
                    }
                    if (timeout == 0u) {
                        retVal = STD_NOT_OK;
                    } else {
                        i2cClearSCD(i2cREG1); /* Clear the Stop condition */
                    }
                }
            } else {
                I2C_SetStopNow();
                retVal = STD_NOT_OK;
            }
        }
    } else {
        I2C_SetStopNow();
        retVal = STD_NOT_OK;
    }

    return retVal;
}

extern STD_RETURN_TYPE_e I2C_ReadDirect(uint32_t slaveAddress, uint32_t nrBytes, uint8_t *readData) {
    FAS_ASSERT(readData != NULL_PTR);

    STD_RETURN_TYPE_e retVal = STD_OK;
    uint16_t timeout         = I2C_TIMEOUT_ITERATIONS;
    bool nack                = false;
    uint8_t *data            = readData;
    uint32_t count           = nrBytes;

    if ((i2cREG1->STR & (uint32_t)I2C_BUSBUSY) == 0u) {
        /* Clear bits */
        i2cREG1->MDR &= ~((uint32_t)I2C_STOP_COND);
        i2cREG1->MDR &= ~((uint32_t)I2C_START_COND);
        i2cREG1->MDR &= ~((uint32_t)I2C_REPEATMODE);
        i2cREG1->STR |= (uint32_t)I2C_TX_INT;
        i2cREG1->STR |= (uint32_t)I2C_RX_INT;
        i2cREG1->STR |= (uint32_t)I2C_TX_INT;
        i2cREG1->STR |= (uint32_t)I2C_RX_INT;

        i2cSetMode(i2cREG1, (uint32_t)I2C_MASTER);        /* Set as master */
        i2cSetDirection(i2cREG1, (uint32_t)I2C_RECEIVER); /* Set as transmitter */
        i2cSetSlaveAdd(i2cREG1, slaveAddress);            /* Set slave address */
        i2cSetCount(i2cREG1, nrBytes);                    /* Send count bytes before STOP condition */
        i2cSetStart(i2cREG1);                             /* Start write */
        i2cSetStop(i2cREG1);                              /* Stop condition after sending nrBytes bytes */

        /* Receive nrBytes bytes in polling mode */
        while (count > 0u) {
            timeout = I2C_TIMEOUT_ITERATIONS;
            while (((i2cREG1->STR & (uint32_t)I2C_RX_INT) == 0u) && (timeout > 0u)) {
                if ((i2cREG1->STR & (uint32_t)I2C_NACK_INT) != 0u) {
                    nack = true;
                    break;
                }
                timeout--;
            }
            if ((nack == true) || (timeout == 0u)) {
                break;
            }
            *data = ((uint8)i2cREG1->DRR);
            data++;
            count--;
        }
        if ((nack == true) || (timeout == 0u)) {
            /* Set repeat flag */
            i2cREG1->MDR |= (uint32_t)I2C_REPEATMODE;
            /* Set Stop condition */
            i2cSetStop(i2cREG1);
            timeout = I2C_TIMEOUT_ITERATIONS;
            while ((i2cIsStopDetected(i2cREG1) == 0u) && (timeout > 0u)) {
                timeout--;
            }
            retVal = STD_NOT_OK;
        } else {
            /* Wait until Stop is detected */
            timeout = I2C_TIMEOUT_ITERATIONS;
            while ((i2cIsStopDetected(i2cREG1) == 0u) && (timeout > 0u)) {
                timeout--;
            }
            if (timeout == 0u) {
                retVal = STD_NOT_OK;
            } else {
                i2cClearSCD(i2cREG1); /* Clear the Stop condition */
            }
        }
    } else {
        I2C_SetStopNow();
        retVal = STD_NOT_OK;
    }

    return retVal;
}

extern STD_RETURN_TYPE_e I2C_Write(uint32_t slaveAddress, uint8_t writeAddress, uint32_t nrBytes, uint8_t *writeData) {
    FAS_ASSERT(writeData != NULL_PTR);

    STD_RETURN_TYPE_e retVal = STD_OK;
    uint16_t timeout         = I2C_TIMEOUT_ITERATIONS;
    bool nack                = false;
    uint8_t *data            = writeData;
    uint32_t count           = nrBytes;

    if ((i2cREG1->STR & (uint32_t)I2C_BUSBUSY) == 0u) {
        /* Clear bits */
        i2cREG1->MDR &= ~((uint32_t)I2C_STOP_COND);
        i2cREG1->MDR &= ~((uint32_t)I2C_START_COND);
        i2cREG1->MDR &= ~((uint32_t)I2C_REPEATMODE);
        i2cREG1->STR |= (uint32_t)I2C_TX_INT;
        i2cREG1->STR |= (uint32_t)I2C_RX_INT;

        i2cSetMode(i2cREG1, (uint32_t)I2C_MASTER);           /* Set as master */
        i2cSetDirection(i2cREG1, (uint32_t)I2C_TRANSMITTER); /* Set as transmitter */
        i2cSetSlaveAdd(i2cREG1, slaveAddress);               /* Set slave address */
        i2cSetStop(i2cREG1);                                 /* Stop condition after sending nrBytes bytes */
        i2cSetCount(i2cREG1, nrBytes + 1u);                  /* Send count bytes before STOP condition */
        i2cSetStart(i2cREG1);                                /* Start write */

        /*  Send register address */
        i2cREG1->DXR = (uint32_t)writeAddress;
        /* Wait until Tx buffer was copied to shift buffer */
        timeout = I2C_TIMEOUT_ITERATIONS;
        while (((i2cREG1->STR & (uint32_t)I2C_TX_INT) == 0u) && (timeout > 0u)) {
            if ((i2cREG1->STR & (uint32_t)I2C_NACK) != 0u) {
                nack = true;
                break;
            }
            timeout--;
        }

        if (timeout == 0u) {
            i2cREG1->MDR |= (uint32_t)I2C_REPEATMODE; /* Set repeat flag */
            i2cSetStop(i2cREG1);                      /* Set Stop condition */

            timeout = I2C_TIMEOUT_ITERATIONS;
            while ((i2cIsStopDetected(i2cREG1) == 0u) && (timeout > 0u)) {
                timeout--;
            }
            retVal = STD_NOT_OK;
        } else {
            /* If slave ACK received, send data */
            if ((i2cREG1->STR & (uint32_t)I2C_NACK) == 0u) {
                /* Send nrBytes bytes in polling mode */
                while (count > 0u) {
                    /* Wait until Tx buffer was copied to shift buffer */
                    timeout = I2C_TIMEOUT_ITERATIONS;
                    while (((i2cREG1->STR & (uint32_t)I2C_TX_INT) == 0u) && (timeout > 0u)) {
                        if ((i2cREG1->STR & (uint32_t)I2C_NACK) != 0u) {
                            nack = true;
                            break;
                        }
                        timeout--;
                    }
                    if ((nack == true) || (timeout == 0u)) {
                        break;
                    }
                    i2cREG1->DXR = (uint32_t)*data;
                    data++;
                    count--;
                }
                if ((nack == true) || (timeout == 0u)) {
                    i2cREG1->MDR |= (uint32_t)I2C_REPEATMODE; /* Set repeat flag */
                    i2cSetStop(i2cREG1);                      /* Set Stop condition */

                    timeout = I2C_TIMEOUT_ITERATIONS;
                    while ((i2cIsStopDetected(i2cREG1) == 0u) && (timeout > 0u)) {
                        timeout--;
                    }
                    retVal = STD_NOT_OK;
                } else {
                    /* Wait until Stop is detected */
                    timeout = I2C_TIMEOUT_ITERATIONS;
                    while ((i2cIsStopDetected(i2cREG1) == 0u) && (timeout > 0u)) {
                        timeout--;
                    }
                    if (timeout == 0u) {
                        retVal = STD_NOT_OK;
                    } else {
                        i2cClearSCD(i2cREG1); /* Clear the Stop condition */
                    }
                }
            } else {
                I2C_SetStopNow();
                retVal = STD_NOT_OK;
            }
        }
    } else {
        I2C_SetStopNow();
        retVal = STD_NOT_OK;
    }

    return retVal;
}

extern STD_RETURN_TYPE_e I2C_WriteDirect(uint32_t slaveAddress, uint32_t nrBytes, uint8_t *writeData) {
    FAS_ASSERT(writeData != NULL_PTR);

    STD_RETURN_TYPE_e retVal = STD_OK;
    uint16_t timeout         = I2C_TIMEOUT_ITERATIONS;
    bool nack                = false;
    uint8_t *data            = writeData;
    uint32_t count           = nrBytes;

    if ((i2cREG1->STR & (uint32_t)I2C_BUSBUSY) == 0u) {
        /* Clear bits */
        i2cREG1->MDR &= ~((uint32_t)I2C_STOP_COND);
        i2cREG1->MDR &= ~((uint32_t)I2C_START_COND);
        i2cREG1->MDR &= ~((uint32_t)I2C_REPEATMODE);
        i2cREG1->STR |= (uint32_t)I2C_TX_INT;
        i2cREG1->STR |= (uint32_t)I2C_RX_INT;

        i2cSetMode(i2cREG1, (uint32_t)I2C_MASTER);           /* Set as master */
        i2cSetDirection(i2cREG1, (uint32_t)I2C_TRANSMITTER); /* Set as transmitter */
        i2cSetSlaveAdd(i2cREG1, slaveAddress);               /* Set slave address */
        i2cSetStop(i2cREG1);                                 /* Stop condition after sending nrBytes bytes */
        i2cSetCount(i2cREG1, nrBytes);                       /* Send count bytes before STOP condition */
        i2cSetStart(i2cREG1);                                /* Start write */

        /* If slave ACK received, send data */
        if ((i2cREG1->STR & (uint32_t)I2C_NACK) == 0u) {
            /* Send nrBytes bytes in polling mode */
            while (count > 0u) {
                /* Wait until Tx buffer was copied to shift buffer */
                timeout = I2C_TIMEOUT_ITERATIONS;
                while (((i2cREG1->STR & (uint32_t)I2C_TX_INT) == 0u) && (timeout > 0u)) {
                    if ((i2cREG1->STR & (uint32_t)I2C_NACK) != 0u) {
                        nack = true;
                        break;
                    }
                    timeout--;
                }
                if ((nack == true) || (timeout == 0u)) {
                    break;
                }
                i2cREG1->DXR = (uint32_t)*data;
                data++;
                count--;
            }
            if ((nack == true) || (timeout == 0u)) {
                i2cREG1->MDR |= (uint32_t)I2C_REPEATMODE; /* Set repeat flag */
                i2cSetStop(i2cREG1);                      /* Set Stop condition */

                timeout = I2C_TIMEOUT_ITERATIONS;
                while ((i2cIsStopDetected(i2cREG1) == 0u) && (timeout > 0u)) {
                    timeout--;
                }
                retVal = STD_NOT_OK;
            } else {
                /* Wait until Stop is detected */
                timeout = I2C_TIMEOUT_ITERATIONS;
                while ((i2cIsStopDetected(i2cREG1) == 0u) && (timeout > 0u)) {
                    timeout--;
                }
                if (timeout == 0u) {
                    retVal = STD_NOT_OK;
                } else {
                    i2cClearSCD(i2cREG1); /* Clear the Stop condition */
                }
            }
        } else {
            I2C_SetStopNow();
            retVal = STD_NOT_OK;
        }
    } else {
        I2C_SetStopNow();
        retVal = STD_NOT_OK;
    }

    return retVal;
}

extern STD_RETURN_TYPE_e I2C_ReadDma(uint32_t slaveAddress, uint8_t readAddress, uint32_t nrBytes, uint8_t *readData) {
    FAS_ASSERT(readData != NULL_PTR);

    STD_RETURN_TYPE_e retVal = STD_OK;
    uint16_t timeout         = I2C_TIMEOUT_ITERATIONS;

    if ((i2cREG1->STR & (uint32_t)I2C_BUSBUSY) == 0u) {
        OS_EnterTaskCritical();

        FAS_ASSERT(FSYS_RaisePrivilege() == 0); /* Go to privileged mode to write DMA config registers */

        /* Set Tx buffer address */
        /* AXIVION Disable Style MisraC2012-1.1: Cast necessary for DMA configuration */
        dmaRAMREG->PCP[(dmaChannel_t)DMA_CHANNEL_I2C_RX].IDADDR = (uint32_t)readData;
        /* AXIVION Enable Style MisraC2012-1.1: */
        /* Set number of Tx bytes to transmit */
        dmaRAMREG->PCP[(dmaChannel_t)DMA_CHANNEL_I2C_RX].ITCOUNT = (nrBytes << 16U) | 1U;

        dmaSetChEnable((dmaChannel_t)DMA_CHANNEL_I2C_RX, (dmaTriggerType_t)DMA_HW);

        FSYS_SwitchToUserMode(); /* DMA config registers written, leave privileged mode */
        OS_ExitTaskCritical();

        i2cREG1->DMACR |= (uint32_t)I2C_RXDMAEN; /* Activate I2C DMA RX */

        i2cSetMode(i2cREG1, (uint32_t)I2C_MASTER);           /* Set to master */
        i2cSetDirection(i2cREG1, (uint32_t)I2C_TRANSMITTER); /* Set as transmitter */
        i2cSetSlaveAdd(i2cREG1, slaveAddress);               /* Set slave address */
        i2cSetCount(i2cREG1, 1u);                            /* Write one byte to select register address */
        i2cSetStart(i2cREG1);                                /* Start write */
        i2cSendByte(i2cREG1, readAddress);                   /* Transmit register address */

        /* Wait until Tx buffer was copied to shift buffer */
        while ((i2cREG1->STR & (uint32_t)I2C_TX_INT) == 0u) {
            if ((i2cREG1->STR & (uint32_t)I2C_NACK) != 0u) {
                break;
            }
        }

        if ((i2cREG1->STR & (uint32_t)I2C_NACK) == 0u) {
            i2cSetDirection(i2cREG1, (uint32_t)I2C_RECEIVER); /* Set as receiver */
            i2cSetCount(i2cREG1, nrBytes);                    /* Receive nrBytes before STOP condition */
            i2cSetMode(i2cREG1, (uint32_t)I2C_MASTER);        /* Set as master */
            i2cSetStop(i2cREG1);                              /* Stop condition after receiving nrBytes bytes */
            i2cSetStart(i2cREG1); /* Set start while bus busy for REPEATED START condition */
        } else {
            i2cREG1->STR |= (uint32_t)I2C_NACK;   /* Clear NACK flag */
            i2cREG1->STR |= (uint32_t)I2C_TX_INT; /* Set Tx ready flag */
            i2cSetStop(i2cREG1);                  /* Set Stop condition */

            /* Wait until Stop is detected */
            timeout = I2C_TIMEOUT_ITERATIONS;
            while ((i2cIsStopDetected(i2cREG1) == 0u) && (timeout > 0u)) {
                timeout--;
            }

            i2cClearSCD(i2cREG1); /* Clear the Stop condition */
            retVal = STD_NOT_OK;
        }
    } else {
        retVal = STD_NOT_OK;
    }

    return retVal;
}

extern STD_RETURN_TYPE_e I2C_WriteDma(
    uint32_t slaveAddress,
    uint8_t writeAddress,
    uint32_t nrBytes,
    uint8_t *writeData) {
    FAS_ASSERT(writeData != NULL_PTR);

    STD_RETURN_TYPE_e retVal = STD_OK;
    uint16_t timeout         = I2C_TIMEOUT_ITERATIONS;

    if ((i2cREG1->STR & (uint32_t)I2C_BUSBUSY) == 0u) {
        OS_EnterTaskCritical();

        FAS_ASSERT(FSYS_RaisePrivilege() == 0); /* Go to privileged mode to write DMA config registers */

        /* Set Tx buffer address */
        /* AXIVION Disable Style MisraC2012-1.1: Cast necessary for DMA configuration */
        dmaRAMREG->PCP[(dmaChannel_t)DMA_CHANNEL_I2C_TX].ISADDR = (uint32_t)writeData;
        /* AXIVION Enable Style MisraC2012-1.1: */
        /* Set number of Tx bytes to transmit */
        dmaRAMREG->PCP[(dmaChannel_t)DMA_CHANNEL_I2C_TX].ITCOUNT = (nrBytes << 16U) | 1U;

        dmaSetChEnable((dmaChannel_t)DMA_CHANNEL_I2C_TX, (dmaTriggerType_t)DMA_HW);

        FSYS_SwitchToUserMode(); /* DMA config registers written, leave privileged mode */
        OS_ExitTaskCritical();

        i2cSetMode(i2cREG1, (uint32_t)I2C_MASTER);           /* Set as master */
        i2cSetDirection(i2cREG1, (uint32_t)I2C_TRANSMITTER); /* Set as transmitter */
        i2cSetSlaveAdd(i2cREG1, slaveAddress);               /* Set slave address */
        i2cSetStop(i2cREG1);                                 /* Stop condition after sending nrBytes bytes */
        i2cSetCount(i2cREG1, nrBytes + 1u);      /* Send (nrBytes+1) before STOP condition (includes register address)*/
        i2cSendByte(i2cREG1, writeAddress);      /*  Send register address */
        i2cREG1->DMACR |= (uint32_t)I2C_TXDMAEN; /* Activate I2C DMA TX */
        i2cSetStart(i2cREG1);                    /* Start write */

        /* Wait until Tx buffer was copied to shift buffer */
        while ((i2cREG1->STR & (uint32_t)I2C_TX_INT) == 0u) {
            if ((i2cREG1->STR & (uint32_t)I2C_NACK) != 0u) {
                break;
            }
        }

        if ((i2cREG1->STR & (uint32_t)I2C_NACK) != 0u) {
            i2cREG1->STR |= (uint32_t)I2C_NACK;   /* Clear NACK flag */
            i2cREG1->STR |= (uint32_t)I2C_TX_INT; /* Set Tx ready flag */
            i2cSetStop(i2cREG1);                  /* Set Stop condition */
            timeout = I2C_TIMEOUT_ITERATIONS;     /* Wait until Stop is detected */

            while ((i2cIsStopDetected(i2cREG1) == 0u) && (timeout > 0u)) {
                timeout--;
            }

            i2cClearSCD(i2cREG1); /* Clear the Stop condition */
            i2cREG1->STR &= ~(uint32_t)I2C_REPEATMODE;
            retVal = STD_NOT_OK;
        }
    } else {
        retVal = STD_NOT_OK;
    }

    return retVal;
}

extern void I2C_SetStopNow(void) {
    /* Clear bits */
    i2cREG1->MDR &= ~((uint32_t)I2C_STOP_COND);
    i2cREG1->MDR &= ~((uint32_t)I2C_START_COND);
    i2cREG1->MDR &= ~((uint32_t)I2C_REPEATMODE);
    i2cREG1->STR |= (uint32_t)I2C_TX_INT;
    i2cREG1->STR |= (uint32_t)I2C_RX_INT;

    i2cREG1->MDR |= (uint32_t)I2C_REPEATMODE;            /* Set repeat flag */
    i2cSetMode(i2cREG1, (uint32_t)I2C_MASTER);           /* Set as master */
    i2cSetDirection(i2cREG1, (uint32_t)I2C_TRANSMITTER); /* Set as transmitter */
    i2cSetStop(i2cREG1);                                 /* Set Stop condition */

    while (i2cIsStopDetected(i2cREG1) == 0u) {
    }

    i2cREG1->MDR &= ~(uint32_t)I2C_REPEATMODE; /* Reset repeat flag */
    i2cClearSCD(i2cREG1);                      /* Clear the Stop condition */
}

/*========== Getter for static Variables (Unit Test) ========================*/
#ifdef UNITY_UNIT_TEST

#endif

/*========== Externalized Static Function Implementations (Unit Test) =======*/
