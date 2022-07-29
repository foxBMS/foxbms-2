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
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY 
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
* THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA; OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
* TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*
 * uc_msg_t.h
 *
 *  Created on: 26.03.2019
 *      Author: nxf19022
 */

#ifndef INC_UC_MSG_T_H_
#define INC_UC_MSG_T_H_

#include <stdint.h>

enum bms1_command {BMS1_CMD_NOP=0, BMS1_CMD_READ, BMS1_CMD_WRITE, BMS1_CMD_RESP};

typedef struct {
	uint16_t dhead;
	uint16_t data[4];
} uc_msg_data_t;

typedef struct {
	uint16_t head;
	uc_msg_data_t data;
	uint16_t crc;
	uint16_t message_length;
} uc_msg_t;

void set_cmd(uc_msg_t*, const uint16_t);
void get_cmd(const uc_msg_t*, uint16_t*);

void set_madd(uc_msg_t*, const uint16_t);
void get_madd(const uc_msg_t*, uint16_t*);

void set_cadd(uc_msg_t*, const uint16_t);
void get_cadd(const uc_msg_t*, uint16_t*);

void set_devadd(uc_msg_t*, const uint16_t);
void get_devadd(const uc_msg_t*, uint16_t*);

void set_msgcnt(uc_msg_t*, const uint16_t);
void get_msgcnt(const uc_msg_t*, uint16_t*);

void set_datalen(uc_msg_t*, const uint16_t);
void get_datalen(const uc_msg_t*, uint16_t*);

void set_regadd(uc_msg_t*, const uint16_t);
void get_regadd(const uc_msg_t*, uint16_t*);

void set_data(uc_msg_t*, const uint16_t, const unsigned);
void get_data(const uc_msg_t*, uint16_t*, const unsigned);

void set_crc(uc_msg_t*, const uint16_t);
void get_crc(const uc_msg_t*, uint16_t*);

uint16_t calc_crc(const uc_msg_t*);

/* returns true if CRC matches argument */
int check_crc(const uc_msg_t*, const uint16_t);

void set_message_length(uc_msg_t*, const uint16_t);
void get_message_length(const uc_msg_t*, uint16_t*);




#endif /* INC_UC_MSG_T_H_ */
