/**
 *
 * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    debug_can.c
 * @author  foxBMS Team
 * @date    2024-04-08 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup DRIVERS
 * @prefix  DECAN
 *
 * @brief   Driver implementation of the fake CAN to AFE driver to receive
 *          CAN messages (cell temperatures and cell voltages) from can via
 *          queues and write the relevant data back to the database.
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "debug_can.h"

#include "can_helper.h"
#include "database.h"
#include "ftask.h"
#include "os.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
/** local copies of database tables */
/**@{*/
static DATA_BLOCK_CELL_VOLTAGE_s decan_cellVoltage         = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_VOLTAGE_s decan_cellVoltageFromRead = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_TEMPERATURE_s decan_cellTemperature = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
static DATA_BLOCK_CELL_TEMPERATURE_s decan_cellTemperatureFromRead = {
    .header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
/**@}*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   A modified modulo operation: the input a can not be 0; if the result of a modulo operation is zero,
 * the returned value will be b
 * @param a First input of a modulo operation
 * @param b Second input of a modulo operation
 * @return a modified modulo result (uint16_t)
 */
static uint16_t DECAN_ModifiedModuloFunction(uint16_t a, uint16_t b);

/**
 * @brief   Convert the one number index of the cell voltage into the indexes of cellVoltage_mV
 * @param s Pointer of string number of the cellVoltage_mV in the database entry structure DATA_BLOCK_CELL_VOLTAGE_s
 * @param m Pointer of module number of the cellVoltage_mV in the database entry structure DATA_BLOCK_CELL_VOLTAGE_s
 * @param cb Pointer of cell number of the cellVoltage_mV in the database entry structure DATA_BLOCK_CELL_VOLTAGE_s
 * @param oneNumIdxOfVoltage One number index of cellVoltage
 */
static void DECAN_ConvertIndexForVoltage(uint16_t *s, uint16_t *m, uint16_t *cb, uint16_t oneNumIdxOfVoltage);

/**
 * @brief   Convert the one number index of the cell temperature into the indexes of cellTemperature_ddegC
 * @param s Pointer of string number of the cellTemperature_ddegC in the database entry structure
 * DATA_BLOCK_CELL_TEMPERATURE_s
 * @param m Pointer of module number of the cellTemperature_ddegC in the database entry structure
 * DATA_BLOCK_CELL_TEMPERATURE_s
 * @param ts Pointer of temperature sensor number of the cellTemperature_ddegC in the database entry structure
 * DATA_BLOCK_CELL_TEMPERATURE_s
 * @param oneNumIdxOfTemperature One number index of cellTemperature
 */
static void DECAN_ConvertIndexForTemperature(uint16_t *s, uint16_t *m, uint16_t *ts, uint16_t oneNumIdxOfTemperature);

/**
 * @brief   Receive can voltage data from queue and write into database
 * @return  #STD_OK if successful, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e DECAN_ReceiveCanCellVoltages(void);

/**
 * @brief   Receive can temperature data from queue and write into database
 * @return  #STD_OK if successful, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e DECAN_ReceiveCanCellTemperatures(void);

/*========== Static Function Implementations ================================*/
static uint16_t DECAN_ModifiedModuloFunction(uint16_t a, uint16_t b) {
    FAS_ASSERT(a > 0);
    FAS_ASSERT(b > 0);

    int16_t c = a % b;
    if (c == 0) {
        c = b;
    }
    return c;
}

static void DECAN_ConvertIndexForVoltage(uint16_t *s, uint16_t *m, uint16_t *cb, uint16_t oneNumIdxOfVoltage) {
    /* oneNumIdxOfVoltage can not be bigger than
    BS_NR_OF_STRINGS * BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_CELL_BLOCKS_PER_MODULE -1 */
    FAS_ASSERT(oneNumIdxOfVoltage < BS_NR_OF_STRINGS * BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_CELL_BLOCKS_PER_MODULE);

    /* The number of cell voltages starts from 1 while the index of the cell voltage starts from 0 */
    uint16_t num_cellVoltages = oneNumIdxOfVoltage + 1;

    /* Initialize the number of cell blocks(in the last string and the last module),
    modules(in the last string), strings */
    uint16_t num_cb = 1;
    uint16_t num_m  = 1;
    uint16_t num_s  = 1;

    /* Initialize a temporal variable for calculating the minimum number of modules*/
    uint16_t temp_m = 1;

    /* Get the number of cell blocks */
    num_cb = (uint16_t)(DECAN_ModifiedModuloFunction(num_cellVoltages, BS_NR_OF_CELL_BLOCKS_PER_MODULE));

    /* In the case that more than one module exists */
    if (num_cellVoltages > BS_NR_OF_CELL_BLOCKS_PER_MODULE) {
        /* Get the number of module while ignoring the exist of string*/
        if ((num_cellVoltages % BS_NR_OF_CELL_BLOCKS_PER_MODULE) == 0) {
            temp_m = (uint16_t)(num_cellVoltages / BS_NR_OF_CELL_BLOCKS_PER_MODULE);
        } else {
            temp_m = (uint16_t)(num_cellVoltages / BS_NR_OF_CELL_BLOCKS_PER_MODULE) + 1;
        }
        num_m = temp_m;

        /* If more than one string exists */
        if (temp_m > BS_NR_OF_MODULES_PER_STRING) {
            num_m = (uint16_t)(DECAN_ModifiedModuloFunction(temp_m, BS_NR_OF_MODULES_PER_STRING));
            /* Get the number of strings */
            if ((temp_m % BS_NR_OF_MODULES_PER_STRING) == 0) {
                num_s = (uint16_t)(temp_m / BS_NR_OF_MODULES_PER_STRING);
            } else {
                num_s = (uint16_t)(temp_m / BS_NR_OF_MODULES_PER_STRING) + 1;
            }
        }
    }

    /* Get the indexes *s, *m, *cb of cell voltage */
    *cb = num_cb - 1;
    *m  = num_m - 1;
    *s  = num_s - 1;
}

static void DECAN_ConvertIndexForTemperature(uint16_t *s, uint16_t *m, uint16_t *ts, uint16_t oneNumIdxOfTemperature) {
    /* oneNumIdxOfTemperature can not be bigger than
    BS_NR_OF_STRINGS * BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_TEMP_SENSORS_PER_MODULE -1 */
    FAS_ASSERT(
        oneNumIdxOfTemperature < BS_NR_OF_STRINGS * BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_TEMP_SENSORS_PER_MODULE);

    /* The number of cell temperature (num_cellTemperature) starts from 1
    while the index of cell temperature (oneNumIdxOfTemperature) starts from 0 */
    uint16_t num_cellTemperatures = oneNumIdxOfTemperature + 1;

    /* Initialize the number of temperature sensors (in the last string and the last module),
    modules(in the last string), strings */
    uint16_t num_ts = 1;
    uint16_t num_m  = 1;
    uint16_t num_s  = 1;

    /* Initialize a temporal variable for calculating the minimum number of modules*/
    uint16_t temp_m = 1;

    /* Get the number of temperature sensors s */
    num_ts = (uint16_t)(DECAN_ModifiedModuloFunction(num_cellTemperatures, BS_NR_OF_TEMP_SENSORS_PER_MODULE));

    /* In the case that more than one module exists */
    if (num_cellTemperatures > BS_NR_OF_TEMP_SENSORS_PER_MODULE) {
        /* Get the number of module while ignoring the exist of string*/
        if ((num_cellTemperatures % BS_NR_OF_TEMP_SENSORS_PER_MODULE) == 0) {
            temp_m = (uint16_t)(num_cellTemperatures / BS_NR_OF_TEMP_SENSORS_PER_MODULE);
        } else {
            temp_m = (uint16_t)(num_cellTemperatures / BS_NR_OF_TEMP_SENSORS_PER_MODULE) + 1;
        }
        num_m = temp_m;

        /* If more than one string exists */
        if (temp_m > BS_NR_OF_MODULES_PER_STRING) {
            num_m = (uint16_t)(DECAN_ModifiedModuloFunction(temp_m, BS_NR_OF_MODULES_PER_STRING));
            /* Get the number of strings */
            if ((temp_m % BS_NR_OF_MODULES_PER_STRING) == 0) {
                num_s = (uint16_t)(temp_m / BS_NR_OF_MODULES_PER_STRING);
            } else {
                num_s = (uint16_t)(temp_m / BS_NR_OF_MODULES_PER_STRING) + 1;
            }
        }
    }

    /* Get the indexes *s, *m, *cb of cell voltage */
    *ts = num_ts - 1;
    *m  = num_m - 1;
    *s  = num_s - 1;
}

static STD_RETURN_TYPE_e DECAN_ReceiveCanCellVoltages(void) {
    STD_RETURN_TYPE_e isSuccessful                                   = STD_NOT_OK;
    CAN_CAN2AFE_CELL_VOLTAGES_QUEUE_s decan_canCellVoltagesFromQueue = {0};

    /* Get decan_canCellVoltagesFromQueue from queue */
    OS_STD_RETURN_e receivedFromQueue = OS_ReceiveFromQueue(
        ftsk_canToAfeCellVoltagesQueue, (void *)&decan_canCellVoltagesFromQueue, DECAN_CAN2AFE_QUEUE_TIMEOUT_MS);
    if (receivedFromQueue == OS_SUCCESS) {
        /* Initialize the indexes for cellVoltage_mV */
        uint16_t s                  = 0u;
        uint16_t m                  = 0u;
        uint16_t cb                 = 0u;
        uint16_t oneNumIdxOfVoltage = decan_canCellVoltagesFromQueue.muxValue *
                                      CAN_NUM_OF_VOLTAGES_IN_CAN_CELL_VOLTAGES_MSG;
        uint16_t upperBoundOneNumIdxOfVoltage =
            BS_NR_OF_STRINGS * BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_CELL_BLOCKS_PER_MODULE - 1;

        /* Loop through all voltages in the received can message */
        for (uint8_t i = 0u; i < CAN_NUM_OF_VOLTAGES_IN_CAN_CELL_VOLTAGES_MSG; i++) {
            /* Check if the one number index of voltage surpass its upper bound */
            if (oneNumIdxOfVoltage > upperBoundOneNumIdxOfVoltage) {
                break;
            }
            /* Convert the one number index of temperature into s, m, ts */
            DECAN_ConvertIndexForVoltage(&s, &m, &cb, oneNumIdxOfVoltage);

            /* Write the received data */
            decan_cellVoltage.cellVoltage_mV[s][m][cb] = (int16_t)decan_canCellVoltagesFromQueue.cellVoltage[i];
            if (decan_canCellVoltagesFromQueue.invalidFlag[i] == DECAN_DATA_IS_VALID) {
                decan_cellVoltage.invalidCellVoltage[s][m][cb] = false;
            } else {
                decan_cellVoltage.invalidCellVoltage[s][m][cb] = true;
            }

            /* Update one number index of the cell voltage */
            oneNumIdxOfVoltage++;
        }

        /* Write this part of cell voltages and invalid flags to database,
        read the total cell voltages and invalid flags, count the number of
        invalid cell voltages and then write them back to database */
        uint16_t nrValidCellVoltagesPerString = 0u;
        int32_t stringVoltage_mV              = 0;
        int32_t moduleVoltage_mV              = 0;
        if ((DATA_WRITE_DATA(&decan_cellVoltage) == STD_OK) && (DATA_READ_DATA(&decan_cellVoltageFromRead) == STD_OK)) {
            for (uint16_t idxString = 0u; idxString < BS_NR_OF_STRINGS; idxString++) {
                nrValidCellVoltagesPerString = 0u;
                stringVoltage_mV             = 0;
                for (uint16_t idxModule = 0u; idxModule < BS_NR_OF_MODULES_PER_STRING; idxModule++) {
                    moduleVoltage_mV = 0;
                    for (uint16_t idxCellBlocks = 0u; idxCellBlocks < BS_NR_OF_CELL_BLOCKS_PER_MODULE;
                         idxCellBlocks++) {
                        if (decan_cellVoltageFromRead.invalidCellVoltage[idxString][idxModule][idxCellBlocks] ==
                            false) {
                            nrValidCellVoltagesPerString++;
                            moduleVoltage_mV +=
                                (int32_t)decan_cellVoltageFromRead.cellVoltage_mV[idxString][idxModule][idxCellBlocks];
                        }
                    }
                    decan_cellVoltageFromRead.moduleVoltage_mV[idxString][idxModule] = moduleVoltage_mV;
                    stringVoltage_mV += moduleVoltage_mV;
                }
                decan_cellVoltageFromRead.stringVoltage_mV[idxString]    = stringVoltage_mV;
                decan_cellVoltageFromRead.nrValidCellVoltages[idxString] = nrValidCellVoltagesPerString;
            }
            isSuccessful = DATA_WRITE_DATA(&decan_cellVoltageFromRead);
        }
    }

    return isSuccessful;
}

static STD_RETURN_TYPE_e DECAN_ReceiveCanCellTemperatures(void) {
    STD_RETURN_TYPE_e isSuccessful = STD_NOT_OK;

    /* Get can cell temperatures from queue */
    CAN_CAN2AFE_CELL_TEMPERATURES_QUEUE_s decan_canCellTemperaturesFromQueue = {0};
    OS_STD_RETURN_e receivedFromQueue                                        = OS_ReceiveFromQueue(
        ftsk_canToAfeCellTemperaturesQueue,
        (void *)&decan_canCellTemperaturesFromQueue,
        DECAN_CAN2AFE_QUEUE_TIMEOUT_MS);
    if (receivedFromQueue == OS_SUCCESS) {
        /* Initialize the indexes for cellTemperature_ddegC */
        uint16_t s                      = 0u;
        uint16_t m                      = 0u;
        uint16_t ts                     = 0u;
        uint16_t oneNumIdxOfTemperature = decan_canCellTemperaturesFromQueue.muxValue *
                                          CAN_NUM_OF_TEMPERATURES_IN_CAN_CELL_TEMPERATURES_MSG;
        uint16_t upperBoundOneNumIdxOfTemperature =
            BS_NR_OF_STRINGS * BS_NR_OF_MODULES_PER_STRING * BS_NR_OF_TEMP_SENSORS_PER_MODULE - 1;

        /* Loop through all temperatures in the received can message */
        for (uint16_t i = 0u; i < CAN_NUM_OF_TEMPERATURES_IN_CAN_CELL_TEMPERATURES_MSG; i++) {
            /* Check if the one number index of temperature surpass its upper bound */
            if (oneNumIdxOfTemperature > upperBoundOneNumIdxOfTemperature) {
                break;
            }

            /* Convert the one number index of temperature into s, m, ts */
            DECAN_ConvertIndexForTemperature(&s, &m, &ts, oneNumIdxOfTemperature);

            /* Write the received data */
            decan_cellTemperature.cellTemperature_ddegC[s][m][ts] =
                decan_canCellTemperaturesFromQueue.cellTemperature[i] * 10;
            if (decan_canCellTemperaturesFromQueue.invalidFlag[i] == DECAN_DATA_IS_VALID) {
                decan_cellTemperature.invalidCellTemperature[s][m][ts] = false;
            } else {
                decan_cellTemperature.invalidCellTemperature[s][m][ts] = true;
            }

            /* Update one number index */
            oneNumIdxOfTemperature++;
        }

        /* Write this part of cell temperatures and invalid flags to database,
        read the total cell temperatures and invalid flags, count the number of
        invalid cell temperatures and then write them back to database */
        if ((DATA_WRITE_DATA(&decan_cellTemperature) == STD_OK) &&
            (DATA_READ_DATA(&decan_cellTemperatureFromRead) == STD_OK)) {
            for (uint16_t idxString = 0u; idxString < BS_NR_OF_STRINGS; idxString++) {
                uint16_t nrValidCellTemperaturesPerString = 0u;
                for (uint16_t idxModule = 0u; idxModule < BS_NR_OF_MODULES_PER_STRING; idxModule++) {
                    for (uint16_t idxTemperatureSensor = 0u; idxTemperatureSensor < BS_NR_OF_TEMP_SENSORS_PER_MODULE;
                         idxTemperatureSensor++) {
                        if (decan_cellTemperatureFromRead
                                .invalidCellTemperature[idxString][idxModule][idxTemperatureSensor] == false) {
                            nrValidCellTemperaturesPerString++;
                        }
                    }
                }
                decan_cellTemperatureFromRead.nrValidTemperatures[idxString] = nrValidCellTemperaturesPerString;
            }
            isSuccessful = DATA_WRITE_DATA(&decan_cellTemperatureFromRead);
        }
    }

    return isSuccessful;
}

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e DECAN_Initialize(void) {
    /* Initialize entry of cell voltage and cell temperature in database */
    for (uint16_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint16_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                decan_cellVoltage.cellVoltage_mV[s][m][cb]     = 0;
                decan_cellVoltage.invalidCellVoltage[s][m][cb] = true;
            }
        }
    }
    for (uint16_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        for (uint16_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint16_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                decan_cellTemperature.cellTemperature_ddegC[s][m][ts]  = 0;
                decan_cellTemperature.invalidCellTemperature[s][m][ts] = true;
            }
        }
    }
    /* Give other tasks time to execute */
    uint32_t current_time = OS_GetTickCount();
    OS_DelayTaskUntil(&current_time, 10u);
    return STD_OK;
}

extern STD_RETURN_TYPE_e DECAN_TriggerAfe(void) {
    STD_RETURN_TYPE_e isSuccessful                            = STD_NOT_OK;
    STD_RETURN_TYPE_e isSuccessful_receiveCanCellVoltages     = STD_NOT_OK;
    STD_RETURN_TYPE_e isSuccessful_receiveCanCellTemperatures = STD_NOT_OK;

    isSuccessful_receiveCanCellVoltages     = DECAN_ReceiveCanCellVoltages();
    isSuccessful_receiveCanCellTemperatures = DECAN_ReceiveCanCellTemperatures();
    if ((isSuccessful_receiveCanCellVoltages == STD_OK) && (isSuccessful_receiveCanCellTemperatures == STD_OK)) {
        isSuccessful = STD_OK;
    }

    uint32_t currentTime = OS_GetTickCount();

    /* Block task without possibility to wake up */
    /* +1: to wait at least waitTime*/
    OS_DelayTaskUntil(&currentTime, 10u);
    return isSuccessful;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern uint16_t TEST_DECAN_ModifiedModuloFunction(uint16_t a, uint16_t b) {
    return DECAN_ModifiedModuloFunction(a, b);
}
extern void TEST_DECAN_ConvertIndexForVoltage(uint16_t *s, uint16_t *m, uint16_t *cb, uint16_t oneNumIdxOfVoltage) {
    DECAN_ConvertIndexForVoltage(s, m, cb, oneNumIdxOfVoltage);
}
extern void TEST_DECAN_ConvertIndexForTemperature(
    uint16_t *s,
    uint16_t *m,
    uint16_t *ts,
    uint16_t oneNumIdxOfTemperature) {
    DECAN_ConvertIndexForTemperature(s, m, ts, oneNumIdxOfTemperature);
}
extern STD_RETURN_TYPE_e TEST_DECAN_ReceiveCanCellVoltages(void) {
    return DECAN_ReceiveCanCellVoltages();
}
extern STD_RETURN_TYPE_e TEST_DECAN_ReceiveCanCellTemperatures(void) {
    return DECAN_ReceiveCanCellTemperatures();
}
#endif
