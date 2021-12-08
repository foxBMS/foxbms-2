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
 * @file    mxm_register_map.h
 * @author  foxBMS Team
 * @date    2019-03-05 (date of creation)
 * @updated 2021-12-06 (date of last update)
 * @ingroup DRIVERS
 * @prefix  MXM
 *
 * @brief   Register map of the MAX1785x monitoring IC
 *
 * @details Register map with named registers of the monitoring ICs
 *          MAX17852 and MAX17853 by Maxim Integrated
 *
 */

#ifndef FOXBMS__MXM_REGISTER_MAP_H_
#define FOXBMS__MXM_REGISTER_MAP_H_

/*========== Includes =======================================================*/
#include "general.h"

/*========== Macros and Definitions =========================================*/
/**
 * @brief      MAX1785x register names
 * @details    Register names and addresses of the monitoring ICs MAX17852 and
 *             MAX17853. Please note that the MAX17853 only supports a subset
 *             of these register values.
 */
typedef enum {
    MXM_REG_VERSION       = 0x00u, /*!< VERSION is a read only
     * accessible register which returns information on the device */
    MXM_REG_ADDRESS       = 0x01u, /*!< ADDRESS is a read and write
     * accessible register which sets the first, last, and device address
     * used by a device in a UART chain (UARTSEL=1). This register has no  NOLINT
     * effect on a device operating in SPI mode (UARTSEL=0).  NOLINT */
    MXM_REG_STATUS1       = 0x02u, /*!< STATUS1 is a read and write
     * accessible register which relates the current status of the device.
     * STATUS1 also contains summary information on STATUS2, STATUS3, and
     * FMEA registers, and other selected registers indicating if additional
     * read back checks are required. */
    MXM_REG_STATUS2       = 0x03u, /*!< STATUS2 is a read and write
     * accessible register which contains summary information on alerts
     * related to interface and communication faults. */
    MXM_REG_STATUS3       = 0x04u, /*!< STATUS3 is a read and write
     * accessible register which contains summary information on alerts
     * related to automated cell balancing operations. */
    MXM_REG_FMEA1         = 0x05u, /*!< FMEA1 is a read and write
     * accessible register which relates current information on possible
     * fault conditions. */
    MXM_REG_FMEA2         = 0x06u, /*!< FMEA2 is a read and write
     * accessible register which relates current information on possible
     * fault conditions. */
    MXM_REG_ALRTSUM       = 0x07u, /*!< ALRTSUM is a read accessible
     * register which relates added, detailed information on the current
     * states of the device, breaking out several summary bits in STATUS1. */
    MXM_REG_ALRTOVCELL    = 0x08u, /*!< ALRTOVCELL is a read
     * accessible register which relates current information on cell over
     * voltage fault alerts based on ADC measurements. */
    MXM_REG_ALRTUVCELL    = 0x09u, /*!< ALRTUVCELL is a read
     * accessible register which relates current information on cell under
     * voltage fault alerts based on ADC measurements. */
    MXM_REG_MINMAXCELL    = 0x0Au, /*!< MINMAX is a read accessible
     * register which relates the cell locations with the highest and lowest
     * values measured. */
    MXM_REG_ALRTAUXPRTCT  = 0x0Bu, /*!< ALRTAUXPRTCT is a read
     * accessible register which relates current information on auxiliary
     * input protection fault alerts. */
    MXM_REG_ALRTAUXOV     = 0x0Cu, /*!< ALRTAUXOV is a read
     * accessible register which relates current information on auxiliary
     * over voltage (cold) fault alerts.  NOLINT */
    MXM_REG_ALRTAUXUV     = 0x0Du, /*!< ALRTAUXUV is a read accessible
     * register which relates current information on auxiliary under voltage
     * fault (hot) alerts.  NOLINT */
    MXM_REG_ALRTCOMPOV    = 0x0Eu, /*!< ALRTCOMPOV is a read accessible
     * register which relates current information on cell over voltage fault
     * alerts based on the redundant comparator. */
    MXM_REG_ALRTCOMPUV    = 0x0Fu, /*!< ALRTCOMPUV is a read accessible
     * register which relates current information on cell under voltage fault
     * alerts based on the redundant comparator. */
    MXM_REG_ALRTCOMPAUXOV = 0x10u, /*!< ALRTCOMPAUXOV is a read
     * accessible register which relates current information on auxiliary over
     * voltage fault (cold) alerts based on the redundant comparator.  NOLINT */
    MXM_REG_ALRTCOMPAUXUV = 0x11u, /*!< ALRTCOMPAUXUV is a read
     * accessible register which relates current information on auxiliary under
     * voltage fault (hot) alerts based on the redundant comparator.  NOLINT */
    MXM_REG_ALRTBALSW     = 0x12u, /*!< ALRTBALSW is a read accessible
     * register which relates current summary information on balancing switch
     * fault alerts. */
    MXM_REG_SWACTION      = 0x13u, /*!< SWACTION is a read and write
     * accessible register which contains bits allowing software exit and
     * reset requests. These requests are not recommended for general use,
     * but may be of use in case of error. */
    MXM_REG_DEVCFG1       = 0x14u, /*!< DEVCFG1 is a read and write
     * accessible register which governs the configuration of the device
     * interface operation. */
    MXM_REG_DEVCFG2       = 0x15u, /*!< DEVCFG2 is a read and write
     * accessible register which governs the configuration of the device
     * filtering, several top level diagnostic modes, and timeout monitors. */
    MXM_REG_AUXGPIOCFG    = 0x16u, /*!< AUXGPIOCFG is a read and write
     * accessible register which governs the configuration of the AUX/GPIO
     * multifunction pins. */
    MXM_REG_GPIOCFG       = 0x17u, /*!< GPIOCFG is a read and write
     * accessible register which governs the output state of GPIO outputs and
     * reads back the input state of GPIO inputs. */
    MXM_REG_PACKCFG       = 0x18u, /*!< PACKCFG is a read and write
     * accessible register which configures the part such that the top most
     * cell and block used in the application is known. Details of Flex Pack
     * applications are also configured within this register. */
    MXM_REG_ALRTIRQEN     = 0x19u, /*!< ALRTIRQEN is a read and write
     * accessible register which selects which STATS1 alerts trigger interrupts
     * via the ALERT interface port(s), and are included in the DCByte and
     * Alert Packet (UART) or ALERT bit (SPI) notifications. Note the information  NOLINT
     * in the STATUS1 register itself (or any component terms rolled up into  NOLINT
     * STATUS1) is not masked/disabled by these settings, allowing the
     * underlying data to always be available via STATUS1 readback. */
    MXM_REG_ALRTOVEN      = 0x1Au, /*!< ALRTOVEN is a read and write
     * accessible register which enables over voltage fault checks on selected
     * input channels during scans using either the ADC or Comparator. */
    MXM_REG_ALRTUVEN      = 0x1Bu, /*!< ALRTUVEN is a read and write
     * accessible register which enables under voltage fault checks on selected
     * input channels during scans using either the ADC or Comparator. */
    MXM_REG_ALRTAUXOVEN   = 0x1Cu, /*!< ALRTAUXOVEN is a read and write
     * accessible register which enables auxiliary over voltage (cold) fault  NOLINT
     * checks on selected Auxiliary channels during scans using either the
     * ADC or Comparator. */
    MXM_REG_ALRTAUXUVEN   = 0x1Du, /*!< ALRTAUXUVEN is a read and write
     * accessible register which enables auxiliary under voltage (hot) fault  NOLINT
     * checks on selected Auxiliary channels using either the
     * ADC or Comparator. */
    MXM_REG_ALRTCALTST    = 0x1Eu, /*!< ALRTCALTST is a read and write
     * accessible register which allows the user to force Calibration alerts
     * to test readback and interrupt logic. The forced alert(s) will remain
     * forced until this register is written back to zeros (assuming the  NOLINT
     * existing calibration data is within range) */
    MXM_REG_OVTHCLR       = 0x1Fu, /*!< OVTHCLR is a read and write
     * accessible register which selects the cell over voltage alert
     * clear threshold used with unipolar ADC measurements. */
    MXM_REG_OVTHSET       = 0x20u, /*!< OVTHSET is a read and write
     * accessible register which selects the cell over voltage alert set
     * threshold used with unipolar ADC measurements. */
    MXM_REG_UVTHCLR       = 0x21u, /*!< UVTHCLR is a read and write
     * accessible register which selects the cell under voltage alert clear
     * threshold used with unipolar ADC measurements. */
    MXM_REG_UVTHSET       = 0x22u, /*!< UVTHSET is a read and write
     * accessible register which selects the cell under voltage alert set
     * threshold used with unipolar ADC measurements. */
    MXM_REG_MSMTCH        = 0x23u, /*!< MSMTCH is a read and write
     * accessible register which selects the cell voltage mismatch alert
     * threshold used with ADC cell scan measurements. */
    MXM_REG_BIPOVTHCLR    = 0x24u, /*!< BIPOVTHCLR is a read and write
     * accessible register which selects the cell over voltage alert clear
     * threshold used with bipolar ADC measurements. */
    MXM_REG_BIPOVTHSET    = 0x25u, /*!< BIPOVTHSET is a read and write
     * accessible register which selects the cell over voltage alert set
     * threshold used with bipolar ADC measurements. */
    MXM_REG_BIPUVTHCLR    = 0x26u, /*!< BIPUVTHCLR is a read and write
     * accessible register which selects the cell under voltage alert clear
     * threshold used with bipolar ADC measurements. */
    MXM_REG_BIPUVTHSET    = 0x27u, /*!< BIPUVTHSET is a read and write
     * accessible register which selects the cell under voltage alert set
     * threshold used with bipolar ADC measurements. */
    MXM_REG_BLKOVTHCLR    = 0x28u, /*!< BLKOVTHCLR is a read and write
     * accessible register which selects the block over voltage alert clear
     * threshold used with ADC measurements. */
    MXM_REG_BLKOVTHSET    = 0x29u, /*!< BLKOVTHSET is a read and write
     * accessible register which selects the block over voltage alert set
     * threshold used with ADC measurements. */
    MXM_REG_BLKUVTHCLR    = 0x2Au, /*!< BLKUVTHCLR is a read and write
     * accessible register which selects the block under voltage alert clear
     * threshold used with ADC measurements. */
    MXM_REG_BLKUVTHSET    = 0x2Bu, /*!< BLKUVTHSET is a read and write
     * accessible register which selects the block under voltage alert set
     * threshold used with ADC measurements. */
    MXM_REG_CSAOVTHCLR    = 0x2Cu, /*!< MAX17852 only: CSAOVTHCLR is a
     * read and write accessible register which selects the CSA over voltage
     * alert clear threshold used with ADC measurements. */
    MXM_REG_CSAOVTHSET    = 0x2Du, /*!< MAX17852 only: CSAOVTHSET is a
     * read and write accessible register which selects the CSA over voltage
     * alert set threshold used with ADC measurements. */
    MXM_REG_CSAUVTHCLR    = 0x2Eu, /*!< MAX17852 only: CSAUVTHCLR is a
     * read and write accessible register which selects the CSA under voltage
     * alert clear threshold used with ADC measurements. */
    MXM_REG_CSAUVTHSET    = 0x2Fu, /*!< MAX17852 only: CSAUVTHSET is a
     * read and write accessible register which selects the CSA under voltage
     * alert set threshold used with ADC measurements. */
    MXM_REG_AUXROVTHCLR   = 0x30u, /*!< AUXROVTHCLR is a read and write
     * accessible register which selects the over voltage (cold) alert  NOLINT
     * clear threshold used with Ratiometric Auxiliary ADC measurements. */
    MXM_REG_AUXROVTHSET   = 0x31u, /*!< AUXROVTHSET is a read and write
     * accessible register which selects the over voltage (cold) alert set  NOLINT
     * threshold used with Ratiometric Auxiliary ADC measurements. */
    MXM_REG_AUXRUVTHCLR   = 0x32u, /*!< AUXRUVTHCLR is a read and write
     * accessible register which selects the under voltage (hot) alert clear  NOLINT
     * threshold used with Ratiometric Auxiliary ADC measurements. */
    MXM_REG_AUXRUVTHSET   = 0x33u, /*!< AUXRUVTHSET is a read and write
     * accessible register which selects the under voltage (hot) alert set  NOLINT
     * threshold used with Ratiometric Auxiliary ADC measurements. */
    MXM_REG_AUXOVTHCLR    = 0x34u, /*!< AUXOVTHCLR is a read and write
     * accessible register which selects the over voltage alert clear
     * threshold used with Absolute Auxiliary ADC measurements. */
    MXM_REG_AUXAOVTHSET   = 0x35u, /*!< AUXAOVTHSET is a read and write
     * accessible register which selects the over voltage alert set
     * threshold used with Absolute Auxiliary ADC measurements. */
    MXM_REG_AUXAUVTHCLR   = 0x36u, /*!< AUXAUVTHCLR is a read and write
     * accessible register which selects the under voltage alert clear
     * threshold used with Absolute Auxiliary ADC measurements. */
    MXM_REG_AUXAUVTHSET   = 0x37u, /*!< AUXAUVTHSET is a read and write
     * accessible register which selects the under voltage alert set threshold
     * used with Absolute Auxiliary ADC measurements. */
    MXM_REG_COMPOVTH      = 0x38u, /*!< COMPOVTH is a read and write
     * accessible register which selects the cell over voltage alert threshold
     * for the redundant comparator. */
    MXM_REG_COMPUVTH      = 0x39u, /*!< COMPUVTH is a read and write
     * accessible register which selects the cell under voltage alert threshold
     * for the redundant comparator. */
    MXM_REG_COMPAUXROVTH  = 0x3Au, /*!< COMPAUXROVTH is a read and write
     * accessible register which selects the over voltage (cold) alert threshold  NOLINT
     * applied during Ratiometric Auxiliary comparator measurements. */
    MXM_REG_COMPAUXRUVTH  = 0x3Bu, /*!< COMPAUXRUVTH is a read and write
     * accessible register which selects the under voltage (hot) alert threshold  NOLINT
     * applied during Ratiometric Auxiliary comparator measurements. */
    MXM_REG_COMPAUXAOVTH  = 0x3Cu, /*!< COMPAUXAOVTH is a read and write
     * accessible register which selects the over voltage alert threshold
     * applied during Absolute Auxiliary comparator measurements. */
    MXM_REG_COMPAUXAUVTH  = 0x3Du, /*!< COMPAUXAUVTH is a read and write
     * accessible register which selects the under voltage alert threshold
     * applied during Absolute Auxiliary comparator measurements. */
    MXM_REG_COMPOPNTH     = 0x3Eu, /*!< COMPOPNTH is a read and write
     * accessible register which selects the under voltage alert threshold
     * applied to Unipolar Cell inputs in Open Diagnostic Mode. */
    MXM_REG_COMPAUXROPNTH = 0x3Fu, /*!< COMPAUXROPNTH is a read and write
     * accessible register which selects the under voltage alert threshold
     * applied to Ratiometric Auxiliary inputs in Open Diagnostic Mode. */
    MXM_REG_COMPAUXAOPNTH = 0x40u, /*!< COMPAUXAOPNTH is a read and write
     * accessible register which selects the under voltage alert threshold
     * applied to Absolute Auxiliary inputs in Open Diagnostic Mode. */
    MXM_REG_COMPACCOVTH   = 0x41u, /*!< COMPACCOVTH is a read and write
     * accessible register which selects the over voltage alert threshold
     * applied during comparator accuracy diagnostics. */
    MXM_REG_COMPACCUVTH   = 0x42u, /*!< COMPACCUVTH is a read and write
     * accessible register which selects the under voltage alert threshold
     * applied during comparator accuracy diagnostics. */
    MXM_REG_BALSHRTTHR    = 0x43u, /*!< BALSHRTTHR is a read and write
     * accessible register which selects alert threshold used during the
     * Balance Switch Short Diagnostic mode. */
    MXM_REG_BALLOWTHR     = 0x44u, /*!< BALLOWTHR is a read and write
     * accessible register which selects alert low threshold used during the
     * Balance Switch Open Diagnostic Mode. */
    MXM_REG_BALHIGHTHR    = 0x45u, /*!< BALHIGHTHR is a read and write
     * accessible register which selects alert High threshold used during the
     * Balance Switch Open Diagnostic mode. */
    MXM_REG_CSA           = 0x46u, /*!< MAX17852 only: CSA is a read
     * accessible register which holds the current value for each enabled
     * individual auxiliary measurement result. */
    MXM_REG_CELL1         = 0x47u, /*!< CELLn is a read accessible
     * register which holds the current value for each individual cell
     * measurement result. */
    MXM_REG_CELL2         = 0x48u, /*!< CELLn is a read accessible
     * register which holds the current value for each individual cell
     * measurement result. */
    MXM_REG_CELL3         = 0x49u, /*!< CELLn is a read accessible
     * register which holds the current value for each individual cell
     * measurement result. */
    MXM_REG_CELL4         = 0x4Au, /*!< CELLn is a read accessible
     * register which holds the current value for each individual cell
     * measurement result. */
    MXM_REG_CELL5         = 0x4Bu, /*!< CELLn is a read accessible
     * register which holds the current value for each individual cell
     * measurement result. */
    MXM_REG_CELL6         = 0x4Cu, /*!< CELLn is a read accessible
     * register which holds the current value for each individual cell
     * measurement result. */
    MXM_REG_CELL7         = 0x4Du, /*!< CELLn is a read accessible
     * register which holds the current value for each individual cell
     * measurement result. */
    MXM_REG_CELL8         = 0x4Eu, /*!< CELLn is a read accessible
     * register which holds the current value for each individual cell
     * measurement result. */
    MXM_REG_CELL9         = 0x4Fu, /*!< CELLn is a read accessible
     * register which holds the current value for each individual cell
     * measurement result. */
    MXM_REG_CELL10        = 0x50u, /*!< CELLn is a read accessible
     * register which holds the current value for each individual cell
     * measurement result. */
    MXM_REG_CELL11        = 0x51u, /*!< CELLn is a read accessible
     * register which holds the current value for each individual cell
     * measurement result. */
    MXM_REG_CELL12        = 0x52u, /*!< CELLn is a read accessible
     * register which holds the current value for each individual cell
     * measurement result. */
    MXM_REG_CELL13        = 0x53u, /*!< CELLn is a read accessible
     * register which holds the current value for each individual cell
     * measurement result. */
    MXM_REG_CELL14        = 0x54u, /*!< CELLn is a read accessible
     * register which holds the current value for each individual cell
     * measurement result. */
    MXM_REG_BLOCK         = 0x55u, /*!< BLOCK is a read accessible
     * register which holds the current value for the total block
     * measurement result. */
    MXM_REG_TOTAL         = 0x56u, /*!< TOTAL is a read accessible
     * register which holds the current value for the sum of all enabled
     * measurement results within the stack. */
    MXM_REG_DIAG1         = 0x57u, /*!< DIAG1 is a read-only
     * register which contains the Diagnostic result requested by the
     * DIAGCFG:DIAGSEL1 selection taken during the last ADC acquisition. */
    MXM_REG_DIAG2         = 0x58u, /*!< DIAG2 is a read-only
     * register which contains the Diagnostic result requested by the
     * DIAGCFG:DIAGSEL2 selection taken during the last ADC acquisition. */
    MXM_REG_AUX0          = 0x59u, /*!< AUXn is a read accessible
     * register which holds the current value for each enabled individual
     * auxiliary measurement result. */
    MXM_REG_AUX1          = 0x5Au, /*!< AUXn is a read accessible
     * register which holds the current value for each enabled individual
     * auxiliary measurement result. */
    MXM_REG_AUX2          = 0x5Bu, /*!< AUXn is a read accessible
     * register which holds the current value for each enabled individual
     * auxiliary measurement result. */
    MXM_REG_AUX3          = 0x5Cu, /*!< AUXn is a read accessible
     * register which holds the current value for each enabled individual
     * auxiliary measurement result. */
    MXM_REG_AUX4          = 0x5Du, /*!< MAX17853 only: AUXn is a
     * read accessible register which holds the current value for each
     * enabled individual auxiliary measurement result. */
    MXM_REG_AUX5          = 0x5Eu, /*!< MAX17853 only: AUXn is a
     * read accessible register which holds the current value for each
     * enabled individual auxiliary measurement result. */
    MXM_REG_POLARITYCTRL  = 0x5Fu, /*!< POLARITYCTRL is a read and
     * write accessible register which governs the measurement type
     * used during scans. In general, Unipolar mode indicates a Cell and
     * Bipolar mode indicates a Bus-Bar. */
    MXM_REG_AUXREFCTRL    = 0x60u, /*!< AUXREFCTRL is a read and
     * write accessible register which governs the reference range used
     * for enabled Auxiliary channels during ADC and COMP acquisition
     * sequences. */
    MXM_REG_AUXTIME       = 0x61u, /*!< AUXTIME is a read
     * and write accessible register which governs the setting time
     * allowed for biasing AUX/GPIO pins prior to measurements. */
    MXM_REG_ACQCFG        = 0x62u, /*!< ACQCFG is a read and write
     * accessible register which governs several aspects of the
     * measurement and acquisition procedure. */
    MXM_REG_BALSWDLY      = 0x63u, /*!< BALSWDLY is a read and
     * write accessible register which selects the delay intervals
     * used within Manual and Automated Cell Balancing operations
     * when ADC measurements are requested. */
    MXM_REG_MEASUREEN1    = 0x64u, /*!< MEASUREEN1 is a read and write
     * accessible register which governs the channels measured during
     * ADC and COMP acquisition sequences. */
    MXM_REG_MEASUREEN2    = 0x65u, /*!< MEASUREEN2 is a read and write
     * accessible register which governs the auxiliary channels measured
     * during ADC and COMP acquisition sequences, as well as
     * IIR initialization. */
    MXM_REG_SCANCTRL      = 0x66u, /*!< SCANCTRL is a read and write
     * accessible register which governs the internal measurement
     * acquisitions (scan) requested of the device. The register also manages  NOLINT
     * the handling of data generated as a result of any scan request.
     *
     * ADC Scans are used for precision measurements of cell and auxiliary
     * voltages.
     *
     * COMP Scans are used for periodic safety/redundancy checking
     * of ADC results, and in some cases, enhanced communication efficiency.
     *
     * On Demand Calibration will run an internal calibration of the ADC and
     * update the Calibration Data Registers. All ADC measurements requested
     * by Scan and Diagnostic Configuration and Control settings will be
     * ignored.
     *
     * Balance Switch and Cell Sense Wire Open ADC Diagnostic Scans are a
     * special class of ADC Scan. Use of these settings temporarily
     * overrides other Scan and Diagnostic Configuration and Control settings.
     * See BALSW and Cell Sense Wire Diagnostics for details. */
    MXM_REG_ADCTEST1A     = 0x67u, /*!< ADCTEST1A is a read and write
     * accessible register which contains user specified arguments used in ALU
     * Diagnostics. */
    MXM_REG_ADCTEST1B     = 0x68u, /*!< ADCTEST1B is a read and write
     * accessible register which contains user specified arguments used in ALU
     * Diagnostics. */
    MXM_REG_ADCTEST2A     = 0x69u, /*!< ADCTEST2A is a read and write
     * accessible register which contains user specified arguments used in ALU
     * Diagnostics. */
    MXM_REG_ADCTEST2B     = 0x6Au, /*!< ADCTEST2B is a read and write
     * accessible register which contains user specified arguments used in ALU
     * Diagnostics. */
    MXM_REG_DIAGCFG       = 0x6Bu, /*!< DIAGCFG is a read and write
     * accessible  register which governs diagnostic source and mode options
     * applied to the internal measurement acquisitions (scans).  NOLINT */
    MXM_REG_CTSTCFG       = 0x6Cu, /*!< CTSTCFG is a read and write
     * accessible register which controls the application of diagnostic
     * current sources to selected cell input channels. */
    MXM_REG_AUXSTCFG      = 0x6Du, /*!< AUXSTCFG is a read and write
     * accessible register which controls the application of diagnostic modes
     * and current source to selected Auxiliary and/or CSA input channels. */
    MXM_REG_DIAGGENCFG    = 0x6Eu, /*!< DIAGGENCFG is a read and write
     * accessible register which controls the application of general diagnostic
     * modes to the selected Auxiliary and/or CSA input paths. */
    MXM_REG_BALSWCTRL     = 0x6Fu, /*!< BALSWCTRL is a read and write
     * accessible register which governs the behavior of the Charge Balancing
     * Switches in Manual and Auto Cell Balancing modes.
     *
     * Write access to this register is blocked during Automated Cell
     * Balancing operations (CBMODE=001, 1xx).  NOLINT */
    MXM_REG_BALEXP1       = 0x70u, /*!< BALEXPn is a read and write
     * accessible register which holds the Cell Balancing Expiration Time
     * for CELLn (using the switch across SWn and SWn-1).
     *
     * BALEXP1 sets the Expiration Time for all Group Auto Cell Balancing and
     * Discharge modes and the Watchdog time out for Manual Cell Balancing mode.
     *
     * Write access to this register is blocked during all Cell Balancing
     * operations (CBMODE!=000).  NOLINT */
    MXM_REG_BALEXP2       = 0x71u, /*!< BALEXPn is a read and write
     * accessible register which holds the Cell Balancing Expiration Time
     * for CELLn (using the switch across SWn and SWn-1). Used in Individual
     * Auto Cell Balancing modes only.
     *
     * Write access to this register is blocked during Automatic Cell
     * Balancing operations (CBMODE=1xx).  NOLINT */
    MXM_REG_BALEXP3       = 0x72u, /*!< BALEXPn is a read and write
     * accessible register which holds the Cell Balancing Expiration Time
     * for CELLn (using the switch across SWn and SWn-1). Used in Individual
     * Auto Cell Balancing modes only.
     *
     * Write access to this register is blocked during Automatic Cell
     * Balancing operations (CBMODE=1xx).  NOLINT */
    MXM_REG_BALEXP4       = 0x73u, /*!< BALEXPn is a read and write
     * accessible register which holds the Cell Balancing Expiration Time
     * for CELLn (using the switch across SWn and SWn-1). Used in Individual
     * Auto Cell Balancing modes only.
     *
     * Write access to this register is blocked during Automatic Cell
     * Balancing operations (CBMODE=1xx).  NOLINT */
    MXM_REG_BALEXP5       = 0x74u, /*!< BALEXPn is a read and write
     * accessible register which holds the Cell Balancing Expiration Time
     * for CELLn (using the switch across SWn and SWn-1). Used in Individual
     * Auto Cell Balancing modes only.
     *
     * Write access to this register is blocked during Automatic Cell
     * Balancing operations (CBMODE=1xx).  NOLINT */
    MXM_REG_BALEXP6       = 0x75u, /*!< BALEXPn is a read and write
     * accessible register which holds the Cell Balancing Expiration Time
     * for CELLn (using the switch across SWn and SWn-1). Used in Individual
     * Auto Cell Balancing modes only.
     *
     * Write access to this register is blocked during Automatic Cell
     * Balancing operations (CBMODE=1xx).  NOLINT */
    MXM_REG_BALEXP7       = 0x76u, /*!< BALEXPn is a read and write
     * accessible register which holds the Cell Balancing Expiration Time
     * for CELLn (using the switch across SWn and SWn-1). Used in Individual
     * Auto Cell Balancing modes only.
     *
     * Write access to this register is blocked during Automatic Cell
     * Balancing operations (CBMODE=1xx).  NOLINT */
    MXM_REG_BALEXP8       = 0x77u, /*!< BALEXPn is a read and write
     * accessible register which holds the Cell Balancing Expiration Time
     * for CELLn (using the switch across SWn and SWn-1). Used in Individual
     * Auto Cell Balancing modes only.
     *
     * Write access to this register is blocked during Automatic Cell
     * Balancing operations (CBMODE=1xx).  NOLINT */
    MXM_REG_BALEXP9       = 0x78u, /*!< BALEXPn is a read and write
     * accessible register which holds the Cell Balancing Expiration Time
     * for CELLn (using the switch across SWn and SWn-1). Used in Individual
     * Auto Cell Balancing modes only.
     *
     * Write access to this register is blocked during Automatic Cell
     * Balancing operations (CBMODE=1xx).  NOLINT */
    MXM_REG_BALEXP10      = 0x79u, /*!< BALEXPn is a read and write
     * accessible register which holds the Cell Balancing Expiration Time
     * for CELLn (using the switch across SWn and SWn-1). Used in Individual
     * Auto Cell Balancing modes only.
     *
     * Write access to this register is blocked during Automatic Cell
     * Balancing operations (CBMODE=1xx).  NOLINT */
    MXM_REG_BALEXP11      = 0x7Au, /*!< BALEXPn is a read and write
     * accessible register which holds the Cell Balancing Expiration Time
     * for CELLn (using the switch across SWn and SWn-1). Used in Individual
     * Auto Cell Balancing modes only.
     *
     * Write access to this register is blocked during Automatic Cell
     * Balancing operations (CBMODE=1xx).  NOLINT */
    MXM_REG_BALEXP12      = 0x7Bu, /*!< BALEXPn is a read and write
     * accessible register which holds the Cell Balancing Expiration Time
     * for CELLn (using the switch across SWn and SWn-1). Used in Individual
     * Auto Cell Balancing modes only.
     *
     * Write access to this register is blocked during Automatic Cell
     * Balancing operations (CBMODE=1xx).  NOLINT */
    MXM_REG_BALEXP13      = 0x7Cu, /*!< BALEXPn is a read and write
     * accessible register which holds the Cell Balancing Expiration Time
     * for CELLn (using the switch across SWn and SWn-1). Used in Individual
     * Auto Cell Balancing modes only.
     *
     * Write access to this register is blocked during Automatic Cell
     * Balancing operations (CBMODE=1xx).  NOLINT */
    MXM_REG_BALEXP14      = 0x7Du, /*!< BALEXPn is a read and write
     * accessible register which holds the Cell Balancing Expiration Time
     * for CELLn (using the switch across SWn and SWn-1). Used in Individual
     * Auto Cell Balancing modes only.
     *
     * Write access to this register is blocked during Automatic Cell
     * Balancing operations (CBMODE=1xx).  NOLINT */
    MXM_REG_BALAUTOUVTHR  = 0x7Eu, /*!< BALAUTOUVTHR is a read and write
     * accessible register which selects the cell under voltage exit threshold
     * for the ADC when used in Automated Cell Balancing operations.
     *
     * A write to this register allows direct setting or automatic selection
     * of this threshold.
     *
     * Write access to this register is blocked during Automatic Cell
     * Balancing operations (CBMODE=1xx). Also, during active measurement scans,  NOLINT
     * all writes with CBUVMINCELL=1 will be blocked and will result in ALRTRJCT  NOLINT
     * being issued (since the MINCELL data may be altered as a result of the  NOLINT
     * scan in progress).
     *
     * A read from this register will display the current value of the threshold
     * and the method used for its selection. */
    MXM_REG_BALDLYCTRL    = 0x7Fu, /*!< BALDLYCTRL is a read and write
     * accessible register which selects the delay/timing intervals used within
     * Automated Cell Balancing operations.
     *
     * Write access to this register is blocked during Automated Cell Balancing
     * operations (CBMODE=001, 1xx).  NOLINT */
    MXM_REG_BALCTRL       = 0x80u, /*!< BALCTRL is a read and write
     * accessible register which initiates and controls all internal Cell
     * Balancing modes and operations.
     *
     * Any write to this register to a mode other than CBMODE=000 (Disable)  NOLINT
     * will restart the CBTIMER at zero and launch the requested mode of
     * operation. */
    MXM_REG_BALSTAT       = 0x81u, /*!< BALSTAT is a read accessible
     * register which allows the monitoring of any Automated Cell Balancing
     * operations currently in progress.
     *
     * Once a CBMODE is initiated, all status bits persist and are cleared
     * only when CBMODE is written to 000 (Disabled) or when a new CBMODE
     * operation is initiated via CBSTART. */
    MXM_REG_BALUVSTAT     = 0x82u, /*!< BALUVSTAT is a read accessible
     * register which relates current summary information on the Cell
     * voltages vs. the CBUVTHR undervoltage threshold. */
    MXM_REG_BALDATA       = 0x83u, /*!< BALDATA is a read accessible
     * register which relates current summary information on the Cell voltage
     * vs. the CBUVTHR undervoltage threshold. */
    MXM_REG_I2CPNTR       = 0x84u, /*!< MAX17852 only: I2CPNTR is a
     * read and write accessible register which contains two Pointer bytes
     * (Register Addresses) available for I2C Master transactions.
     *
     * Once I2CSEND initiates a read or write transaction, attempts to write
     * I2CPNTR during the transaction will be ignored and will case an
     * I2CRJCT fault to be issued. */
    MXM_REG_I2CWDATA1     = 0x85u, /*!< MAX17852 only: I2CWDATA1 is a
     * read and write accessible register which contains the upper data bytes
     * available for I2C Master Write Mode transactions.
     *
     * Once I2CSEND initiates an I2C read or write transaction, attempts to
     * write I2CWDATA1 during the transaction will be ignored and will cause
     * an I2CRJCT fault to be issued. */
    MXM_REG_I2CWDATA2     = 0x86u, /*!< MAX17852 only: I2CWDATA2 is a
     * read and write accessible register which contains the lower data bytes
     * available for I2C Master Write Mode transactions.
     *
     * Once I2CSEND initiates an I2C read or write transaction, attempts to
     * write I2CWDATA2 during the transaction will be ignored and will cause
     * an I2CRJCT fault to be issued. */
    MXM_REG_I2CRDATA1     = 0x87u, /*!< MAX17852 only: I2CRDATA1 is a
     * read accessible register which contains the upper data bytes received
     * for I2C Master Read Mode transactions.
     *
     * Note during I2C read transactions, data is updated as each byte is
     * received/acknowledged, so reading back this register during active
     * I2C read transactions will cause an I2CRJCT fault to be issued. */
    MXM_REG_I2CRDATA2     = 0x88u, /*!< MAX17852 only: I2CRDATA2 is a
     * read accessible register which contains the lower data bytes received
     * for I2C Master Read Mode transactions.
     *
     * Note during I2C read transactions, data is updated as each byte is
     * received/acknowledged, so reading back this register during active
     * I2C read transactions will cause an I2CRJCT fault to be issued. */
    MXM_REG_I2CCFG        = 0x89u, /*!< MAX17852 only: I2CCFG is a
     * read and write accessible register which configures I2C Master
     * modes and transaction formats.
     *
     * Once I2CSEND initiates a read or write transaction, attempts to
     * write I2CCFG during the transaction will be ignored and will cause
     * and I2CRJCT fault to be issued. */
    MXM_REG_I2CSTAT       = 0x8Au, /*!< MAX17852 only: I2CSTAT is
     * a read and write accessible register which shows the current status
     * of the I2C Master.
     *
     * The I2CSTATUS bits are updated in real time, indicating the current
     * state of the I2C Master and any requested transaction. This status
     * can be cleared by write operations and modified by transaction progress
     * or subsequent transaction requests.
     *
     * The second byte contains I2C Fault bits, indicating a fault was
     * observed during an I2C transaction. These bits are updated as they
     * occur and are only cleared by writing to zero. Several faults may
     * occur during a corrupted transaction, so it is best to wait until
     * I2CSTATUS reads 10 (Transaction Error) to ensure all errors have  NOLINT
     * been reported.
     *
     * While not advisable, if further I2C transactions are requested before
     * the I2C Fault bits from previous transactions are read back and cleared,
     * a cumulative history of faults will be listed, even if subsequent
     * transactions are successful. */
    MXM_REG_I2CSEND       = 0x8Bu, /*!< MAX17852 only: I2CSEND is a
     * read and write accessible register which configures and initiates
     * an I2C Master transaction.
     *
     * A write to this register will initiate an I2C Master transaction.
     * Only one transaction is supported at any given time. If a write to
     * I2CSEND occurs during an active I2C transaction already in progress,
     * the latest transaction request will be ignored and the I2CSEND
     * contents will not be updated. If this occurs, an I2CRJCT fault will
     * be issued.
     *
     * A read from this register will readback the current contents. This
     * will represent the last transaction request accepted by the I2C Master. */
    MXM_REG_ID1           = 0x8Cu, /*!< ID1 is a read accessible register
     * which contains the 2 LSBytes of the unique Device ID stored in ROM and
     * subject to ROMCRC validation. */
    MXM_REG_ID2           = 0x8Du, /*!< ID2 is a read accessible register
     * which contains the 2 MSBytes of the unique Device ID stored in ROM and
     * subject to ROMCRC validation. */
    MXM_REG_OTP2          = 0x8Eu, /*!< Factory Calibration Data ROM
         * and subject to ROMCRC validation. */
    MXM_REG_OTP3          = 0x8Fu, /*!< Factory Calibration Data ROM
         * and subject to ROMCRC validation. */
    MXM_REG_OTP4          = 0x90u, /*!< Factory Calibration Data ROM
         * and subject to ROMCRC validation. */
    MXM_REG_OTP5          = 0x91u, /*!< Factory Calibration Data ROM
         * and subject to ROMCRC validation. */
    MXM_REG_OTP6          = 0x92u, /*!< Factory Calibration Data ROM
         * and subject to ROMCRC validation. */
    MXM_REG_OTP7          = 0x93u, /*!< Factory Calibration Data ROM
         * and subject to ROMCRC validation. */
    MXM_REG_OTP8          = 0x94u, /*!< Factory Calibration Data ROM
         * and subject to ROMCRC validation. */
    MXM_REG_OTP9          = 0x95u, /*!< Factory Calibration Data ROM
         * and subject to ROMCRC validation. */
    MXM_REG_OTP10         = 0x96u, /*!< Factory Calibration Data ROM
         * and subject to ROMCRC validation. */
    MXM_REG_OTP11         = 0x97u, /*!< Factory Calibration Data ROM
         * and subject to ROMCRC validation. */
    MXM_REG_OTP12         = 0x98u, /*!< Factory Calibration Data ROM
         * and subject to ROMCRC validation. */
} MXM_REG_NAME_e;

/**
 * @brief Type for register access for monitoring ICs.
 */
typedef uint16_t MXM_REG_BM;

/**
 * @brief      Define enumeration values for bitmasks
 * @details    Generates a bitmask from start to end.
 *             Examples:
 *                  - 0000 0001b == MXM_REG_BIT_DEFN(0, 0)
 *                  - 0000 1000b == MXM_REG_BIT_DEFN(3, 3)
 *                  - 0011 1000b == MXM_REG_BIT_DEFN(3, 5)
 */
#define MXM_REG_BIT_DEFN(start, end) ((MXM_REG_BM)(((MXM_REG_BM)1u << ((end) - (start) + 1u)) - 1u) << (start))

/**
 * @defgroup mxm_bm Bitmasks for register access of MAX1785x
 * @{
 */
/**
 * @brief Monitoring Register Null byte
 */
#define MXM_BM_NULL ((MXM_REG_BM)0x00u)
/**
 * @brief Monitoring Register LSB
 */
#define MXM_BM_LSB ((MXM_REG_BM)MXM_REG_BIT_DEFN(0u, 7u))
/**
 * @brief Monitoring Register MSB
 */
#define MXM_BM_MSB ((MXM_REG_BM)MXM_REG_BIT_DEFN(8u, 15u))
/**
 * @brief All bits of monitoring register
 */
#define MXM_BM_WHOLE_REG ((MXM_REG_BM)0xFFFFu)
/**
 * @brief Monitoring Register Version/Model
 */
#define MXM_REG_VERSION_MOD ((MXM_REG_BM)MXM_REG_BIT_DEFN(4u, 15u))
/**
 * @brief Monitoring Register Version/Silicon Version
 */
#define MXM_REG_VERSION_VER ((MXM_REG_BM)MXM_REG_BIT_DEFN(0u, 3u))
/**
 * @brief Monitoring Register 14bit ADC value
 */
#define MXM_REG_ADC_14BIT_VALUE ((MXM_REG_BM)MXM_REG_BIT_DEFN(2u, 15u))
/**
 * @brief ALRTRST bit in STATUS1
 */
#define MXM_REG_STATUS1_ALRTRST ((MXM_REG_BM)((MXM_REG_BM)(1u) << (14u)))
/**
 * @brief ALRTINTRFC bit in STATUS1
 */
#define MXM_REG_STATUS1_ALRTINTRFC ((MXM_REG_BM)((1u) << (4u)))

/**
 * @}
 */

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__MXM_REGISTER_MAP_H_ */
