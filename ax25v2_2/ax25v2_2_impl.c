/*
 *  Project: ax25c - File: ax25v2_2_impl.c
 *  Copyright (C) 2019 - Tania Hagn - tania@df9ry.de
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../runtime/primitive.h"
#include "../runtime/memory.h"

#include "ax25v2_2.h"
#include "callsign.h"

#include <errno.h>
#include <assert.h>

static inline uint16_t updateCrc(uint8_t b, uint16_t crc)
{
	uint8_t ch = (b ^ (crc & 0x00ff));
	ch = (ch ^ (ch << 4));
	return ((crc >> 8) ^ (ch << 8) ^ (ch << 3) ^ (ch >> 4));
}

static inline uint16_t getCrc(uint8_t *p, uint16_t c)
{
	uint16_t res = 0xffff;
	for (; c-- > 0; ++p)
		res = updateCrc(*p, res);
	return res;
}

primitive_t *new_AX25_I(
		uint16_t cH, uint16_t sH,
		enum L3_PROTOCOL pid,
		bool modulo128,
		struct addressField *af,
		uint8_t nr, uint8_t ns,
		const uint8_t *data, size_t size,
		struct exception *ex)
{
	size_t frame_size;
	size_t i = 0;
	primitive_t *prim;
	uint16_t crc;

	assert(af);
	assert(nr >= 0);
	assert(ns >= 0);
	frame_size =
			getFrameAddressLength(af) /* Adress Field  */
			+ (modulo128 ? 2 : 1)     /* Control field */
			+ 1                       /* PID           */
			+ size                    /* Data field    */
			+ 2;                      /* CRC           */
	prim = new_prim(frame_size, AX25, AX25_I, cH, sH, ex);
	if (!prim)
		return NULL;
	i += putFrameAddress(af, &prim->payload[i]);
	prim->payload[i-1] |= 0x01; /* Set X-Bit - just to be sure. */
	if (modulo128) {
		prim->payload[i++] = (nr << 1) | 0x01;
		prim->payload[i++] = ns << 1;
	} else {
		assert(nr < 8);
		assert(ns < 8);
		prim->payload[i++] = (nr << 5) | (ns << 1) | 0x10;
	}
	prim->payload[i++] = pid;
	memcpy(&prim->payload[i], data, size);
	i += size;
	crc = getCrc(prim->payload, i);
	prim->payload[i++] = crc / 0x0100;
	prim->payload[i++] = crc % 0x0100;
	assert(i == frame_size);
	mem_chck(prim);
	return prim;
}

primitive_t *new_AX25_Supervisory(
		uint16_t cH, uint16_t sH,
		AX25_CMD_t ax25_cmd,
		enum L3_PROTOCOL pid,
		bool modulo128,
		struct addressField *af,
		uint8_t nr,
		bool poll,
		struct exception *ex)
{
	size_t frame_size;
	size_t i = 0;
	primitive_t *prim;
	uint16_t crc;

	assert(af);
	assert(nr >= 0);
	frame_size =
			getFrameAddressLength(af) /* Adress Field  */
			+ (modulo128 ? 2 : 1)     /* Control field */
			+ 1                       /* PID           */
			+ 2;                      /* CRC           */
	prim = new_prim(frame_size, AX25, ax25_cmd, cH, sH, ex);
	if (!prim)
		return NULL;
	i += putFrameAddress(af, &prim->payload[i]);
	prim->payload[i-1] |= 0x01; /* Set X-Bit - just to be sure. */
	if (modulo128) {
		prim->payload[i++] = (nr << 1) | poll ? 0x01 : 0x00;
		prim->payload[i++] = ax25_cmd;
	} else {
		assert(nr < 8);
		prim->payload[i++] = (nr << 5) | poll ? 0x80 : 0x00;
	}
	prim->payload[i++] = pid;
	crc = getCrc(prim->payload, i);
	prim->payload[i++] = crc / 0x0100;
	prim->payload[i++] = crc % 0x0100;
	assert(i == frame_size);
	mem_chck(prim);
	return prim;
}

primitive_t *new_AX25_Unnumbered(
		uint16_t cH, uint16_t sH,
		AX25_CMD_t ax25_cmd,
		enum L3_PROTOCOL pid,
		struct addressField *af,
		bool cmd, bool poll,
		const uint8_t *data, size_t size,
		struct exception *ex)
{
	size_t frame_size;
	int i = 0;
	primitive_t *prim;
	uint16_t crc;

	assert(af);
	frame_size =
			getFrameAddressLength(af) /* Adress Field  */
			+ 1                       /* Control field */
			+ 1                       /* PID           */
			+ size                    /* Data field    */
			+ 2;                      /* CRC           */
	prim = new_prim(frame_size, AX25, ax25_cmd, cH, sH, ex);
	if (!prim)
		return NULL;
	i += putFrameAddress(af, &prim->payload[i]);
	prim->payload[i-1] |= 0x01; /* Set X-Bit - just to be sure. */
	prim->payload[i++] = ax25_cmd | poll ? 0x10 : 0x00;
	memcpy(&prim->payload[i], data, size);
	if (cmd) {
		prim->payload[6]  |= 0x10;
		prim->payload[13] &= 0xef;
	} else {
		prim->payload[6]  &= 0xef;
		prim->payload[13] |= 0x10;
	}
	prim->payload[i++] = pid;
	memcpy(&prim->payload[i], data, size);
	i += size;
	crc = getCrc(prim->payload, i);
	prim->payload[i++] = crc / 0x0100;
	prim->payload[i++] = crc % 0x0100;
	assert(i == frame_size);
	mem_chck(prim);
	return prim;
}

AX25_CMD_t prim_get_AX25_CMD(primitive_t *prim)
{
	assert(prim);
	if (prim->protocol != AX25)
		return AX25_INVAL;
	else
		return (AX25_CMD_t) prim->cmd;
}

bool prim_check_AX25_CRC(primitive_t *prim)
{
	assert(prim);
	return false;
}

bool prim_get_AX25_addressField(primitive_t *prim,
		struct addressField *af, struct exception *ex)
{
	assert(prim);
	return false;
}

bool prim_get_AX25_V2(primitive_t *prim)
{
	assert(prim);
	return false;
}

bool prim_get_AX25_CmdRes(primitive_t *prim)
{
	assert(prim);
	return false;
}

bool prim_get_AX25_PollFinal(primitive_t *prim)
{
	assert(prim);
	return false;
}

int8_t prim_get_AX25_NR(primitive_t *prim)
{
	assert(prim);
	return -1;
}

int8_t prim_get_AX25_NS(primitive_t *prim)
{
	assert(prim);
	return -1;
}

bool prim_get_AX25_data(primitive_t *prim,
		uint8_t ** pdata, size_t *psize)
{
	assert(prim);
	return false;
}
