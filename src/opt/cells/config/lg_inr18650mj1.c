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
 * @file    lg_inr18650mj1.c
 * @author  foxBMS Team
 * @date    2017-11-07 (date of creation)
 * @updated 2020-07-31 (date of last update)
 * @ingroup BATTERY_CELL_CONF
 * @prefix  BC
 *
 * @brief   Configuration of the battery cell
 *
 * This files contains battery cell LUTs
 *
 */

/*========== Includes =======================================================*/
#include "lg_inr18650mj1.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/** SOC lookup table starting with 100% */
const BC_LUT_s bc_lginr18650mj1SocLut[] = {
    {4123, 100.0f}, {4088, 99.0f}, {4072, 98.0f}, {4060, 97.0f}, {4052, 96.0f}, {4045, 95.0f}, {4040, 94.0f},
    {4036, 93.0f},  {4032, 92.0f}, {4028, 91.0f}, {4024, 90.0f}, {4020, 89.0f}, {4015, 88.0f}, {4009, 87.0f},
    {4001, 86.0f},  {3992, 85.0f}, {3982, 84.0f}, {3972, 83.0f}, {3961, 82.0f}, {3950, 81.0f}, {3939, 80.0f},
    {3927, 79.0f},  {3916, 78.0f}, {3905, 77.0f}, {3895, 76.0f}, {3885, 75.0f}, {3876, 74.0f}, {3867, 73.0f},
    {3857, 72.0f},  {3849, 71.0f}, {3840, 70.0f}, {3831, 69.0f}, {3822, 68.0f}, {3812, 67.0f}, {3802, 66.0f},
    {3791, 65.0f},  {3780, 64.0f}, {3769, 63.0f}, {3757, 62.0f}, {3746, 61.0f}, {3735, 60.0f}, {3724, 59.0f},
    {3713, 58.0f},  {3703, 57.0f}, {3693, 56.0f}, {3683, 55.0f}, {3673, 54.0f}, {3663, 53.0f}, {3653, 52.0f},
    {3644, 51.0f},  {3636, 50.0f}, {3628, 49.0f}, {3620, 48.0f}, {3613, 47.0f}, {3606, 46.0f}, {3599, 45.0f},
    {3592, 44.0f},  {3586, 43.0f}, {3580, 42.0f}, {3574, 41.0f}, {3568, 40.0f}, {3563, 39.0f}, {3557, 38.0f},
    {3551, 37.0f},  {3544, 36.0f}, {3538, 35.0f}, {3532, 34.0f}, {3525, 33.0f}, {3518, 32.0f}, {3510, 31.0f},
    {3501, 30.0f},  {3493, 29.0f}, {3483, 28.0f}, {3472, 27.0f}, {3461, 26.0f}, {3448, 25.0f}, {3435, 24.0f},
    {3422, 23.0f},  {3410, 22.0f}, {3400, 21.0f}, {3392, 20.0f}, {3383, 19.0f}, {3374, 18.0f}, {3363, 17.0f},
    {3351, 16.0f},  {3334, 15.0f}, {3310, 14.0f}, {3287, 13.0f}, {3261, 12.0f}, {3234, 11.0f}, {3202, 10.0f},
    {3167, 9.0f},   {3129, 8.0f},  {3087, 7.0f},  {3046, 6.0f},  {3003, 5.0f},  {2957, 4.0f},  {2901, 3.0f},
    {2825, 2.0f},   {2716, 1.0f},
};

/** SOE Lookup table in 1% steps starting with 100% */
const BC_LUT_s bc_lginr18650mj1SoeLut[] = {
    {4163, 100.0f}, {4145, 99.0f}, {4131, 98.0f}, {4121, 97.0f}, {4113, 96.0f}, {4106, 95.0f}, {4101, 94.0f},
    {4096, 93.0f},  {4092, 92.0f}, {4089, 91.0f}, {4085, 90.0f}, {4081, 89.0f}, {4077, 88.0f}, {4072, 87.0f},
    {4066, 86.0f},  {4058, 85.0f}, {4050, 84.0f}, {4041, 83.0f}, {4032, 82.0f}, {4022, 81.0f}, {4011, 80.0f},
    {4001, 79.0f},  {3991, 78.0f}, {3980, 77.0f}, {3970, 76.0f}, {3961, 75.0f}, {3951, 74.0f}, {3942, 73.0f},
    {3934, 72.0f},  {3926, 71.0f}, {3918, 70.0f}, {3910, 69.0f}, {3902, 68.0f}, {3895, 67.0f}, {3887, 66.0f},
    {3878, 65.0f},  {3870, 64.0f}, {3860, 63.0f}, {3851, 62.0f}, {3841, 61.0f}, {3831, 60.0f}, {3821, 59.0f},
    {3811, 58.0f},  {3800, 57.0f}, {3789, 56.0f}, {3778, 55.0f}, {3767, 54.0f}, {3756, 53.0f}, {3746, 52.0f},
    {3736, 51.0f},  {3726, 50.0f}, {3717, 49.0f}, {3708, 48.0f}, {3699, 47.0f}, {3691, 46.0f}, {3683, 45.0f},
    {3675, 44.0f},  {3668, 43.0f}, {3661, 42.0f}, {3655, 41.0f}, {3648, 40.0f}, {3642, 39.0f}, {3636, 38.0f},
    {3630, 37.0f},  {3624, 36.0f}, {3618, 35.0f}, {3612, 34.0f}, {3605, 33.0f}, {3599, 32.0f}, {3592, 31.0f},
    {3585, 30.0f},  {3578, 29.0f}, {3569, 28.0f}, {3561, 27.0f}, {3551, 26.0f}, {3540, 25.0f}, {3528, 24.0f},
    {3516, 23.0f},  {3504, 22.0f}, {3492, 21.0f}, {3482, 20.0f}, {3471, 19.0f}, {3460, 18.0f}, {3449, 17.0f},
    {3437, 16.0f},  {3423, 15.0f}, {3406, 14.0f}, {3390, 13.0f}, {3373, 12.0f}, {3354, 11.0f}, {3332, 10.0f},
    {3307, 9.0f},   {3276, 8.0f},  {3244, 7.0f},  {3212, 6.0f},  {3176, 5.0f},  {3126, 4.0f},  {3050, 3.0f},
    {2929, 2.0f},   {2572, 1.0f},
};

/** length of the SOC lookup table #bc_lginr18650mj1SocLut */
uint16_t bc_lginr18650mj1SocLutLength = sizeof(bc_lginr18650mj1SocLut) / sizeof(BC_LUT_s);
/** length of the SOE lookup table #bc_lginr18650mj1SoeLut */
uint16_t bc_lginr18650mj1SoeLutLength = sizeof(bc_lginr18650mj1SoeLut) / sizeof(BC_LUT_s);

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
