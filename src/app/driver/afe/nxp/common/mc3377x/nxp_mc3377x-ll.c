/* Copyright 2019 NXP
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following terms are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions
 * and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions,
 * and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ?AS IS? AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA; OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file    nxp_mc3377x-ll.c
 * @author  NXP
 * @date    2022-07-29 (date of creation)
 * @updated 2025-06-25 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  N77X
 *
 * @brief   Low level driver for the NXP MC3377xA
 * @details Implements the low level interface for the NXP MC3377xA IC.
 *          These functions are consumed by the abstracted, high level driver
 *          implemented nxp_mc3377x.c.
 *
 * Changes made to the original NXP MC3377xA driver:
 *      - Changed prefix to N77X
 *      - introduced variable responseIndex and N77X_TX_MIRROR_OFFSET to reflect
 *        the message structure better.
 */

/*========== Includes =======================================================*/
#include "nxp_mc3377x-ll.h"

#include "spi_cfg.h"

#include "dma.h"
#include "fassert.h"
#include "io.h"
#include "mcu.h"
#include "os.h"
#include "spi.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** Number of words (16 bits) used for write messages */
#define N77X_WRITE_SPI_BUFFER_SIZE (4u)
/** Number of words (16 bits) in read answers that do not contain data */
#define N77X_READ_HEADER_SPI_BUFFER_SIZE (3u)
/**
 * Number of words (16 bits) in read answers that contain data.
 * Based on maximum payload, which is 4 register per answer frame.
 */
#define N77X_READ_PAYLOAD_SPI_BUFFER_SIZE (4u)
/**
 *  Maximum number of groups of registers that can be read,
 *  to limit buffer size.
 *  Groups can be of size 1 to 4 registers.
 *  One frame = header (e.g., register address) +
 *              payload (data, 1 to 4 registers)
 */
#define N77X_MAX_ANSWER_FRAMES (30u)
/** Time to wait in microseconds after a write command */
#define N77X_WAIT_TIME_AFTER_WRITE_US (5u)
/** Time to wait in microseconds after a read command */
#define N77X_WAIT_TIME_AFTER_READ_US (5u)
/** Timeout to wait in microseconds for SPI send interrupt. */
#define N77X_SPI_WRITE_TIMEOUT_US (500u)
/**
 *  Timeout to wait in microseconds for reception of a read answer.
 *  After that time the communication is considered to have failed.
 */
#define N77X_SPI_READ_TIMEOUT_US (2000u)

/*========== Static Constant and Variable Definitions =======================*/
static uint16_t referenceMessageCounter[BS_NR_OF_STRINGS][512]; /* fits for all possible nodes */

#pragma SET_DATA_SECTION(".sharedRAM")
static uint16_t N77xToTplTxBuffer[N77X_WRITE_SPI_BUFFER_SIZE] = {0u};
static uint16_t N77xToTplRxBuffer[N77X_WRITE_SPI_BUFFER_SIZE] = {0u};
static uint16_t N77xFromTplTxBuffer
    [N77X_WRITE_SPI_BUFFER_SIZE +
     ((N77X_READ_HEADER_SPI_BUFFER_SIZE + N77X_READ_PAYLOAD_SPI_BUFFER_SIZE) * N77X_MAX_ANSWER_FRAMES)] = {0u};
static uint16_t N77xFromTplRxBuffer
    [N77X_WRITE_SPI_BUFFER_SIZE +
     ((N77X_READ_HEADER_SPI_BUFFER_SIZE + N77X_READ_PAYLOAD_SPI_BUFFER_SIZE) * N77X_MAX_ANSWER_FRAMES)] = {0u};
#pragma SET_DATA_SECTION()

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Copies a message to the buffer to be passed to the SPI transmit
 *          functions
 * @param   pBuffer
 * @param   message
 */
static void N77x_ConvertMessageToBuffer(uint16_t *pBuffer, uc_msg_t message);

/**
 * @brief   Wait for the SPI transmit communication to complete, using notifications
 *
 * return   N77X_TX_NOTIFIED_VALUE if notification received,
 *          N77X_NO_NOTIFIED_VALUE if timeout reached
 */
static uint32_t N77x_WaitForTxCompletedNotification(void);

/**
 * @brief   Wait for the SPI receive communication to complete, using notifications
 *
 * return   N77X_RX_NOTIFIED_VALUE if notification received,
 *          N77X_NO_NOTIFIED_VALUE if timeout reached
 */
static uint32_t N77x_WaitForRxCompletedNotification(void);

/*========== Static Function Implementations ================================*/

static void N77x_ConvertMessageToBuffer(uint16_t *pBuffer, uc_msg_t message) {
    FAS_ASSERT(pBuffer != NULL_PTR);
    pBuffer[0u] = message.head;
    pBuffer[1u] = message.data.dhead;
    pBuffer[2u] = message.data.data[0u];
    pBuffer[3u] = message.crc;
}

static uint32_t N77x_WaitForTxCompletedNotification(void) {
    uint32_t notifiedValueTx = N77X_NO_NOTIFIED_VALUE;
    /**
     * Suspend task and wait for SPI send DMA RX finished notification,
     * clear notification value on entry and exit
     */
    OS_WaitForNotificationIndexed(N77X_NOTIFICATION_TX_INDEX, &notifiedValueTx, N77X_NOTIFICATION_TX_TIMEOUT_ms);
    return notifiedValueTx;
}

static uint32_t N77x_WaitForRxCompletedNotification(void) {
    uint32_t notifiedValueRx = N77X_NO_NOTIFIED_VALUE;
    /**
     * Suspend task and wait for DMA RX notification,
     * clear notification value on entry and exit
     */
    OS_WaitForNotificationIndexed(N77X_NOTIFICATION_RX_INDEX, &notifiedValueRx, N77X_NOTIFICATION_RX_TIMEOUT_ms);
    return notifiedValueRx;
}

/*========== Extern Function Implementations ================================*/

extern void N77x_CommunicationWrite(
    uint16_t deviceAddress,
    uint16_t registerAddress,
    uint16_t value,
    SPI_INTERFACE_CONFIG_s *pSpiInterface) {
    FAS_ASSERT(pSpiInterface != NULL_PTR);
    uc_msg_t message = {0}; /* Message for read command */

    /** The function gets the device address
     * The N77x_CommunicationComposeMessage function adds the chain address.
     * Chain address = 1 used so (deviceAddress | (1u << 6u)) is used
     */
    N77x_CommunicationComposeMessage(
        BMS1_CMD_WRITE, 0, (deviceAddress | (1u << 6u)), registerAddress, 0, &value, &message);
    N77x_ConvertMessageToBuffer(N77xToTplTxBuffer, message);

    /*
     * Used to clear a pending Tx notification made by the read function
     * when writing to registers: when reading, commands are sent on SPI1.
     * In the SPI1 ISR, a notification is made, but it is not used: the system
     * waits for the SPI Rx notification from SPI4. As a consequence, when the
     * write function is called after the read function, there is already a
     * notification pending. The function waiting for the notification in the
     * ISR of SPI1 exits immediately instead of waiting for the end of the
     * transmission.
     */
    (void)OS_ClearNotificationIndexed(N77X_NOTIFICATION_TX_INDEX);

    SPI_TransmitReceiveDataDma(pSpiInterface, N77xToTplTxBuffer, N77xToTplRxBuffer, 4u);

    uint32_t notificationTx = N77x_WaitForTxCompletedNotification();
    if (notificationTx != N77X_TX_NOTIFIED_VALUE) {
        /* Tx DMA interrupt has not come, release Tx SPI interface */
        const uint8_t spiIndex = SPI_GetSpiIndex(spiREG1);
        OS_EnterTaskCritical();
        spi_busyFlags[spiIndex] = SPI_IDLE;
        OS_ExitTaskCritical();
    }
}

extern N77X_COMMUNICATION_STATUS_e N77x_CommunicationRead(
    uint16_t deviceAddress,
    uint16_t registerAddress,
    uint16_t *pValue,
    N77X_STATE_s *pState) {
    FAS_ASSERT(pValue != NULL_PTR);
    FAS_ASSERT(pState != NULL_PTR);
    FAS_ASSERT(pState->pSpiTxSequence != NULL_PTR);
    FAS_ASSERT(pState->pSpiRxSequence != NULL_PTR);
    return N77x_CommunicationReadMultiple(deviceAddress, 1, 1, registerAddress, pValue, pState);
}

extern N77X_COMMUNICATION_STATUS_e N77x_CommunicationReadMultiple(
    uint16_t deviceAddress,
    uint16_t numberOfItems,
    uint16_t responseLength,
    uint16_t registerAddress,
    uint16_t *pValues,
    N77X_STATE_s *pState) {
    FAS_ASSERT(pValues != NULL_PTR);
    FAS_ASSERT(pState != NULL_PTR);
    FAS_ASSERT(pState->pSpiTxSequence != NULL_PTR);
    FAS_ASSERT(pState->pSpiRxSequence != NULL_PTR);

    /* Number of registers to read */
    uint16_t itemsReadRemaining = numberOfItems;
    uint16_t *pReadValues       = pValues;

    /**
     * numberOfItems = 0 --> corresponds to one frame
     * (max_frames - 1): because first frame is for the mirroring of Tx
     * So (numberOfItems - 1u) is used in the following
     */

    /** The function gets the device address
     * The pack_msg function adds the chain address.
     * Chain address = 1 used so (deviceAddress | (1u << 6u)) is used in the following
     * */
    /* responseLength = 0 means one data word per answer frame so responseIndex is used in the following */

    uc_msg_t txMessage = {0u}; /* Message for read command */
    uc_msg_t rxMessage = {0u}; /* Message for response */

    N77X_COMMUNICATION_STATUS_e communicationStatus; /* Reception return code */
    uint16_t read_parameter;                         /* Read parameter (padding_en, responseLength, num_regs) */

    uint16_t rsp_cmd; /* Response parts */
    uint16_t rsp_mst_addr;
    uint16_t rsp_dev_addr;
    uint16_t rsp_reg_addr;
    uint16_t rsp_length;
    uint16_t rsp_values[4];

    uint16_t responseIndex = responseLength - 1u;

    read_parameter = ((uint16_t)1u << 10u) +       /* Use padding */
                     (responseIndex << 8u) +       /* responseLength */
                     ((numberOfItems - 1u) << 0u); /* (numberOfItems-1u) register to read */

    N77x_CommunicationComposeMessage(
        BMS1_CMD_READ, 0, (deviceAddress | (1u << 6u)), registerAddress, 0, &read_parameter, &txMessage);
    N77x_ConvertMessageToBuffer(N77xToTplTxBuffer, txMessage);

    /**
     * After transmission to daisy-chain, daisy-chain will normally answer
     * Already prepare SPI slave for reception
     */

    /**
     * Compute the number of answer frames needed by the MC3377xA.
     * Padding is used: always same number of registers in each answer frame
     * (1,2,3 or 4 registers).
     */
    uint16_t nrAnswerFrames = (((numberOfItems - 1u) + 1u) / (responseIndex + 1u));
    /**
     * Additional answer frame if number of registers to read and
     * number of registers per answer frame are not multiples
     * Example: 22 registers to read, 4 registers per answer frame.
     * Answer: (5*4 register) + (2 registers + 2 words padded with 0)
     */
    if ((((numberOfItems - 1u) + 1u) % (responseIndex + 1u)) != 0u) {
        nrAnswerFrames++;
    }
    /**
     * N77X_WRITE_SPI_BUFFER_SIZE (4u): because of Tx mirroring, the command is received, too
     * N77X_READ_HEADER_SPI_BUFFER_SIZE + responseLength: size of one answer frame */

    uint16_t rxBufferLength = N77X_WRITE_SPI_BUFFER_SIZE +
                              ((N77X_READ_HEADER_SPI_BUFFER_SIZE + responseLength) * nrAnswerFrames);

    FAS_ASSERT(
        rxBufferLength <=
        (N77X_WRITE_SPI_BUFFER_SIZE +
         ((N77X_READ_HEADER_SPI_BUFFER_SIZE + N77X_READ_PAYLOAD_SPI_BUFFER_SIZE) * N77X_MAX_ANSWER_FRAMES)));
    SPI_SlaveSetReceiveDataDma(pState->pSpiRxSequence, N77xFromTplTxBuffer, N77xFromTplRxBuffer, rxBufferLength);

    /* send message */
    SPI_TransmitReceiveDataDma(pState->pSpiTxSequence, N77xToTplTxBuffer, N77xToTplRxBuffer, N77X_RX_FRAME_LENGTH);
    bool N77x_rxCompleted = true;

    uint32_t notificationRx = N77x_WaitForRxCompletedNotification();
    if (notificationRx != N77X_RX_NOTIFIED_VALUE) {
        N77x_rxCompleted = false;
        /* Rx has not come, release Tx SPI interface */
        const uint8_t spiIndex = SPI_GetSpiIndex(spiREG1);
        OS_EnterTaskCritical();
        spi_busyFlags[spiIndex] = SPI_IDLE;
        OS_ExitTaskCritical();
    }

    if (N77x_rxCompleted == false) {
        pState->pSpiRxSequence->pNode->INT0 &= ~DMA_REQUEST_ENABLE_BIT;
        pState->pSpiRxSequence->pNode->GCR1 &= ~DMA_SPI_ENABLE_BIT;
        communicationStatus = N77X_COMMUNICATION_ERROR_TIMEOUT;
    } else {
        for (uint16_t i = 0; i < nrAnswerFrames; i++) {
            /* Currently only support for 64 bit communication */
            rxMessage.message_length = N77X_TX_MIRROR_OFFSET + (responseIndex);
            /* +4u: because of Tx mirroring, the command is received, too */

            rxMessage.head =
                N77xFromTplRxBuffer[N77X_TX_MIRROR_OFFSET + (i * (N77X_TX_MIRROR_OFFSET + responseIndex)) + 0u];
            rxMessage.data.dhead =
                N77xFromTplRxBuffer[N77X_TX_MIRROR_OFFSET + (i * (N77X_TX_MIRROR_OFFSET + responseIndex)) + 1u];
            rxMessage.data.data[0u] =
                N77xFromTplRxBuffer[N77X_TX_MIRROR_OFFSET + (i * (N77X_TX_MIRROR_OFFSET + (responseIndex))) + 2u];
            if (rxMessage.message_length <= 4u) {
                rxMessage.crc = N77xFromTplRxBuffer[N77X_TX_MIRROR_OFFSET + (i * N77X_TX_MIRROR_OFFSET) + 3u];
            } else {
                if (rxMessage.message_length == 5u) {
                    rxMessage.data.data[1u] = N77xFromTplRxBuffer
                        [N77X_TX_MIRROR_OFFSET + (i * (N77X_TX_MIRROR_OFFSET + (responseIndex))) + 3u];
                    rxMessage.crc = N77xFromTplRxBuffer
                        [N77X_TX_MIRROR_OFFSET + (i * (N77X_TX_MIRROR_OFFSET + (responseIndex))) + 4u];
                } else {
                    if (rxMessage.message_length == 6u) {
                        rxMessage.data.data[1u] = N77xFromTplRxBuffer
                            [N77X_TX_MIRROR_OFFSET + (i * (N77X_TX_MIRROR_OFFSET + (responseIndex))) + 3u];
                        rxMessage.data.data[2u] = N77xFromTplRxBuffer
                            [N77X_TX_MIRROR_OFFSET + (i * (N77X_TX_MIRROR_OFFSET + (responseIndex))) + 4u];
                        rxMessage.crc = N77xFromTplRxBuffer
                            [N77X_TX_MIRROR_OFFSET + (i * (N77X_TX_MIRROR_OFFSET + (responseIndex))) + 5u];
                    } else {
                        rxMessage.data.data[1u] = N77xFromTplRxBuffer
                            [N77X_TX_MIRROR_OFFSET + (i * (N77X_TX_MIRROR_OFFSET + (responseIndex))) + 3u];
                        rxMessage.data.data[2u] = N77xFromTplRxBuffer
                            [N77X_TX_MIRROR_OFFSET + (i * (N77X_TX_MIRROR_OFFSET + (responseIndex))) + 4u];
                        rxMessage.data.data[3u] = N77xFromTplRxBuffer
                            [N77X_TX_MIRROR_OFFSET + (i * (N77X_TX_MIRROR_OFFSET + (responseIndex))) + 5u];
                        rxMessage.crc = N77xFromTplRxBuffer
                            [N77X_TX_MIRROR_OFFSET + (i * (N77X_TX_MIRROR_OFFSET + (responseIndex))) + 6u];
                    }
                }
            }

            communicationStatus = N77x_CommunicationDecomposeMessage(
                &rxMessage,
                &rsp_cmd,
                &rsp_mst_addr,
                &rsp_dev_addr,
                &rsp_reg_addr,
                &rsp_length,
                rsp_values,
                pState->currentString);

            if (communicationStatus == N77X_COMMUNICATION_OK) {
                /* SM.e.30 : Communication - Unique ID */
                if (rsp_dev_addr == (deviceAddress | (1u << 6u))) {
                    if (rsp_reg_addr == (registerAddress + (i * ((responseIndex) + 1u)))) {
                        for (uint16_t j = 0u; j < (rsp_length + 1u); j++) {
                            /* To take padded answer frames into account
                             * Stop before reaching the words padded with 0s
                             */
                            if (itemsReadRemaining > 0u) {
                                *pReadValues = rsp_values[j];
                                pReadValues++;
                                itemsReadRemaining--;
                            } else {
                                break;
                            }
                        }
                    } else {
                        communicationStatus = N77X_COMMUNICATION_ERROR_NOT_MATCHING_REGISTER_ADDRESS;
                    }
                } else {
                    communicationStatus = N77X_COMMUNICATION_ERROR_NOT_MATCHING_DEVICE_ADDRESS;
                }
            }
        }
    }

    return communicationStatus;
}

/* Reset the message counter for one device */
extern void N77x_ResetMessageCounter(uint16_t deviceAddress, uint8_t string) {
    /* Chain address = 1 used so (deviceAddress | (1u << 6u)) is used */
    referenceMessageCounter[string][deviceAddress | (1u << 6u)] = 0u;
}

/* Low level communication functions */
/* Pack message */
extern void N77x_CommunicationComposeMessage(
    uint16_t cmd,
    uint16_t masterAddress,
    uint16_t deviceAddress,
    uint16_t registerAddress,
    uint16_t length,
    uint16_t *pValue,
    uc_msg_t *pMessage) {
    FAS_ASSERT(pValue != NULL_PTR);
    FAS_ASSERT(pMessage != NULL_PTR);
    /* Create message */
    set_cmd(pMessage, cmd);
    set_madd(pMessage, masterAddress);
    set_cadd(pMessage, deviceAddress >> 6u);
    set_devadd(pMessage, deviceAddress & 0x3Fu);
    set_msgcnt(pMessage, 0); /* not used by MCU, therefore fixed 0 */
    set_datalen(pMessage, length);
    set_regadd(pMessage, registerAddress);
    for (uint16_t i = 0; i <= length; i++) {
        set_data(pMessage, pValue[i], i);
    }
    set_message_length(pMessage, length + 4u);

    /* Create CRC */
    set_crc(pMessage, calc_crc(pMessage));
}

/* Unpack a message */
extern N77X_COMMUNICATION_STATUS_e N77x_CommunicationDecomposeMessage(
    uc_msg_t *pMessage,
    uint16_t *pCommand,
    uint16_t *pMasterAddress,
    uint16_t *pDeviceAddress,
    uint16_t *pRegisterAddress,
    uint16_t *pLength,
    uint16_t *pValue,
    uint8_t string) {
    FAS_ASSERT(pMessage != NULL_PTR);
    FAS_ASSERT(pCommand != NULL_PTR);
    FAS_ASSERT(pMasterAddress != NULL_PTR);
    FAS_ASSERT(pDeviceAddress != NULL_PTR);
    FAS_ASSERT(pRegisterAddress != NULL_PTR);
    FAS_ASSERT(pLength != NULL_PTR);
    FAS_ASSERT(pValue != NULL_PTR);
    uint16_t messageLength;        /* length of received message */
    uint16_t messageCount;         /* message count from message */
    uint16_t chainAddress;         /* chain address */
    uint16_t deviceAddressInChain; /* device address in the chain */

    bool errorCodeMatch                             = false;
    N77X_COMMUNICATION_STATUS_e communicationStatus = N77X_COMMUNICATION_OK;

    /* check if we have any content */
    /* SM.e.28 : Communication - Timeout monitoring */
    get_message_length(pMessage, &messageLength);
    if (messageLength == 0u) {
        errorCodeMatch      = true;
        communicationStatus = N77X_COMMUNICATION_ERROR_TIMEOUT;
    }

    if ((errorCodeMatch == false) && (messageLength < 4u)) {
        errorCodeMatch      = true;
        communicationStatus = N77X_COMMUNICATION_ERROR_SHORT_MESSAGE;
    }

    /* Check CRC */
    /* SM.e.27 : Communication - Information redundancy */
    if ((errorCodeMatch == false) && (!check_crc(pMessage, calc_crc(pMessage)))) {
        errorCodeMatch = true;
        uint16_t receivedCrc;
        get_crc(pMessage, &receivedCrc);
        communicationStatus = N77X_COMMUNICATION_ERROR_WRONG_CRC;
    }

    /* Extract message parts */
    get_cmd(pMessage, pCommand);
    get_madd(pMessage, pMasterAddress);
    get_cadd(pMessage, &chainAddress);
    get_devadd(pMessage, &deviceAddressInChain);
    *pDeviceAddress = (chainAddress << 6u) | deviceAddressInChain;
    get_msgcnt(pMessage, &messageCount);
    get_datalen(pMessage, pLength);
    get_regadd(pMessage, pRegisterAddress);

    /* Check message counter */
    /* SM.e.29 : Communication - Message counter */
    if ((errorCodeMatch == false) && (messageCount != referenceMessageCounter[string][*pDeviceAddress])) {
        errorCodeMatch                                   = true;
        referenceMessageCounter[string][*pDeviceAddress] = (messageCount + 1u) & 0x0Fu;
        communicationStatus                              = N77X_COMMUNICATION_ERROR_WRONG_MESSAGE_COUNT;
    }
    /* Increment message counter */
    referenceMessageCounter[string][*pDeviceAddress] = (referenceMessageCounter[string][*pDeviceAddress] + 1u) & 0x0Fu;

    /* Check error address */
    if ((errorCodeMatch == false) && (*pRegisterAddress == N77X_ERROR_REGISTER_ADDRESS)) {
        communicationStatus = N77X_COMMUNICATION_ERROR_NO_ACCESS;
    }

    /* return data */
    if (errorCodeMatch == false) {
        for (int i = 0u; i <= *pLength; i++) {
            get_data(pMessage, &pValue[i], i);
        }
        if (*pCommand != BMS1_CMD_RESP) { /* Check for response */
            communicationStatus = N77X_COMMUNICATION_ERROR_NO_RESPONSE;
        }
    }

    /* TODO: check for N77X_COMMUNICATION_ERROR_NOT_MATCHING_DEVICE_ADDRESS */
    /* TODO: check for N77X_COMMUNICATION_ERROR_NOT_MATCHING_REGISTER_ADDRESS */

    return communicationStatus;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
