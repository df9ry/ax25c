/*
 *  Project: ax25c - File: dl_prim.c
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

#include "dl_prim.h"
#include "primitive.h"

primitive_t *new_DL_CONNECT_Request(
		uint16_t clientHandle,
		const uint8_t *dstAddrPtr, uint8_t dstAddrSize,
		const uint8_t *srcAddrPtr, uint8_t srcAddrSize,
		struct exception *ex)
{
	int i = 0;
	uint32_t payloadSize = dstAddrSize + srcAddrSize + 4;
	primitive_t *prim = new_prim(payloadSize, DL, DL_CONNECT_REQUEST,
			clientHandle, 0, ex);
	if (!prim)
		return NULL;
	i = put_prim_param(prim, i, dstAddrPtr, dstAddrSize);
	i = put_prim_param(prim, i, srcAddrPtr, srcAddrSize);
	mem_chck(prim);
	return prim;
}

primitive_t *new_DL_CONNECT_Indication(
		uint16_t serverHandle,
		const uint8_t *dstAddrPtr, uint8_t dstAddrSize,
		const uint8_t *srcAddrPtr, uint8_t srcAddrSize,
		struct exception *ex)
{
	int i = 0;
	uint32_t payloadSize = dstAddrSize + srcAddrSize + 4;
	primitive_t *prim = new_prim(payloadSize, DL, DL_CONNECT_INDICATION,
			0, serverHandle, ex);
	if (!prim)
		return NULL;
	i = put_prim_param(prim, i, dstAddrPtr, dstAddrSize);
	i = put_prim_param(prim, i, srcAddrPtr, srcAddrSize);
	mem_chck(prim);
	return prim;
}

primitive_t *new_DL_DATA_Request(
		uint16_t clientHandle, uint16_t serverHandle,
		const uint8_t *pData, uint16_t sData,
		struct exception *ex)
{
	int i = 0;
	uint32_t payloadSize = sData + 2;
	primitive_t *prim = new_prim(payloadSize, DL, DL_DATA_REQUEST,
			clientHandle, serverHandle, ex);
	if (!prim)
		return NULL;
	i = put_prim_param(prim, i, pData, sData);
	mem_chck(prim);
	return prim;
}

primitive_t *new_DL_DATA_Indication(
		uint16_t clientHandle, uint16_t serverHandle,
		const uint8_t *pData, uint16_t sData,
		struct exception *ex)
{
	int i = 0;
	uint32_t payloadSize = sData + 2;
	primitive_t *prim = new_prim(payloadSize, DL, DL_DATA_INDICATION,
			clientHandle, serverHandle, ex);
	if (!prim)
		return NULL;
	i = put_prim_param(prim, i, pData, sData);
	mem_chck(prim);
	return prim;
}

primitive_t *new_DL_UNIT_DATA_Request(
		uint16_t clientHandle,
		const uint8_t *dstAddrPtr, uint8_t dstAddrSize,
		const uint8_t *srcAddrPtr, uint8_t srcAddrSize,
		const uint8_t *pData, uint16_t sData,
		struct exception *ex)
{
	int i = 0;
	uint32_t payloadSize = dstAddrSize + srcAddrSize + sData + 6;
	primitive_t *prim = new_prim(payloadSize, DL, DL_UNIT_DATA_REQUEST,
			clientHandle, 0, ex);
	if (!prim)
		return NULL;
	i = put_prim_param(prim, i, dstAddrPtr, dstAddrSize);
	i = put_prim_param(prim, i, srcAddrPtr, srcAddrSize);
	i = put_prim_param(prim, i, pData, sData);
	mem_chck(prim);
	return prim;
}

primitive_t *new_DL_UNIT_DATA_Indication(
		uint16_t serverHandle,
		const uint8_t *dstAddrPtr, uint8_t dstAddrSize,
		const uint8_t *srcAddrPtr, uint8_t srcAddrSize,
		const uint8_t *pData, uint16_t sData,
		struct exception *ex)
{
	int i = 0;
	uint32_t payloadSize = dstAddrSize + srcAddrSize + sData + 6;
	primitive_t *prim = new_prim(payloadSize, DL, DL_UNIT_DATA_INDICATION,
			0, serverHandle, ex);
	if (!prim)
		return NULL;
	i = put_prim_param(prim, i, dstAddrPtr, dstAddrSize);
	i = put_prim_param(prim, i, srcAddrPtr, srcAddrSize);
	i = put_prim_param(prim, i, pData, sData);
	mem_chck(prim);
	return prim;
}

primitive_t *new_DL_TEST_Request(
		uint16_t clientHandle,
		const uint8_t *dstAddrPtr, uint8_t dstAddrSize,
		const uint8_t *srcAddrPtr, uint8_t srcAddrSize,
		const uint8_t *pData, uint16_t sData,
		struct exception *ex)
{
	int i = 0;
	uint32_t payloadSize = dstAddrSize + srcAddrSize + sData + 6;
	primitive_t *prim = new_prim(payloadSize, DL, DL_TEST_REQUEST,
			clientHandle, 0, ex);
	if (!prim)
		return NULL;
	i = put_prim_param(prim, i, dstAddrPtr, dstAddrSize);
	i = put_prim_param(prim, i, srcAddrPtr, srcAddrSize);
	i = put_prim_param(prim, i, pData, sData);
	mem_chck(prim);
	return prim;
}

primitive_t *new_DL_TEST_Indication(
		uint16_t serverHandle,
		const uint8_t *dstAddrPtr, uint8_t dstAddrSize,
		const uint8_t *srcAddrPtr, uint8_t srcAddrSize,
		const uint8_t *pData, uint16_t sData,
		struct exception *ex)
{
	int i = 0;
	uint32_t payloadSize = dstAddrSize + srcAddrSize + sData + 6;
	primitive_t *prim = new_prim(payloadSize, DL, DL_TEST_INDICATION,
			0, serverHandle, ex);
	if (!prim)
		return NULL;
	i = put_prim_param(prim, i, dstAddrPtr, dstAddrSize);
	i = put_prim_param(prim, i, srcAddrPtr, srcAddrSize);
	i = put_prim_param(prim, i, pData, sData);
	mem_chck(prim);
	return prim;
}

primitive_t *new_DL_TEST_Confirmation(
		uint16_t clientHandle, uint16_t serverHandle,
		const uint8_t *dstAddrPtr, uint8_t dstAddrSize,
		const uint8_t *srcAddrPtr, uint8_t srcAddrSize,
		const uint8_t *pData, uint16_t sData,
		struct exception *ex)
{
	int i = 0;
	uint32_t payloadSize = dstAddrSize + srcAddrSize + sData + 6;
	primitive_t *prim = new_prim(payloadSize, DL, DL_TEST_CONFIRM,
			clientHandle, serverHandle, ex);
	if (!prim)
		return NULL;
	i = put_prim_param(prim, i, dstAddrPtr, dstAddrSize);
	i = put_prim_param(prim, i, srcAddrPtr, srcAddrSize);
	i = put_prim_param(prim, i, pData, sData);
	mem_chck(prim);
	return prim;
}
