/**
 *
 * @copyright &copy; 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * - "This product uses parts of foxBMS&reg;"
 * - "This product includes parts of foxBMS&reg;"
 * - "This product is derived from foxBMS&reg;"
 *
 */

/**
 * @file    boot.c
 * @author  foxBMS Team
 * @date    2021-08-02 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  BOOT
 *
 * @brief   File that contains all functions that are relevant into the
 *          workflow of bootloader and will be directly called in main.c.
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "boot.h"

#include "boot_cfg.h"
#include "flash_cfg.h"

#include "HL_reg_system.h"
#include "HL_sys_core.h"
#include "HL_system.h"

#include "boot_helper.h"
#include "can.h"
#include "crc.h"
#include "flash.h"
#include "fstd_types.h"
#include "fstring.h"
#include "fsystem.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
#ifndef UNITY_UNIT_TEST
/** This variable stores the number of times the function
 * #BOOT_WriteAndValidateCurrentSector has been entered during the loading
 * session of one sector.
 * It will be reset in two cases:
 *  1: if the CAN FSM state is not CAN_FSM_STATE_RECEIVED_8_BYTES_CRC
 *     while entering #BOOT_GetBootStateDuringLoad;
 *  2: if the function BOOT_WriteAndValidateCurrentSector return STD_NOT_OK;
*/
static uint8_t boot_numOfCurrentCrcValidation = 0u;

#endif

#ifndef UNITY_UNIT_TEST
#define BOOT_TRANSFER_ADDRESS  ((uint32_t)BOOT_VECTOR_TABLE_START_ADDRESS)
#define BOOT_TRANSFER_TO_APP() (((void (*)(void))(BOOT_TRANSFER_ADDRESS))())
#else
#define BOOT_TRANSFER_TO_APP()
#endif

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Write and validate the current sector.
 * @details This function will be called every time sector data is successfully
 *          transferred and needs to be validated and written into the
 *          corresponding address in flash.
 *          To validate the received sector data, which is temporarily stored
 *          in RAM, a 64-bits CRC signature will be calculated using the
 *          onboard CRC module and the calculated CRC signature will then be
 *          compared with the received CRC signature.
 *          The validation result is positive if these two values are the same;
 *          otherwise the received sector data will be evaluated as invalid.
 *          If the received sector data is valid, it will then be flashed into
 *          its corresponding onboard flash sector.
 * @return  #STD_OK if the current sector can be written and validated;
 *          otherwise #STD_NOT_OK.
 */
static STD_RETURN_TYPE_e BOOT_WriteAndValidateCurrentSector(void);

/*========== Static Function Implementations ================================*/
static STD_RETURN_TYPE_e BOOT_WriteAndValidateCurrentSector(void) {
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;
    bool gotoNext            = true;

    /* There should be no interrupt (especially from CAN RX) happening
     * while writing and validating the current sector. */
    _disable_IRQ_interrupt_();

    /* Entry protection to prevent this function being called more than one
     * time during the CAN FSM state CAN_FSM_STATE_RECEIVED_8_BYTES_CRC.
     * The value of boot_numOfCurrentCrcValidation will be reset in any other
     * CAN FSM state, or when this function is not successful. */
    boot_numOfCurrentCrcValidation++;

    /* Write the program sector that is stored in buffer to the flash (disable
     * the IRQ interrupt to prevent interruptions during the flash process.) */
    if (FSYS_RaisePrivilegeToSystemMode() == STD_NOT_OK) {
        gotoNext = false;
    }

    FLASH_FLASH_SECTOR_s currentSector = flash_kFlashSectorsInvalid;
    if (gotoNext) {
        /* Disable IRQ interrupt before every function that will be run from
         * RAM */
        _disable_IRQ_interrupt_();
        currentSector = FLASH_GetFlashSector((uint32_t *)can_infoOfDataTransfer.programCurrentSectorAddressU8);
        if (currentSector.isThisSectorValid == false) {
            gotoNext = false;
        }
    }

    uint8_t retValWriteFlash    = 1u;
    uint64_t crc_root           = 0u;
    uint32_t lenOfCurrentSector = 0u;
    if (gotoNext) {
        lenOfCurrentSector = (uint32_t)currentSector.pU8SectorAddressEnd -
                             (uint32_t)currentSector.pU32SectorAddressStart + 1u;

        /* Disable IRQ interrupt before every function that will be run from
         * RAM */
        _disable_IRQ_interrupt_();
        retValWriteFlash = FLASH_WriteFlashSector(
            (uint32_t *)can_infoOfDataTransfer.programCurrentSectorAddressU8,
            (uint8_t *)BOOT_SECTOR_BUFFER_START_ADDRESS,
            lenOfCurrentSector);
        FSYS_SwitchToUserMode();

        /* Calculate the CRC signature for this sector using the CRC signature
         * of last sector as the root value. */
        crc_root = can_infoOfDataTransfer.programCrc8BytesOnBoard;
        if (FSYS_RaisePrivilegeToSystemMode() == STD_NOT_OK) {
            gotoNext = false;
        }
    }

    if (gotoNext) {
        uint64_t crc_calc = CRC_SemiAutoCrcCalculation(
            (uint32_t)can_infoOfDataTransfer.programCurrentSectorAddressU8,
            (uint32_t)(lenOfCurrentSector / BOOT_NUM_OF_BYTES_IN_64_BITS),
            (uint64_t)crc_root);
        FSYS_SwitchToUserMode();

        /* Check if the current sector has been successfully written into the
         * flash memory and if the calculated CRC signature is the same as the
         * one received. */
        if ((retValWriteFlash == 0u) && (crc_calc == can_infoOfDataTransfer.programCrc8Bytes)) {
            /* The returned value will be STD_OK if everything went well, which
             * means the sector data has been successfully written into flash
             * and the CRC signature has been successfully calculated and
             * validated. */
            retVal = STD_OK;

            /* Update the CRC signature for the next calculation */
            can_infoOfDataTransfer.programCrc8BytesOnBoard = can_infoOfDataTransfer.programCrc8Bytes;
            /* Check if the end of data transfer has been reached */
            if (can_infoOfDataTransfer.numOfCurrentLoop == can_infoOfDataTransfer.totalNumOfDataTransferLoops) {
                /* Finished the flash process and set state to finished */
                can_stateOfCanCommunication = CAN_FSM_STATE_FINISHED_FINAL_VALIDATION;
                /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
                 * ref-comment-drop-return-value:
                 * The return value of the function is discarded because we
                 * intentionally do not evaluated to keep error handling in the
                 * embedded program simple.
                 * If the host program has dropped one CAN message, it will
                 * request to send it again.
                 * And if the host has dropped all messages, that means there
                 * are some problems with the CAN connection, and it will
                 * indicate an error status at the host side. */
                CAN_SendAcknowledgeMessage(RECEIVED, RECEIVED_8_BYTES_CRC, RECEIVED_AND_PROCESSED, YES);
            } else {
                /* Reset sector buffer address and update the sector address
                 * for next sector */
                can_infoOfDataTransfer.sectorBufferCurrentAddressU8 = BOOT_SECTOR_BUFFER_START_ADDRESS;
                can_infoOfDataTransfer.programCurrentSectorAddressU8 += lenOfCurrentSector;
                /* Send back a message with 'yes' only if the sector is valid */
                /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
                 * intentionally discarded see 'ref-comment-drop-return-value'
                 * in this file. */
                CAN_SendAcknowledgeMessage(RECEIVED, RECEIVED_8_BYTES_CRC, RECEIVED_AND_PROCESSED, YES);
            }
        } else {
            /* Reset the global variable can_infoOfDataTransfer using the value
             * saved in can_copyOfInfoOfDataTransfer if the current sector can
             * not be successfully written into flash memory or the CRC values
             * do not match. */
            CAN_CopyCanDataTransferInfo(&can_copyOfInfoOfDataTransfer, &can_infoOfDataTransfer);
            /* In case of first sector loops */
            if (can_infoOfDataTransfer.numOfCurrentLoop == 1u) {
                can_stateOfCanCommunication = CAN_FSM_STATE_WAIT_FOR_DATA_LOOPS;
            }
            /* Send back a message with 'no' to indicate that this sector is
             * invalid */
            /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
             * intentionally discarded see 'ref-comment-drop-return-value'
             * in this file. */
            CAN_SendAcknowledgeMessage(RECEIVED, RECEIVED_8_BYTES_CRC, RECEIVED_AND_PROCESSED, NO);
        }
    }

    return retVal;
}

/*========== Extern Function Implementations ================================*/
extern BOOT_FSM_STATES_e BOOT_GetBootState(void) {
    BOOT_FSM_STATES_e bootFsmState = BOOT_FSM_STATE_WAIT;

    switch (can_stateOfCanCommunication) {
        case CAN_FSM_STATE_NO_COMMUNICATION:
            /* Continue to wait */
            break;
        case CAN_FSM_STATE_RUN_PROGRAM:
            /* Return the state to run program */
            bootFsmState = BOOT_FSM_STATE_RUN;
            break;
        case CAN_FSM_STATE_RESET_BOOT:
            /* Return the state to reset bootloader  */
            bootFsmState = BOOT_FSM_STATE_RESET;
            break;
        case CAN_FSM_STATE_WAIT_FOR_INFO:
        case CAN_FSM_STATE_WAIT_FOR_DATA_LOOPS:
        case CAN_FSM_STATE_RECEIVED_LOOP_NUMBER:
        case CAN_FSM_STATE_RECEIVED_8_BYTES_DATA:
        case CAN_FSM_STATE_RECEIVED_8_BYTES_CRC:
        case CAN_FSM_STATE_FINISHED_FINAL_VALIDATION:
        case CAN_FSM_STATE_FINISHED_TRANSFER_VECTOR_TABLE:
        case CAN_FSM_STATE_VALIDATED_VECTOR_TABLE:
            /* Return the state to inform the loading program status */
            bootFsmState = BOOT_FSM_STATE_LOAD;
            break;
        case CAN_FSM_STATE_ERROR:
            /* Error in CAN FSM state triggers the error in boot fsm state */
            bootFsmState = BOOT_FSM_STATE_ERROR;
            break;
        default:
            /* Not registered states also trigger the error state */
            bootFsmState = BOOT_FSM_STATE_ERROR;
            break;
    }

    return bootFsmState;
}

extern BOOT_FSM_STATES_e BOOT_GetBootStateDuringError(void) {
    BOOT_FSM_STATES_e bootFsmState = BOOT_FSM_STATE_ERROR;
    /* CAN FSM state CAN_FSM_STATE_RESET_BOOT triggers the reset state */
    if (can_stateOfCanCommunication == CAN_FSM_STATE_RESET_BOOT) {
        bootFsmState = BOOT_FSM_STATE_RESET;
    }
    return bootFsmState;
}

extern BOOT_FSM_STATES_e BOOT_GetBootStateDuringLoad(void) {
    BOOT_FSM_STATES_e bootFsmState    = BOOT_FSM_STATE_LOAD;
    bool gotoNext                     = true;
    bool isBackupVectorTableAvailable = true;

    /* IRQ Interrupt (CAN RX) should be disabled once the CAN state of
     * communication (can_stateOfCanCommunication) is going to be checked, so
     * that it will not be changed during the relevant operations. */
    _disable_IRQ_interrupt_();

    switch (can_stateOfCanCommunication) {
        /* If one sector data and its CRC signature have been received */
        case CAN_FSM_STATE_RECEIVED_8_BYTES_CRC:
            if (boot_numOfCurrentCrcValidation == 0u) {
                if (BOOT_WriteAndValidateCurrentSector() == STD_NOT_OK) {
                    /* Trigger an CAN FSM error if any error happened when
                     * writing and validating the current sector. */
                    boot_numOfCurrentCrcValidation = 0u;
                    bootFsmState                   = BOOT_FSM_STATE_ERROR;
                }
            }
            break;

        /* If the received vector table has been validated, then it will be
         * written into flash together with other program infomation */
        case CAN_FSM_STATE_VALIDATED_VECTOR_TABLE:
            /* Reset boot_numOfCurrentCrcValidation to enable writing and
             * validating flash sectors once a sector data and its CRC
             * signature have been received. */
            boot_numOfCurrentCrcValidation = 0u;

            /* Following steps is to backup the vector table if it is not been
             * done yet. */
            /* Load the program info from flash first to make sure the vector
             * table of the bootloader has been loaded to
             * boot_currentVectorTable */
            BOOT_LoadProgramInfoFromFlash();

            /* Check if there is a back up vector table available */
            for (uint8_t i_vector = 0u; i_vector < BOOT_NUM_OF_VECTOR_TABLE_8_BYTES; i_vector++) {
                if (boot_backupVectorTable.vectorTable[i_vector] == 0u) {
                    isBackupVectorTableAvailable = false;
                    break;
                }
            }
            if (isBackupVectorTableAvailable == false) {
                for (uint8_t i_vector = 0u; i_vector < BOOT_NUM_OF_VECTOR_TABLE_8_BYTES; i_vector++) {
                    boot_backupVectorTable.vectorTable[i_vector] = boot_currentVectorTable.vectorTable[i_vector];
                }
            }

            /* Update the current vector table with the received vector table
             * from the CAN module. If any part of the vector table is 0, that
             * means that the relevant variables do not keep the real value of
             * the received CAN vector table and the current vector table will
             * not be updated. Meanwhile, it will also trigger an error state
             * in the boot FSM. */
            for (uint8_t i_canVector = 0u; i_canVector < BOOT_NUM_OF_VECTOR_TABLE_8_BYTES; i_canVector++) {
                if (can_infoOfDataTransfer.vectorTable[i_canVector] == 0u) {
                    gotoNext     = false;
                    bootFsmState = BOOT_FSM_STATE_ERROR;
                    break;
                }
            }

            if (gotoNext) {
                /* Update the current vector table using the received vector
                 * table via CAN */
                for (uint8_t i_canVector = 0u; i_canVector < BOOT_NUM_OF_VECTOR_TABLE_8_BYTES; i_canVector++) {
                    boot_currentVectorTable.vectorTable[i_canVector] = can_infoOfDataTransfer.vectorTable[i_canVector];
                }

                /* Transfer the program relevant information from CAN module
                 * variable into the boot module variable */
                boot_infoOfLastFlashedProgram.programLength        = can_infoOfDataTransfer.programLengthInBytes;
                boot_infoOfLastFlashedProgram.programStartAddress  = can_infoOfDataTransfer.programStartAddressU8;
                boot_infoOfLastFlashedProgram.programCrc8Bytes     = can_infoOfDataTransfer.programCrc8Bytes;
                boot_infoOfLastFlashedProgram.vectorTableCrc8Bytes = can_infoOfDataTransfer.crc64OfVectorTable;

                /* Set the program to be available */
                boot_infoOfLastFlashedProgram.isProgramAvailable = BOOT_PROGRAM_IS_AVAILABLE;

                /* Update the information (program infomation and vector table)
                 * into flash */
                if (BOOT_UpdateProgramInfoIntoFlash() == STD_OK) {
                    /* Clean up the RAM */
                    BOOT_CleanUpRam();
                    /* If everything went well and the program info (including
                     * the vector table of the application) have also been
                     * successfully written into flash, a software reset will
                     * be triggered. */
                    BOOT_SoftwareResetMcu();
                    /* If the previous function is successfully, the following
                     * line will not be reached.
                     * Otherwise, it will trigger an error. */
                    bootFsmState = BOOT_FSM_STATE_ERROR;
                } else {
                    /* If the information can not be successfully updated to
                     * flash, an error state will be triggered. */
                    bootFsmState = BOOT_FSM_STATE_ERROR;
                }
            }
            break;

        /* If the CAN FSM state is any other state which could happen while
         * loading the application but does not trigger any operation in boot
         * module */
        case CAN_FSM_STATE_WAIT_FOR_INFO:
        case CAN_FSM_STATE_WAIT_FOR_DATA_LOOPS:
        case CAN_FSM_STATE_RECEIVED_LOOP_NUMBER:
        case CAN_FSM_STATE_RECEIVED_8_BYTES_DATA:
        case CAN_FSM_STATE_FINISHED_FINAL_VALIDATION:
        case CAN_FSM_STATE_FINISHED_TRANSFER_VECTOR_TABLE:
            /* Reset boot_numOfCurrentCrcValidation to to ensure it is 0u
             * before CAN FSM state is changed to
             * CAN_FSM_STATE_RECEIVED_8_BYTES_CRC. */
            boot_numOfCurrentCrcValidation = 0u;
            break;

        case CAN_FSM_STATE_RESET_BOOT:
            /* Reset boot_numOfCurrentCrcValidation to enable writing and
             * validating flash sectors once a sector data and its CRC
             * signature have been received. */
            boot_numOfCurrentCrcValidation = 0u;
            bootFsmState                   = BOOT_FSM_STATE_RESET;
            break;

        case CAN_FSM_STATE_ERROR:
            /* Reset boot_numOfCurrentCrcValidation to enable writing and
             * validating flash sectors once a sector data and its CRC
             * signature have been received. */
            boot_numOfCurrentCrcValidation = 0u;
            bootFsmState                   = BOOT_FSM_STATE_ERROR;
            break;

        default:
            /* If CAN FSM state is any state which is not supposed to be there,
             * an error will triggered. */
            boot_numOfCurrentCrcValidation = 0u;
            bootFsmState                   = BOOT_FSM_STATE_ERROR;
            break;
    }

    /* Enable IRQ to ensure the CAN callback functions can be triggered again
    by incoming CAN messages */
    _enable_IRQ_interrupt_();

    return bootFsmState;
}

extern STD_RETURN_TYPE_e BOOT_JumpInToLastFlashedProgram(void) {
    bool gotoNext = true;

    /* Disable interrupts to prevent the execution of the following functions
     * being interfered by interrupt triggered by e.g., CAN RX */
    _disable_IRQ_interrupt_();

    if (FSYS_RaisePrivilegeToSystemMode() == STD_NOT_OK) {
        gotoNext = false;
    }

    if (gotoNext) {
        /* Initialize the memory and clean up the RAM */
        _memInit_();
        BOOT_CleanUpRam();
        _cacheDisable_();
        /* Jump into application */
        BOOT_TRANSFER_TO_APP();
        FSYS_SwitchToUserMode();
    }

    /* Enable IRQ interrupts again to ensure that the CAN callback functions
     * can be triggered again by incoming CAN messages */
    _enable_IRQ_interrupt_();

    return STD_NOT_OK;
}

extern bool BOOT_IsProgramAvailableAndValidated(void) {
    bool retVal   = false;
    bool gotoNext = true;

    /* Disable interrupts to prevent the execution of the following functions
     * being interfered by interrupt triggered by e.g., CAN RX */
    _disable_IRQ_interrupt_();

    /* Update the boot_infoOfLastFlashedProgram variable */
    BOOT_LoadProgramInfoFromFlash();
    if (boot_infoOfLastFlashedProgram.isProgramAvailable == BOOT_PROGRAM_IS_AVAILABLE) {
        /* If the current user mode has not been successfully changed into the
         * system mode, the function will return false, as the following
         * routines need to be run under in priviledged mode. */
        if (FSYS_RaisePrivilegeToSystemMode() == STD_NOT_OK) {
            gotoNext = false;
        }

        if (gotoNext) {
            /* Since the following CRC calculation takes the whole program
             * flash area as its input data source, this validation will fail
             * if the program flash area has not been totally filled during
             * last loading session. */
            uint64_t crc8BytesOnBoardCalculation = CRC_SemiAutoCrcCalculation(
                (uint32_t)BOOT_PROGRAM_START_ADDRESS,
                (uint32_t)boot_infoOfLastFlashedProgram.programLength / BOOT_NUM_OF_BYTES_IN_64_BITS,
                (uint64_t)0u);
            uint64_t crc64OfVectorTableOnBoard = CRC_CalculateCrc64(
                (const uint64_t *)boot_currentVectorTable.vectorTable, BOOT_NUM_OF_VECTOR_TABLE_8_BYTES, 0u);
            FSYS_SwitchToUserMode();
            /* Compared the saved CRC signature from last time with the new
             * calculated CRC */
            if ((boot_infoOfLastFlashedProgram.programCrc8Bytes == crc8BytesOnBoardCalculation) &&
                (boot_infoOfLastFlashedProgram.vectorTableCrc8Bytes == crc64OfVectorTableOnBoard)) {
                /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
                 * intentionally discarded see 'ref-comment-drop-return-value'
                 * in this file. */
                CAN_SendAcknowledgeMessage(RECEIVED, RECEIVED_CMD_TO_RUN_PROGRAM, RECEIVED_AND_PROCESSED, YES);

                retVal   = true;
                gotoNext = false;
            }
        }
    }

    if (gotoNext) {
        /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7:
         * intentionally discarded see 'ref-comment-drop-return-value' in this
         * file. */
        CAN_SendAcknowledgeMessage(RECEIVED, RECEIVED_CMD_TO_RUN_PROGRAM, RECEIVED_AND_PROCESSED, NO);
    }

    /* Reset can fsm state */
    can_stateOfCanCommunication = CAN_FSM_STATE_NO_COMMUNICATION;

    /* Enable IRQ to ensure the CAN callback functions can be triggered again
    by incoming CAN messages */
    _enable_IRQ_interrupt_();

    return retVal;
}

extern STD_RETURN_TYPE_e BOOT_ResetBootloader(void) {
    bool resetMcu = true;
    bool eraseApp = true;

    /* Disable interrupts to prevent the execution of the following functions
    being interfered by interrupt triggered by e.g. CAN RX */
    _disable_IRQ_interrupt_();

    /* Reset can communication relevant variables */
    CAN_ResetCanCommunication();

    /* Reset boot relevant variables and current vector table, and update them
     * to flash */
    if (BOOT_ResetBootInfo() == STD_NOT_OK) {
        /* BOOT_ResetBootInfo can fail for the following reasons:
            1. An app has never been successfully uploaded; therefore, there
                is no backup vector table available. Since the state of this operation
                is coupled with the returned value of BOOT_UpdateProgramInfoIntoFlash,
                it will fail 100% of the time due to the protection mechanism that
                returns STD_NOT_OK if any vector table member is zero.
            2. There are some problems with entering privileged mode or the flash
                function.
            If any reasons above happen, it can only lead the boolean 'eraseApp'
            to false and stop the process to erase app in flash memory. */
        eraseApp = false;
    }

    /* Clean up the common RAM area and the RAM area of the temporary sector
     * data */
    BOOT_CleanUpRam();

    if (eraseApp) {
        /* Clean up the area where the program and the flashed information will
         * be saved (temporally the flash bank 1) XXXXXXXXX */
        if (FSYS_RaisePrivilegeToSystemMode() == STD_NOT_OK) {
            eraseApp = false;
            resetMcu = false;
        }
    }

    if (eraseApp) {
        /* Disable IRQ interrupt before every function that will be run from
         * RAM */
        _disable_IRQ_interrupt_();
        /* If the flash can not be successfully erased, it leads to STD_NOT_OK. */
        if (FLASH_EraseFlashForApp() != true) {
            resetMcu = false;
        }
    }

    if (resetMcu) {
        /* PC jump to start */
        BOOT_SoftwareResetMcu();
    }

    if (eraseApp) {
        /* If we can ever get here and the boolean eraseApp is true. */
        FSYS_SwitchToUserMode();
    }

    /* Enable IRQ to ensure the CAN callback functions can be triggered again
     * by incoming CAN messages */
    _enable_IRQ_interrupt_();

    /* If we get here, some unforseen problems must have happened */
    return STD_NOT_OK;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern STD_RETURN_TYPE_e TEST_BOOT_WriteAndValidateCurrentSector(void) {
    return BOOT_WriteAndValidateCurrentSector();
}
#endif
