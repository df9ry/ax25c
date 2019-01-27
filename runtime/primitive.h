/*
 *  Project: ax25c - File: primitive.h
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

#ifndef RUNTIME_PRIMITIVE_H_
#define RUNTIME_PRIMITIVE_H_

#include "runtime.h"

#include <stdint.h>
#include <unistd.h>
#include <assert.h>

/**
 * Maximum size of a payload.
 */
#define MAX_PAYLOAD_SIZE 32768

/**
 * @brief List of known protocols. There might be more.
 */
enum protocol {
	DL   = 0, /**< Data Link Layer (3)        */
	MDL  = 1, /**< Data Link Layer Management */
	LM   = 2, /**< Link Multiplexer           */
	PH   = 3, /**< Physical Layer             */
	AX25 = 4, /**< AX.25 Frame                */
};

/**
 * @brief Type for protocol enum.
 */
typedef enum protocol protocol_t;

/**
 * @brief Base for data primitives.
 */
struct primitive {
	uint16_t size;        /**< Total size of the primitive.   */
	uint8_t  protocol;    /**< Protocol, value of protocol_t. */
	uint8_t  cmd;         /**< Protocol specific command.     */
	uint8_t  payload[0];  /**< Specific payload.              */
};

/**
 * @brief Type for primitives.
 */
typedef struct primitive primitive_t;

/**
 * @brief Get size of a primitive.
 * @param prim Pointer to the primitive.
 * @return Size of the primitive.
 */
static inline uint16_t get_prim_size(primitive_t *prim)
{
	assert(prim);
	return prim->size;
}

/**
 * @brief Get protocol of a primitive.
 * @param prim Pointer to the primitive.
 * @return Protocol of the primitive.
 */
static inline protocol_t get_prim_protocol(primitive_t *prim)
{
	assert(prim);
	return (protocol_t)prim->protocol;
}

/**
 * @brief Get protocol specific command of a primitive.
 * @param prim Pointer to the primitive.
 * @return Protocol specific command of the primitive.
 */
static inline uint8_t get_prim_cmd(primitive_t *prim)
{
	assert(prim);
	return prim->cmd;
}

/**
 * @brief Get pointer to payload of a primitive.
 * @param prim Pointer to the primitive.
 * @return Pointer to payload of the primitive.
 */
static inline void* get_prim_payload(primitive_t *prim)
{
	assert(prim);
	return &prim->payload;
}

/**
 * @brief Allocate a new prim.
 * @param payload_size Size of the payload.
 * @param protocol Protocol of the prim.
 * @param cmd Protocol specific command.
 * @return Pointer to the new prim or NULL, if the payload was too large
 *         or no more memory is available.
 */
static inline primitive_t *new_prim(uint16_t payload_size, protocol_t protocol,
		uint8_t cmd)
{
	if (payload_size > MAX_PAYLOAD_SIZE)
		return NULL;
	uint16_t size = sizeof(primitive_t) + payload_size;
	primitive_t *prim = mem_alloc(size, NULL);
	if (prim) {
		prim->size = size;
		prim->protocol = protocol;
		prim->cmd = cmd;
	}
	return prim;
}

/**
 * @brief Lock the prim.
 *        The prim is guaranteed to be available until you call del_prim(prim).
 * @param prim The prim to lock.
 */
static inline void use_prim(primitive_t *prim)
{
	if (prim)
		mem_lock(prim);
}

/**
 * @brief Release the prim.
 *        Please note, that this only decreases a useage counter. A real
 *        deletion is postponed until the usage counter reaches 0.
 * @param prim The prim to release.
 */
static inline void del_prim(primitive_t *prim)
{
	if (prim)
		mem_free(prim);
}

#endif /* RUNTIME_PRIMITIVE_H_ */
