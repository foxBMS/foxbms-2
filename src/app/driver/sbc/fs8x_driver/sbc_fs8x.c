/*
 * Copyright (c) 2016 - 2018, NXP Semiconductors, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @file sbc_fs8x.c
 * @brief Driver functions for the FS8x SBC.
 *
 * @author nxf44615
 * @version 1.1
 * @date 9-Oct-2018
 * @copyright Copyright (c) 2016 - 2018, NXP Semiconductors, Inc.
 *
 * @updated 2020-03-18 (date of last update)
 * Adapted driver to pass interface configuration as parameter into the
 * functions to enable the usage of multiple ICs in the system.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include <stddef.h>

#include "sbc_fs8x.h"
#include "sbc_fs8x_map.h"
#include "sbc_fs8x_assert.h"
#include "sbc_fs8x_communication.h"
#include "sbc_fs8x_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/** @brief Returns true if value VAL is in the range defined by MIN and MAX values
 * (range includes the border values).
 * @param val Comparison value.
 * @param min Minimal value of the range.
 * @param max Maximal value of the range.
 * @return True if value is the range. False otherwise. */
#define FS8x_IS_IN_RANGE(val, min, max)   (((val) >= (min)) && ((val) <= (max)))

/*******************************************************************************
 * Local Functions Prototypes
 ******************************************************************************/
#if (FS8x_WD_TYPE == FS8x_WD_CHALLENGER)

/* Reads challenge token (next generated LFSR state) from the SBC. */
static fs8x_status_t FS8x_WD_ReadChallengeToken(SPI_INTERFACE_CONFIG_s* spiInterface, fs8x_drv_data_t* drvData,
        uint16_t* wdSeed);

/* Compute Challenger Watchdog answer. */
static uint16_t FS8x_WD_ComputeAnswer(uint16_t token);

#endif

/*******************************************************************************
 * Local Variables
 ******************************************************************************/

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

/*******************************************************************************
 * Local Functions - Implementation
 ******************************************************************************/

#if (FS8x_WD_TYPE == FS8x_WD_CHALLENGER)
/* Reads challenge token (next generated LFSR state) from the SBC and stores it
 * into the driver data structure. */
static fs8x_status_t FS8x_WD_ReadChallengeToken(SPI_INTERFACE_CONFIG_s* spiInterface, fs8x_drv_data_t* drvData,
        uint16_t* wdSeed)
{
    fs8x_status_t status = fs8xStatusOk;    /* Status variable. */
    fs8x_rx_frame_t response;               /* Response to the command. */

    FS_ASSERT(drvData != NULL);

    status = FS8x_ReadRegister(spiInterface, drvData, true, FS8X_FS_WD_SEED_ADDR, &response);
    if (status != fs8xStatusOk)
    {
        return status;
    }

    /* Store the watchdog seed from the SBC device. */
    *wdSeed = response.readData;

    return status;
}

/* Compute Challenger Watchdog answer. */
static uint16_t FS8x_WD_ComputeAnswer(uint16_t token)
{
    uint32_t mr = token; /* Monitoring result. */

    /* Simulates ALU Checker on the MCU side. */
    mr *= 4U;
    mr += 6U;
    mr -= 4U;
    mr = ~mr;
    mr /= 4U;

    return (uint16_t)mr;
}
#endif

/*******************************************************************************
 * API - Implementation
 ******************************************************************************/
/* Changes seed of LFSR used for watchdog. */
fs8x_status_t FS8x_WD_ChangeSeed(SPI_INTERFACE_CONFIG_s* spiInterface, fs8x_drv_data_t* drvData, uint16_t wdSeed)
{
    fs8x_status_t status = fs8xStatusOk;   /* Status variable. */
    fs8x_rx_frame_t rxData;
    uint8_t registerAddress;

    FS_ASSERT(drvData != NULL);

#if (FS8x_WD_TYPE == FS8x_WD_SIMPLE)
    registerAddress = FS8X_FS_WD_ANSWER_ADDR;
#elif (FS8x_WD_TYPE == FS8x_WD_CHALLENGER)
    registerAddress = FS8X_FS_WD_SEED_ADDR;
#endif
    status = FS8x_WriteRegister(spiInterface, drvData, true, registerAddress, wdSeed);
    status |= FS8x_ReadRegister(spiInterface, drvData, true, registerAddress, &rxData);
    if (wdSeed != rxData.readData)
    {
        status = fs8xStatusError;
    }
    return status;
}

/* Performs the watchdog refresh. */
fs8x_status_t FS8x_WD_Refresh(SPI_INTERFACE_CONFIG_s* spiInterface, fs8x_drv_data_t* drvData)
{
    fs8x_status_t status = fs8xStatusError;   /* Status variable. */

    FS_ASSERT(drvData != NULL);

#if (FS8x_WD_TYPE == FS8x_WD_SIMPLE)
    status = FS8x_WriteRegister(spiInterface, drvData, true, FS8X_FS_WD_ANSWER_ADDR,
            drvData->watchdogSeed);
#elif (FS8x_WD_TYPE == FS8x_WD_CHALLENGER)
    uint16_t answer;    /* Calculated monitoring result. */

    /* Synchronize with LFSR generator on the device. */
    status = FS8x_WD_ReadChallengeToken(spiInterface, drvData, &(drvData->watchdogSeed));
    if (status != fs8xStatusOk)
    {
        return status;
    }
    answer = FS8x_WD_ComputeAnswer(drvData->watchdogSeed);
    status = FS8x_WriteRegister(spiInterface, drvData, true, FS8X_FS_WD_ANSWER_ADDR, answer);
#endif
    return status; /* Return error if no WD is selected. */
}

fs8x_status_t FS8x_FS0B_Release(SPI_INTERFACE_CONFIG_s* spiInterface, fs8x_drv_data_t* drvData)
{
    fs8x_status_t status;       /* Status variable. */
    fs8x_rx_frame_t rxData;     /* Response to the command. */
    uint16_t fs0b_write = 0;    /* Value to be written to the FS_RELEASE_FS0B register. */
    uint8_t i = 0;              /* Counter. */
    uint16_t diagSafetyMask;    /* Mask for reading of value from the FS_DIAG_SAFETY reg. */
    uint16_t diagSafetyValue;   /* FS_DIAG_SAFETY bits defined by the former mask should have this value. */
    uint8_t errorCounter;       /* Fault Error Counter value. */

    FS_ASSERT(drvData != NULL);

    /* Preconditions check */
    /* Fault Error Counter = 0 */
    status = FS8x_GetFaultErrorCounterValue(spiInterface, drvData, &errorCounter);
    if (status != fs8xStatusOk)
    {
        return status;
    }
    if (errorCounter > 0)
    {
        return fs8xStatusError;
    }
    /* LBIST_OK = ABIST1_OK = ABIST2_OK = 1 */
    status = FS8x_ReadRegister(spiInterface, drvData, true, FS8X_FS_DIAG_SAFETY_ADDR, &rxData);
    if (status != fs8xStatusOk)
    {
        return status;
    }
    diagSafetyMask = (FS8X_FS_LBIST_OK_MASK | FS8X_FS_ABIST2_OK_MASK | FS8X_FS_ABIST1_OK_MASK);
    diagSafetyValue = (FS8X_FS_LBIST_OK_PASS | FS8X_FS_ABIST2_OK_PASS | FS8X_FS_ABIST1_OK_PASS);
    if ((rxData.readData & diagSafetyMask) != diagSafetyValue)
    {
        return fs8xStatusError;
    }
    /* RELEASE_FS0B register filled with ongoing WD_SEED reversed and complemented */
    status = FS8x_ReadRegister(spiInterface, drvData, true, FS8X_FS_WD_SEED_ADDR, &rxData);
    if (status != fs8xStatusOk)
    {
        return status;
    }

    for (i = 0; i < 16; i++)
    {
        fs0b_write |= (uint16_t)(((rxData.readData >> (15 - i)) & 1U) << i);
    }
    fs0b_write = ~fs0b_write;
    return FS8x_WriteRegister(spiInterface, drvData, true, FS8X_FS_RELEASE_FS0B_ADDR, fs0b_write);
}

/* Switches a desired channel to the AMUX pin. */
fs8x_status_t FS8x_SwitchAMUXchannel(SPI_INTERFACE_CONFIG_s* spiInterface, fs8x_drv_data_t* drvData, fs8x_amux_selection_t channelSelection)
{
    FS_ASSERT(drvData != NULL);

    /* Check of AMUX channel selection range. */
#pragma diag_push
#pragma diag_suppress 188
    if ((!FS8x_IS_IN_RANGE(channelSelection, fs8xAmuxGnd, fs8xAmuxPsync) &&
         !FS8x_IS_IN_RANGE(channelSelection, fs8xAmuxVsup1VoltDiv14, fs8xAmuxWake2VoltDiv14)) ||
        channelSelection == FS8X_M_AMUX_RESERVED)
#pragma diag_pop
    {
        return fs8xStatusError;
    }
    return FS8x_WriteRegister(spiInterface, drvData, false, FS8X_M_AMUX_ADDR, (uint16_t)channelSelection);
}

/* Sets state (enable/disable) of the selected voltage regulator. */
fs8x_status_t FS8x_SetRegulatorState(SPI_INTERFACE_CONFIG_s* spiInterface, fs8x_drv_data_t* drvData,
        fs8x_reg_output_t vreg, bool enable)
{
    uint16_t shift = (uint16_t)vreg;    /* Shift of the mask for disable value. */

    FS_ASSERT(drvData != NULL);

    if (!enable)
    {
        shift += 8; /* Disable value is shifted 8 bits left. */
    }

    /* Register read is not necessary here as a bit value of 0 is ignored anyway. */
    return FS8x_WriteRegister(spiInterface, drvData, false, FS8X_M_REG_CTRL1_ADDR, (uint16_t)(1U << shift));
}

/* Reads actual Fault Error Counter value. */
fs8x_status_t FS8x_GetFaultErrorCounterValue(SPI_INTERFACE_CONFIG_s* spiInterface, fs8x_drv_data_t* drvData,
        uint8_t* faultErrorCounterValue)
{
    fs8x_status_t status;   /* Status value. */
    fs8x_rx_frame_t rxData; /* Register received data. */

    FS_ASSERT(drvData != NULL);
    FS_ASSERT(faultErrorCounterValue != NULL);

    status = FS8x_ReadRegister(spiInterface, drvData, true, FS8X_FS_I_FSSM_ADDR, &rxData);
    if (status != fs8xStatusOk)
    {
        return status;
    }

    *faultErrorCounterValue = FS8x_BO_GET_REG_VALUE(rxData.readData, FS8X_FS_I_FLT_ERR_CNT_MASK,
            FS8X_FS_I_FLT_ERR_CNT_SHIFT);

    return fs8xStatusOk;
}
