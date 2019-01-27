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

/**
 * @brief Create a new DL_CONNECT Request.
 * @param localHandle Handle that will be returned in the response.
 * @param dstAddrPtr Pointer to destination address.
 * @param dstAddrSize Size of destination address.
 * @param srcAddrPtr Pointer to source address.
 * @param srcAddrSie Pointer to source address.
 * @return New DL connect request.
 */
extern primitive_t *new_DL_CONNECT_Request(
		uint16_t localHandle,
		const uint8_t *dstAddrPtr, uint16_t dstAddrSize,
		const uint8_t *srcAddrPtr, uint16_t srcAddrSize);

#endif /* RUNTIME_DL_PRIM_H_ */
