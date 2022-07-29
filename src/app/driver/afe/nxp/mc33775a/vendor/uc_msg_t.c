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
 * uc_msg_t implementation
 *
 *  Created on: 27.03.2019
 *      Author: dep10505
 */

#include <uc_msg_t.h>

void set_cmd(uc_msg_t *msg, const uint16_t cmd) {
	(msg->head) = ((cmd &0x3) << 14) | ((msg->head) & 0x3FFF);
}

void get_cmd(const uc_msg_t *msg, uint16_t *cmd) {
	*cmd = ((msg->head) & 0xC000) >> 14;
}

void set_madd(uc_msg_t *msg, const uint16_t madd) {
	(msg->head) = ((madd & 0x1)<< 13) | ((msg->head) & 0xDFFF);
}

void get_madd(const uc_msg_t *msg, uint16_t *madd) {
	*madd = ((msg->head) & 0x2000) >> 13;
}

void set_cadd(uc_msg_t *msg, const uint16_t cadd) {
	(msg->head) = ((cadd & 0x7) << 10) | ((msg->head) & 0xE3FF);
}

void get_cadd(const uc_msg_t *msg, uint16_t *cadd) {
	*cadd = ((msg->head) & 0x1C00) >> 10;
}

void set_devadd(uc_msg_t *msg, const uint16_t devadd) {
	(msg->head) = ((devadd & 0x3F) << 4) | ((msg->head) & 0xFC0F);
}

void get_devadd(const uc_msg_t *msg, uint16_t *devadd) {
	*devadd = ((msg->head) & 0x03F0) >> 4;
}

void set_msgcnt(uc_msg_t *msg, const uint16_t msgcnt) {
	(msg->head) = (msgcnt & 0xF) | ((msg->head) & 0xFFF0);
}

void get_msgcnt(const uc_msg_t *msg, uint16_t *msgcnt) {
	*msgcnt = ((msg->head) & 0x000F);
}

void set_datalen(uc_msg_t *msg, const uint16_t datalen) {
	(msg->data.dhead) = ((datalen & 0x3) << 14) | ((msg->data.dhead) & 0x3FFF);
}

void get_datalen(const uc_msg_t *msg, uint16_t *datalen) {
	*datalen = ((msg->data.dhead) & 0xC000) >> 14;
}

void set_regadd(uc_msg_t *msg, const uint16_t regadd) {
	(msg->data.dhead) = (regadd & 0x3FFF) | ((msg->data.dhead) & 0xC000);
}

void get_regadd(const uc_msg_t *msg, uint16_t *regadd) {
	*regadd = (msg->data.dhead) &  0x3FFF;
}

void set_data(uc_msg_t *msg, const uint16_t data, const unsigned index) {
	(msg->data.data[index]) = data;
}

void get_data(const uc_msg_t *msg, uint16_t *data, const unsigned index) {
	*data = msg->data.data[index];
}

void set_crc(uc_msg_t *msg, const uint16_t crc) {
	(msg->crc) = crc;
}

void get_crc(const uc_msg_t *msg, uint16_t *crc) {
	*crc = (msg->crc);
}

/* Calculate the CRC of a message */
uint16_t crc_add_item(uint16_t remainder, uint16_t item) {
    int      i;

    remainder ^= item;

    for (i=0; i < 16; i++)
    {
        /*
         * Try to divide the current data bit.
         */
        if (remainder & 0x8000) {
            remainder = (remainder << 1) ^ ((0x9eb2 << 1) + 0x1);
        } else {
            remainder = (remainder << 1);
        }
    }

    return (remainder);
}

uint16_t calc_crc(const uc_msg_t *msg) {
	uint16_t  remainder = 0;
	int       i;

	remainder = crc_add_item(remainder, msg->head);
	remainder = crc_add_item(remainder, msg->data.dhead);

	for (i=0; i < (msg->message_length - 3); i++) {
		remainder = crc_add_item(remainder, msg->data.data[i]);
	}

	return(remainder);
}

/* returns true if CRC matches argument */
int check_crc(const uc_msg_t *msg, const uint16_t ref_crc) {
    uint16_t msg_crc;
    
    get_crc(msg, &msg_crc);
    
	return (msg_crc ==  ref_crc);
}

void set_message_length(uc_msg_t *msg, const uint16_t length) {
	msg->message_length = length;
}

void get_message_length(const uc_msg_t *msg, uint16_t *length) {
	*length = msg->message_length;
}
