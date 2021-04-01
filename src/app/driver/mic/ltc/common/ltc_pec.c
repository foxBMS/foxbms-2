/*!
    LTC6804-1 Multicell Battery Monitor
@verbatim
    The LTC6804 is a 3rd generation multicell battery stack
    monitor that measures up to 12 series connected battery
    cells with a total measurement error of less than 1.2mV. The
    cell measurement range of 0V to 5V makes the LTC6804
    suitable for most battery chemistries. All 12 cell voltages
    can be captured in 290uS, and lower data acquisition rates
    can be selected for high noise reduction.

    Using the LTC6804-1, multiple devices are connected in
    a daisy-chain with one host processor connection for all
    devices.
@endverbatim
REVISION HISTORY
$Revision: 1000 $
$Date: 2013-12-13

Copyright (c) 2013, Linear Technology Corp.(LTC)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of Linear Technology Corp.

The Linear Technology Linduino is not affiliated with the official Arduino team.
However, the Linduino is only possible because of the Arduino team's commitment
to the open-source community.  Please, visit http://www.arduino.cc and
http://store.arduino.cc , and consider a purchase that will help fund their
ongoing work.

Copyright 2013 Linear Technology Corp. (LTC)
***********************************************************/

/*!**********************************************************
 \brief calaculates  and returns the CRC15

  @param[in] uint8_t len: the length of the data array being passed to the function

  @param[in] uint8_t data[] : the array of data that the PEC will be generated from


  @returns The calculated pec15 as an unsigned int
***********************************************************/

/*========== Includes =======================================================*/
#include "ltc_pec.h"

/*========== Static Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Extern Function Implementations ================================*/

uint16_t LTC_pec15_calc(uint8_t len, uint8_t *data) {
    uint16_t remainder = 0;
    uint16_t addr      = 0;

    remainder = 16;                                        /* initialize the PEC */
    for (uint8_t i = 0u; i < len; i++) {                   /* loops for each byte in data array */
        addr      = ((remainder >> 7u) ^ data[i]) & 0xFFu; /* calculate PEC table address */
        remainder = (remainder << 8u) ^ crc15Table[addr];
    }
    return (remainder * 2u); /* The CRC15 has a 0 in the LSB so the remainder must be multiplied by 2 */
}
/*================== Public functions =====================================*/

/*================== Static functions =====================================*/
