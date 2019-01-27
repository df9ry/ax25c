/*
 *  Project: ax25c - File: dl_prim.h
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

#ifndef RUNTIME_DL_PRIM_H_
#define RUNTIME_DL_PRIM_H_

#include "primitive.h"

#include <stdint.h>

enum CMD {
	DL_CONNECT_REQUEST       =  0,
	DL_CONNECT_INDICATION    =  1,
	DL_CONNECT_CONFIRM       =  2,
	DL_DISCONNECT_REQUEST    =  3,
	DL_DISCONNECT_INDICATION =  4,
	DL_DISCONNECT_CONFIRM    =  5,
	DL_DATA_REQUEST          =  6,
	DL_DATA_INDICATION       =  7,
	DL_UNIT_DATA_REQUEST     =  8,
	DL_UNIT_DATA_INDICATION  =  9,
	DL_ERROR_INDICATION      = 10,
	DL_FLOW_OFF_REQUEST      = 11,
	DL_FLOW_ON_REQUEST       = 12,
	MDL_NEGOTIATE_REQUEST    = 13,
	MDL_NEGOTIATE_CONFIRM    = 14,
	MDL_ERROR_INDICATION     = 15,
	DL_TEST_REQUEST          = 16,
	DL_TEST_INDICATION       = 17,
	DL_TEST_CONFIRM          = 18
};

/**
 * @brief Create a new DL_CONNECT Request.
 * @param clientHandle Handle that will be returned in the response.
 * @param dstAddrPtr Pointer to destination address.
 * @param dstAddrSize Size of destination address.
 * @param srcAddrPtr Pointer to source address.
 * @param srcAddrSie Pointer to source address.
 * @param ex Exception struct.
 * @return New primitive.
 */
extern primitive_t *new_DL_CONNECT_Request(
		uint16_t clientHandle,
		const uint8_t *dstAddrPtr, uint8_t dstAddrSize,
		const uint8_t *srcAddrPtr, uint8_t srcAddrSize,
		struct exception *ex);

/**
 * @brief Create a new DL_CONNECT Indication.
 * @param serverHandle Handle that will be forwarded to the client.
 * @param dstAddrPtr Pointer to destination address.
 * @param dstAddrSize Size of destination address.
 * @param srcAddrPtr Pointer to source address.
 * @param srcAddrSie Pointer to source address.
 * @param ex Exception struct.
 * @return New primitive.
 */
extern primitive_t *new_DL_CONNECT_Indication(
		uint16_t serverHandle,
		const uint8_t *dstAddrPtr, uint8_t dstAddrSize,
		const uint8_t *srcAddrPtr, uint8_t srcAddrSize,
		struct exception *ex);

/**
 * @brief Create a new DL_CONNECT Confirm.
 * @param clientHandle Handle that will be returned in the response.
 * @param serverHandle Handle that will be forwarded to the client.
 * @param ex Exception struct.
 * @return New primitive.
 */
static inline primitive_t *new_DL_CONNECT_Confirm(
		uint16_t clientHandle, uint16_t serverHandle, struct exception *ex)
{
	return new_prim(0, DL, DL_CONNECT_CONFIRM, clientHandle, serverHandle, ex);
}

/**
 * @brief Create a new DL_DISCONNECT Request.
 * @param clientHandle Handle that will be returned in the response.
 * @param serverHandle Handle that will be forwarded to the client.
 * @param ex Exception struct.
 * @return New primitive.
 */
static inline primitive_t *new_DL_DISCONNECT_Request(
		uint16_t clientHandle, uint16_t serverHandle, struct exception *ex)
{
	return new_prim(0, DL, DL_DISCONNECT_REQUEST, clientHandle, serverHandle, ex);
}

/**
 * @brief Create a new DL_DISCONNECT Indication.
 * @param clientHandle Handle that will be returned in the response.
 * @param serverHandle Handle that will be forwarded to the client.
 * @param ex Exception struct.
 * @return New primitive.
 */
static inline primitive_t *new_DL_DISCONNECT_Indication(
		uint16_t clientHandle, uint16_t serverHandle, struct exception *ex)
{
	return new_prim(0, DL, DL_DISCONNECT_INDICATION, clientHandle, serverHandle, ex);
}

/**
 * @brief Create a new DL_DISCONNECT Confirm.
 * @param clientHandle Handle that will be returned in the response.
 * @param serverHandle Handle that will be forwarded to the client.
 * @param ex Exception struct.
 * @return New primitive.
 */
static inline primitive_t *new_DL_DISCONNECT_Confirm(
		uint16_t clientHandle, uint16_t serverHandle, struct exception *ex)
{
	return new_prim(0, DL, DL_DISCONNECT_CONFIRM, clientHandle, serverHandle, ex);
}

/**
 * @brief Create a new DL_DATA_Request.
 * @param clientHandle Handle that will be returned in the response.
 * @param serverHandle Handle that will be forwarded to the client.
 * @param pData Pointer to the data.
 * @param sData Size of the data.
 * @param ex Exception struct.
 * @return New primitive.
 */
extern primitive_t *new_DL_DATA_Request(
		uint16_t clientHandle, uint16_t serverHandle,
		const uint8_t *pData, uint16_t sData,
		struct exception *ex);

/**
 * @brief Create a new DL_DATA_Indication.
 * @param clientHandle Handle that will be returned in the response.
 * @param serverHandle Handle that will be forwarded to the client.
 * @param pData Pointer to the data.
 * @param sData Size of the data.
 * @param ex Exception struct.
 * @return New primitive.
 */
extern primitive_t *new_DL_DATA_Indication(
		uint16_t clientHandle, uint16_t serverHandle,
		const uint8_t *pData, uint16_t sData,
		struct exception *ex);

/**
 * @brief Create a new DL_UNIT_DATA_Request.
 * @param clientHandle Handle that will be returned in the response.
 * @param dstAddrPtr Pointer to destination address.
 * @param dstAddrSize Size of destination address.
 * @param srcAddrPtr Pointer to source address.
 * @param srcAddrSie Pointer to source address.
 * @param pData Pointer to the data.
 * @param sData Size of the data.
 * @param ex Exception struct.
 * @return New primitive.
 */
extern primitive_t *new_DL_UNIT_DATA_Request(
		uint16_t clientHandle,
		const uint8_t *dstAddrPtr, uint8_t dstAddrSize,
		const uint8_t *srcAddrPtr, uint8_t srcAddrSize,
		const uint8_t *pData, uint16_t sData,
		struct exception *ex);

/**
 * @brief Create a new DL_UNIT_DATA_Indication.
 * @param serverHandle Handle that will be forwarded to the client.
 * @param dstAddrPtr Pointer to destination address.
 * @param dstAddrSize Size of destination address.
 * @param srcAddrPtr Pointer to source address.
 * @param srcAddrSie Pointer to source address.
 * @param pData Pointer to the data.
 * @param sData Size of the data.
 * @param ex Exception struct.
 * @return New primitive.
 */
extern primitive_t *new_DL_UNIT_DATA_Indication(
		uint16_t serverHandle,
		const uint8_t *dstAddrPtr, uint8_t dstAddrSize,
		const uint8_t *srcAddrPtr, uint8_t srcAddrSize,
		const uint8_t *pData, uint16_t sData,
		struct exception *ex);

/**
 * @brief Create a new DL_ERROR_Indication.
 * @param clientHandle Handle that will be returned in the response.
 * @param serverHandle Handle that will be forwarded to the client.
 * @param error The error indicator.
 * @param ex Exception struct.
 * @return New primitive.
 */
static inline primitive_t *new_DL_ERROR_Indication(
		uint16_t clientHandle, uint16_t serverHandle, char error, struct exception *ex)
{
	primitive_t *prim = new_prim(1, DL, DL_ERROR_INDICATION, clientHandle, serverHandle, ex);
	if (prim)
		prim->payload[0] = error;
	return prim;
}

/**
 * @brief Create a new DL_FLOW_OFF_Request.
 * @param clientHandle Handle that will be returned in the response.
 * @param serverHandle Handle that will be forwarded to the client.
 * @param ex Exception struct.
 * @return New primitive.
 */
static inline primitive_t *new_DL_FLOW_OFF_Request(
		uint16_t clientHandle, uint16_t serverHandle, struct exception *ex)
{
	return new_prim(0, DL, DL_FLOW_OFF_REQUEST, clientHandle, serverHandle, ex);
}

/**
 * @brief Create a new DL_FLOW_ON_Request.
 * @param clientHandle Handle that will be returned in the response.
 * @param serverHandle Handle that will be forwarded to the client.
 * @param ex Exception struct.
 * @return New primitive.
 */
static inline primitive_t *new_DL_FLOW_ON_Request(
		uint16_t clientHandle, uint16_t serverHandle, struct exception *ex)
{
	return new_prim(0, DL, DL_FLOW_ON_REQUEST, clientHandle, serverHandle, ex);
}

/**
 * @brief Create a new MDL_ERROR_Indication.
 * @param clientHandle Handle that will be returned in the response.
 * @param serverHandle Handle that will be forwarded to the client.
 * @param error The error indicator.
 * @param ex Exception struct.
 * @return New primitive.
 */
static inline primitive_t *new_MDL_ERROR_Indication(
		uint16_t clientHandle, uint16_t serverHandle, char error, struct exception *ex)
{
	primitive_t *prim = new_prim(1, DL, MDL_ERROR_INDICATION, clientHandle, serverHandle, ex);
	if (prim)
		prim->payload[0] = error;
	return prim;
}

/**
 * @brief Create a new DL_TEST_Request.
 * @param clientHandle Handle that will be returned in the response.
 * @param dstAddrPtr Pointer to destination address.
 * @param dstAddrSize Size of destination address.
 * @param srcAddrPtr Pointer to source address.
 * @param srcAddrSie Pointer to source address.
 * @param pData Pointer to the data.
 * @param sData Size of the data.
 * @param ex Exception struct.
 * @return New primitive.
 */
extern primitive_t *new_DL_TEST_Request(
		uint16_t clientHandle,
		const uint8_t *dstAddrPtr, uint8_t dstAddrSize,
		const uint8_t *srcAddrPtr, uint8_t srcAddrSize,
		const uint8_t *pData, uint16_t sData,
		struct exception *ex);

/**
 * @brief Create a new DL_TEST_Indication.
 * @param serverHandle Handle that will be forwarded to the client.
 * @param dstAddrPtr Pointer to destination address.
 * @param dstAddrSize Size of destination address.
 * @param srcAddrPtr Pointer to source address.
 * @param srcAddrSie Pointer to source address.
 * @param pData Pointer to the data.
 * @param sData Size of the data.
 * @param ex Exception struct.
 * @return New primitive.
 */
extern primitive_t *new_DL_TEST_Indication(
		uint16_t serverHandle,
		const uint8_t *dstAddrPtr, uint8_t dstAddrSize,
		const uint8_t *srcAddrPtr, uint8_t srcAddrSize,
		const uint8_t *pData, uint16_t sData,
		struct exception *ex);

/**
 * @brief Create a new DL_TEST_Confirmation.
 * @param clientHandle Handle that will be returned in the response.
 * @param serverHandle Handle that will be forwarded to the client.
 * @param dstAddrPtr Pointer to destination address.
 * @param dstAddrSize Size of destination address.
 * @param srcAddrPtr Pointer to source address.
 * @param srcAddrSie Pointer to source address.
 * @param pData Pointer to the data.
 * @param sData Size of the data.
 * @param ex Exception struct.
 * @return New primitive.
 */
extern primitive_t *new_DL_TEST_Confirmation(
		uint16_t clientHandle, uint16_t serverHandle,
		const uint8_t *dstAddrPtr, uint8_t dstAddrSize,
		const uint8_t *srcAddrPtr, uint8_t srcAddrSize,
		const uint8_t *pData, uint16_t sData,
		struct exception *ex);

#endif /* RUNTIME_DL_PRIM_H_ */
