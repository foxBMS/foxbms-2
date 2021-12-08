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

/** @file sbc_fs8x_map.h
 * @brief FS8x register map.
 *
 * This header file contains addresses, masks, shifts and shifted discreet values
 * for runtime registers of the FS8x. Note that some registers have the same address;
 * the reason is that there are 2 register groups: main and fail-safe.
 * See data sheet for details.
 *
 * @author nxf44615
 * @version 1.1
 * @date 9-Oct-2018
 * @copyright Copyright (c) 2016 - 2018, NXP Semiconductors, Inc.
 *
 * @warning Some macro names are not compliant with MISRA rule 5.4 if C90 standard is used
 *          (the first 31 characters of macro identifiers are significant).
 *          If C99 standard is used, there is no issue, as the first 63 characters of macro
 *          identifiers are significant.
 */

#ifndef SBC_FS8X_MAP_H__
#define SBC_FS8X_MAP_H__

/******************************************************************************/
/* M_FLAG - Type: RW                                                          */
/******************************************************************************/

#define FS8X_M_FLAG_ADDR    0x00U
#define FS8X_M_FLAG_DEFAULT 0x0000U

/**
 * Invalid Main Domain I2C access (Wrong Write or Read, Write to INIT registers in normal mode, wrong address).
 */
#define FS8X_M_I2C_M_REQ_MASK 0x0001U
/**
 * Main Domain I2C communication CRC issue.
 */
#define FS8X_M_I2C_M_CRC_MASK 0x0002U
/**
 * Main Domain SPI communication CRC issue
 */
#define FS8X_M_SPI_M_CRC_MASK 0x0004U
/**
 * Invalid Main domain SPI access (Wrong Write or Read, Write to INIT registers in normal mode, wrong address).
 */
#define FS8X_M_SPI_M_REQ_MASK 0x0008U
/**
 * Main Domain SPI SCLK error detection.
 */
#define FS8X_M_SPI_M_CLK_MASK 0x0010U
/**
 * Report an event on LDO2 (status change or failure)
 */
#define FS8X_M_VLDO2_G_MASK 0x0080U
/**
 * Report an event on LDO1 (status change or failure)
 */
#define FS8X_M_VLDO1_G_MASK 0x0100U
/**
 * Report an event on BUCK3 (status change or failure)
 */
#define FS8X_M_VBUCK3_G_MASK 0x0200U
/**
 * Report an event on BUCK2 (status change or failure)
 */
#define FS8X_M_VBUCK2_G_MASK 0x0400U
/**
 * Report an event on BUCK1 (status change or failure)
 */
#define FS8X_M_VBUCK1_G_MASK 0x0800U
/**
 * Report an event on VBOOST (status change or failure)
 */
#define FS8X_M_VBOOST_G_MASK 0x1000U
/**
 * Report an event on VPRE (status change or failure)
 */
#define FS8X_M_VPRE_G_MASK 0x2000U
/**
 * Report a wake-up event. Logical OR of WAKE1 and WAKE2 source
 */
#define FS8X_M_WU_G_MASK 0x4000U
/**
 * Report an error in the Communication (SPI or I2C)
 */
#define FS8X_M_COM_ERR_MASK 0x8000U

/**
 * Invalid Main Domain I2C access (Wrong Write or Read, Write to INIT registers in normal mode, wrong address).
 */
#define FS8X_M_I2C_M_REQ_SHIFT 0x0000U
/**
 * Main Domain I2C communication CRC issue.
 */
#define FS8X_M_I2C_M_CRC_SHIFT 0x0001U
/**
 * Main Domain SPI communication CRC issue
 */
#define FS8X_M_SPI_M_CRC_SHIFT 0x0002U
/**
 * Invalid Main domain SPI access (Wrong Write or Read, Write to INIT registers in normal mode, wrong address).
 */
#define FS8X_M_SPI_M_REQ_SHIFT 0x0003U
/**
 * Main Domain SPI SCLK error detection.
 */
#define FS8X_M_SPI_M_CLK_SHIFT 0x0004U
/**
 * Report an event on LDO2 (status change or failure)
 */
#define FS8X_M_VLDO2_G_SHIFT 0x0007U
/**
 * Report an event on LDO1 (status change or failure)
 */
#define FS8X_M_VLDO1_G_SHIFT 0x0008U
/**
 * Report an event on BUCK3 (status change or failure)
 */
#define FS8X_M_VBUCK3_G_SHIFT 0x0009U
/**
 * Report an event on BUCK2 (status change or failure)
 */
#define FS8X_M_VBUCK2_G_SHIFT 0x000AU
/**
 * Report an event on BUCK1 (status change or failure)
 */
#define FS8X_M_VBUCK1_G_SHIFT 0x000BU
/**
 * Report an event on VBOOST (status change or failure)
 */
#define FS8X_M_VBOOST_G_SHIFT 0x000CU
/**
 * Report an event on VPRE (status change or failure)
 */
#define FS8X_M_VPRE_G_SHIFT 0x000DU
/**
 * Report a wake-up event. Logical OR of WAKE1 and WAKE2 source
 */
#define FS8X_M_WU_G_SHIFT 0x000EU
/**
 * Report an error in the Communication (SPI or I2C)
 */
#define FS8X_M_COM_ERR_SHIFT 0x000FU

/**
 * No error
 */
#define FS8X_M_I2C_M_REQ_NO_ERROR (0x0000U << FS8X_M_I2C_M_REQ_SHIFT)
/**
 * I2C Violation
 */
#define FS8X_M_I2C_M_REQ_I2C_V (0x0001U << FS8X_M_I2C_M_REQ_SHIFT)

/**
 * No error
 */
#define FS8X_M_I2C_M_CRC_NO_ERROR (0x0000U << FS8X_M_I2C_M_CRC_SHIFT)
/**
 * Error detected in the I2C CRC
 */
#define FS8X_M_I2C_M_CRC_ERROR (0x0001U << FS8X_M_I2C_M_CRC_SHIFT)

/**
 * No error
 */
#define FS8X_M_SPI_M_CRC_NO_ERROR (0x0000U << FS8X_M_SPI_M_CRC_SHIFT)
/**
 * Error detected in the SPI CRC
 */
#define FS8X_M_SPI_M_CRC_ERROR (0x0001U << FS8X_M_SPI_M_CRC_SHIFT)

/**
 * No error
 */
#define FS8X_M_SPI_M_REQ_NO_ERROR (0x0000U << FS8X_M_SPI_M_REQ_SHIFT)
/**
 * SPI Violation
 */
#define FS8X_M_SPI_M_REQ_SPI_V (0x0001U << FS8X_M_SPI_M_REQ_SHIFT)

/**
 * No error
 */
#define FS8X_M_SPI_M_CLK_NO_ERROR (0x0000U << FS8X_M_SPI_M_CLK_SHIFT)
/**
 * Wrong number of clock cycles (<32 or >32)
 */
#define FS8X_M_SPI_M_CLK_WRONG (0x0001U << FS8X_M_SPI_M_CLK_SHIFT)

/**
 * No event
 */
#define FS8X_M_VLDO2_G_NO_EVENT (0x0000U << FS8X_M_VLDO2_G_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_VLDO2_G_EVENT_OCCURRED (0x0001U << FS8X_M_VLDO2_G_SHIFT)

/**
 * No event
 */
#define FS8X_M_VLDO1_G_NO_EVENT (0x0000U << FS8X_M_VLDO1_G_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_VLDO1_G_EVENT_OCCURRED (0x0001U << FS8X_M_VLDO1_G_SHIFT)

/**
 * No event
 */
#define FS8X_M_VBUCK3_G_NO_EVENT (0x0000U << FS8X_M_VBUCK3_G_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_VBUCK3_G_EVENT_OCCURRED (0x0001U << FS8X_M_VBUCK3_G_SHIFT)

/**
 * No event
 */
#define FS8X_M_VBUCK2_G_NO_EVENT (0x0000U << FS8X_M_VBUCK2_G_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_VBUCK2_G_EVENT_OCCURRED (0x0001U << FS8X_M_VBUCK2_G_SHIFT)

/**
 * No event
 */
#define FS8X_M_VBUCK1_G_NO_EVENT (0x0000U << FS8X_M_VBUCK1_G_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_VBUCK1_G_EVENT_OCCURRED (0x0001U << FS8X_M_VBUCK1_G_SHIFT)

/**
 * No event
 */
#define FS8X_M_VBOOST_G_NO_EVENT (0x0000U << FS8X_M_VBOOST_G_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_VBOOST_G_EVENT_OCCURRED (0x0001U << FS8X_M_VBOOST_G_SHIFT)

/**
 * No event
 */
#define FS8X_M_VPRE_G_NO_EVENT (0x0000U << FS8X_M_VPRE_G_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_VPRE_G_EVENT_OCCURRED (0x0001U << FS8X_M_VPRE_G_SHIFT)

/**
 * No Wake event
 */
#define FS8X_M_WU_G_NO_WAKE_EVENT (0x0000U << FS8X_M_WU_G_SHIFT)
/**
 * Wake event
 */
#define FS8X_M_WU_G_WAKE_EVENT (0x0001U << FS8X_M_WU_G_SHIFT)

/**
 * No Failure
 */
#define FS8X_M_COM_ERR_NO_FAILURE (0x0000U << FS8X_M_COM_ERR_SHIFT)
/**
 * Failure
 */
#define FS8X_M_COM_ERR_FAILURE (0x0001U << FS8X_M_COM_ERR_SHIFT)

/******************************************************************************/
/* M_MODE - Type: RW                                                          */
/******************************************************************************/

#define FS8X_M_MODE_ADDR    0x01U
#define FS8X_M_MODE_DEFAULT 0x0000U

/**
 * Entry in Standby mode
 */
#define FS8X_M_GOTOSTBY_MASK 0x0001U
/**
 * WAKE1 wake up disable
 */
#define FS8X_M_W1DIS_MASK 0x0002U
/**
 * WAKE2 wake up disable
 */
#define FS8X_M_W2DIS_MASK 0x0004U
/**
 * State machine Normal mode
 */
#define FS8X_M_MAIN_NORMAL_MASK 0x0020U
/**
 * EXT FIN selection disable at PLL input request
 */
#define FS8X_M_EXT_FIN_DIS_MASK 0x0040U
/**
 * Real time status of FIN clock selection
 */
#define FS8X_M_EXT_FIN_SEL_RT_MASK 0x0080U
/**
 * Real time status of PLL lock flag
 */
#define FS8X_M_PLL_LOCK_RT_MASK 0x0100U

/**
 * Entry in Standby mode
 */
#define FS8X_M_GOTOSTBY_SHIFT 0x0000U
/**
 * WAKE1 wake up disable
 */
#define FS8X_M_W1DIS_SHIFT 0x0001U
/**
 * WAKE2 wake up disable
 */
#define FS8X_M_W2DIS_SHIFT 0x0002U
/**
 * State machine Normal mode
 */
#define FS8X_M_MAIN_NORMAL_SHIFT 0x0005U
/**
 * EXT FIN selection disable at PLL input request
 */
#define FS8X_M_EXT_FIN_DIS_SHIFT 0x0006U
/**
 * Real time status of FIN clock selection
 */
#define FS8X_M_EXT_FIN_SEL_RT_SHIFT 0x0007U
/**
 * Real time status of PLL lock flag
 */
#define FS8X_M_PLL_LOCK_RT_SHIFT 0x0008U

/**
 * Device remains in current state
 */
#define FS8X_M_GOTOSTBY_REMAINS_IN_CURRENT_STATE (0x0000U << FS8X_M_GOTOSTBY_SHIFT)
/**
 * Device will enter in Standby mode
 */
#define FS8X_M_GOTOSTBY_ENTER_IN_STANDBY_MODE (0x0001U << FS8X_M_GOTOSTBY_SHIFT)

/**
 * wake up enable
 */
#define FS8X_M_W1DIS_WAKE_UP_ENABLE (0x0000U << FS8X_M_W1DIS_SHIFT)
/**
 * wake up disable
 */
#define FS8X_M_W1DIS_WAKE_UP_DISABLE (0x0001U << FS8X_M_W1DIS_SHIFT)

/**
 * wake up enable
 */
#define FS8X_M_W2DIS_WAKE_UP_ENABLE (0x0000U << FS8X_M_W2DIS_SHIFT)
/**
 * wake up disable
 */
#define FS8X_M_W2DIS_WAKE_UP_DISABLE (0x0001U << FS8X_M_W2DIS_SHIFT)

/**
 * State Machine NOT in Normal mode
 */
#define FS8X_M_MAIN_NORMAL_NOT_IN_NORMAL_MODE (0x0000U << FS8X_M_MAIN_NORMAL_SHIFT)
/**
 * State Machine in Normal mode - Entry in Standby mode possible
 */
#define FS8X_M_MAIN_NORMAL_IN_NORMAL_MODE (0x0001U << FS8X_M_MAIN_NORMAL_SHIFT)

/**
 * No effect
 */
#define FS8X_M_EXT_FIN_DIS_NO_EFFECT (0x0000U << FS8X_M_EXT_FIN_DIS_SHIFT)
/**
 * Disable FIN selection
 */
#define FS8X_M_EXT_FIN_DIS_DISABLE_FIN (0x0001U << FS8X_M_EXT_FIN_DIS_SHIFT)

/**
 * Internal clock oscillator is selected
 */
#define FS8X_M_EXT_FIN_SEL_RT_INT_OSC (0x0000U << FS8X_M_EXT_FIN_SEL_RT_SHIFT)
/**
 * External FIN clock is selected
 */
#define FS8X_M_EXT_FIN_SEL_RT_EXT_OSC (0x0001U << FS8X_M_EXT_FIN_SEL_RT_SHIFT)

/**
 * PLL not locked
 */
#define FS8X_M_PLL_LOCK_RT_UNLOCKED (0x0000U << FS8X_M_PLL_LOCK_RT_SHIFT)
/**
 * PLL locked
 */
#define FS8X_M_PLL_LOCK_RT_LOCKED (0x0001U << FS8X_M_PLL_LOCK_RT_SHIFT)

/******************************************************************************/
/* M_REG_CTRL1 - Type: RW                                                     */
/******************************************************************************/

#define FS8X_M_REG_CTRL1_ADDR    0x02U
#define FS8X_M_REG_CTRL1_DEFAULT 0x0000U

/**
 * Enable request of LDO2
 */
#define FS8X_M_LDO2EN_MASK 0x0001U
/**
 * Enable request of LDO1
 */
#define FS8X_M_LDO1EN_MASK 0x0002U
/**
 * Enable request of BUCK3
 */
#define FS8X_M_BUCK3EN_MASK 0x0004U
/**
 * Enable request of BUCK2
 */
#define FS8X_M_BUCK2EN_MASK 0x0008U
/**
 * Enable request of BUCK1
 */
#define FS8X_M_BUCK1EN_MASK 0x0010U
/**
 * Enable request of BOOST
 */
#define FS8X_M_BOOSTEN_MASK 0x0020U
/**
 * Enable request of VPRE
 */
#define FS8X_M_VPEN_MASK 0x0040U
/**
 * Disable request of LDO2
 */
#define FS8X_M_LDO2DIS_MASK 0x0100U
/**
 * Disable request of LDO1
 */
#define FS8X_M_LDO1DIS_MASK 0x0200U
/**
 * Disable request of BUCK3
 */
#define FS8X_M_BUCK3DIS_MASK 0x0400U
/**
 * Disable request of BUCK2
 */
#define FS8X_M_BUCK2DIS_MASK 0x0800U
/**
 * Disable request of BUCK1
 */
#define FS8X_M_BUCK1DIS_MASK 0x1000U
/**
 * Disable request of BOOST
 */
#define FS8X_M_BOOSTDIS_MASK 0x2000U
/**
 * Disable request of VPRE
 */
#define FS8X_M_VPDIS_MASK 0x4000U
/**
 * Force disable of VPRE pull down.
 */
#define FS8X_M_VPRE_PD_DIS_MASK 0x8000U

/**
 * Enable request of LDO2
 */
#define FS8X_M_LDO2EN_SHIFT 0x0000U
/**
 * Enable request of LDO1
 */
#define FS8X_M_LDO1EN_SHIFT 0x0001U
/**
 * Enable request of BUCK3
 */
#define FS8X_M_BUCK3EN_SHIFT 0x0002U
/**
 * Enable request of BUCK2
 */
#define FS8X_M_BUCK2EN_SHIFT 0x0003U
/**
 * Enable request of BUCK1
 */
#define FS8X_M_BUCK1EN_SHIFT 0x0004U
/**
 * Enable request of BOOST
 */
#define FS8X_M_BOOSTEN_SHIFT 0x0005U
/**
 * Enable request of VPRE
 */
#define FS8X_M_VPEN_SHIFT 0x0006U
/**
 * Disable request of LDO2
 */
#define FS8X_M_LDO2DIS_SHIFT 0x0008U
/**
 * Disable request of LDO1
 */
#define FS8X_M_LDO1DIS_SHIFT 0x0009U
/**
 * Disable request of BUCK3
 */
#define FS8X_M_BUCK3DIS_SHIFT 0x000AU
/**
 * Disable request of BUCK2
 */
#define FS8X_M_BUCK2DIS_SHIFT 0x000BU
/**
 * Disable request of BUCK1
 */
#define FS8X_M_BUCK1DIS_SHIFT 0x000CU
/**
 * Disable request of BOOST
 */
#define FS8X_M_BOOSTDIS_SHIFT 0x000DU
/**
 * Disable request of VPRE
 */
#define FS8X_M_VPDIS_SHIFT 0x000EU
/**
 * Force disable of VPRE pull down.
 */
#define FS8X_M_VPRE_PD_DIS_SHIFT 0x000FU

/**
 * no effect (regulator remains in existing state - if ON it remains ON, if OFF it remains OFF)
 */
#define FS8X_M_LDO2EN_NO_EFFECT (0x0000U << FS8X_M_LDO2EN_SHIFT)
/**
 * LDO2 Enable Request
 */
#define FS8X_M_LDO2EN_LDO2_ENABLE_REQUEST (0x0001U << FS8X_M_LDO2EN_SHIFT)

/**
 * no effect (regulator remains in existing state)
 */
#define FS8X_M_LDO1EN_NO_EFFECT (0x0000U << FS8X_M_LDO1EN_SHIFT)
/**
 * LDO1 Enable Request
 */
#define FS8X_M_LDO1EN_LDO1_ENABLE_REQUEST (0x0001U << FS8X_M_LDO1EN_SHIFT)

/**
 * no effect (regulator remains in existing state)
 */
#define FS8X_M_BUCK3EN_NO_EFFECT (0x0000U << FS8X_M_BUCK3EN_SHIFT)
/**
 * BUCK3 Enable Request
 */
#define FS8X_M_BUCK3EN_BUCK3_ENABLE_REQUEST (0x0001U << FS8X_M_BUCK3EN_SHIFT)

/**
 * no effect (regulator remains in existing state)
 */
#define FS8X_M_BUCK2EN_NO_EFFECT (0x0000U << FS8X_M_BUCK2EN_SHIFT)
/**
 * BUCK2 Enable Request
 */
#define FS8X_M_BUCK2EN_BUCK2_ENABLE_REQUEST (0x0001U << FS8X_M_BUCK2EN_SHIFT)

/**
 * no effect (regulator remains in existing state)
 */
#define FS8X_M_BUCK1EN_NO_EFFECT (0x0000U << FS8X_M_BUCK1EN_SHIFT)
/**
 * BUCK1 Enable Request
 */
#define FS8X_M_BUCK1EN_BUCK1_ENABLE_REQUEST (0x0001U << FS8X_M_BUCK1EN_SHIFT)

/**
 * no effect (regulator remains in existing state)
 */
#define FS8X_M_BOOSTEN_NO_EFFECT (0x0000U << FS8X_M_BOOSTEN_SHIFT)
/**
 * BOOST Enable Request
 */
#define FS8X_M_BOOSTEN_BOOST_ENABLE_REQUEST (0x0001U << FS8X_M_BOOSTEN_SHIFT)

/**
 * no effect (regulator remains in existing state)
 */
#define FS8X_M_VPEN_NO_EFFECT (0x0000U << FS8X_M_VPEN_SHIFT)
/**
 * VPRE Enable Request
 */
#define FS8X_M_VPEN_VPRE_ENABLE_REQUEST (0x0001U << FS8X_M_VPEN_SHIFT)

/**
 * no effect (regulator remains in existing state)
 */
#define FS8X_M_LDO2DIS_NO_EFFECT (0x0000U << FS8X_M_LDO2DIS_SHIFT)
/**
 * LDO2 Disable Request
 */
#define FS8X_M_LDO2DIS_LDO2_DISABLE_REQUEST (0x0001U << FS8X_M_LDO2DIS_SHIFT)

/**
 * no effect (regulator remains in existing state)
 */
#define FS8X_M_LDO1DIS_NO_EFFECT (0x0000U << FS8X_M_LDO1DIS_SHIFT)
/**
 * LDO1 Disable Request
 */
#define FS8X_M_LDO1DIS_LDO1_DISABLE_REQUEST (0x0001U << FS8X_M_LDO1DIS_SHIFT)

/**
 * no effect (regulator remains in existing state)
 */
#define FS8X_M_BUCK3DIS_NO_EFFECT (0x0000U << FS8X_M_BUCK3DIS_SHIFT)
/**
 * BUCK3 Disable Request
 */
#define FS8X_M_BUCK3DIS_BUCK3_DISABLE_REQUEST (0x0001U << FS8X_M_BUCK3DIS_SHIFT)

/**
 * no effect (regulator remains in existing state)
 */
#define FS8X_M_BUCK2DIS_NO_EFFECT (0x0000U << FS8X_M_BUCK2DIS_SHIFT)
/**
 * BUCK2 Disable Request
 */
#define FS8X_M_BUCK2DIS_BUCK2_DISABLE_REQUEST (0x0001U << FS8X_M_BUCK2DIS_SHIFT)

/**
 * no effect (regulator remains in existing state)
 */
#define FS8X_M_BUCK1DIS_NO_EFFECT (0x0000U << FS8X_M_BUCK1DIS_SHIFT)
/**
 * BUCK1 Disable Request
 */
#define FS8X_M_BUCK1DIS_BUCK1_DISABLE_REQUEST (0x0001U << FS8X_M_BUCK1DIS_SHIFT)

/**
 * no effect (regulator remains in existing state)
 */
#define FS8X_M_BOOSTDIS_NO_EFFECT (0x0000U << FS8X_M_BOOSTDIS_SHIFT)
/**
 * BOOST Disable Request
 */
#define FS8X_M_BOOSTDIS_BOOST_DISABLE_REQUEST (0x0001U << FS8X_M_BOOSTDIS_SHIFT)

/**
 * no effect (regulator remains in existing state)
 */
#define FS8X_M_VPDIS_NO_EFFECT (0x0000U << FS8X_M_VPDIS_SHIFT)
/**
 * VPRE Disable Request
 */
#define FS8X_M_VPDIS_VPRE_DISABLE_REQUEST (0x0001U << FS8X_M_VPDIS_SHIFT)

/**
 * no effect (VPRE pull down is automatically controlled by the logic)
 */
#define FS8X_M_VPRE_PD_DIS_NO_EFFECT (0x0000U << FS8X_M_VPRE_PD_DIS_SHIFT)
/**
 * VPRE pull down disable Request
 */
#define FS8X_M_VPRE_PD_DIS_VPRE_DISABLE_REQUEST (0x0001U << FS8X_M_VPRE_PD_DIS_SHIFT)

/******************************************************************************/
/* M_REG_CTRL2 - Type: RW                                                     */
/******************************************************************************/

#define FS8X_M_REG_CTRL2_ADDR    0x03U
#define FS8X_M_REG_CTRL2_DEFAULT 0x0009U

/**
 * VPRE High Side slew rate control
 */
#define FS8X_M_VPRESRHS_MASK 0x0003U
/**
 * VPRE Low Side slew rate control
 */
#define FS8X_M_VPRESRLS_MASK 0x0018U
/**
 * Regulator behavior in case of TSD
 */
#define FS8X_M_LDO2TSDCFG_MASK 0x0100U
/**
 * Regulator behavior in case of TSD
 */
#define FS8X_M_LDO1TSDCFG_MASK 0x0200U
/**
 * Regulator behavior in case of TSD
 */
#define FS8X_M_BUCK3TSDCFG_MASK 0x0400U
/**
 * Regulator behavior in case of TSD
 */
#define FS8X_M_BUCK2TSDCFG_MASK 0x0800U
/**
 * Regulator behavior in case of TSD
 */
#define FS8X_M_BUCK1TSDCFG_MASK 0x1000U
/**
 * Regulator behavior in case of TSD
 */
#define FS8X_M_BOOSTTSDCFG_MASK 0x2000U
/**
 * VBOOST Low Side slew rate control
 */
#define FS8X_M_VBSTSR_MASK 0xC000U

/**
 * VPRE High Side slew rate control
 */
#define FS8X_M_VPRESRHS_SHIFT 0x0000U
/**
 * VPRE Low Side slew rate control
 */
#define FS8X_M_VPRESRLS_SHIFT 0x0003U
/**
 * Regulator behavior in case of TSD
 */
#define FS8X_M_LDO2TSDCFG_SHIFT 0x0008U
/**
 * Regulator behavior in case of TSD
 */
#define FS8X_M_LDO1TSDCFG_SHIFT 0x0009U
/**
 * Regulator behavior in case of TSD
 */
#define FS8X_M_BUCK3TSDCFG_SHIFT 0x000AU
/**
 * Regulator behavior in case of TSD
 */
#define FS8X_M_BUCK2TSDCFG_SHIFT 0x000BU
/**
 * Regulator behavior in case of TSD
 */
#define FS8X_M_BUCK1TSDCFG_SHIFT 0x000CU
/**
 * Regulator behavior in case of TSD
 */
#define FS8X_M_BOOSTTSDCFG_SHIFT 0x000DU
/**
 * VBOOST Low Side slew rate control
 */
#define FS8X_M_VBSTSR_SHIFT 0x000EU

/**
 * 130mA typical drive capability - slow
 */
#define FS8X_M_VPRESRHS_130MA_DRIVE_CAPABILITY (0x0000U << FS8X_M_VPRESRHS_SHIFT)
/**
 * 260mA typical drive capability - medium
 */
#define FS8X_M_VPRESRHS_260MA_DRIVE_CAPABILITY (0x0001U << FS8X_M_VPRESRHS_SHIFT)
/**
 * 520mA typical drive capability - fast
 */
#define FS8X_M_VPRESRHS_520MA_DRIVE_CAPABILITY (0x0002U << FS8X_M_VPRESRHS_SHIFT)
/**
 * 900mA typical drive capability - ultra fast
 */
#define FS8X_M_VPRESRHS_900MA_DRIVE_CAPABILITY (0x0003U << FS8X_M_VPRESRHS_SHIFT)

/**
 * 130mA typical drive capability - slow
 */
#define FS8X_M_VPRESRLS_130MA_DRIVE_CAPABILITY (0x0000U << FS8X_M_VPRESRLS_SHIFT)
/**
 * 260mA typical drive capability - medium
 */
#define FS8X_M_VPRESRLS_260MA_DRIVE_CAPABILITY (0x0001U << FS8X_M_VPRESRLS_SHIFT)
/**
 * 520mA typical drive capability - fast
 */
#define FS8X_M_VPRESRLS_520MA_DRIVE_CAPABILITY (0x0002U << FS8X_M_VPRESRLS_SHIFT)
/**
 * 900mA typical drive capability - ultra fast
 */
#define FS8X_M_VPRESRLS_900MA_DRIVE_CAPABILITY (0x0003U << FS8X_M_VPRESRLS_SHIFT)

/**
 * Regulator Shutdown
 */
#define FS8X_M_LDO2TSDCFG_REGULATOR_SHUTDOWN (0x0000U << FS8X_M_LDO2TSDCFG_SHIFT)
/**
 * Regulator Shutdown + State machine transition to Standby mode
 */
#define FS8X_M_LDO2TSDCFG_REGULATOR_SHUTDOWN_AND_DEEP_FAIL_SAFE (0x0001U << FS8X_M_LDO2TSDCFG_SHIFT)

/**
 * Regulator Shutdown
 */
#define FS8X_M_LDO1TSDCFG_REGULATOR_SHUTDOWN (0x0000U << FS8X_M_LDO1TSDCFG_SHIFT)
/**
 * Regulator Shutdown + State machine transition to Standby mode
 */
#define FS8X_M_LDO1TSDCFG_REGULATOR_SHUTDOWN_AND_DEEP_FAIL_SAFE (0x0001U << FS8X_M_LDO1TSDCFG_SHIFT)

/**
 * Regulator Shutdown
 */
#define FS8X_M_BUCK3TSDCFG_REGULATOR_SHUTDOWN (0x0000U << FS8X_M_BUCK3TSDCFG_SHIFT)
/**
 * Regulator Shutdown + State machine transition to Standby mode
 */
#define FS8X_M_BUCK3TSDCFG_REGULATOR_SHUTDOWN_AND_DEEP_FAIL_SAFE (0x0001U << FS8X_M_BUCK3TSDCFG_SHIFT)

/**
 * Regulator Shutdown
 */
#define FS8X_M_BUCK2TSDCFG_REGULATOR_SHUTDOWN (0x0000U << FS8X_M_BUCK2TSDCFG_SHIFT)
/**
 * Regulator Shutdown + State machine transition to Standby mode
 */
#define FS8X_M_BUCK2TSDCFG_REGULATOR_SHUTDOWN_AND_DEEP_FAIL_SAFE (0x0001U << FS8X_M_BUCK2TSDCFG_SHIFT)

/**
 * Regulator Shutdown
 */
#define FS8X_M_BUCK1TSDCFG_REGULATOR_SHUTDOWN (0x0000U << FS8X_M_BUCK1TSDCFG_SHIFT)
/**
 * Regulator Shutdown + State machine transition to Standby mode
 */
#define FS8X_M_BUCK1TSDCFG_REGULATOR_SHUTDOWN_AND_DEEP_FAIL_SAFE (0x0001U << FS8X_M_BUCK1TSDCFG_SHIFT)

/**
 * Regulator Shutdown
 */
#define FS8X_M_BOOSTTSDCFG_REGULATOR_SHUTDOWN (0x0000U << FS8X_M_BOOSTTSDCFG_SHIFT)
/**
 * Regulator Shutdown + State machine transition to Standby mode
 */
#define FS8X_M_BOOSTTSDCFG_REGULATOR_SHUTDOWN_AND_DEEP_FAIL_SAFE (0x0001U << FS8X_M_BOOSTTSDCFG_SHIFT)

/**
 * 50V/us
 */
#define FS8X_M_VBSTSR_50V_US (0x0000U << FS8X_M_VBSTSR_SHIFT)
/**
 * 100V/us
 */
#define FS8X_M_VBSTSR_100V_US (0x0001U << FS8X_M_VBSTSR_SHIFT)
/**
 * 300V/us
 */
#define FS8X_M_VBSTSR_300V_US (0x0002U << FS8X_M_VBSTSR_SHIFT)
/**
 * 500V/us
 */
#define FS8X_M_VBSTSR_500V_US (0x0003U << FS8X_M_VBSTSR_SHIFT)

/******************************************************************************/
/* M_AMUX - Type: RW                                                          */
/******************************************************************************/

#define FS8X_M_AMUX_ADDR    0x04U
#define FS8X_M_AMUX_DEFAULT 0x0000U

/**
 * Signal selection for AMUX output
 */
#define FS8X_M_AMUX_MASK 0x001FU
/**
 * Selection of divider ratio for Vsup, Wake1, Wake 2 inputs
 */
#define FS8X_M_RATIO_MASK 0x0020U

/**
 * Signal selection for AMUX output
 */
#define FS8X_M_AMUX_SHIFT 0x0000U
/**
 * Selection of divider ratio for Vsup, Wake1, Wake 2 inputs
 */
#define FS8X_M_RATIO_SHIFT 0x0005U

/**
 * GND
 */
#define FS8X_M_AMUX_GND (0x0000U << FS8X_M_AMUX_SHIFT)
/**
 * VDDIO
 */
#define FS8X_M_AMUX_VDDIO (0x0001U << FS8X_M_AMUX_SHIFT)
/**
 * Temperature Sensor : T(_C) = (VAMUX _ VTEMP25) / VTEMP_COEFF + 25
 */
#define FS8X_M_AMUX_TEMPERATURE_SENSOR (0x0002U << FS8X_M_AMUX_SHIFT)
/**
 * Band Gap Main
 */
#define FS8X_M_AMUX_BAND_GAP_MAIN (0x0003U << FS8X_M_AMUX_SHIFT)
/**
 * Band Gap Fail Safe
 */
#define FS8X_M_AMUX_BAND_GAP_FAIL_SAFE (0x0004U << FS8X_M_AMUX_SHIFT)
/**
 * VBUCK1 voltage
 */
#define FS8X_M_AMUX_VBUCK1_VOLTAGE (0x0005U << FS8X_M_AMUX_SHIFT)
/**
 * VBUCK2 voltage
 */
#define FS8X_M_AMUX_VBUCK2_VOLTAGE (0x0006U << FS8X_M_AMUX_SHIFT)
/**
 * VBUCK3 voltage divided by 2.5
 */
#define FS8X_M_AMUX_VBUCK3_VOLTAGE_DIVIDED (0x0007U << FS8X_M_AMUX_SHIFT)
/**
 * VPRE voltage divided by 2.5
 */
#define FS8X_M_AMUX_VPRE_VOLTAGE_DIVIDED (0x0008U << FS8X_M_AMUX_SHIFT)
/**
 * VBOOST Voltage divided by 2.5
 */
#define FS8X_M_AMUX_VBOOST_VOLTAGE_DIVIDED (0x0009U << FS8X_M_AMUX_SHIFT)
/**
 * VLDO1 voltage divided by 2.5
 */
#define FS8X_M_AMUX_VLDO1_VOLTAGE_DIVIDED (0x000AU << FS8X_M_AMUX_SHIFT)
/**
 * VLDO2 voltage divided by 2.5
 */
#define FS8X_M_AMUX_VLDO2_VOLTAGE_DIVIDED (0x000BU << FS8X_M_AMUX_SHIFT)
/**
 * VBOS voltage divided by 2.5
 */
#define FS8X_M_AMUX_VBOS_VOLTAGE_DIVIDED (0x000CU << FS8X_M_AMUX_SHIFT)
/**
 * RESERVED
 */
#define FS8X_M_AMUX_RESERVED (0x000DU << FS8X_M_AMUX_SHIFT)
/**
 * VSUP1 voltage divided by 7.5 or 14 (SPI/I2C configuration with bit RATIO)
 */
#define FS8X_M_AMUX_VSUP1_VOLTAGE_DIVIDED (0x000EU << FS8X_M_AMUX_SHIFT)
/**
 * WAKE1 voltage divided by 7.5 or 14 (SPI/I2C configuration with bit RATIO)
 */
#define FS8X_M_AMUX_WAKE1_VOLTAGE_DIVIDED (0x000FU << FS8X_M_AMUX_SHIFT)
/**
 * WAKE2 voltage divided by 7.5 or 14 (SPI/I2C configuration with bit RATIO)
 */
#define FS8X_M_AMUX_WAKE2_VOLTAGE_DIVIDED (0x0010U << FS8X_M_AMUX_SHIFT)
/**
 * Vana: internal Main analog voltage supp
 */
#define FS8X_M_AMUX_VANA (0x0011U << FS8X_M_AMUX_SHIFT)
/**
 * Vdig: internal Main digital voltage suppl
 */
#define FS8X_M_AMUX_VDIG (0x0012U << FS8X_M_AMUX_SHIFT)
/**
 * Vdig_fs: internal Fail Safe digital voltage supp
 */
#define FS8X_M_AMUX_VDIG_FS (0x0013U << FS8X_M_AMUX_SHIFT)
/**
 * PSYNC voltage
 */
#define FS8X_M_AMUX_PSYNC_VOLTAGE (0x0014U << FS8X_M_AMUX_SHIFT)

/**
 * Ratio = 7.5
 */
#define FS8X_M_RATIO_RATIO_7_5 (0x0000U << FS8X_M_RATIO_SHIFT)
/**
 * Ratio = 14
 */
#define FS8X_M_RATIO_RATIO_14 (0x0001U << FS8X_M_RATIO_SHIFT)

/******************************************************************************/
/* M_CLOCK - Type: RW                                                         */
/******************************************************************************/

#define FS8X_M_CLOCK_ADDR    0x05U
#define FS8X_M_CLOCK_DEFAULT 0x0000U

/**
 * Oscillator Frequency [MHz]
 */
#define FS8X_M_CLK_TUNE_MASK 0x000FU
/**
 * CLOCK Modulation
 */
#define FS8X_M_MOD_EN_MASK 0x0010U
/**
 * FIN input signal divider selection
 */
#define FS8X_M_FIN_DIV_MASK 0x0020U
/**
 * EXT FIN selection at PLL input
 */
#define FS8X_M_EXT_FIN_SEL_MASK 0x0040U
/**
 * FOUT frequency selection (CLK1 or CLK2)
 */
#define FS8X_M_FOUT_CLK_SEL_MASK 0x0080U
/**
 * FOUT phase shifting configuration.
 */
#define FS8X_M_FOUT_PHASE_MASK 0x0700U
/**
 * Fout Multiplexer selection
 */
#define FS8X_M_FOUT_MUX_SEL_MASK 0x7800U
/**
 * CLOCK Modulation Configuration
 */
#define FS8X_M_MOD_CONF_MASK 0x8000U

/**
 * Oscillator Frequency [MHz]
 */
#define FS8X_M_CLK_TUNE_SHIFT 0x0000U
/**
 * CLOCK Modulation
 */
#define FS8X_M_MOD_EN_SHIFT 0x0004U
/**
 * FIN input signal divider selection
 */
#define FS8X_M_FIN_DIV_SHIFT 0x0005U
/**
 * EXT FIN selection at PLL input
 */
#define FS8X_M_EXT_FIN_SEL_SHIFT 0x0006U
/**
 * FOUT frequency selection (CLK1 or CLK2)
 */
#define FS8X_M_FOUT_CLK_SEL_SHIFT 0x0007U
/**
 * FOUT phase shifting configuration.
 */
#define FS8X_M_FOUT_PHASE_SHIFT 0x0008U
/**
 * Fout Multiplexer selection
 */
#define FS8X_M_FOUT_MUX_SEL_SHIFT 0x000BU
/**
 * CLOCK Modulation Configuration
 */
#define FS8X_M_MOD_CONF_SHIFT 0x000FU

/**
 * 20
 */
#define FS8X_M_CLK_TUNE_20 (0x0000U << FS8X_M_CLK_TUNE_SHIFT)
/**
 * 21
 */
#define FS8X_M_CLK_TUNE_21 (0x0001U << FS8X_M_CLK_TUNE_SHIFT)
/**
 * 22
 */
#define FS8X_M_CLK_TUNE_22 (0x0002U << FS8X_M_CLK_TUNE_SHIFT)
/**
 * 23
 */
#define FS8X_M_CLK_TUNE_23 (0x0003U << FS8X_M_CLK_TUNE_SHIFT)
/**
 * 24
 */
#define FS8X_M_CLK_TUNE_24 (0x0004U << FS8X_M_CLK_TUNE_SHIFT)
/**
 * 16
 */
#define FS8X_M_CLK_TUNE_16 (0x0009U << FS8X_M_CLK_TUNE_SHIFT)
/**
 * 17
 */
#define FS8X_M_CLK_TUNE_17 (0x000AU << FS8X_M_CLK_TUNE_SHIFT)
/**
 * 18
 */
#define FS8X_M_CLK_TUNE_18 (0x000BU << FS8X_M_CLK_TUNE_SHIFT)
/**
 * 19
 */
#define FS8X_M_CLK_TUNE_19 (0x000CU << FS8X_M_CLK_TUNE_SHIFT)

/**
 * Modulation Disable
 */
#define FS8X_M_MOD_EN_MODULATION_DISABLE (0x0000U << FS8X_M_MOD_EN_SHIFT)
/**
 * Modulation Enable
 */
#define FS8X_M_MOD_EN_MODULATION_ENABLE (0x0001U << FS8X_M_MOD_EN_SHIFT)

/**
 * Divider by 1
 */
#define FS8X_M_FIN_DIV_DIVIDER_BY_1 (0x0000U << FS8X_M_FIN_DIV_SHIFT)
/**
 * Divider by 6
 */
#define FS8X_M_FIN_DIV_DIVIDER_BY_6 (0x0001U << FS8X_M_FIN_DIV_SHIFT)

/**
 * DIS
 */
#define FS8X_M_EXT_FIN_SEL_DIS (0x0000U << FS8X_M_EXT_FIN_SEL_SHIFT)
/**
 * EN
 */
#define FS8X_M_EXT_FIN_SEL_EN (0x0001U << FS8X_M_EXT_FIN_SEL_SHIFT)

/**
 * CLK1
 */
#define FS8X_M_FOUT_CLK_SEL_CLK1 (0x0000U << FS8X_M_FOUT_CLK_SEL_SHIFT)
/**
 * CLK2
 */
#define FS8X_M_FOUT_CLK_SEL_CLK2 (0x0001U << FS8X_M_FOUT_CLK_SEL_SHIFT)

/**
 * No shift
 */
#define FS8X_M_FOUT_PHASE_NO_SHIFT (0x0000U << FS8X_M_FOUT_PHASE_SHIFT)
/**
 * Shifted by 1 clock cycle of CLK running at 20MHz
 */
#define FS8X_M_FOUT_PHASE_1CLK (0x0001U << FS8X_M_FOUT_PHASE_SHIFT)
/**
 * Shifted by 2 clock cycle of CLK running at 20MHz
 */
#define FS8X_M_FOUT_PHASE_2CLK (0x0002U << FS8X_M_FOUT_PHASE_SHIFT)
/**
 * Shifted by 3 clock cycle of CLK running at 20MHz
 */
#define FS8X_M_FOUT_PHASE_3CLK (0x0003U << FS8X_M_FOUT_PHASE_SHIFT)
/**
 * Shifted by 4 clock cycle of CLK running at 20MHz
 */
#define FS8X_M_FOUT_PHASE_4CLK (0x0004U << FS8X_M_FOUT_PHASE_SHIFT)
/**
 * Shifted by 5 clock cycle of CLK running at 20MHz
 */
#define FS8X_M_FOUT_PHASE_5CLK (0x0005U << FS8X_M_FOUT_PHASE_SHIFT)
/**
 * Shifted by 6 clock cycle of CLK running at 20MHz
 */
#define FS8X_M_FOUT_PHASE_6CLK (0x0006U << FS8X_M_FOUT_PHASE_SHIFT)
/**
 * Shifted by 7 clock cycle of CLK running at 20MHz
 */
#define FS8X_M_FOUT_PHASE_7CLK (0x0007U << FS8X_M_FOUT_PHASE_SHIFT)

/**
 * No signal, Fout Low
 */
#define FS8X_M_FOUT_MUX_SEL_NO_SIGNAL (0x0000U << FS8X_M_FOUT_MUX_SEL_SHIFT)
/**
 * VPRE_clk
 */
#define FS8X_M_FOUT_MUX_SEL_VPRE_CLK (0x0001U << FS8X_M_FOUT_MUX_SEL_SHIFT)
/**
 * BOOST_clk
 */
#define FS8X_M_FOUT_MUX_SEL_BOOST_CLK (0x0002U << FS8X_M_FOUT_MUX_SEL_SHIFT)
/**
 * BUCK1_clk
 */
#define FS8X_M_FOUT_MUX_SEL_BUCK1_CLK (0x0003U << FS8X_M_FOUT_MUX_SEL_SHIFT)
/**
 * BUCK2_clk
 */
#define FS8X_M_FOUT_MUX_SEL_BUCK2_CLK (0x0004U << FS8X_M_FOUT_MUX_SEL_SHIFT)
/**
 * BUCK3_clk
 */
#define FS8X_M_FOUT_MUX_SEL_BUCK3_CLK (0x0005U << FS8X_M_FOUT_MUX_SEL_SHIFT)
/**
 * CLK1 or CLK2 selected with FOUT_CLK_SEL bit
 */
#define FS8X_M_FOUT_MUX_SEL_FOUT_CLK (0x0006U << FS8X_M_FOUT_MUX_SEL_SHIFT)
/**
 * OSC_MAIN/48
 */
#define FS8X_M_FOUT_MUX_SEL_OSC_MAIN_48 (0x0007U << FS8X_M_FOUT_MUX_SEL_SHIFT)
/**
 * OSC_FS/48
 */
#define FS8X_M_FOUT_MUX_SEL_OSC_FS_48 (0x0008U << FS8X_M_FOUT_MUX_SEL_SHIFT)
/**
 * CLOCK_FIN_DIV
 */
#define FS8X_M_FOUT_MUX_SEL_CLOCK_FIN_DIV (0x0009U << FS8X_M_FOUT_MUX_SEL_SHIFT)

/**
 * range +- 5% 23kHz
 */
#define FS8X_M_MOD_CONF_23KHZ (0x0000U << FS8X_M_MOD_CONF_SHIFT)
/**
 * range +- 5% 94kHz
 */
#define FS8X_M_MOD_CONF_94KHZ (0x0001U << FS8X_M_MOD_CONF_SHIFT)

/******************************************************************************/
/* M_INT_MASK1 - Type: RW                                                     */
/******************************************************************************/

#define FS8X_M_INT_MASK1_ADDR    0x06U
#define FS8X_M_INT_MASK1_DEFAULT 0x0000U

/**
 * Inhibit INTERRUPT for LDO2 over temperature shutdown event
 */
#define FS8X_M_LDO2TSD_M_MASK 0x0001U
/**
 * Inhibit INTERRUPT for LDO1 over temperature shutdown event
 */
#define FS8X_M_LDO1TSD_M_MASK 0x0002U
/**
 * Inhibit INTERRUPT for BUCK3 over temperature shutdown event
 */
#define FS8X_M_BUCK3TSD_M_MASK 0x0004U
/**
 * Inhibit INTERRUPT for BUCK2 over temperature shutdown event
 */
#define FS8X_M_BUCK2TSD_M_MASK 0x0008U
/**
 * Inhibit INTERRUPT for BUCK1 over temperature shutdown event
 */
#define FS8X_M_BUCK1TSD_M_MASK 0x0010U
/**
 * Inhibit INTERRUPT for BOOST over temperature shutdown event
 */
#define FS8X_M_BOOSTTSD_M_MASK 0x0020U
/**
 * Inhibit INTERRUPT for Best Of Supply over temperature shutdown event
 */
#define FS8X_M_BOSTSD_M_MASK 0x0040U
/**
 * Inhibit INTERRUPT for LDO2 Over current
 */
#define FS8X_M_LDO2OC_M_MASK 0x0100U
/**
 * Inhibit INTERRUPT for LDO1 Over current
 */
#define FS8X_M_LDO1OC_M_MASK 0x0200U
/**
 * Inhibit INTERRUPT for BUCK3 Over current
 */
#define FS8X_M_BUCK3OC_M_MASK 0x0400U
/**
 * Inhibit INTERRUPT for BUCK2 Over current
 */
#define FS8X_M_BUCK2OC_M_MASK 0x0800U
/**
 * Inhibit INTERRUPT for BUCK1 Over current
 */
#define FS8X_M_BUCK1OC_M_MASK 0x1000U
/**
 * Inhibit INTERRUPT for VBOOST Over current
 */
#define FS8X_M_BOOSTOC_M_MASK 0x2000U
/**
 * Inhibit INTERRUPT for VPRE Over current
 */
#define FS8X_M_VPREOC_M_MASK 0x4000U

/**
 * Inhibit INTERRUPT for LDO2 over temperature shutdown event
 */
#define FS8X_M_LDO2TSD_M_SHIFT 0x0000U
/**
 * Inhibit INTERRUPT for LDO1 over temperature shutdown event
 */
#define FS8X_M_LDO1TSD_M_SHIFT 0x0001U
/**
 * Inhibit INTERRUPT for BUCK3 over temperature shutdown event
 */
#define FS8X_M_BUCK3TSD_M_SHIFT 0x0002U
/**
 * Inhibit INTERRUPT for BUCK2 over temperature shutdown event
 */
#define FS8X_M_BUCK2TSD_M_SHIFT 0x0003U
/**
 * Inhibit INTERRUPT for BUCK1 over temperature shutdown event
 */
#define FS8X_M_BUCK1TSD_M_SHIFT 0x0004U
/**
 * Inhibit INTERRUPT for BOOST over temperature shutdown event
 */
#define FS8X_M_BOOSTTSD_M_SHIFT 0x0005U
/**
 * Inhibit INTERRUPT for Best Of Supply over temperature shutdown event
 */
#define FS8X_M_BOSTSD_M_SHIFT 0x0006U
/**
 * Inhibit INTERRUPT for LDO2 Over current
 */
#define FS8X_M_LDO2OC_M_SHIFT 0x0008U
/**
 * Inhibit INTERRUPT for LDO1 Over current
 */
#define FS8X_M_LDO1OC_M_SHIFT 0x0009U
/**
 * Inhibit INTERRUPT for BUCK3 Over current
 */
#define FS8X_M_BUCK3OC_M_SHIFT 0x000AU
/**
 * Inhibit INTERRUPT for BUCK2 Over current
 */
#define FS8X_M_BUCK2OC_M_SHIFT 0x000BU
/**
 * Inhibit INTERRUPT for BUCK1 Over current
 */
#define FS8X_M_BUCK1OC_M_SHIFT 0x000CU
/**
 * Inhibit INTERRUPT for VBOOST Over current
 */
#define FS8X_M_BOOSTOC_M_SHIFT 0x000DU
/**
 * Inhibit INTERRUPT for VPRE Over current
 */
#define FS8X_M_VPREOC_M_SHIFT 0x000EU

/**
 * INT not masked
 */
#define FS8X_M_LDO2TSD_M_INT_NOT_MASKED (0x0000U << FS8X_M_LDO2TSD_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_LDO2TSD_M_INT_MASKED (0x0001U << FS8X_M_LDO2TSD_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_LDO1TSD_M_INT_NOT_MASKED (0x0000U << FS8X_M_LDO1TSD_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_LDO1TSD_M_INT_MASKED (0x0001U << FS8X_M_LDO1TSD_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_BUCK3TSD_M_INT_NOT_MASKED (0x0000U << FS8X_M_BUCK3TSD_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_BUCK3TSD_M_INT_MASKED (0x0001U << FS8X_M_BUCK3TSD_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_BUCK2TSD_M_INT_NOT_MASKED (0x0000U << FS8X_M_BUCK2TSD_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_BUCK2TSD_M_INT_MASKED (0x0001U << FS8X_M_BUCK2TSD_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_BUCK1TSD_M_INT_NOT_MASKED (0x0000U << FS8X_M_BUCK1TSD_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_BUCK1TSD_M_INT_MASKED (0x0001U << FS8X_M_BUCK1TSD_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_BOOSTTSD_M_INT_NOT_MASKED (0x0000U << FS8X_M_BOOSTTSD_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_BOOSTTSD_M_INT_MASKED (0x0001U << FS8X_M_BOOSTTSD_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_BOSTSD_M_INT_NOT_MASKED (0x0000U << FS8X_M_BOSTSD_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_BOSTSD_M_INT_MASKED (0x0001U << FS8X_M_BOSTSD_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_LDO2OC_M_INT_NOT_MASKED (0x0000U << FS8X_M_LDO2OC_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_LDO2OC_M_INT_MASKED (0x0001U << FS8X_M_LDO2OC_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_LDO1OC_M_INT_NOT_MASKED (0x0000U << FS8X_M_LDO1OC_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_LDO1OC_M_INT_MASKED (0x0001U << FS8X_M_LDO1OC_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_BUCK3OC_M_INT_NOT_MASKED (0x0000U << FS8X_M_BUCK3OC_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_BUCK3OC_M_INT_MASKED (0x0001U << FS8X_M_BUCK3OC_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_BUCK2OC_M_INT_NOT_MASKED (0x0000U << FS8X_M_BUCK2OC_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_BUCK2OC_M_INT_MASKED (0x0001U << FS8X_M_BUCK2OC_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_BUCK1OC_M_INT_NOT_MASKED (0x0000U << FS8X_M_BUCK1OC_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_BUCK1OC_M_INT_MASKED (0x0001U << FS8X_M_BUCK1OC_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_BOOSTOC_M_INT_NOT_MASKED (0x0000U << FS8X_M_BOOSTOC_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_BOOSTOC_M_INT_MASKED (0x0001U << FS8X_M_BOOSTOC_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_VPREOC_M_INT_NOT_MASKED (0x0000U << FS8X_M_VPREOC_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_VPREOC_M_INT_MASKED (0x0001U << FS8X_M_VPREOC_M_SHIFT)

/******************************************************************************/
/* M_INT_MASK2 - Type: RW                                                     */
/******************************************************************************/

#define FS8X_M_INT_MASK2_ADDR    0x07U
#define FS8X_M_INT_MASK2_DEFAULT 0x0000U

/**
 * Inhibit INTERRUPT for WAKE2 any transition
 */
#define FS8X_M_WAKE2_M_MASK 0x0001U
/**
 * Inhibit INTERRUPT for WAKE1 any transition
 */
#define FS8X_M_WAKE1_M_MASK 0x0002U
/**
 * Inhibit INTERRUPT for VSUP_UVH
 */
#define FS8X_M_VSUPUVH_M_MASK 0x0004U
/**
 * Inhibit INTERRUPT for VSUP_UVL
 */
#define FS8X_M_VSUPUVL_M_MASK 0x0008U
/**
 * Inhibit INTERRUPT for VPRE_UVH
 */
#define FS8X_M_VPREUVH_M_MASK 0x0010U
/**
 * Inhibit INTERRUPT for VPRE_UVL
 */
#define FS8X_M_VPREUVL_M_MASK 0x0020U
/**
 * Inhibit INTERRUPT for VSUP_UV7
 */
#define FS8X_M_VSUPUV7_M_MASK 0x0040U
/**
 * Inhibit INTERRUPT for VBOOST_UVH
 */
#define FS8X_M_VBOOST_UVH_M_MASK 0x0080U
/**
 * Inhibit INTERRUPT for VPRE_FB_OV
 */
#define FS8X_M_VPRE_FB_OV_M_MASK 0x0100U
/**
 * Inhibit INTERRUPT for COM any transition
 */
#define FS8X_M_COM_M_MASK 0x0200U
/**
 * Inhibit INTERRUPT for VBOS_UVH any transition
 */
#define FS8X_M_VBOSUVH_M_MASK 0x0400U
/**
 * Inhibit INTERRUPT for VBOOST_OV any transition
 */
#define FS8X_M_VBOOSTOV_M_MASK 0x0800U

/**
 * Inhibit INTERRUPT for WAKE2 any transition
 */
#define FS8X_M_WAKE2_M_SHIFT 0x0000U
/**
 * Inhibit INTERRUPT for WAKE1 any transition
 */
#define FS8X_M_WAKE1_M_SHIFT 0x0001U
/**
 * Inhibit INTERRUPT for VSUP_UVH
 */
#define FS8X_M_VSUPUVH_M_SHIFT 0x0002U
/**
 * Inhibit INTERRUPT for VSUP_UVL
 */
#define FS8X_M_VSUPUVL_M_SHIFT 0x0003U
/**
 * Inhibit INTERRUPT for VPRE_UVH
 */
#define FS8X_M_VPREUVH_M_SHIFT 0x0004U
/**
 * Inhibit INTERRUPT for VPRE_UVL
 */
#define FS8X_M_VPREUVL_M_SHIFT 0x0005U
/**
 * Inhibit INTERRUPT for VSUP_UV7
 */
#define FS8X_M_VSUPUV7_M_SHIFT 0x0006U
/**
 * Inhibit INTERRUPT for VBOOST_UVH
 */
#define FS8X_M_VBOOST_UVH_M_SHIFT 0x0007U
/**
 * Inhibit INTERRUPT for VPRE_FB_OV
 */
#define FS8X_M_VPRE_FB_OV_M_SHIFT 0x0008U
/**
 * Inhibit INTERRUPT for COM any transition
 */
#define FS8X_M_COM_M_SHIFT 0x0009U
/**
 * Inhibit INTERRUPT for VBOS_UVH any transition
 */
#define FS8X_M_VBOSUVH_M_SHIFT 0x000AU
/**
 * Inhibit INTERRUPT for VBOOST_OV any transition
 */
#define FS8X_M_VBOOSTOV_M_SHIFT 0x000BU

/**
 * INT not masked
 */
#define FS8X_M_WAKE2_M_INT_NOT_MASKED (0x0000U << FS8X_M_WAKE2_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_WAKE2_M_INT_MASKED (0x0001U << FS8X_M_WAKE2_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_WAKE1_M_INT_NOT_MASKED (0x0000U << FS8X_M_WAKE1_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_WAKE1_M_INT_MASKED (0x0001U << FS8X_M_WAKE1_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_VSUPUVH_M_INT_NOT_MASKED (0x0000U << FS8X_M_VSUPUVH_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_VSUPUVH_M_INT_MASKED (0x0001U << FS8X_M_VSUPUVH_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_VSUPUVL_M_INT_NOT_MASKED (0x0000U << FS8X_M_VSUPUVL_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_VSUPUVL_M_INT_MASKED (0x0001U << FS8X_M_VSUPUVL_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_VPREUVH_M_INT_NOT_MASKED (0x0000U << FS8X_M_VPREUVH_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_VPREUVH_M_INT_MASKED (0x0001U << FS8X_M_VPREUVH_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_VPREUVL_M_INT_NOT_MASKED (0x0000U << FS8X_M_VPREUVL_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_VPREUVL_M_INT_MASKED (0x0001U << FS8X_M_VPREUVL_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_VSUPUV7_M_INT_NOT_MASKED (0x0000U << FS8X_M_VSUPUV7_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_VSUPUV7_M_INT_MASKED (0x0001U << FS8X_M_VSUPUV7_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_VBOOST_UVH_M_INT_NOT_MASKED (0x0000U << FS8X_M_VBOOST_UVH_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_VBOOST_UVH_M_INT_MASKED (0x0001U << FS8X_M_VBOOST_UVH_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_VPRE_FB_OV_M_INT_NOT_MASKED (0x0000U << FS8X_M_VPRE_FB_OV_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_VPRE_FB_OV_M_INT_MASKED (0x0001U << FS8X_M_VPRE_FB_OV_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_COM_M_INT_NOT_MASKED (0x0000U << FS8X_M_COM_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_COM_M_INT_MASKED (0x0001U << FS8X_M_COM_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_VBOSUVH_M_INT_NOT_MASKED (0x0000U << FS8X_M_VBOSUVH_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_VBOSUVH_M_INT_MASKED (0x0001U << FS8X_M_VBOSUVH_M_SHIFT)

/**
 * INT not masked
 */
#define FS8X_M_VBOOSTOV_M_INT_NOT_MASKED (0x0000U << FS8X_M_VBOOSTOV_M_SHIFT)
/**
 * INT masked
 */
#define FS8X_M_VBOOSTOV_M_INT_MASKED (0x0001U << FS8X_M_VBOOSTOV_M_SHIFT)

/******************************************************************************/
/* M_FLAG1 - Type: RW                                                         */
/******************************************************************************/

#define FS8X_M_FLAG1_ADDR    0x08U
#define FS8X_M_FLAG1_DEFAULT 0x0000U

/**
 * LDO2 over temperature shutdown event
 */
#define FS8X_M_LDO2OT_MASK 0x0001U
/**
 * LDO1 over temperature shutdown event
 */
#define FS8X_M_LDO1OT_MASK 0x0002U
/**
 * BUCK3 over temperature shutdown event
 */
#define FS8X_M_BUCK3OT_MASK 0x0004U
/**
 * BUCK2 over temperature shutdown event
 */
#define FS8X_M_BUCK2OT_MASK 0x0008U
/**
 * BUCK1 over temperature shutdown event
 */
#define FS8X_M_BUCK1OT_MASK 0x0010U
/**
 * VBOOST over temperature shutdown event
 */
#define FS8X_M_VBOOSTOT_MASK 0x0020U
/**
 * VBOOST Over voltage protection event
 */
#define FS8X_M_VBOOSTOV_MASK 0x0040U
/**
 * CLK_FIN_DIV monitoring
 */
#define FS8X_M_CLK_FIN_DIV_OK_MASK 0x0080U
/**
 * LDO2 Over current event
 */
#define FS8X_M_LDO2OC_MASK 0x0100U
/**
 * LDO1 Over current event
 */
#define FS8X_M_LDO1OC_MASK 0x0200U
/**
 * BUCK3 Over current
 */
#define FS8X_M_BUCK3OC_MASK 0x0400U
/**
 * BUCK2 Over current
 */
#define FS8X_M_BUCK2OC_MASK 0x0800U
/**
 * BUCK1 Over current
 */
#define FS8X_M_BUCK1OC_MASK 0x1000U
/**
 * VPRE Over current event
 */
#define FS8X_M_VPREOC_MASK 0x2000U
/**
 * VBOOST Under voltage high event (falling)
 */
#define FS8X_M_VBOOSTUVH_MASK 0x4000U
/**
 * VBOS Under voltage high event (falling)
 */
#define FS8X_M_VBOSUVH_MASK 0x8000U

/**
 * LDO2 over temperature shutdown event
 */
#define FS8X_M_LDO2OT_SHIFT 0x0000U
/**
 * LDO1 over temperature shutdown event
 */
#define FS8X_M_LDO1OT_SHIFT 0x0001U
/**
 * BUCK3 over temperature shutdown event
 */
#define FS8X_M_BUCK3OT_SHIFT 0x0002U
/**
 * BUCK2 over temperature shutdown event
 */
#define FS8X_M_BUCK2OT_SHIFT 0x0003U
/**
 * BUCK1 over temperature shutdown event
 */
#define FS8X_M_BUCK1OT_SHIFT 0x0004U
/**
 * VBOOST over temperature shutdown event
 */
#define FS8X_M_VBOOSTOT_SHIFT 0x0005U
/**
 * VBOOST Over voltage protection event
 */
#define FS8X_M_VBOOSTOV_SHIFT 0x0006U
/**
 * CLK_FIN_DIV monitoring
 */
#define FS8X_M_CLK_FIN_DIV_OK_SHIFT 0x0007U
/**
 * LDO2 Over current event
 */
#define FS8X_M_LDO2OC_SHIFT 0x0008U
/**
 * LDO1 Over current event
 */
#define FS8X_M_LDO1OC_SHIFT 0x0009U
/**
 * BUCK3 Over current
 */
#define FS8X_M_BUCK3OC_SHIFT 0x000AU
/**
 * BUCK2 Over current
 */
#define FS8X_M_BUCK2OC_SHIFT 0x000BU
/**
 * BUCK1 Over current
 */
#define FS8X_M_BUCK1OC_SHIFT 0x000CU
/**
 * VPRE Over current event
 */
#define FS8X_M_VPREOC_SHIFT 0x000DU
/**
 * VBOOST Under voltage high event (falling)
 */
#define FS8X_M_VBOOSTUVH_SHIFT 0x000EU
/**
 * VBOS Under voltage high event (falling)
 */
#define FS8X_M_VBOSUVH_SHIFT 0x000FU

/**
 * No event
 */
#define FS8X_M_LDO2OT_NO_EVENT (0x0000U << FS8X_M_LDO2OT_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_LDO2OT_EVENT_OCCURRED (0x0001U << FS8X_M_LDO2OT_SHIFT)

/**
 * No event
 */
#define FS8X_M_LDO1OT_NO_EVENT (0x0000U << FS8X_M_LDO1OT_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_LDO1OT_EVENT_OCCURRED (0x0001U << FS8X_M_LDO1OT_SHIFT)

/**
 * No event
 */
#define FS8X_M_BUCK3OT_NO_EVENT (0x0000U << FS8X_M_BUCK3OT_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_BUCK3OT_EVENT_OCCURRED (0x0001U << FS8X_M_BUCK3OT_SHIFT)

/**
 * No event
 */
#define FS8X_M_BUCK2OT_NO_EVENT (0x0000U << FS8X_M_BUCK2OT_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_BUCK2OT_EVENT_OCCURRED (0x0001U << FS8X_M_BUCK2OT_SHIFT)

/**
 * No event
 */
#define FS8X_M_BUCK1OT_NO_EVENT (0x0000U << FS8X_M_BUCK1OT_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_BUCK1OT_EVENT_OCCURRED (0x0001U << FS8X_M_BUCK1OT_SHIFT)

/**
 * No event
 */
#define FS8X_M_VBOOSTOT_NO_EVENT (0x0000U << FS8X_M_VBOOSTOT_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_VBOOSTOT_EVENT_OCCURRED (0x0001U << FS8X_M_VBOOSTOT_SHIFT)

/**
 * No event
 */
#define FS8X_M_VBOOSTOV_NO_EVENT (0x0000U << FS8X_M_VBOOSTOV_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_VBOOSTOV_EVENT_OCCURRED (0x0001U << FS8X_M_VBOOSTOV_SHIFT)

/**
 * Not OK : FINERR_LONG < CLK_FIN_DIV deviation < FINERR_SHORT
 */
#define FS8X_M_CLK_FIN_DIV_OK_NOT_OK (0x0000U << FS8X_M_CLK_FIN_DIV_OK_SHIFT)
/**
 * OK : FINERR_SHORT < CLK_FIN_DIV deviation < FINERR_LONG
 */
#define FS8X_M_CLK_FIN_DIV_OK_OK (0x0001U << FS8X_M_CLK_FIN_DIV_OK_SHIFT)

/**
 * No event
 */
#define FS8X_M_LDO2OC_NO_EVENT (0x0000U << FS8X_M_LDO2OC_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_LDO2OC_EVENT_OCCURRED (0x0001U << FS8X_M_LDO2OC_SHIFT)

/**
 * No event
 */
#define FS8X_M_LDO1OC_NO_EVENT (0x0000U << FS8X_M_LDO1OC_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_LDO1OC_EVENT_OCCURRED (0x0001U << FS8X_M_LDO1OC_SHIFT)

/**
 * No event
 */
#define FS8X_M_BUCK3OC_NO_EVENT (0x0000U << FS8X_M_BUCK3OC_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_BUCK3OC_EVENT_OCCURRED (0x0001U << FS8X_M_BUCK3OC_SHIFT)

/**
 * No event
 */
#define FS8X_M_BUCK2OC_NO_EVENT (0x0000U << FS8X_M_BUCK2OC_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_BUCK2OC_EVENT_OCCURRED (0x0001U << FS8X_M_BUCK2OC_SHIFT)

/**
 * No event
 */
#define FS8X_M_BUCK1OC_NO_EVENT (0x0000U << FS8X_M_BUCK1OC_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_BUCK1OC_EVENT_OCCURRED (0x0001U << FS8X_M_BUCK1OC_SHIFT)

/**
 * No event
 */
#define FS8X_M_VPREOC_NO_EVENT (0x0000U << FS8X_M_VPREOC_SHIFT)
/**
 * Event occured
 */
#define FS8X_M_VPREOC_EVENT_OCCURRED (0x0001U << FS8X_M_VPREOC_SHIFT)

/**
 * No event
 */
#define FS8X_M_VBOOSTUVH_NO_EVENT (0x0000U << FS8X_M_VBOOSTUVH_SHIFT)
/**
 * Event occured
 */
#define FS8X_M_VBOOSTUVH_EVENT_OCCURRED (0x0001U << FS8X_M_VBOOSTUVH_SHIFT)

/**
 * No event
 */
#define FS8X_M_VBOSUVH_NO_EVENT (0x0000U << FS8X_M_VBOSUVH_SHIFT)
/**
 * Event occured
 */
#define FS8X_M_VBOSUVH_EVENT_OCCURRED (0x0001U << FS8X_M_VBOSUVH_SHIFT)

/******************************************************************************/
/* M_FLAG2 - Type: RW                                                         */
/******************************************************************************/

#define FS8X_M_FLAG2_ADDR    0x09U
#define FS8X_M_FLAG2_DEFAULT 0x0000U

/**
 * WAKE1 wake up souce flag
 */
#define FS8X_M_WK1FLG_MASK 0x0001U
/**
 * WAKE2 wake up souce flag
 */
#define FS8X_M_WK2FLG_MASK 0x0002U
/**
 * Report event: WAKE1 real time state
 */
#define FS8X_M_WK1RT_MASK 0x0004U
/**
 * Report event: WAKE2 real time state
 */
#define FS8X_M_WK2RT_MASK 0x0008U
/**
 * VSUP_UVHH event
 */
#define FS8X_M_VSUPUVH_MASK 0x0010U
/**
 * VSUP_UVHL event
 */
#define FS8X_M_VSUPUVL_MASK 0x0020U
/**
 * VPRE_UVH event
 */
#define FS8X_M_VPREUVH_MASK 0x0040U
/**
 * VPRE_UVL event
 */
#define FS8X_M_VPREUVL_MASK 0x0080U
/**
 * LDO2 state
 */
#define FS8X_M_LDO2_ST_MASK 0x0100U
/**
 * LDO1 state
 */
#define FS8X_M_LDO1_ST_MASK 0x0200U
/**
 * BUCK3 state
 */
#define FS8X_M_BUCK3_ST_MASK 0x0400U
/**
 * BUCK2 state
 */
#define FS8X_M_BUCK2_ST_MASK 0x0800U
/**
 * BUCK1 state
 */
#define FS8X_M_BUCK1_ST_MASK 0x1000U
/**
 * BOOST state
 */
#define FS8X_M_BOOST_ST_MASK 0x2000U
/**
 * VSUP_UV7 event
 */
#define FS8X_M_VSUPUV7_MASK 0x4000U
/**
 * VPRE_FB_OV event
 */
#define FS8X_M_VPRE_FB_OV_MASK 0x8000U

/**
 * WAKE1 wake up souce flag
 */
#define FS8X_M_WK1FLG_SHIFT 0x0000U
/**
 * WAKE2 wake up souce flag
 */
#define FS8X_M_WK2FLG_SHIFT 0x0001U
/**
 * Report event: WAKE1 real time state
 */
#define FS8X_M_WK1RT_SHIFT 0x0002U
/**
 * Report event: WAKE2 real time state
 */
#define FS8X_M_WK2RT_SHIFT 0x0003U
/**
 * VSUP_UVHH event
 */
#define FS8X_M_VSUPUVH_SHIFT 0x0004U
/**
 * VSUP_UVHL event
 */
#define FS8X_M_VSUPUVL_SHIFT 0x0005U
/**
 * VPRE_UVH event
 */
#define FS8X_M_VPREUVH_SHIFT 0x0006U
/**
 * VPRE_UVL event
 */
#define FS8X_M_VPREUVL_SHIFT 0x0007U
/**
 * LDO2 state
 */
#define FS8X_M_LDO2_ST_SHIFT 0x0008U
/**
 * LDO1 state
 */
#define FS8X_M_LDO1_ST_SHIFT 0x0009U
/**
 * BUCK3 state
 */
#define FS8X_M_BUCK3_ST_SHIFT 0x000AU
/**
 * BUCK2 state
 */
#define FS8X_M_BUCK2_ST_SHIFT 0x000BU
/**
 * BUCK1 state
 */
#define FS8X_M_BUCK1_ST_SHIFT 0x000CU
/**
 * BOOST state
 */
#define FS8X_M_BOOST_ST_SHIFT 0x000DU
/**
 * VSUP_UV7 event
 */
#define FS8X_M_VSUPUV7_SHIFT 0x000EU
/**
 * VPRE_FB_OV event
 */
#define FS8X_M_VPRE_FB_OV_SHIFT 0x000FU

/**
 * No event
 */
#define FS8X_M_WK1FLG_NO_EVENT (0x0000U << FS8X_M_WK1FLG_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_WK1FLG_EVENT_OCCURRED (0x0001U << FS8X_M_WK1FLG_SHIFT)

/**
 * No event
 */
#define FS8X_M_WK2FLG_NO_EVENT (0x0000U << FS8X_M_WK2FLG_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_WK2FLG_EVENT_OCCURRED (0x0001U << FS8X_M_WK2FLG_SHIFT)

/**
 * WAKE1 is low level
 */
#define FS8X_M_WK1RT_WAKE1_LOW (0x0000U << FS8X_M_WK1RT_SHIFT)
/**
 * WAKE1 is high
 */
#define FS8X_M_WK1RT_WAKE1_HIGH (0x0001U << FS8X_M_WK1RT_SHIFT)

/**
 * WAKE2 is low level
 */
#define FS8X_M_WK2RT_WAKE2_LOW (0x0000U << FS8X_M_WK2RT_SHIFT)
/**
 * WAKE2 is high
 */
#define FS8X_M_WK2RT_WAKE2_HIGH (0x0001U << FS8X_M_WK2RT_SHIFT)

/**
 * No event
 */
#define FS8X_M_VSUPUVH_NO_EVENT (0x0000U << FS8X_M_VSUPUVH_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_VSUPUVH_EVENT_OCCURRED (0x0001U << FS8X_M_VSUPUVH_SHIFT)

/**
 * No event
 */
#define FS8X_M_VSUPUVL_NO_EVENT (0x0000U << FS8X_M_VSUPUVL_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_VSUPUVL_EVENT_OCCURRED (0x0001U << FS8X_M_VSUPUVL_SHIFT)

/**
 * No event
 */
#define FS8X_M_VPREUVH_NO_EVENT (0x0000U << FS8X_M_VPREUVH_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_VPREUVH_EVENT_OCCURRED (0x0001U << FS8X_M_VPREUVH_SHIFT)

/**
 * No event
 */
#define FS8X_M_VPREUVL_NO_EVENT (0x0000U << FS8X_M_VPREUVL_SHIFT)
/**
 * Event occurred
 */
#define FS8X_M_VPREUVL_EVENT_OCCURRED (0x0001U << FS8X_M_VPREUVL_SHIFT)

/**
 * regulator OFF
 */
#define FS8X_M_LDO2_ST_REGULATOR_OFF (0x0000U << FS8X_M_LDO2_ST_SHIFT)
/**
 * regulator ON
 */
#define FS8X_M_LDO2_ST_REGULATOR_ON (0x0001U << FS8X_M_LDO2_ST_SHIFT)

/**
 * regulator OFF
 */
#define FS8X_M_LDO1_ST_REGULATOR_OFF (0x0000U << FS8X_M_LDO1_ST_SHIFT)
/**
 * regulator ON
 */
#define FS8X_M_LDO1_ST_REGULATOR_ON (0x0001U << FS8X_M_LDO1_ST_SHIFT)

/**
 * regulator OFF
 */
#define FS8X_M_BUCK3_ST_REGULATOR_OFF (0x0000U << FS8X_M_BUCK3_ST_SHIFT)
/**
 * regulator ON
 */
#define FS8X_M_BUCK3_ST_REGULATOR_ON (0x0001U << FS8X_M_BUCK3_ST_SHIFT)

/**
 * regulator OFF
 */
#define FS8X_M_BUCK2_ST_REGULATOR_OFF (0x0000U << FS8X_M_BUCK2_ST_SHIFT)
/**
 * regulator ON
 */
#define FS8X_M_BUCK2_ST_REGULATOR_ON (0x0001U << FS8X_M_BUCK2_ST_SHIFT)

/**
 * regulator OFF
 */
#define FS8X_M_BUCK1_ST_REGULATOR_OFF (0x0000U << FS8X_M_BUCK1_ST_SHIFT)
/**
 * regulator ON
 */
#define FS8X_M_BUCK1_ST_REGULATOR_ON (0x0001U << FS8X_M_BUCK1_ST_SHIFT)

/**
 * regulator OFF
 */
#define FS8X_M_BOOST_ST_REGULATOR_OFF (0x0000U << FS8X_M_BOOST_ST_SHIFT)
/**
 * regulator ON
 */
#define FS8X_M_BOOST_ST_REGULATOR_ON (0x0001U << FS8X_M_BOOST_ST_SHIFT)

/**
 * No event
 */
#define FS8X_M_VSUPUV7_NO_EVENT (0x0000U << FS8X_M_VSUPUV7_SHIFT)
/**
 * Event occured
 */
#define FS8X_M_VSUPUV7_EVENT_OCCURRED (0x0001U << FS8X_M_VSUPUV7_SHIFT)

/**
 * No event
 */
#define FS8X_M_VPRE_FB_OV_NO_EVENT (0x0000U << FS8X_M_VPRE_FB_OV_SHIFT)
/**
 * Event occured
 */
#define FS8X_M_VPRE_FB_OV_EVENT_OCCURRED (0x0001U << FS8X_M_VPRE_FB_OV_SHIFT)

/******************************************************************************/
/* M_VMON_REGX - Type: RW                                                     */
/******************************************************************************/

#define FS8X_M_VMON_REGX_ADDR    0x0AU
#define FS8X_M_VMON_REGX_DEFAULT 0x0000U

/**
 * Regulator Assignment to VMON1
 */
#define FS8X_M_VMON1_REG_MASK 0x0007U
/**
 * Regulator Assignment to VMON2
 */
#define FS8X_M_VMON2_REG_MASK 0x0038U
/**
 * Regulator Assignment to VMON3
 */
#define FS8X_M_VMON3_REG_MASK 0x01C0U
/**
 * Regulator Assignment to VMON4
 */
#define FS8X_M_VMON4_REG_MASK 0x0E00U

/**
 * Regulator Assignment to VMON1
 */
#define FS8X_M_VMON1_REG_SHIFT 0x0000U
/**
 * Regulator Assignment to VMON2
 */
#define FS8X_M_VMON2_REG_SHIFT 0x0003U
/**
 * Regulator Assignment to VMON3
 */
#define FS8X_M_VMON3_REG_SHIFT 0x0006U
/**
 * Regulator Assignment to VMON4
 */
#define FS8X_M_VMON4_REG_SHIFT 0x0009U

/**
 * External Regulator
 */
#define FS8X_M_VMON1_REG_EXTERNAL_REGULATOR (0x0000U << FS8X_M_VMON1_REG_SHIFT)
/**
 * VPRE
 */
#define FS8X_M_VMON1_REG_VPRE (0x0001U << FS8X_M_VMON1_REG_SHIFT)
/**
 * LDO1
 */
#define FS8X_M_VMON1_REG_LDO1 (0x0002U << FS8X_M_VMON1_REG_SHIFT)
/**
 * LDO2
 */
#define FS8X_M_VMON1_REG_LDO2 (0x0003U << FS8X_M_VMON1_REG_SHIFT)
/**
 * BUCK2
 */
#define FS8X_M_VMON1_REG_BUCK2 (0x0004U << FS8X_M_VMON1_REG_SHIFT)
/**
 * BUCK3
 */
#define FS8X_M_VMON1_REG_BUCK3 (0x0005U << FS8X_M_VMON1_REG_SHIFT)

/**
 * External Regulator
 */
#define FS8X_M_VMON2_REG_EXTERNAL_REGULATOR (0x0000U << FS8X_M_VMON2_REG_SHIFT)
/**
 * VPRE
 */
#define FS8X_M_VMON2_REG_VPRE (0x0001U << FS8X_M_VMON2_REG_SHIFT)
/**
 * LDO1
 */
#define FS8X_M_VMON2_REG_LDO1 (0x0002U << FS8X_M_VMON2_REG_SHIFT)
/**
 * LDO2
 */
#define FS8X_M_VMON2_REG_LDO2 (0x0003U << FS8X_M_VMON2_REG_SHIFT)
/**
 * BUCK2
 */
#define FS8X_M_VMON2_REG_BUCK2 (0x0004U << FS8X_M_VMON2_REG_SHIFT)
/**
 * BUCK3
 */
#define FS8X_M_VMON2_REG_BUCK3 (0x0005U << FS8X_M_VMON2_REG_SHIFT)

/**
 * External Regulator
 */
#define FS8X_M_VMON3_REG_EXTERNAL_REGULATOR (0x0000U << FS8X_M_VMON3_REG_SHIFT)
/**
 * VPRE
 */
#define FS8X_M_VMON3_REG_VPRE (0x0001U << FS8X_M_VMON3_REG_SHIFT)
/**
 * LDO1
 */
#define FS8X_M_VMON3_REG_LDO1 (0x0002U << FS8X_M_VMON3_REG_SHIFT)
/**
 * LDO2
 */
#define FS8X_M_VMON3_REG_LDO2 (0x0003U << FS8X_M_VMON3_REG_SHIFT)
/**
 * BUCK2
 */
#define FS8X_M_VMON3_REG_BUCK2 (0x0004U << FS8X_M_VMON3_REG_SHIFT)
/**
 * BUCK3
 */
#define FS8X_M_VMON3_REG_BUCK3 (0x0005U << FS8X_M_VMON3_REG_SHIFT)

/**
 * External Regulator
 */
#define FS8X_M_VMON4_REG_EXTERNAL_REGULATOR (0x0000U << FS8X_M_VMON4_REG_SHIFT)
/**
 * VPRE
 */
#define FS8X_M_VMON4_REG_VPRE (0x0001U << FS8X_M_VMON4_REG_SHIFT)
/**
 * LDO1
 */
#define FS8X_M_VMON4_REG_LDO1 (0x0002U << FS8X_M_VMON4_REG_SHIFT)
/**
 * LDO2
 */
#define FS8X_M_VMON4_REG_LDO2 (0x0003U << FS8X_M_VMON4_REG_SHIFT)
/**
 * BUCK2
 */
#define FS8X_M_VMON4_REG_BUCK2 (0x0004U << FS8X_M_VMON4_REG_SHIFT)
/**
 * BUCK3
 */
#define FS8X_M_VMON4_REG_BUCK3 (0x0005U << FS8X_M_VMON4_REG_SHIFT)

/******************************************************************************/
/* M_LVB1_SVS - Type: R                                                       */
/******************************************************************************/

#define FS8X_M_LVB1_SVS_ADDR    0x0BU
#define FS8X_M_LVB1_SVS_DEFAULT 0x0000U

/**
 * Static Voltage Scaling negative offset
 */
#define FS8X_M_LVB1_SVS_MASK 0x001FU

/**
 * Static Voltage Scaling negative offset
 */
#define FS8X_M_LVB1_SVS_SHIFT 0x0000U

/**
 * 0mV
 */
#define FS8X_M_LVB1_SVS_0MV (0x0000U << FS8X_M_LVB1_SVS_SHIFT)
/**
 * -6.25mV
 */
#define FS8X_M_LVB1_SVS_M6_25MV (0x0001U << FS8X_M_LVB1_SVS_SHIFT)
/**
 * -12.50mV
 */
#define FS8X_M_LVB1_SVS_M12_50MV (0x0002U << FS8X_M_LVB1_SVS_SHIFT)
/**
 * -18.75mV
 */
#define FS8X_M_LVB1_SVS_M18_75MV (0x0003U << FS8X_M_LVB1_SVS_SHIFT)
/**
 * -25mV
 */
#define FS8X_M_LVB1_SVS_M25MV (0x0004U << FS8X_M_LVB1_SVS_SHIFT)
/**
 * -31.25mV
 */
#define FS8X_M_LVB1_SVS_M31_25MV (0x0005U << FS8X_M_LVB1_SVS_SHIFT)
/**
 * -37.5mV
 */
#define FS8X_M_LVB1_SVS_M37_5MV (0x0006U << FS8X_M_LVB1_SVS_SHIFT)
/**
 * -43.75mV
 */
#define FS8X_M_LVB1_SVS_M43_75MV (0x0007U << FS8X_M_LVB1_SVS_SHIFT)
/**
 * -50mV
 */
#define FS8X_M_LVB1_SVS_M50MV (0x0008U << FS8X_M_LVB1_SVS_SHIFT)
/**
 * -56.25mV
 */
#define FS8X_M_LVB1_SVS_M56_25MV (0x0009U << FS8X_M_LVB1_SVS_SHIFT)
/**
 * -62.5mV
 */
#define FS8X_M_LVB1_SVS_M62_5MV (0x000AU << FS8X_M_LVB1_SVS_SHIFT)
/**
 * -68.75mV
 */
#define FS8X_M_LVB1_SVS_M68_75MV (0x000BU << FS8X_M_LVB1_SVS_SHIFT)
/**
 * -75mV
 */
#define FS8X_M_LVB1_SVS_M75MV (0x000CU << FS8X_M_LVB1_SVS_SHIFT)
/**
 * -81.25mV
 */
#define FS8X_M_LVB1_SVS_M81_25MV (0x000DU << FS8X_M_LVB1_SVS_SHIFT)
/**
 * -87.5mV
 */
#define FS8X_M_LVB1_SVS_M87_5MV (0x000EU << FS8X_M_LVB1_SVS_SHIFT)
/**
 * -93.75mV
 */
#define FS8X_M_LVB1_SVS_M93_75MV (0x000FU << FS8X_M_LVB1_SVS_SHIFT)
/**
 * -100mV
 */
#define FS8X_M_LVB1_SVS_M100MV (0x0010U << FS8X_M_LVB1_SVS_SHIFT)

/******************************************************************************/
/* M_MEMORY0 - Type: RW                                                       */
/******************************************************************************/

#define FS8X_M_MEMORY0_ADDR    0x23U
#define FS8X_M_MEMORY0_DEFAULT 0x0000U

/**
 * Free field for data storage
 */
#define FS8X_M_M_MEMORY0_MASK 0xFFFFU

/**
 * Free field for data storage
 */
#define FS8X_M_M_MEMORY0_SHIFT 0x0000U

/******************************************************************************/
/* M_MEMORY1 - Type: RW                                                       */
/******************************************************************************/

#define FS8X_M_MEMORY1_ADDR    0x24U
#define FS8X_M_MEMORY1_DEFAULT 0x0000U

/**
 * Free field for data storage
 */
#define FS8X_M_M_MEMORY1_MASK 0xFFFFU

/**
 * Free field for data storage
 */
#define FS8X_M_M_MEMORY1_SHIFT 0x0000U

/******************************************************************************/
/* M_DEVICEID - Type: R                                                       */
/******************************************************************************/

#define FS8X_M_DEVICEID_ADDR    0x25U
#define FS8X_M_DEVICEID_DEFAULT 0x0000U

/**
 * Device ID
 */
#define FS8X_M_M_DEVICE_MASK 0x00FFU

/**
 * Device ID
 */
#define FS8X_M_M_DEVICE_SHIFT 0x0000U

/******************************************************************************/
/* FS_I_OVUV_SAFE_REACTION1 - Type: RW                                        */
/******************************************************************************/

#define FS8X_FS_I_OVUV_SAFE_REACTION1_ADDR    0x01U
#define FS8X_FS_I_OVUV_SAFE_REACTION1_DEFAULT 0xD00DU

/**
 * VDDIO UV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VDDIO_UV_FS_IMPACT_MASK 0x0003U
/**
 * VDDIO OV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VDDIO_OV_FS_IMPACT_MASK 0x000CU
/**
 * VMON4 ABIST2 configuration
 */
#define FS8X_FS_I_VMON4_ABIST2_MASK 0x0020U
/**
 * VMON3 ABIST2 configuration
 */
#define FS8X_FS_I_VMON3_ABIST2_MASK 0x0040U
/**
 * VMON2 ABIST2 configuration
 */
#define FS8X_FS_I_VMON2_ABIST2_MASK 0x0080U
/**
 * VMON1 ABIST2 configuration
 */
#define FS8X_FS_I_VMON1_ABIST2_MASK 0x0100U
/**
 * VDDIO ABIST2 configuration
 */
#define FS8X_FS_I_VDDIO_ABIST2_MASK 0x0200U
/**
 * VCOREMON ABIST2 configuration
 */
#define FS8X_FS_I_VCOREMON_ABIST2_MASK 0x0400U
/**
 * VCOREMON UV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VCOREMON_UV_FS_IMPACT_MASK 0x3000U
/**
 * VCOREMON OV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VCOREMON_OV_FS_IMPACT_MASK 0xC000U

/**
 * VDDIO UV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VDDIO_UV_FS_IMPACT_SHIFT 0x0000U
/**
 * VDDIO OV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VDDIO_OV_FS_IMPACT_SHIFT 0x0002U
/**
 * VMON4 ABIST2 configuration
 */
#define FS8X_FS_I_VMON4_ABIST2_SHIFT 0x0005U
/**
 * VMON3 ABIST2 configuration
 */
#define FS8X_FS_I_VMON3_ABIST2_SHIFT 0x0006U
/**
 * VMON2 ABIST2 configuration
 */
#define FS8X_FS_I_VMON2_ABIST2_SHIFT 0x0007U
/**
 * VMON1 ABIST2 configuration
 */
#define FS8X_FS_I_VMON1_ABIST2_SHIFT 0x0008U
/**
 * VDDIO ABIST2 configuration
 */
#define FS8X_FS_I_VDDIO_ABIST2_SHIFT 0x0009U
/**
 * VCOREMON ABIST2 configuration
 */
#define FS8X_FS_I_VCOREMON_ABIST2_SHIFT 0x000AU
/**
 * VCOREMON UV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VCOREMON_UV_FS_IMPACT_SHIFT 0x000CU
/**
 * VCOREMON OV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VCOREMON_OV_FS_IMPACT_SHIFT 0x000EU

/**
 * No effect on RSTB and FS0B
 */
#define FS8X_FS_I_VDDIO_UV_FS_IMPACT_NO_EFFECT (0x0000U << FS8X_FS_I_VDDIO_UV_FS_IMPACT_SHIFT)
/**
 * FS0B only is asserted
 */
#define FS8X_FS_I_VDDIO_UV_FS_IMPACT_FS0B (0x0001U << FS8X_FS_I_VDDIO_UV_FS_IMPACT_SHIFT)
/**
 * FS0B and RSTB are asserted
 */
#define FS8X_FS_I_VDDIO_UV_FS_IMPACT_FS0B_RSTB (0x0002U << FS8X_FS_I_VDDIO_UV_FS_IMPACT_SHIFT)

/**
 * No effect on RSTB and FS0B
 */
#define FS8X_FS_I_VDDIO_OV_FS_IMPACT_NO_EFFECT (0x0000U << FS8X_FS_I_VDDIO_OV_FS_IMPACT_SHIFT)
/**
 * FS0B only is asserted
 */
#define FS8X_FS_I_VDDIO_OV_FS_IMPACT_FS0B (0x0001U << FS8X_FS_I_VDDIO_OV_FS_IMPACT_SHIFT)
/**
 * FS0B and RSTB are asserted
 */
#define FS8X_FS_I_VDDIO_OV_FS_IMPACT_FS0B_RSTB (0x0002U << FS8X_FS_I_VDDIO_OV_FS_IMPACT_SHIFT)

/**
 * No ABIST
 */
#define FS8X_FS_I_VMON4_ABIST2_NO_ABIST (0x0000U << FS8X_FS_I_VMON4_ABIST2_SHIFT)
/**
 * VMON4 BIST executed during ABIST2
 */
#define FS8X_FS_I_VMON4_ABIST2_VMON4_BIST (0x0001U << FS8X_FS_I_VMON4_ABIST2_SHIFT)

/**
 * No ABIST
 */
#define FS8X_FS_I_VMON3_ABIST2_NO_ABIST (0x0000U << FS8X_FS_I_VMON3_ABIST2_SHIFT)
/**
 * VMON3 BIST executed during ABIST2
 */
#define FS8X_FS_I_VMON3_ABIST2_VMON3_BIST (0x0001U << FS8X_FS_I_VMON3_ABIST2_SHIFT)

/**
 * No ABIST
 */
#define FS8X_FS_I_VMON2_ABIST2_NO_ABIST (0x0000U << FS8X_FS_I_VMON2_ABIST2_SHIFT)
/**
 * VMON2 BIST executed during ABIST2
 */
#define FS8X_FS_I_VMON2_ABIST2_VMON2_BIST (0x0001U << FS8X_FS_I_VMON2_ABIST2_SHIFT)

/**
 * No ABIST
 */
#define FS8X_FS_I_VMON1_ABIST2_NO_ABIST (0x0000U << FS8X_FS_I_VMON1_ABIST2_SHIFT)
/**
 * VMON1 BIST executed during ABIST2
 */
#define FS8X_FS_I_VMON1_ABIST2_VMON1_BIST (0x0001U << FS8X_FS_I_VMON1_ABIST2_SHIFT)

/**
 * No ABIST
 */
#define FS8X_FS_I_VDDIO_ABIST2_NO_ABIST (0x0000U << FS8X_FS_I_VDDIO_ABIST2_SHIFT)
/**
 * VDDIO BIST executed during ABIST2
 */
#define FS8X_FS_I_VDDIO_ABIST2_VDDIO_BIST (0x0001U << FS8X_FS_I_VDDIO_ABIST2_SHIFT)

/**
 * No ABIST
 */
#define FS8X_FS_I_VCOREMON_ABIST2_NO_ABIST (0x0000U << FS8X_FS_I_VCOREMON_ABIST2_SHIFT)
/**
 * VCOREMON BIST executed during ABIST2
 */
#define FS8X_FS_I_VCOREMON_ABIST2_VCOREMON_BIST (0x0001U << FS8X_FS_I_VCOREMON_ABIST2_SHIFT)

/**
 * No effect on RSTB and FS0B
 */
#define FS8X_FS_I_VCOREMON_UV_FS_IMPACT_NO_EFFECT (0x0000U << FS8X_FS_I_VCOREMON_UV_FS_IMPACT_SHIFT)
/**
 * FS0B only is asserted
 */
#define FS8X_FS_I_VCOREMON_UV_FS_IMPACT_FS0B (0x0001U << FS8X_FS_I_VCOREMON_UV_FS_IMPACT_SHIFT)
/**
 * FS0B and RSTB are asserted
 */
#define FS8X_FS_I_VCOREMON_UV_FS_IMPACT_FS0B_RSTB (0x0002U << FS8X_FS_I_VCOREMON_UV_FS_IMPACT_SHIFT)

/**
 * No effect on RSTB and FS0B
 */
#define FS8X_FS_I_VCOREMON_OV_FS_IMPACT_NO_EFFECT (0x0000U << FS8X_FS_I_VCOREMON_OV_FS_IMPACT_SHIFT)
/**
 * FS0B only is asserted
 */
#define FS8X_FS_I_VCOREMON_OV_FS_IMPACT_FS0B (0x0001U << FS8X_FS_I_VCOREMON_OV_FS_IMPACT_SHIFT)
/**
 * FS0B and RSTB are asserted
 */
#define FS8X_FS_I_VCOREMON_OV_FS_IMPACT_FS0B_RSTB (0x0002U << FS8X_FS_I_VCOREMON_OV_FS_IMPACT_SHIFT)

/******************************************************************************/
/* FS_I_OVUV_SAFE_REACTION2 - Type: RW                                        */
/******************************************************************************/

#define FS8X_FS_I_OVUV_SAFE_REACTION2_ADDR    0x03U
#define FS8X_FS_I_OVUV_SAFE_REACTION2_DEFAULT 0xDDDDU

/**
 * VMON1 UV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VMON1_UV_FS_IMPACT_MASK 0x0003U
/**
 * VMON1 OV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VMON1_OV_FS_IMPACT_MASK 0x000CU
/**
 * VMON2 UV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VMON2_UV_FS_IMPACT_MASK 0x0030U
/**
 * VMON2 OV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VMON2_OV_FS_IMPACT_MASK 0x00C0U
/**
 * VMON3 UV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VMON3_UV_FS_IMPACT_MASK 0x0300U
/**
 * VMON3 OV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VMON3_OV_FS_IMPACT_MASK 0x0C00U
/**
 * VMON4 UV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VMON4_UV_FS_IMPACT_MASK 0x3000U
/**
 * VMON4 OV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VMON4_OV_FS_IMPACT_MASK 0xC000U

/**
 * VMON1 UV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VMON1_UV_FS_IMPACT_SHIFT 0x0000U
/**
 * VMON1 OV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VMON1_OV_FS_IMPACT_SHIFT 0x0002U
/**
 * VMON2 UV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VMON2_UV_FS_IMPACT_SHIFT 0x0004U
/**
 * VMON2 OV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VMON2_OV_FS_IMPACT_SHIFT 0x0006U
/**
 * VMON3 UV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VMON3_UV_FS_IMPACT_SHIFT 0x0008U
/**
 * VMON3 OV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VMON3_OV_FS_IMPACT_SHIFT 0x000AU
/**
 * VMON4 UV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VMON4_UV_FS_IMPACT_SHIFT 0x000CU
/**
 * VMON4 OV impact on RSTB/FS0B
 */
#define FS8X_FS_I_VMON4_OV_FS_IMPACT_SHIFT 0x000EU

/**
 * No effect on RSTB and FS0B
 */
#define FS8X_FS_I_VMON1_UV_FS_IMPACT_NO_EFFECT (0x0000U << FS8X_FS_I_VMON1_UV_FS_IMPACT_SHIFT)
/**
 * FS0B only is asserted
 */
#define FS8X_FS_I_VMON1_UV_FS_IMPACT_FS0B (0x0001U << FS8X_FS_I_VMON1_UV_FS_IMPACT_SHIFT)
/**
 * FS0B and RSTB are asserted
 */
#define FS8X_FS_I_VMON1_UV_FS_IMPACT_FS0B_RSTB (0x0002U << FS8X_FS_I_VMON1_UV_FS_IMPACT_SHIFT)

/**
 * No effect on RSTB and FS0B
 */
#define FS8X_FS_I_VMON1_OV_FS_IMPACT_NO_EFFECT (0x0000U << FS8X_FS_I_VMON1_OV_FS_IMPACT_SHIFT)
/**
 * FS0B only is asserted
 */
#define FS8X_FS_I_VMON1_OV_FS_IMPACT_FS0B (0x0001U << FS8X_FS_I_VMON1_OV_FS_IMPACT_SHIFT)
/**
 * FS0B and RSTB are asserted
 */
#define FS8X_FS_I_VMON1_OV_FS_IMPACT_FS0B_RSTB (0x0002U << FS8X_FS_I_VMON1_OV_FS_IMPACT_SHIFT)

/**
 * No effect on RSTB and FS0B
 */
#define FS8X_FS_I_VMON2_UV_FS_IMPACT_NO_EFFECT (0x0000U << FS8X_FS_I_VMON2_UV_FS_IMPACT_SHIFT)
/**
 * FS0B only is asserted
 */
#define FS8X_FS_I_VMON2_UV_FS_IMPACT_FS0B (0x0001U << FS8X_FS_I_VMON2_UV_FS_IMPACT_SHIFT)
/**
 * FS0B and RSTB are asserted
 */
#define FS8X_FS_I_VMON2_UV_FS_IMPACT_FS0B_RSTB (0x0002U << FS8X_FS_I_VMON2_UV_FS_IMPACT_SHIFT)

/**
 * No effect on RSTB and FS0B
 */
#define FS8X_FS_I_VMON2_OV_FS_IMPACT_NO_EFFECT (0x0000U << FS8X_FS_I_VMON2_OV_FS_IMPACT_SHIFT)
/**
 * FS0B only is asserted
 */
#define FS8X_FS_I_VMON2_OV_FS_IMPACT_FS0B (0x0001U << FS8X_FS_I_VMON2_OV_FS_IMPACT_SHIFT)
/**
 * FS0B and RSTB are asserted
 */
#define FS8X_FS_I_VMON2_OV_FS_IMPACT_FS0B_RSTB (0x0002U << FS8X_FS_I_VMON2_OV_FS_IMPACT_SHIFT)

/**
 * No effect on RSTB and FS0B
 */
#define FS8X_FS_I_VMON3_UV_FS_IMPACT_NO_EFFECT (0x0000U << FS8X_FS_I_VMON3_UV_FS_IMPACT_SHIFT)
/**
 * FS0B only is asserted
 */
#define FS8X_FS_I_VMON3_UV_FS_IMPACT_FS0B (0x0001U << FS8X_FS_I_VMON3_UV_FS_IMPACT_SHIFT)
/**
 * FS0B and RSTB are asserted
 */
#define FS8X_FS_I_VMON3_UV_FS_IMPACT_FS0B_RSTB (0x0002U << FS8X_FS_I_VMON3_UV_FS_IMPACT_SHIFT)

/**
 * No effect on RSTB and FS0B
 */
#define FS8X_FS_I_VMON3_OV_FS_IMPACT_NO_EFFECT (0x0000U << FS8X_FS_I_VMON3_OV_FS_IMPACT_SHIFT)
/**
 * FS0B only is asserted
 */
#define FS8X_FS_I_VMON3_OV_FS_IMPACT_FS0B (0x0001U << FS8X_FS_I_VMON3_OV_FS_IMPACT_SHIFT)
/**
 * FS0B and RSTB are asserted
 */
#define FS8X_FS_I_VMON3_OV_FS_IMPACT_FS0B_RSTB (0x0002U << FS8X_FS_I_VMON3_OV_FS_IMPACT_SHIFT)

/**
 * No effect on RSTB and FS0B
 */
#define FS8X_FS_I_VMON4_UV_FS_IMPACT_NO_EFFECT (0x0000U << FS8X_FS_I_VMON4_UV_FS_IMPACT_SHIFT)
/**
 * FS0B only is asserted
 */
#define FS8X_FS_I_VMON4_UV_FS_IMPACT_FS0B (0x0001U << FS8X_FS_I_VMON4_UV_FS_IMPACT_SHIFT)
/**
 * FS0B and RSTB are asserted
 */
#define FS8X_FS_I_VMON4_UV_FS_IMPACT_FS0B_RSTB (0x0002U << FS8X_FS_I_VMON4_UV_FS_IMPACT_SHIFT)

/**
 * No effect on RSTB and FS0B
 */
#define FS8X_FS_I_VMON4_OV_FS_IMPACT_NO_EFFECT (0x0000U << FS8X_FS_I_VMON4_OV_FS_IMPACT_SHIFT)
/**
 * FS0B only is asserted
 */
#define FS8X_FS_I_VMON4_OV_FS_IMPACT_FS0B (0x0001U << FS8X_FS_I_VMON4_OV_FS_IMPACT_SHIFT)
/**
 * FS0B and RSTB are asserted
 */
#define FS8X_FS_I_VMON4_OV_FS_IMPACT_FS0B_RSTB (0x0002U << FS8X_FS_I_VMON4_OV_FS_IMPACT_SHIFT)

/******************************************************************************/
/* FS_I_WD_CFG - Type: RW                                                     */
/******************************************************************************/

#define FS8X_FS_I_WD_CFG_ADDR    0x05U
#define FS8X_FS_I_WD_CFG_DEFAULT 0x4200U

/**
 * Reflect the value of the Watchdog Error Counter.
 */
#define FS8X_FS_I_WD_ERR_CNT_MASK 0x000FU
/**
 * Reflect the value of the Watchdog Refresh Counter
 */
#define FS8X_FS_I_WD_RFR_CNT_MASK 0x0070U
/**
 * Watchdog Error Impact on RSTB/FS0B
 */
#define FS8X_FS_I_WD_FS_IMPACT_MASK 0x0300U
/**
 * Watchdog Refresh Counter value
 */
#define FS8X_FS_I_WD_RFR_LIMIT_MASK 0x1800U
/**
 * Watchdog Error Counter value
 */
#define FS8X_FS_I_WD_ERR_LIMIT_MASK 0xC000U

/**
 * Reflect the value of the Watchdog Error Counter.
 */
#define FS8X_FS_I_WD_ERR_CNT_SHIFT 0x0000U
/**
 * Reflect the value of the Watchdog Refresh Counter
 */
#define FS8X_FS_I_WD_RFR_CNT_SHIFT 0x0004U
/**
 * Watchdog Error Impact on RSTB/FS0B
 */
#define FS8X_FS_I_WD_FS_IMPACT_SHIFT 0x0008U
/**
 * Watchdog Refresh Counter value
 */
#define FS8X_FS_I_WD_RFR_LIMIT_SHIFT 0x000BU
/**
 * Watchdog Error Counter value
 */
#define FS8X_FS_I_WD_ERR_LIMIT_SHIFT 0x000EU

/**
 * 0
 */
#define FS8X_FS_I_WD_ERR_CNT_0 (0x0000U << FS8X_FS_I_WD_ERR_CNT_SHIFT)
/**
 * 1
 */
#define FS8X_FS_I_WD_ERR_CNT_1 (0x0001U << FS8X_FS_I_WD_ERR_CNT_SHIFT)
/**
 * 2
 */
#define FS8X_FS_I_WD_ERR_CNT_2 (0x0002U << FS8X_FS_I_WD_ERR_CNT_SHIFT)
/**
 * 3
 */
#define FS8X_FS_I_WD_ERR_CNT_3 (0x0003U << FS8X_FS_I_WD_ERR_CNT_SHIFT)
/**
 * 4
 */
#define FS8X_FS_I_WD_ERR_CNT_4 (0x0004U << FS8X_FS_I_WD_ERR_CNT_SHIFT)
/**
 * 5
 */
#define FS8X_FS_I_WD_ERR_CNT_5 (0x0005U << FS8X_FS_I_WD_ERR_CNT_SHIFT)
/**
 * 6
 */
#define FS8X_FS_I_WD_ERR_CNT_6 (0x0006U << FS8X_FS_I_WD_ERR_CNT_SHIFT)
/**
 * 7
 */
#define FS8X_FS_I_WD_ERR_CNT_7 (0x0007U << FS8X_FS_I_WD_ERR_CNT_SHIFT)
/**
 * 8
 */
#define FS8X_FS_I_WD_ERR_CNT_8 (0x0008U << FS8X_FS_I_WD_ERR_CNT_SHIFT)
/**
 * 9
 */
#define FS8X_FS_I_WD_ERR_CNT_9 (0x0009U << FS8X_FS_I_WD_ERR_CNT_SHIFT)
/**
 * 10
 */
#define FS8X_FS_I_WD_ERR_CNT_10 (0x000AU << FS8X_FS_I_WD_ERR_CNT_SHIFT)
/**
 * 11
 */
#define FS8X_FS_I_WD_ERR_CNT_11 (0x000BU << FS8X_FS_I_WD_ERR_CNT_SHIFT)
/**
 * 12
 */
#define FS8X_FS_I_WD_ERR_CNT_12 (0x000CU << FS8X_FS_I_WD_ERR_CNT_SHIFT)

/**
 * 0
 */
#define FS8X_FS_I_WD_RFR_CNT_0 (0x0000U << FS8X_FS_I_WD_RFR_CNT_SHIFT)
/**
 * 1
 */
#define FS8X_FS_I_WD_RFR_CNT_1 (0x0001U << FS8X_FS_I_WD_RFR_CNT_SHIFT)
/**
 * 2
 */
#define FS8X_FS_I_WD_RFR_CNT_2 (0x0002U << FS8X_FS_I_WD_RFR_CNT_SHIFT)
/**
 * 3
 */
#define FS8X_FS_I_WD_RFR_CNT_3 (0x0003U << FS8X_FS_I_WD_RFR_CNT_SHIFT)
/**
 * 4
 */
#define FS8X_FS_I_WD_RFR_CNT_4 (0x0004U << FS8X_FS_I_WD_RFR_CNT_SHIFT)
/**
 * 5
 */
#define FS8X_FS_I_WD_RFR_CNT_5 (0x0005U << FS8X_FS_I_WD_RFR_CNT_SHIFT)
/**
 * 6
 */
#define FS8X_FS_I_WD_RFR_CNT_6 (0x0006U << FS8X_FS_I_WD_RFR_CNT_SHIFT)
/**
 * 7
 */
#define FS8X_FS_I_WD_RFR_CNT_7 (0x0007U << FS8X_FS_I_WD_RFR_CNT_SHIFT)

/**
 * No action on RSTB and FS0B
 */
#define FS8X_FS_I_WD_FS_IMPACT_NO_ACTION (0x0000U << FS8X_FS_I_WD_FS_IMPACT_SHIFT)
/**
 * FS0B only is asserted if WD error counter = WD_ERR_LIMIT[1:0]
 */
#define FS8X_FS_I_WD_FS_IMPACT_FS0B (0x0001U << FS8X_FS_I_WD_FS_IMPACT_SHIFT)
/**
 * FS0B and RSTB are asserted if WD error counter = WD_ERR_LIMIT[1:0]
 */
#define FS8X_FS_I_WD_FS_IMPACT_FS0B_RSTB (0x0002U << FS8X_FS_I_WD_FS_IMPACT_SHIFT)

/**
 * 6
 */
#define FS8X_FS_I_WD_RFR_LIMIT_6 (0x0000U << FS8X_FS_I_WD_RFR_LIMIT_SHIFT)
/**
 * 4
 */
#define FS8X_FS_I_WD_RFR_LIMIT_4 (0x0001U << FS8X_FS_I_WD_RFR_LIMIT_SHIFT)
/**
 * 2
 */
#define FS8X_FS_I_WD_RFR_LIMIT_2 (0x0002U << FS8X_FS_I_WD_RFR_LIMIT_SHIFT)
/**
 * 1
 */
#define FS8X_FS_I_WD_RFR_LIMIT_1 (0x0003U << FS8X_FS_I_WD_RFR_LIMIT_SHIFT)

/**
 * 8
 */
#define FS8X_FS_I_WD_ERR_LIMIT_8 (0x0000U << FS8X_FS_I_WD_ERR_LIMIT_SHIFT)
/**
 * 6
 */
#define FS8X_FS_I_WD_ERR_LIMIT_6 (0x0001U << FS8X_FS_I_WD_ERR_LIMIT_SHIFT)
/**
 * 4
 */
#define FS8X_FS_I_WD_ERR_LIMIT_4 (0x0002U << FS8X_FS_I_WD_ERR_LIMIT_SHIFT)
/**
 * 2
 */
#define FS8X_FS_I_WD_ERR_LIMIT_2 (0x0003U << FS8X_FS_I_WD_ERR_LIMIT_SHIFT)

/******************************************************************************/
/* FS_I_SAFE_INPUTS - Type: RW                                                */
/******************************************************************************/

#define FS8X_FS_I_SAFE_INPUTS_ADDR    0x07U
#define FS8X_FS_I_SAFE_INPUTS_DEFAULT 0x41C6U

/**
 * ERRMON impact on RSTB/FS0B
 */
#define FS8X_FS_I_ERRMON_FS_IMPACT_MASK 0x0002U
/**
 * ERRMON acknowledgement timing
 */
#define FS8X_FS_I_ERRMON_ACK_TIME_MASK 0x000CU
/**
 * ERRMON polarity
 */
#define FS8X_FS_I_ERRMON_FLT_POLARITY_MASK 0x0010U
/**
 * Reaction on RSTB or FAIL SAFE output in case of FAULT DETECTION ON FCCU2
 */
#define FS8X_FS_I_FCCU2_FS_REACT_MASK 0x0040U
/**
 * Reaction on RSTB or FAIL SAFE output in case of FAULT DETECTION ON FCCU1
 */
#define FS8X_FS_I_FCCU1_FS_REACT_MASK 0x0080U
/**
 * FCCU12 impact on RSTB/FS0B
 */
#define FS8X_FS_I_FCCU12_FS_IMPACT_MASK 0x0100U
/**
 * FCCU2 polarity
 */
#define FS8X_FS_I_FCCU2_FLT_POL_MASK 0x0400U
/**
 * FCCU1 polarity
 */
#define FS8X_FS_I_FCCU1_FLT_POL_MASK 0x0800U
/**
 * FCCU12 polarity
 */
#define FS8X_FS_I_FCCU12_FLT_POL_MASK 0x1000U
/**
 * FCCU pins configuration
 */
#define FS8X_FS_I_FCCU_CFG_MASK 0xC000U

/**
 * ERRMON impact on RSTB/FS0B
 */
#define FS8X_FS_I_ERRMON_FS_IMPACT_SHIFT 0x0001U
/**
 * ERRMON acknowledgement timing
 */
#define FS8X_FS_I_ERRMON_ACK_TIME_SHIFT 0x0002U
/**
 * ERRMON polarity
 */
#define FS8X_FS_I_ERRMON_FLT_POLARITY_SHIFT 0x0004U
/**
 * Reaction on RSTB or FAIL SAFE output in case of FAULT DETECTION ON FCCU2
 */
#define FS8X_FS_I_FCCU2_FS_REACT_SHIFT 0x0006U
/**
 * Reaction on RSTB or FAIL SAFE output in case of FAULT DETECTION ON FCCU1
 */
#define FS8X_FS_I_FCCU1_FS_REACT_SHIFT 0x0007U
/**
 * FCCU12 impact on RSTB/FS0B
 */
#define FS8X_FS_I_FCCU12_FS_IMPACT_SHIFT 0x0008U
/**
 * FCCU2 polarity
 */
#define FS8X_FS_I_FCCU2_FLT_POL_SHIFT 0x000AU
/**
 * FCCU1 polarity
 */
#define FS8X_FS_I_FCCU1_FLT_POL_SHIFT 0x000BU
/**
 * FCCU12 polarity
 */
#define FS8X_FS_I_FCCU12_FLT_POL_SHIFT 0x000CU
/**
 * FCCU pins configuration
 */
#define FS8X_FS_I_FCCU_CFG_SHIFT 0x000EU

/**
 * FS0B only is asserted when ERRMON fault is detected
 */
#define FS8X_FS_I_ERRMON_FS_IMPACT_FS0B (0x0000U << FS8X_FS_I_ERRMON_FS_IMPACT_SHIFT)
/**
 * FS0B and RSTB are asserted when ERRMON fault is detected
 */
#define FS8X_FS_I_ERRMON_FS_IMPACT_FS0B_RSTB (0x0001U << FS8X_FS_I_ERRMON_FS_IMPACT_SHIFT)

/**
 * 1ms
 */
#define FS8X_FS_I_ERRMON_ACK_TIME_1MS (0x0000U << FS8X_FS_I_ERRMON_ACK_TIME_SHIFT)
/**
 * 8ms
 */
#define FS8X_FS_I_ERRMON_ACK_TIME_8MS (0x0001U << FS8X_FS_I_ERRMON_ACK_TIME_SHIFT)
/**
 * 16ms
 */
#define FS8X_FS_I_ERRMON_ACK_TIME_16MS (0x0002U << FS8X_FS_I_ERRMON_ACK_TIME_SHIFT)
/**
 * 32ms
 */
#define FS8X_FS_I_ERRMON_ACK_TIME_32MS (0x0003U << FS8X_FS_I_ERRMON_ACK_TIME_SHIFT)

/**
 * LOW LEVEL IS A FAULT after a negative edge transition
 */
#define FS8X_FS_I_ERRMON_FLT_POLARITY_NEGATIVE_EDGE (0x0000U << FS8X_FS_I_ERRMON_FLT_POLARITY_SHIFT)
/**
 * HIGH LEVEL IS A FAULT after a positive  edge transition
 */
#define FS8X_FS_I_ERRMON_FLT_POLARITY_POSITIVE_EDGE (0x0001U << FS8X_FS_I_ERRMON_FLT_POLARITY_SHIFT)

/**
 * FS0B only is asserted
 */
#define FS8X_FS_I_FCCU2_FS_REACT_FS0B (0x0000U << FS8X_FS_I_FCCU2_FS_REACT_SHIFT)
/**
 * FS0B and RSTB are asserted
 */
#define FS8X_FS_I_FCCU2_FS_REACT_FS0B_RSTB (0x0001U << FS8X_FS_I_FCCU2_FS_REACT_SHIFT)

/**
 * FS0B only is asserted
 */
#define FS8X_FS_I_FCCU1_FS_REACT_FS0B (0x0000U << FS8X_FS_I_FCCU1_FS_REACT_SHIFT)
/**
 * FS0B and RSTB are asserted
 */
#define FS8X_FS_I_FCCU1_FS_REACT_FS0B_RSTB (0x0001U << FS8X_FS_I_FCCU1_FS_REACT_SHIFT)

/**
 * FS0B only is asserted
 */
#define FS8X_FS_I_FCCU12_FS_IMPACT_FS0B (0x0000U << FS8X_FS_I_FCCU12_FS_IMPACT_SHIFT)
/**
 * FS0B and RSTB are asserted
 */
#define FS8X_FS_I_FCCU12_FS_IMPACT_FS0B_RSTB (0x0001U << FS8X_FS_I_FCCU12_FS_IMPACT_SHIFT)

/**
 * FCCU2 low level is a fault
 */
#define FS8X_FS_I_FCCU2_FLT_POL_FCCU2_L (0x0000U << FS8X_FS_I_FCCU2_FLT_POL_SHIFT)
/**
 * FCCU2 hign level is a fault
 */
#define FS8X_FS_I_FCCU2_FLT_POL_FCCU2_H (0x0001U << FS8X_FS_I_FCCU2_FLT_POL_SHIFT)

/**
 * FCCU1 low level is a fault
 */
#define FS8X_FS_I_FCCU1_FLT_POL_FCCU1_L (0x0000U << FS8X_FS_I_FCCU1_FLT_POL_SHIFT)
/**
 * FCCU1 high level is a fault
 */
#define FS8X_FS_I_FCCU1_FLT_POL_FCCU1_H (0x0001U << FS8X_FS_I_FCCU1_FLT_POL_SHIFT)

/**
 * FCCU1=0 or FCCU2=1 level is a fault
 */
#define FS8X_FS_I_FCCU12_FLT_POL_FCCU1_L_FCCU2_H (0x0000U << FS8X_FS_I_FCCU12_FLT_POL_SHIFT)
/**
 * FCCU1=1 or FCCU2=0 level is a fault
 */
#define FS8X_FS_I_FCCU12_FLT_POL_FCCU1_H_FCCU2_L (0x0001U << FS8X_FS_I_FCCU12_FLT_POL_SHIFT)

/**
 * No monitoring
 */
#define FS8X_FS_I_FCCU_CFG_NO_MONITORING (0x0000U << FS8X_FS_I_FCCU_CFG_SHIFT)
/**
 * FCCU1 and FCCU2 monitoring by pair (bi-stable protocol)
 */
#define FS8X_FS_I_FCCU_CFG_FCCU1_FCCU2_PAIR (0x0001U << FS8X_FS_I_FCCU_CFG_SHIFT)
/**
 * FCCU1 or FCCU2 input monitoring
 */
#define FS8X_FS_I_FCCU_CFG_FCCU1_FCCU2_INPUT (0x0002U << FS8X_FS_I_FCCU_CFG_SHIFT)
/**
 * FCCU1 input monitoring only
 */
#define FS8X_FS_I_FCCU_CFG_FCCU1_INPUT (0x0003U << FS8X_FS_I_FCCU_CFG_SHIFT)

/******************************************************************************/
/* FS_I_FSSM - Type: RW                                                       */
/******************************************************************************/

#define FS8X_FS_I_FSSM_ADDR    0x09U
#define FS8X_FS_I_FSSM_DEFAULT 0x5180U

/**
 * Reflect the value of the Fault Error Counter
 */
#define FS8X_FS_I_FLT_ERR_CNT_MASK 0x000FU
/**
 * Disable 8S timer
 */
#define FS8X_FS_I_DIS_8S_MASK 0x0010U
/**
 * Disable Clock Monitoring
 */
#define FS8X_FS_I_CLK_MON_DIS_MASK 0x0020U
/**
 * Assert RSTB in case of a short to high detected on FS0B
 */
#define FS8X_FS_I_FS0B_SC_HIGH_CFG_MASK 0x0080U
/**
 * RSTB pulse duration configuration
 */
#define FS8X_FS_I_RSTB_DUR_MASK 0x0200U
/**
 * Fault Error Counter intermediate value impact on RSTB/FS0B
 */
#define FS8X_FS_I_FLT_ERR_IMPACT_MASK 0x1800U
/**
 * Fault Error Counter max value configuration
 */
#define FS8X_FS_I_FLT_ERR_CNT_LIMIT_MASK 0xC000U

/**
 * Reflect the value of the Fault Error Counter
 */
#define FS8X_FS_I_FLT_ERR_CNT_SHIFT 0x0000U
/**
 * Disable 8S timer
 */
#define FS8X_FS_I_DIS_8S_SHIFT 0x0004U
/**
 * Disable Clock Monitoring
 */
#define FS8X_FS_I_CLK_MON_DIS_SHIFT 0x0005U
/**
 * Assert RSTB in case of a short to high detected on FS0B
 */
#define FS8X_FS_I_FS0B_SC_HIGH_CFG_SHIFT 0x0007U
/**
 * RSTB pulse duration configuration
 */
#define FS8X_FS_I_RSTB_DUR_SHIFT 0x0009U
/**
 * Fault Error Counter intermediate value impact on RSTB/FS0B
 */
#define FS8X_FS_I_FLT_ERR_IMPACT_SHIFT 0x000BU
/**
 * Fault Error Counter max value configuration
 */
#define FS8X_FS_I_FLT_ERR_CNT_LIMIT_SHIFT 0x000EU

/**
 * 0
 */
#define FS8X_FS_I_FLT_ERR_CNT_0 (0x0000U << FS8X_FS_I_FLT_ERR_CNT_SHIFT)
/**
 * 1
 */
#define FS8X_FS_I_FLT_ERR_CNT_1 (0x0001U << FS8X_FS_I_FLT_ERR_CNT_SHIFT)
/**
 * 2
 */
#define FS8X_FS_I_FLT_ERR_CNT_2 (0x0002U << FS8X_FS_I_FLT_ERR_CNT_SHIFT)
/**
 * 3
 */
#define FS8X_FS_I_FLT_ERR_CNT_3 (0x0003U << FS8X_FS_I_FLT_ERR_CNT_SHIFT)
/**
 * 4
 */
#define FS8X_FS_I_FLT_ERR_CNT_4 (0x0004U << FS8X_FS_I_FLT_ERR_CNT_SHIFT)
/**
 * 5
 */
#define FS8X_FS_I_FLT_ERR_CNT_5 (0x0005U << FS8X_FS_I_FLT_ERR_CNT_SHIFT)
/**
 * 6
 */
#define FS8X_FS_I_FLT_ERR_CNT_6 (0x0006U << FS8X_FS_I_FLT_ERR_CNT_SHIFT)
/**
 * 7
 */
#define FS8X_FS_I_FLT_ERR_CNT_7 (0x0007U << FS8X_FS_I_FLT_ERR_CNT_SHIFT)
/**
 * 8
 */
#define FS8X_FS_I_FLT_ERR_CNT_8 (0x0008U << FS8X_FS_I_FLT_ERR_CNT_SHIFT)
/**
 * 9
 */
#define FS8X_FS_I_FLT_ERR_CNT_9 (0x0009U << FS8X_FS_I_FLT_ERR_CNT_SHIFT)
/**
 * 10
 */
#define FS8X_FS_I_FLT_ERR_CNT_10 (0x000AU << FS8X_FS_I_FLT_ERR_CNT_SHIFT)
/**
 * 11
 */
#define FS8X_FS_I_FLT_ERR_CNT_11 (0x000BU << FS8X_FS_I_FLT_ERR_CNT_SHIFT)
/**
 * 12
 */
#define FS8X_FS_I_FLT_ERR_CNT_12 (0x000CU << FS8X_FS_I_FLT_ERR_CNT_SHIFT)

/**
 * RSTB LOW 8s Counter enabled
 */
#define FS8X_FS_I_DIS_8S_COUNTER_ENABLED (0x0000U << FS8X_FS_I_DIS_8S_SHIFT)
/**
 * RSTB LOW 8s Counter disabled
 */
#define FS8X_FS_I_DIS_8S_COUNTER_DISABLED (0x0001U << FS8X_FS_I_DIS_8S_SHIFT)

/**
 * Clock Monitoring active
 */
#define FS8X_FS_I_CLK_MON_DIS_MONITORING_ACTIVE (0x0000U << FS8X_FS_I_CLK_MON_DIS_SHIFT)
/**
 * Clock Monitoring disabled
 */
#define FS8X_FS_I_CLK_MON_DIS_MONITORING_DISABLED (0x0001U << FS8X_FS_I_CLK_MON_DIS_SHIFT)

/**
 * No assertion of the RESET
 */
#define FS8X_FS_I_FS0B_SC_HIGH_CFG_NO_ASSERTION (0x0000U << FS8X_FS_I_FS0B_SC_HIGH_CFG_SHIFT)
/**
 * RESET asserted
 */
#define FS8X_FS_I_FS0B_SC_HIGH_CFG_RESET_ASSERTED (0x0001U << FS8X_FS_I_FS0B_SC_HIGH_CFG_SHIFT)

/**
 * 10 ms
 */
#define FS8X_FS_I_RSTB_DUR_10MS (0x0000U << FS8X_FS_I_RSTB_DUR_SHIFT)
/**
 * 1 ms
 */
#define FS8X_FS_I_RSTB_DUR_1MS (0x0001U << FS8X_FS_I_RSTB_DUR_SHIFT)

/**
 * No effect on RSTB and FS0B
 */
#define FS8X_FS_I_FLT_ERR_IMPACT_NO_EFFECT (0x0000U << FS8X_FS_I_FLT_ERR_IMPACT_SHIFT)
/**
 * FS0B only is asserted if FLT_ERR_CNT=intermediate value
 */
#define FS8X_FS_I_FLT_ERR_IMPACT_FS0B (0x0001U << FS8X_FS_I_FLT_ERR_IMPACT_SHIFT)
/**
 * FS0B and RSTB are asserted if FLT_ERR_CNT=intermediate value
 */
#define FS8X_FS_I_FLT_ERR_IMPACT_FS0B_RSTB (0x0002U << FS8X_FS_I_FLT_ERR_IMPACT_SHIFT)

/**
 * 2
 */
#define FS8X_FS_I_FLT_ERR_CNT_LIMIT_2 (0x0000U << FS8X_FS_I_FLT_ERR_CNT_LIMIT_SHIFT)
/**
 * 6
 */
#define FS8X_FS_I_FLT_ERR_CNT_LIMIT_6 (0x0001U << FS8X_FS_I_FLT_ERR_CNT_LIMIT_SHIFT)
/**
 * 8
 */
#define FS8X_FS_I_FLT_ERR_CNT_LIMIT_8 (0x0002U << FS8X_FS_I_FLT_ERR_CNT_LIMIT_SHIFT)
/**
 * 12
 */
#define FS8X_FS_I_FLT_ERR_CNT_LIMIT_12 (0x0003U << FS8X_FS_I_FLT_ERR_CNT_LIMIT_SHIFT)

/******************************************************************************/
/* FS_I_SVS - Type: RW                                                        */
/******************************************************************************/

#define FS8X_FS_I_SVS_ADDR    0x0BU
#define FS8X_FS_I_SVS_DEFAULT 0x0000U

/**
 * Static Voltage Scaling negative offset
 */
#define FS8X_FS_I_SVS_OFFSET_MASK 0xF800U

/**
 * Static Voltage Scaling negative offset
 */
#define FS8X_FS_I_SVS_OFFSET_SHIFT 0x000BU

/**
 * 0mV
 */
#define FS8X_FS_I_SVS_OFFSET_EXTERNAL_REGULATOR (0x0000U << FS8X_FS_I_SVS_OFFSET_SHIFT)
/**
 * -6.25mV
 */
#define FS8X_FS_I_SVS_OFFSET_M6_25MV (0x0001U << FS8X_FS_I_SVS_OFFSET_SHIFT)
/**
 * -12.50mV
 */
#define FS8X_FS_I_SVS_OFFSET_M12_50MV (0x0002U << FS8X_FS_I_SVS_OFFSET_SHIFT)
/**
 * -18.75mV
 */
#define FS8X_FS_I_SVS_OFFSET_M18_75MV (0x0003U << FS8X_FS_I_SVS_OFFSET_SHIFT)
/**
 * -25mV
 */
#define FS8X_FS_I_SVS_OFFSET_M25MV (0x0004U << FS8X_FS_I_SVS_OFFSET_SHIFT)
/**
 * -31.25mV
 */
#define FS8X_FS_I_SVS_OFFSET_M31_25MV (0x0005U << FS8X_FS_I_SVS_OFFSET_SHIFT)
/**
 * -37.5mV
 */
#define FS8X_FS_I_SVS_OFFSET_M37_5MV (0x0006U << FS8X_FS_I_SVS_OFFSET_SHIFT)
/**
 * -43.75mV
 */
#define FS8X_FS_I_SVS_OFFSET_M43_75MV (0x0007U << FS8X_FS_I_SVS_OFFSET_SHIFT)
/**
 * -50mV
 */
#define FS8X_FS_I_SVS_OFFSET_M50MV (0x0008U << FS8X_FS_I_SVS_OFFSET_SHIFT)
/**
 * -56.25mV
 */
#define FS8X_FS_I_SVS_OFFSET_M56_25MV (0x0009U << FS8X_FS_I_SVS_OFFSET_SHIFT)
/**
 * -62.5mV
 */
#define FS8X_FS_I_SVS_OFFSET_M62_5MV (0x000AU << FS8X_FS_I_SVS_OFFSET_SHIFT)
/**
 * -68.75mV
 */
#define FS8X_FS_I_SVS_OFFSET_M68_75MV (0x000BU << FS8X_FS_I_SVS_OFFSET_SHIFT)
/**
 * -75mV
 */
#define FS8X_FS_I_SVS_OFFSET_M75MV (0x000CU << FS8X_FS_I_SVS_OFFSET_SHIFT)
/**
 * -81.25mV
 */
#define FS8X_FS_I_SVS_OFFSET_M81_25MV (0x000DU << FS8X_FS_I_SVS_OFFSET_SHIFT)
/**
 * -87.5mV
 */
#define FS8X_FS_I_SVS_OFFSET_M87_5MV (0x000EU << FS8X_FS_I_SVS_OFFSET_SHIFT)
/**
 * -93.75mV
 */
#define FS8X_FS_I_SVS_OFFSET_M93_75MV (0x000FU << FS8X_FS_I_SVS_OFFSET_SHIFT)
/**
 * -100mV
 */
#define FS8X_FS_I_SVS_OFFSET_M100MV (0x0010U << FS8X_FS_I_SVS_OFFSET_SHIFT)

/******************************************************************************/
/* FS_GRL_FLAGS - Type: R                                                     */
/******************************************************************************/

#define FS8X_FS_GRL_FLAGS_ADDR    0x00U
#define FS8X_FS_GRL_FLAGS_DEFAULT 0x0000U

/**
 * Report an issue in one of the voltage monitoring (OV or UV)
 */
#define FS8X_FS_FS_REG_OVUV_G_MASK 0x1000U
/**
 * Report an issue in one of the Fail Safe IOs
 */
#define FS8X_FS_FS_IO_G_MASK 0x2000U
/**
 * Report an issue on the Watchdog refresh
 */
#define FS8X_FS_FS_WD_G_MASK 0x4000U
/**
 * Report an issue in the communication (SPI or I2C)
 */
#define FS8X_FS_FS_COM_G_MASK 0x8000U

/**
 * Report an issue in one of the voltage monitoring (OV or UV)
 */
#define FS8X_FS_FS_REG_OVUV_G_SHIFT 0x000CU
/**
 * Report an issue in one of the Fail Safe IOs
 */
#define FS8X_FS_FS_IO_G_SHIFT 0x000DU
/**
 * Report an issue on the Watchdog refresh
 */
#define FS8X_FS_FS_WD_G_SHIFT 0x000EU
/**
 * Report an issue in the communication (SPI or I2C)
 */
#define FS8X_FS_FS_COM_G_SHIFT 0x000FU

/**
 * No Failure
 */
#define FS8X_FS_FS_REG_OVUV_G_NO_FAILURE (0x0000U << FS8X_FS_FS_REG_OVUV_G_SHIFT)
/**
 * Failure
 */
#define FS8X_FS_FS_REG_OVUV_G_FAILURE (0x0001U << FS8X_FS_FS_REG_OVUV_G_SHIFT)

/**
 * No Failure
 */
#define FS8X_FS_FS_IO_G_NO_FAILURE (0x0000U << FS8X_FS_FS_IO_G_SHIFT)
/**
 * Failure
 */
#define FS8X_FS_FS_IO_G_FAILURE (0x0001U << FS8X_FS_FS_IO_G_SHIFT)

/**
 * Good WD Refresh
 */
#define FS8X_FS_FS_WD_G_GOOD_WD_REFRESH (0x0000U << FS8X_FS_FS_WD_G_SHIFT)
/**
 * Bad WD Refresh
 */
#define FS8X_FS_FS_WD_G_BAD_WD_REFRESH (0x0001U << FS8X_FS_FS_WD_G_SHIFT)

/**
 * No Failure
 */
#define FS8X_FS_FS_COM_G_NO_FAILURE (0x0000U << FS8X_FS_FS_COM_G_SHIFT)
/**
 * Failure
 */
#define FS8X_FS_FS_COM_G_FAILURE (0x0001U << FS8X_FS_FS_COM_G_SHIFT)

/******************************************************************************/
/* FS_WD_WINDOW - Type: RW                                                    */
/******************************************************************************/

#define FS8X_FS_WD_WINDOW_ADDR    0x0DU
#define FS8X_FS_WD_WINDOW_DEFAULT 0x3200U

/**
 * Watchdog Window Duration when the device is in Fault Recovery Strategy.
 */
#define FS8X_FS_WDW_RECOVERY_MASK 0x000FU
/**
 * CLOSED window
 */
#define FS8X_FS_WDW_DC_MASK 0x0700U
/**
 * Watchdog Window Duration
 */
#define FS8X_FS_WD_WINDOW_MASK 0xF000U

/**
 * Watchdog Window Duration when the device is in Fault Recovery Strategy.
 */
#define FS8X_FS_WDW_RECOVERY_SHIFT 0x0000U
/**
 * CLOSED window
 */
#define FS8X_FS_WDW_DC_SHIFT 0x0008U
/**
 * Watchdog Window Duration
 */
#define FS8X_FS_WD_WINDOW_SHIFT 0x000CU

/**
 * DISABLE
 */
#define FS8X_FS_WDW_RECOVERY_DISABLE (0x0000U << FS8X_FS_WDW_RECOVERY_SHIFT)
/**
 * 1.0ms
 */
#define FS8X_FS_WDW_RECOVERY_1MS (0x0001U << FS8X_FS_WDW_RECOVERY_SHIFT)
/**
 * 2.0ms
 */
#define FS8X_FS_WDW_RECOVERY_2MS (0x0002U << FS8X_FS_WDW_RECOVERY_SHIFT)
/**
 * 3.0ms
 */
#define FS8X_FS_WDW_RECOVERY_3MS (0x0003U << FS8X_FS_WDW_RECOVERY_SHIFT)
/**
 * 4.0ms
 */
#define FS8X_FS_WDW_RECOVERY_4MS (0x0004U << FS8X_FS_WDW_RECOVERY_SHIFT)
/**
 * 6.0ms
 */
#define FS8X_FS_WDW_RECOVERY_6MS (0x0005U << FS8X_FS_WDW_RECOVERY_SHIFT)
/**
 * 8.0ms
 */
#define FS8X_FS_WDW_RECOVERY_8MS (0x0006U << FS8X_FS_WDW_RECOVERY_SHIFT)
/**
 * 12ms
 */
#define FS8X_FS_WDW_RECOVERY_12MS (0x0007U << FS8X_FS_WDW_RECOVERY_SHIFT)
/**
 * 16ms
 */
#define FS8X_FS_WDW_RECOVERY_16MS (0x0008U << FS8X_FS_WDW_RECOVERY_SHIFT)
/**
 * 24ms
 */
#define FS8X_FS_WDW_RECOVERY_24MS (0x0009U << FS8X_FS_WDW_RECOVERY_SHIFT)
/**
 * 32ms
 */
#define FS8X_FS_WDW_RECOVERY_32MS (0x000AU << FS8X_FS_WDW_RECOVERY_SHIFT)
/**
 * 64ms
 */
#define FS8X_FS_WDW_RECOVERY_64MS (0x000BU << FS8X_FS_WDW_RECOVERY_SHIFT)
/**
 * 128ms
 */
#define FS8X_FS_WDW_RECOVERY_128MS (0x000CU << FS8X_FS_WDW_RECOVERY_SHIFT)
/**
 * 256ms
 */
#define FS8X_FS_WDW_RECOVERY_256MS (0x000DU << FS8X_FS_WDW_RECOVERY_SHIFT)
/**
 * 512ms
 */
#define FS8X_FS_WDW_RECOVERY_512MS (0x000EU << FS8X_FS_WDW_RECOVERY_SHIFT)
/**
 * 1024ms
 */
#define FS8X_FS_WDW_RECOVERY_1024MS (0x000FU << FS8X_FS_WDW_RECOVERY_SHIFT)

/**
 * 31.25%
 */
#define FS8X_FS_WDW_DC_31_25 (0x0000U << FS8X_FS_WDW_DC_SHIFT)
/**
 * 37.5%
 */
#define FS8X_FS_WDW_DC_37_5 (0x0001U << FS8X_FS_WDW_DC_SHIFT)
/**
 * 50%
 */
#define FS8X_FS_WDW_DC_50 (0x0002U << FS8X_FS_WDW_DC_SHIFT)
/**
 * 62.5%
 */
#define FS8X_FS_WDW_DC_62_5 (0x0003U << FS8X_FS_WDW_DC_SHIFT)
/**
 * 68.75%
 */
#define FS8X_FS_WDW_DC_68_75 (0x0004U << FS8X_FS_WDW_DC_SHIFT)

/**
 * DISABLE (during INIT_FS only)
 */
#define FS8X_FS_WD_WINDOW_DISABLE (0x0000U << FS8X_FS_WD_WINDOW_SHIFT)
/**
 * 1.0ms
 */
#define FS8X_FS_WD_WINDOW_1MS (0x0001U << FS8X_FS_WD_WINDOW_SHIFT)
/**
 * 2.0ms
 */
#define FS8X_FS_WD_WINDOW_2MS (0x0002U << FS8X_FS_WD_WINDOW_SHIFT)
/**
 * 3.0ms
 */
#define FS8X_FS_WD_WINDOW_3MS (0x0003U << FS8X_FS_WD_WINDOW_SHIFT)
/**
 * 4.0ms
 */
#define FS8X_FS_WD_WINDOW_4MS (0x0004U << FS8X_FS_WD_WINDOW_SHIFT)
/**
 * 6.0ms
 */
#define FS8X_FS_WD_WINDOW_6MS (0x0005U << FS8X_FS_WD_WINDOW_SHIFT)
/**
 * 8.0ms
 */
#define FS8X_FS_WD_WINDOW_8MS (0x0006U << FS8X_FS_WD_WINDOW_SHIFT)
/**
 * 12ms
 */
#define FS8X_FS_WD_WINDOW_12MS (0x0007U << FS8X_FS_WD_WINDOW_SHIFT)
/**
 * 16ms
 */
#define FS8X_FS_WD_WINDOW_16MS (0x0008U << FS8X_FS_WD_WINDOW_SHIFT)
/**
 * 24ms
 */
#define FS8X_FS_WD_WINDOW_24MS (0x0009U << FS8X_FS_WD_WINDOW_SHIFT)
/**
 * 32ms
 */
#define FS8X_FS_WD_WINDOW_32MS (0x000AU << FS8X_FS_WD_WINDOW_SHIFT)
/**
 * 64ms
 */
#define FS8X_FS_WD_WINDOW_64MS (0x000BU << FS8X_FS_WD_WINDOW_SHIFT)
/**
 * 128ms
 */
#define FS8X_FS_WD_WINDOW_128MS (0x000CU << FS8X_FS_WD_WINDOW_SHIFT)
/**
 * 256ms
 */
#define FS8X_FS_WD_WINDOW_256MS (0x000DU << FS8X_FS_WD_WINDOW_SHIFT)
/**
 * 512ms
 */
#define FS8X_FS_WD_WINDOW_512MS (0x000EU << FS8X_FS_WD_WINDOW_SHIFT)
/**
 * 1024ms
 */
#define FS8X_FS_WD_WINDOW_1024MS (0x000FU << FS8X_FS_WD_WINDOW_SHIFT)

/******************************************************************************/
/* FS_WD_SEED - Type: RW                                                      */
/******************************************************************************/

#define FS8X_FS_WD_SEED_ADDR    0x0FU
#define FS8X_FS_WD_SEED_DEFAULT 0x5AB2U

/**
 * Seed for the LFSR
 */
#define FS8X_FS_WD_SEED_MASK 0xFFFFU

/**
 * Seed for the LFSR
 */
#define FS8X_FS_WD_SEED_SHIFT 0x0000U

/******************************************************************************/
/* FS_WD_ANSWER - Type: RW                                                    */
/******************************************************************************/

#define FS8X_FS_WD_ANSWER_ADDR    0x10U
#define FS8X_FS_WD_ANSWER_DEFAULT 0x0000U

/**
 * WATCHDOG LFSR VALUE
 */
#define FS8X_FS_WD_ANSWER_MASK 0xFFFFU

/**
 * WATCHDOG LFSR VALUE
 */
#define FS8X_FS_WD_ANSWER_SHIFT 0x0000U

/******************************************************************************/
/* FS_OVUVREG_STATUS - Type: RW                                               */
/******************************************************************************/

#define FS8X_FS_OVUVREG_STATUS_ADDR    0x11U
#define FS8X_FS_OVUVREG_STATUS_DEFAULT 0x5550U

/**
 * Drift of the Fail Safe OSC
 */
#define FS8X_FS_FS_OSC_DRIFT_MASK 0x0002U
/**
 * Overvoltage of the Internal Digital Fail Safe reference voltage
 */
#define FS8X_FS_FS_DIG_REF_OV_MASK 0x0004U
/**
 * Undervoltage Monitoring on VMON1
 */
#define FS8X_FS_VMON1_UV_MASK 0x0010U
/**
 * Overvoltage Monitoring on VMON1
 */
#define FS8X_FS_VMON1_OV_MASK 0x0020U
/**
 * Undervoltage Monitoring on VMON2
 */
#define FS8X_FS_VMON2_UV_MASK 0x0040U
/**
 * Overvoltage Monitoring on VMON2
 */
#define FS8X_FS_VMON2_OV_MASK 0x0080U
/**
 * Undervoltage Monitoring on VMON3
 */
#define FS8X_FS_VMON3_UV_MASK 0x0100U
/**
 * Overvoltage Monitoring on VMON3
 */
#define FS8X_FS_VMON3_OV_MASK 0x0200U
/**
 * Undervoltage Monitoring on VMON4
 */
#define FS8X_FS_VMON4_UV_MASK 0x0400U
/**
 * Overvoltage Monitoring on VMON4
 */
#define FS8X_FS_VMON4_OV_MASK 0x0800U
/**
 * Undervoltage Monitoring on VDDIO
 */
#define FS8X_FS_VDDIO_UV_MASK 0x1000U
/**
 * Overvoltage Monitoring on VDDIO
 */
#define FS8X_FS_VDDIO_OV_MASK 0x2000U
/**
 * Undervoltage Monitoring on VCOREMON
 */
#define FS8X_FS_VCOREMON_UV_MASK 0x4000U
/**
 * Overvoltage Monitoring on VCOREMON
 */
#define FS8X_FS_VCOREMON_OV_MASK 0x8000U

/**
 * Drift of the Fail Safe OSC
 */
#define FS8X_FS_FS_OSC_DRIFT_SHIFT 0x0001U
/**
 * Overvoltage of the Internal Digital Fail Safe reference voltage
 */
#define FS8X_FS_FS_DIG_REF_OV_SHIFT 0x0002U
/**
 * Undervoltage Monitoring on VMON1
 */
#define FS8X_FS_VMON1_UV_SHIFT 0x0004U
/**
 * Overvoltage Monitoring on VMON1
 */
#define FS8X_FS_VMON1_OV_SHIFT 0x0005U
/**
 * Undervoltage Monitoring on VMON2
 */
#define FS8X_FS_VMON2_UV_SHIFT 0x0006U
/**
 * Overvoltage Monitoring on VMON2
 */
#define FS8X_FS_VMON2_OV_SHIFT 0x0007U
/**
 * Undervoltage Monitoring on VMON3
 */
#define FS8X_FS_VMON3_UV_SHIFT 0x0008U
/**
 * Overvoltage Monitoring on VMON3
 */
#define FS8X_FS_VMON3_OV_SHIFT 0x0009U
/**
 * Undervoltage Monitoring on VMON4
 */
#define FS8X_FS_VMON4_UV_SHIFT 0x000AU
/**
 * Overvoltage Monitoring on VMON4
 */
#define FS8X_FS_VMON4_OV_SHIFT 0x000BU
/**
 * Undervoltage Monitoring on VDDIO
 */
#define FS8X_FS_VDDIO_UV_SHIFT 0x000CU
/**
 * Overvoltage Monitoring on VDDIO
 */
#define FS8X_FS_VDDIO_OV_SHIFT 0x000DU
/**
 * Undervoltage Monitoring on VCOREMON
 */
#define FS8X_FS_VCOREMON_UV_SHIFT 0x000EU
/**
 * Overvoltage Monitoring on VCOREMON
 */
#define FS8X_FS_VCOREMON_OV_SHIFT 0x000FU

/**
 * No Drift
 */
#define FS8X_FS_FS_OSC_DRIFT_NO_DRIFT (0x0000U << FS8X_FS_FS_OSC_DRIFT_SHIFT)
/**
 * Oscillator Drift
 */
#define FS8X_FS_FS_OSC_DRIFT_OSCILLATOR_DRIFT (0x0001U << FS8X_FS_FS_OSC_DRIFT_SHIFT)

/**
 * No Overvoltage
 */
#define FS8X_FS_FS_DIG_REF_OV_NO_OVERVOLTAGE (0x0000U << FS8X_FS_FS_DIG_REF_OV_SHIFT)
/**
 * Overvoltage reported of the internal digital fail safe reference voltage
 */
#define FS8X_FS_FS_DIG_REF_OV_OVERVOLTAGE_REPORTED (0x0001U << FS8X_FS_FS_DIG_REF_OV_SHIFT)

/**
 * No Undervoltage
 */
#define FS8X_FS_VMON1_UV_NO_UNDERVOLTAGE (0x0000U << FS8X_FS_VMON1_UV_SHIFT)
/**
 * Undervoltage Reported on VMON1
 */
#define FS8X_FS_VMON1_UV_UNDERVOLTAGE_REPORTED (0x0001U << FS8X_FS_VMON1_UV_SHIFT)

/**
 * No Overvoltage
 */
#define FS8X_FS_VMON1_OV_NO_OVERVOLTAGE (0x0000U << FS8X_FS_VMON1_OV_SHIFT)
/**
 * Overvoltage Reported on VMON1
 */
#define FS8X_FS_VMON1_OV_OVERVOLTAGE_REPORTED (0x0001U << FS8X_FS_VMON1_OV_SHIFT)

/**
 * No Undervoltage
 */
#define FS8X_FS_VMON2_UV_NO_UNDERVOLTAGE (0x0000U << FS8X_FS_VMON2_UV_SHIFT)
/**
 * Undervoltage Reported on VMON2
 */
#define FS8X_FS_VMON2_UV_UNDERVOLTAGE_REPORTED (0x0001U << FS8X_FS_VMON2_UV_SHIFT)

/**
 * No Overvoltage
 */
#define FS8X_FS_VMON2_OV_NO_OVERVOLTAGE (0x0000U << FS8X_FS_VMON2_OV_SHIFT)
/**
 * Overvoltage Reported on VMON2
 */
#define FS8X_FS_VMON2_OV_OVERVOLTAGE_REPORTED (0x0001U << FS8X_FS_VMON2_OV_SHIFT)

/**
 * No Undervoltage
 */
#define FS8X_FS_VMON3_UV_NO_UNDERVOLTAGE (0x0000U << FS8X_FS_VMON3_UV_SHIFT)
/**
 * Undervoltage Reported on VMON3
 */
#define FS8X_FS_VMON3_UV_UNDERVOLTAGE_REPORTED (0x0001U << FS8X_FS_VMON3_UV_SHIFT)

/**
 * No Overvoltage
 */
#define FS8X_FS_VMON3_OV_NO_OVERVOLTAGE (0x0000U << FS8X_FS_VMON3_OV_SHIFT)
/**
 * Overvoltage Reported on VMON3
 */
#define FS8X_FS_VMON3_OV_OVERVOLTAGE_REPORTED (0x0001U << FS8X_FS_VMON3_OV_SHIFT)

/**
 * No Undervoltage
 */
#define FS8X_FS_VMON4_UV_NO_UNDERVOLTAGE (0x0000U << FS8X_FS_VMON4_UV_SHIFT)
/**
 * Undervoltage Reported on VMON4
 */
#define FS8X_FS_VMON4_UV_UNDERVOLTAGE_REPORTED (0x0001U << FS8X_FS_VMON4_UV_SHIFT)

/**
 * No Overvoltage
 */
#define FS8X_FS_VMON4_OV_NO_OVERVOLTAGE (0x0000U << FS8X_FS_VMON4_OV_SHIFT)
/**
 * Overvoltage Reported on VMON4
 */
#define FS8X_FS_VMON4_OV_OVERVOLTAGE_REPORTED (0x0001U << FS8X_FS_VMON4_OV_SHIFT)

/**
 * No Undervoltage
 */
#define FS8X_FS_VDDIO_UV_NO_UNDERVOLTAGE (0x0000U << FS8X_FS_VDDIO_UV_SHIFT)
/**
 * Undervoltage Reported on VDDIO
 */
#define FS8X_FS_VDDIO_UV_UNDERVOLTAGE_REPORTED (0x0001U << FS8X_FS_VDDIO_UV_SHIFT)

/**
 * No Overvoltage
 */
#define FS8X_FS_VDDIO_OV_NO_OVERVOLTAGE (0x0000U << FS8X_FS_VDDIO_OV_SHIFT)
/**
 * Overvoltage Reported on VDDIO
 */
#define FS8X_FS_VDDIO_OV_OVERVOLTAGE_REPORTED (0x0001U << FS8X_FS_VDDIO_OV_SHIFT)

/**
 * No Undervoltage
 */
#define FS8X_FS_VCOREMON_UV_NO_UNDERVOLTAGE (0x0000U << FS8X_FS_VCOREMON_UV_SHIFT)
/**
 * Undervoltage Reported on VCOREMON
 */
#define FS8X_FS_VCOREMON_UV_UNDERVOLTAGE_REPORTED (0x0001U << FS8X_FS_VCOREMON_UV_SHIFT)

/**
 * No Overvoltage
 */
#define FS8X_FS_VCOREMON_OV_NO_OVERVOLTAGE (0x0000U << FS8X_FS_VCOREMON_OV_SHIFT)
/**
 * Overvoltage Reported on VCOREMON
 */
#define FS8X_FS_VCOREMON_OV_OVERVOLTAGE_REPORTED (0x0001U << FS8X_FS_VCOREMON_OV_SHIFT)

/******************************************************************************/
/* FS_RELEASE_FS0B - Type: RW                                                 */
/******************************************************************************/

#define FS8X_FS_RELEASE_FS0B_ADDR    0x12U
#define FS8X_FS_RELEASE_FS0B_DEFAULT 0x0000U

/**
 * Secure 16bits word to release FS0B
 */
#define FS8X_FS_RELEASE_FS0B_MASK 0xFFFFU

/**
 * Secure 16bits word to release FS0B
 */
#define FS8X_FS_RELEASE_FS0B_SHIFT 0x0000U

/******************************************************************************/
/* FS_SAFE_IOS - Type: RW                                                     */
/******************************************************************************/

#define FS8X_FS_SAFE_IOS_ADDR    0x13U
#define FS8X_FS_SAFE_IOS_DEFAULT 0x0000U

/**
 * Report FCCU1 pin real time state
 */
#define FS8X_FS_FCCU1_RT_MASK 0x0001U
/**
 * Report FCCU2 pin real time state
 */
#define FS8X_FS_FCCU2_RT_MASK 0x0002U
/**
 * Go back to INIT Fail Safe request
 */
#define FS8X_FS_GO_TO_INITFS_MASK 0x0004U
/**
 * Request assertion of FS0B
 */
#define FS8X_FS_FS0B_REQ_MASK 0x0008U
/**
 * Report a Failure on FS0B
 */
#define FS8X_FS_FS0B_DIAG_MASK 0x0010U
/**
 * Sense of FS0B pad
 */
#define FS8X_FS_FS0B_SNS_MASK 0x0020U
/**
 * FS0B driver _ digital command
 */
#define FS8X_FS_FS0B_DRV_MASK 0x0040U
/**
 * Request assertion of RSTB (Pulse)
 */
#define FS8X_FS_RSTB_REQ_MASK 0x0080U
/**
 * Report a RSTB Short to High
 */
#define FS8X_FS_RSTB_DIAG_MASK 0x0100U
/**
 * Report a RSTB event
 */
#define FS8X_FS_RSTB_EVENT_MASK 0x0200U
/**
 * Sense of RSTB pad
 */
#define FS8X_FS_RSTB_SNS_MASK 0x0400U
/**
 * RSTB driver _ digital command
 */
#define FS8X_FS_RSTB_DRV_MASK 0x0800U
/**
 * Report an External RESET
 */
#define FS8X_FS_EXT_RSTB_MASK 0x1000U
/**
 * Sense of PGOOD pad
 */
#define FS8X_FS_PGOOD_SNS_MASK 0x2000U
/**
 * Report a Power GOOD event
 */
#define FS8X_FS_PGOOD_EVENT_MASK 0x4000U
/**
 * Report a PGOOD Short to High
 */
#define FS8X_FS_PGOOD_DIAG_MASK 0x8000U

/**
 * Report FCCU1 pin real time state
 */
#define FS8X_FS_FCCU1_RT_SHIFT 0x0000U
/**
 * Report FCCU2 pin real time state
 */
#define FS8X_FS_FCCU2_RT_SHIFT 0x0001U
/**
 * Go back to INIT Fail Safe request
 */
#define FS8X_FS_GO_TO_INITFS_SHIFT 0x0002U
/**
 * Request assertion of FS0B
 */
#define FS8X_FS_FS0B_REQ_SHIFT 0x0003U
/**
 * Report a Failure on FS0B
 */
#define FS8X_FS_FS0B_DIAG_SHIFT 0x0004U
/**
 * Sense of FS0B pad
 */
#define FS8X_FS_FS0B_SNS_SHIFT 0x0005U
/**
 * FS0B driver _ digital command
 */
#define FS8X_FS_FS0B_DRV_SHIFT 0x0006U
/**
 * Request assertion of RSTB (Pulse)
 */
#define FS8X_FS_RSTB_REQ_SHIFT 0x0007U
/**
 * Report a RSTB Short to High
 */
#define FS8X_FS_RSTB_DIAG_SHIFT 0x0008U
/**
 * Report a RSTB event
 */
#define FS8X_FS_RSTB_EVENT_SHIFT 0x0009U
/**
 * Sense of RSTB pad
 */
#define FS8X_FS_RSTB_SNS_SHIFT 0x000AU
/**
 * RSTB driver _ digital command
 */
#define FS8X_FS_RSTB_DRV_SHIFT 0x000BU
/**
 * Report an External RESET
 */
#define FS8X_FS_EXT_RSTB_SHIFT 0x000CU
/**
 * Sense of PGOOD pad
 */
#define FS8X_FS_PGOOD_SNS_SHIFT 0x000DU
/**
 * Report a Power GOOD event
 */
#define FS8X_FS_PGOOD_EVENT_SHIFT 0x000EU
/**
 * Report a PGOOD Short to High
 */
#define FS8X_FS_PGOOD_DIAG_SHIFT 0x000FU

/**
 * Low level
 */
#define FS8X_FS_FCCU1_RT_LOW_LEVEL (0x0000U << FS8X_FS_FCCU1_RT_SHIFT)
/**
 * High level
 */
#define FS8X_FS_FCCU1_RT_HIGH_LEVEL (0x0001U << FS8X_FS_FCCU1_RT_SHIFT)

/**
 * Low level
 */
#define FS8X_FS_FCCU2_RT_LOW_LEVEL (0x0000U << FS8X_FS_FCCU2_RT_SHIFT)
/**
 * High level
 */
#define FS8X_FS_FCCU2_RT_HIGH_LEVEL (0x0001U << FS8X_FS_FCCU2_RT_SHIFT)

/**
 * No action
 */
#define FS8X_FS_GO_TO_INITFS_NO_ACTION (0x0000U << FS8X_FS_GO_TO_INITFS_SHIFT)
/**
 * Go back to INIT_FS
 */
#define FS8X_FS_GO_TO_INITFS_GO_BACK_TO_INIT_FS (0x0001U << FS8X_FS_GO_TO_INITFS_SHIFT)

/**
 * No Assertion
 */
#define FS8X_FS_FS0B_REQ_NO_ASSERTION (0x0000U << FS8X_FS_FS0B_REQ_SHIFT)
/**
 * FS0B Assertion
 */
#define FS8X_FS_FS0B_REQ_FS0B_ASSERTION (0x0001U << FS8X_FS_FS0B_REQ_SHIFT)

/**
 * No Failure
 */
#define FS8X_FS_FS0B_DIAG_NO_FAILURE (0x0000U << FS8X_FS_FS0B_DIAG_SHIFT)
/**
 * Short Circuit High
 */
#define FS8X_FS_FS0B_DIAG_SHORT_CIRCUIT_HIGH (0x0001U << FS8X_FS_FS0B_DIAG_SHIFT)

/**
 * FS0B pad sensed low
 */
#define FS8X_FS_FS0B_SNS_PAD_SENSED_LOW (0x0000U << FS8X_FS_FS0B_SNS_SHIFT)
/**
 * FS0B pad sensed high
 */
#define FS8X_FS_FS0B_SNS_PAD_SENSED_HIGH (0x0001U << FS8X_FS_FS0B_SNS_SHIFT)

/**
 * FS0B driver command sensed low
 */
#define FS8X_FS_FS0B_DRV_COMMAND_SENSED_LOW (0x0000U << FS8X_FS_FS0B_DRV_SHIFT)
/**
 * FS0B driver command sensed high
 */
#define FS8X_FS_FS0B_DRV_COMMAND_SENSED_HIGH (0x0001U << FS8X_FS_FS0B_DRV_SHIFT)

/**
 * No Assertion
 */
#define FS8X_FS_RSTB_REQ_NO_ASSERTION (0x0000U << FS8X_FS_RSTB_REQ_SHIFT)
/**
 * RSTB Assertion (Pulse)
 */
#define FS8X_FS_RSTB_REQ_RSTB_ASSERTION (0x0001U << FS8X_FS_RSTB_REQ_SHIFT)

/**
 * No Failure
 */
#define FS8X_FS_RSTB_DIAG_NO_FAILURE (0x0000U << FS8X_FS_RSTB_DIAG_SHIFT)
/**
 * Short Circuit High
 */
#define FS8X_FS_RSTB_DIAG_SHORT_CIRCUIT_HIGH (0x0001U << FS8X_FS_RSTB_DIAG_SHIFT)

/**
 * No RESET
 */
#define FS8X_FS_RSTB_EVENT_NO_RESET (0x0000U << FS8X_FS_RSTB_EVENT_SHIFT)
/**
 * RESET occurred
 */
#define FS8X_FS_RSTB_EVENT_RESET_OCCURRED (0x0001U << FS8X_FS_RSTB_EVENT_SHIFT)

/**
 * RSTB pad sensed low
 */
#define FS8X_FS_RSTB_SNS_PAD_SENSED_LOW (0x0000U << FS8X_FS_RSTB_SNS_SHIFT)
/**
 * RSTB pad sensed high
 */
#define FS8X_FS_RSTB_SNS_PAD_SENSED_HIGH (0x0001U << FS8X_FS_RSTB_SNS_SHIFT)

/**
 * RSTB driver command sensed low
 */
#define FS8X_FS_RSTB_DRV_COMMAND_SENSED_LOW (0x0000U << FS8X_FS_RSTB_DRV_SHIFT)
/**
 * RSTB driver command sensed high
 */
#define FS8X_FS_RSTB_DRV_COMMAND_SENSED_HIGH (0x0001U << FS8X_FS_RSTB_DRV_SHIFT)

/**
 * No External RESET
 */
#define FS8X_FS_EXT_RSTB_NO_EXTERNAL_RESET (0x0000U << FS8X_FS_EXT_RSTB_SHIFT)
/**
 * External RESET
 */
#define FS8X_FS_EXT_RSTB_EXTERNAL_RESET (0x0001U << FS8X_FS_EXT_RSTB_SHIFT)

/**
 * PGOOD pad sensed low
 */
#define FS8X_FS_PGOOD_SNS_PAD_SENSED_LOW (0x0000U << FS8X_FS_PGOOD_SNS_SHIFT)
/**
 * PGOOD pad sensed high
 */
#define FS8X_FS_PGOOD_SNS_PAD_SENSED_HIGH (0x0001U << FS8X_FS_PGOOD_SNS_SHIFT)

/**
 * No Power GOOD
 */
#define FS8X_FS_PGOOD_EVENT_NO_POWER_GOOD (0x0000U << FS8X_FS_PGOOD_EVENT_SHIFT)
/**
 * Power Good event occurred
 */
#define FS8X_FS_PGOOD_EVENT_POWER_GOOD_EVENT_OCCURRED (0x0001U << FS8X_FS_PGOOD_EVENT_SHIFT)

/**
 * No Failure
 */
#define FS8X_FS_PGOOD_DIAG_NO_FAILURE (0x0000U << FS8X_FS_PGOOD_DIAG_SHIFT)
/**
 * Short-Circuit HIGH
 */
#define FS8X_FS_PGOOD_DIAG_SHORT_CIRCUIT_HIGH (0x0001U << FS8X_FS_PGOOD_DIAG_SHIFT)

/******************************************************************************/
/* FS_DIAG_SAFETY - Type: RW                                                  */
/******************************************************************************/

#define FS8X_FS_DIAG_SAFETY_ADDR    0x14U
#define FS8X_FS_DIAG_SAFETY_DEFAULT 0x0000U

/**
 * Diagnostic of Logical BIST
 */
#define FS8X_FS_LBIST_OK_MASK 0x0001U
/**
 * Invalid Fail Safe I2C access (Wrong Write or Read, Write to INIT registers in normal mode, wrong address)
 */
#define FS8X_FS_I2C_FS_REQ_MASK 0x0002U
/**
 * Fail Safe I2C communication CRC issue
 */
#define FS8X_FS_I2C_FS_CRC_MASK 0x0004U
/**
 * Fail Safe SPI communication CRC issue
 */
#define FS8X_FS_SPI_FS_CRC_MASK 0x0008U
/**
 * Invalid Fail Safe SPI access (Wrong Write or Read, Write to INIT registers in normal mode, wrong address)
 */
#define FS8X_FS_SPI_FS_REQ_MASK 0x0010U
/**
 * Fail Safe SPI SCLK error detection
 */
#define FS8X_FS_SPI_FS_CLK_MASK 0x0020U
/**
 * Diagnostic of Analog BIST2
 */
#define FS8X_FS_ABIST2_OK_MASK 0x0040U
/**
 * Diagnostic of Analog BIST1
 */
#define FS8X_FS_ABIST1_OK_MASK 0x0080U
/**
 * WD refresh status - Timing
 */
#define FS8X_FS_BAD_WD_TIMING_MASK 0x0100U
/**
 * WD Refresh status - Data
 */
#define FS8X_FS_BAD_WD_DATA_MASK 0x0200U
/**
 * Report ERRMON pin level
 */
#define FS8X_FS_ERRMON_STATUS_MASK 0x0400U
/**
 * Report an error in the ERRMON input
 */
#define FS8X_FS_ERRMON_MASK 0x0800U
/**
 * Acknowledge ERRMON failure detection from MCU
 */
#define FS8X_FS_ERRMON_ACK_MASK 0x1000U
/**
 * Report an error in the FCCU2 input
 */
#define FS8X_FS_FCCU2_MASK 0x2000U
/**
 * Report an error in the FCCU1 input
 */
#define FS8X_FS_FCCU1_MASK 0x4000U
/**
 * Report an error in the FCCU12 input
 */
#define FS8X_FS_FCCU12_MASK 0x8000U

/**
 * Diagnostic of Logical BIST
 */
#define FS8X_FS_LBIST_OK_SHIFT 0x0000U
/**
 * Invalid Fail Safe I2C access (Wrong Write or Read, Write to INIT registers in normal mode, wrong address)
 */
#define FS8X_FS_I2C_FS_REQ_SHIFT 0x0001U
/**
 * Fail Safe I2C communication CRC issue
 */
#define FS8X_FS_I2C_FS_CRC_SHIFT 0x0002U
/**
 * Fail Safe SPI communication CRC issue
 */
#define FS8X_FS_SPI_FS_CRC_SHIFT 0x0003U
/**
 * Invalid Fail Safe SPI access (Wrong Write or Read, Write to INIT registers in normal mode, wrong address)
 */
#define FS8X_FS_SPI_FS_REQ_SHIFT 0x0004U
/**
 * Fail Safe SPI SCLK error detection
 */
#define FS8X_FS_SPI_FS_CLK_SHIFT 0x0005U
/**
 * Diagnostic of Analog BIST2
 */
#define FS8X_FS_ABIST2_OK_SHIFT 0x0006U
/**
 * Diagnostic of Analog BIST1
 */
#define FS8X_FS_ABIST1_OK_SHIFT 0x0007U
/**
 * WD refresh status - Timing
 */
#define FS8X_FS_BAD_WD_TIMING_SHIFT 0x0008U
/**
 * WD Refresh status - Data
 */
#define FS8X_FS_BAD_WD_DATA_SHIFT 0x0009U
/**
 * Report ERRMON pin level
 */
#define FS8X_FS_ERRMON_STATUS_SHIFT 0x000AU
/**
 * Report an error in the ERRMON input
 */
#define FS8X_FS_ERRMON_SHIFT 0x000BU
/**
 * Acknowledge ERRMON failure detection from MCU
 */
#define FS8X_FS_ERRMON_ACK_SHIFT 0x000CU
/**
 * Report an error in the FCCU2 input
 */
#define FS8X_FS_FCCU2_SHIFT 0x000DU
/**
 * Report an error in the FCCU1 input
 */
#define FS8X_FS_FCCU1_SHIFT 0x000EU
/**
 * Report an error in the FCCU12 input
 */
#define FS8X_FS_FCCU12_SHIFT 0x000FU

/**
 * LBIST FAIL
 */
#define FS8X_FS_LBIST_OK_FAIL (0x0000U << FS8X_FS_LBIST_OK_SHIFT)
/**
 * LBIST PASS
 */
#define FS8X_FS_LBIST_OK_PASS (0x0001U << FS8X_FS_LBIST_OK_SHIFT)

/**
 * No error
 */
#define FS8X_FS_I2C_FS_REQ_NO_ERROR (0x0000U << FS8X_FS_I2C_FS_REQ_SHIFT)
/**
 * I2c Violation
 */
#define FS8X_FS_I2C_FS_REQ_I2C_VIOLATION (0x0001U << FS8X_FS_I2C_FS_REQ_SHIFT)

/**
 * No error
 */
#define FS8X_FS_I2C_FS_CRC_NO_ERROR (0x0000U << FS8X_FS_I2C_FS_CRC_SHIFT)
/**
 * Error detected in the CRC
 */
#define FS8X_FS_I2C_FS_CRC_ERROR_DETECTED (0x0001U << FS8X_FS_I2C_FS_CRC_SHIFT)

/**
 * No error
 */
#define FS8X_FS_SPI_FS_CRC_NO_ERROR (0x0000U << FS8X_FS_SPI_FS_CRC_SHIFT)
/**
 * Error detected in the CRC
 */
#define FS8X_FS_SPI_FS_CRC_ERROR_DETECTED (0x0001U << FS8X_FS_SPI_FS_CRC_SHIFT)

/**
 * No error
 */
#define FS8X_FS_SPI_FS_REQ_NO_ERROR (0x0000U << FS8X_FS_SPI_FS_REQ_SHIFT)
/**
 * SPI Violation
 */
#define FS8X_FS_SPI_FS_REQ_SPI_VIOLATION (0x0001U << FS8X_FS_SPI_FS_REQ_SHIFT)

/**
 * No error
 */
#define FS8X_FS_SPI_FS_CLK_NO_ERROR (0x0000U << FS8X_FS_SPI_FS_CLK_SHIFT)
/**
 * Wrong number of clock cycles <32 or >32)
 */
#define FS8X_FS_SPI_FS_CLK_WRONG_NUMBER_OF_CLOCK_CYCLES (0x0001U << FS8X_FS_SPI_FS_CLK_SHIFT)

/**
 * ABIST2 FAIL or NOT EXECUTED
 */
#define FS8X_FS_ABIST2_OK_ABIST2_FAIL (0x0000U << FS8X_FS_ABIST2_OK_SHIFT)
/**
 * PASS
 */
#define FS8X_FS_ABIST2_OK_PASS (0x0001U << FS8X_FS_ABIST2_OK_SHIFT)

/**
 * ABIST1 FAIL or NOT EXECUTED
 */
#define FS8X_FS_ABIST1_OK_ABIST1_FAIL (0x0000U << FS8X_FS_ABIST1_OK_SHIFT)
/**
 * PASS
 */
#define FS8X_FS_ABIST1_OK_PASS (0x0001U << FS8X_FS_ABIST1_OK_SHIFT)

/**
 * Good WD Refresh
 */
#define FS8X_FS_BAD_WD_TIMING_GOOD_WD_REFRESH (0x0000U << FS8X_FS_BAD_WD_TIMING_SHIFT)
/**
 * Bad WD refresh, wrong window or in timeout
 */
#define FS8X_FS_BAD_WD_TIMING_BAD_WD_REFRESH (0x0001U << FS8X_FS_BAD_WD_TIMING_SHIFT)

/**
 * Good WD Refresh
 */
#define FS8X_FS_BAD_WD_DATA_GOOD_WD_REFRESH (0x0000U << FS8X_FS_BAD_WD_DATA_SHIFT)
/**
 * Bad WD refresh, error in the DATA
 */
#define FS8X_FS_BAD_WD_DATA_BAD_WD_REFRESH (0x0001U << FS8X_FS_BAD_WD_DATA_SHIFT)

/**
 * LOW Level
 */
#define FS8X_FS_ERRMON_STATUS_LOW_LEVEL (0x0000U << FS8X_FS_ERRMON_STATUS_SHIFT)
/**
 * HIGH Level
 */
#define FS8X_FS_ERRMON_STATUS_HIGH_LEVEL (0x0001U << FS8X_FS_ERRMON_STATUS_SHIFT)

/**
 * No error
 */
#define FS8X_FS_ERRMON_NO_ERROR (0x0000U << FS8X_FS_ERRMON_SHIFT)
/**
 * Error detected
 */
#define FS8X_FS_ERRMON_ERROR_DETECTED (0x0001U << FS8X_FS_ERRMON_SHIFT)

/**
 * No effect
 */
#define FS8X_FS_ERRMON_ACK_NO_EFFECT (0x0000U << FS8X_FS_ERRMON_ACK_SHIFT)
/**
 * Acknowledge ERRMON failure detection
 */
#define FS8X_FS_ERRMON_ACK_FAILURE_DETECTION (0x0001U << FS8X_FS_ERRMON_ACK_SHIFT)

/**
 * No error
 */
#define FS8X_FS_FCCU2_NO_ERROR (0x0000U << FS8X_FS_FCCU2_SHIFT)
/**
 * Error detected
 */
#define FS8X_FS_FCCU2_ERROR_DETECTED (0x0001U << FS8X_FS_FCCU2_SHIFT)

/**
 * No error
 */
#define FS8X_FS_FCCU1_NO_ERROR (0x0000U << FS8X_FS_FCCU1_SHIFT)
/**
 * Error detected
 */
#define FS8X_FS_FCCU1_ERROR_DETECTED (0x0001U << FS8X_FS_FCCU1_SHIFT)

/**
 * No error
 */
#define FS8X_FS_FCCU12_NO_ERROR (0x0000U << FS8X_FS_FCCU12_SHIFT)
/**
 * Error detected
 */
#define FS8X_FS_FCCU12_ERROR_DETECTED (0x0001U << FS8X_FS_FCCU12_SHIFT)

/******************************************************************************/
/* FS_INTB_MASK - Type: RW                                                    */
/******************************************************************************/

#define FS8X_FS_INTB_MASK_ADDR    0x15U
#define FS8X_FS_INTB_MASK_DEFAULT 0x0000U

/**
 * Inhibit INTERRUPT on FCCU1 event
 */
#define FS8X_FS_INT_INH_FCCU1_MASK 0x0001U
/**
 * Inhibit INTERRUPT on FCCU2 event
 */
#define FS8X_FS_INT_INH_FCCU2_MASK 0x0002U
/**
 * Inhibit INTERRUPT on ERRMON event
 */
#define FS8X_FS_INT_INH_ERRMON_MASK 0x0004U
/**
 * Inhibit INTERRUPT on bad WD refresh event
 */
#define FS8X_FS_INT_INH_BAD_WD_REFRESH_MASK 0x0008U
/**
 * Inhibit INTERRUPT on VCOREMON OV and UV event
 */
#define FS8X_FS_INT_INH_VCOREMON_OV_UV_MASK 0x0010U
/**
 * Inhibit INTERRUPT on VDDIO OV and UV event
 */
#define FS8X_FS_INT_INH_VDDIO_OV_UV_MASK 0x0020U
/**
 * Inhibit INTERRUPT on VMON1 OV and UV event
 */
#define FS8X_FS_INT_INH_VMON1_OV_UV_MASK 0x0040U
/**
 * Inhibit INTERRUPT on VMON2 OV and UV event
 */
#define FS8X_FS_INT_INH_VMON2_OV_UV_MASK 0x0080U
/**
 * Inhibit INTERRUPT on VMON3 OV and UV event
 */
#define FS8X_FS_INT_INH_VMON3_OV_UV_MASK 0x0100U
/**
 * Inhibit INTERRUPT on VMON4 OV and UV event
 */
#define FS8X_FS_INT_INH_VMON4_OV_UV_MASK 0x0200U

/**
 * Inhibit INTERRUPT on FCCU1 event
 */
#define FS8X_FS_INT_INH_FCCU1_SHIFT 0x0000U
/**
 * Inhibit INTERRUPT on FCCU2 event
 */
#define FS8X_FS_INT_INH_FCCU2_SHIFT 0x0001U
/**
 * Inhibit INTERRUPT on ERRMON event
 */
#define FS8X_FS_INT_INH_ERRMON_SHIFT 0x0002U
/**
 * Inhibit INTERRUPT on bad WD refresh event
 */
#define FS8X_FS_INT_INH_BAD_WD_REFRESH_SHIFT 0x0003U
/**
 * Inhibit INTERRUPT on VCOREMON OV and UV event
 */
#define FS8X_FS_INT_INH_VCOREMON_OV_UV_SHIFT 0x0004U
/**
 * Inhibit INTERRUPT on VDDIO OV and UV event
 */
#define FS8X_FS_INT_INH_VDDIO_OV_UV_SHIFT 0x0005U
/**
 * Inhibit INTERRUPT on VMON1 OV and UV event
 */
#define FS8X_FS_INT_INH_VMON1_OV_UV_SHIFT 0x0006U
/**
 * Inhibit INTERRUPT on VMON2 OV and UV event
 */
#define FS8X_FS_INT_INH_VMON2_OV_UV_SHIFT 0x0007U
/**
 * Inhibit INTERRUPT on VMON3 OV and UV event
 */
#define FS8X_FS_INT_INH_VMON3_OV_UV_SHIFT 0x0008U
/**
 * Inhibit INTERRUPT on VMON4 OV and UV event
 */
#define FS8X_FS_INT_INH_VMON4_OV_UV_SHIFT 0x0009U

/**
 * Interruption NOT MASKED
 */
#define FS8X_FS_INT_INH_FCCU1_INTERRUPTION_NOT_MASKED (0x0000U << FS8X_FS_INT_INH_FCCU1_SHIFT)
/**
 * Interruption MASKED
 */
#define FS8X_FS_INT_INH_FCCU1_INTERRUPTION_MASKED (0x0001U << FS8X_FS_INT_INH_FCCU1_SHIFT)

/**
 * Interruption NOT MASKED
 */
#define FS8X_FS_INT_INH_FCCU2_INTERRUPTION_NOT_MASKED (0x0000U << FS8X_FS_INT_INH_FCCU2_SHIFT)
/**
 * Interruption MASKED
 */
#define FS8X_FS_INT_INH_FCCU2_INTERRUPTION_MASKED (0x0001U << FS8X_FS_INT_INH_FCCU2_SHIFT)

/**
 * Interruption NOT MASKED
 */
#define FS8X_FS_INT_INH_ERRMON_INTERRUPTION_NOT_MASKED (0x0000U << FS8X_FS_INT_INH_ERRMON_SHIFT)
/**
 * Interruption MASKED
 */
#define FS8X_FS_INT_INH_ERRMON_INTERRUPTION_MASKED (0x0001U << FS8X_FS_INT_INH_ERRMON_SHIFT)

/**
 * Interruption NOT MASKED
 */
#define FS8X_FS_INT_INH_BAD_WD_REFRESH_INTERRUPTION_NOT_MASKED (0x0000U << FS8X_FS_INT_INH_BAD_WD_REFRESH_SHIFT)
/**
 * Interruption MASKED
 */
#define FS8X_FS_INT_INH_BAD_WD_REFRESH_INTERRUPTION_MASKED (0x0001U << FS8X_FS_INT_INH_BAD_WD_REFRESH_SHIFT)

/**
 * Interruption NOT MASKED
 */
#define FS8X_FS_INT_INH_VCOREMON_OV_UV_INTERRUPTION_NOT_MASKED (0x0000U << FS8X_FS_INT_INH_VCOREMON_OV_UV_SHIFT)
/**
 * Interruption MASKED
 */
#define FS8X_FS_INT_INH_VCOREMON_OV_UV_INTERRUPTION_MASKED (0x0001U << FS8X_FS_INT_INH_VCOREMON_OV_UV_SHIFT)

/**
 * Interruption NOT MASKED
 */
#define FS8X_FS_INT_INH_VDDIO_OV_UV_INTERRUPTION_NOT_MASKED (0x0000U << FS8X_FS_INT_INH_VDDIO_OV_UV_SHIFT)
/**
 * Interruption MASKED
 */
#define FS8X_FS_INT_INH_VDDIO_OV_UV_INTERRUPTION_MASKED (0x0001U << FS8X_FS_INT_INH_VDDIO_OV_UV_SHIFT)

/**
 * Interruption NOT MASKED
 */
#define FS8X_FS_INT_INH_VMON1_OV_UV_INTERRUPTION_NOT_MASKED (0x0000U << FS8X_FS_INT_INH_VMON1_OV_UV_SHIFT)
/**
 * Interruption MASKED
 */
#define FS8X_FS_INT_INH_VMON1_OV_UV_INTERRUPTION_MASKED (0x0001U << FS8X_FS_INT_INH_VMON1_OV_UV_SHIFT)

/**
 * Interruption NOT MASKED
 */
#define FS8X_FS_INT_INH_VMON2_OV_UV_INTERRUPTION_NOT_MASKED (0x0000U << FS8X_FS_INT_INH_VMON2_OV_UV_SHIFT)
/**
 * Interruption MASKED
 */
#define FS8X_FS_INT_INH_VMON2_OV_UV_INTERRUPTION_MASKED (0x0001U << FS8X_FS_INT_INH_VMON2_OV_UV_SHIFT)

/**
 * Interruption NOT MASKED
 */
#define FS8X_FS_INT_INH_VMON3_OV_UV_INTERRUPTION_NOT_MASKED (0x0000U << FS8X_FS_INT_INH_VMON3_OV_UV_SHIFT)
/**
 * Interruption MASKED
 */
#define FS8X_FS_INT_INH_VMON3_OV_UV_INTERRUPTION_MASKED (0x0001U << FS8X_FS_INT_INH_VMON3_OV_UV_SHIFT)

/**
 * Interruption NOT MASKED
 */
#define FS8X_FS_INT_INH_VMON4_OV_UV_INTERRUPTION_NOT_MASKED (0x0000U << FS8X_FS_INT_INH_VMON4_OV_UV_SHIFT)
/**
 * Interruption MASKED
 */
#define FS8X_FS_INT_INH_VMON4_OV_UV_INTERRUPTION_MASKED (0x0001U << FS8X_FS_INT_INH_VMON4_OV_UV_SHIFT)

/******************************************************************************/
/* FS_STATES - Type: RW                                                       */
/******************************************************************************/

#define FS8X_FS_STATES_ADDR    0x16U
#define FS8X_FS_STATES_DEFAULT 0x0000U

/**
 * FS FSM current state (MSB bit 4 = 0 - extended for future uage)
 */
#define FS8X_FS_FSM_STATE_MASK 0x001FU
/**
 * INIT register corruption detection
 */
#define FS8X_FS_REG_CORRUPT_MASK 0x0200U
/**
 * OTP bits corruption detection (5ms cyclic check)
 */
#define FS8X_FS_OTP_CORRUPT_MASK 0x0800U
/**
 * DEBUG mode status
 */
#define FS8X_FS_DBG_MODE_MASK 0x2000U
/**
 * Leave DEBUG mode
 */
#define FS8X_FS_DBG_EXIT_MASK 0x4000U
/**
 * Test Mode Activation Status
 */
#define FS8X_FS_TM_ACTIVE_MASK 0x8000U

/**
 * FS FSM current state (MSB bit 4 = 0 - extended for future uage)
 */
#define FS8X_FS_FSM_STATE_SHIFT 0x0000U
/**
 * INIT register corruption detection
 */
#define FS8X_FS_REG_CORRUPT_SHIFT 0x0009U
/**
 * OTP bits corruption detection (5ms cyclic check)
 */
#define FS8X_FS_OTP_CORRUPT_SHIFT 0x000BU
/**
 * DEBUG mode status
 */
#define FS8X_FS_DBG_MODE_SHIFT 0x000DU
/**
 * Leave DEBUG mode
 */
#define FS8X_FS_DBG_EXIT_SHIFT 0x000EU
/**
 * Test Mode Activation Status
 */
#define FS8X_FS_TM_ACTIVE_SHIFT 0x000FU

/**
 * INIT_FS
 */
#define FS8X_FS_FSM_STATE_INIT_FS (0x0006U << FS8X_FS_FSM_STATE_SHIFT)
/**
 * WAIT_ABIST2
 */
#define FS8X_FS_FSM_STATE_WAIT_ABIST2 (0x0007U << FS8X_FS_FSM_STATE_SHIFT)
/**
 * ABIST2
 */
#define FS8X_FS_FSM_STATE_ABIST2 (0x0008U << FS8X_FS_FSM_STATE_SHIFT)
/**
 * ASSERT_FS0B
 */
#define FS8X_FS_FSM_STATE_ASSERT_FS0B (0x0009U << FS8X_FS_FSM_STATE_SHIFT)
/**
 * NORMAL_FS
 */
#define FS8X_FS_FSM_STATE_NORMAL_FS (0x000AU << FS8X_FS_FSM_STATE_SHIFT)

/**
 * No corruption detected in init registers (i.e. no mismatch between register/register_NOT) pair
 */
#define FS8X_FS_REG_CORRUPT_NO_ERROR (0x0000U << FS8X_FS_REG_CORRUPT_SHIFT)
/**
 * Data content corruption detected in init registers (i.e. mismatch between register/register_NOT) pair
 */
#define FS8X_FS_REG_CORRUPT_ERROR (0x0001U << FS8X_FS_REG_CORRUPT_SHIFT)

/**
 * No OTP content CRC error detected
 */
#define FS8X_FS_OTP_CORRUPT_NO_ERROR (0x0000U << FS8X_FS_OTP_CORRUPT_SHIFT)
/**
 * OTP content CRC error detected
 */
#define FS8X_FS_OTP_CORRUPT_ERROR (0x0001U << FS8X_FS_OTP_CORRUPT_SHIFT)

/**
 * NOT in DEBUG mode
 */
#define FS8X_FS_DBG_MODE_NO_DEBUG (0x0000U << FS8X_FS_DBG_MODE_SHIFT)
/**
 * In DEBUG mode
 */
#define FS8X_FS_DBG_MODE_DEBUG (0x0001U << FS8X_FS_DBG_MODE_SHIFT)

/**
 * No Action
 */
#define FS8X_FS_DBG_EXIT_NO_ACTION (0x0000U << FS8X_FS_DBG_EXIT_SHIFT)
/**
 * Leave DEBUG mode
 */
#define FS8X_FS_DBG_EXIT_LEAVE_DEBUG_MODE (0x0001U << FS8X_FS_DBG_EXIT_SHIFT)

/**
 * Test mode is not activated
 */
#define FS8X_FS_TM_ACTIVE_NOT_ACTIVE (0x0000U << FS8X_FS_TM_ACTIVE_SHIFT)
/**
 * Test mode is activated
 */
#define FS8X_FS_TM_ACTIVE_ACTIVE (0x0001U << FS8X_FS_TM_ACTIVE_SHIFT)

#endif /* SBC_FS8X_MAP_H__ */
