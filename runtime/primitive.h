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

#include <stringc/stringc.h>

#include <stdint.h>
#include <unistd.h>
#include <assert.h>

/**
 * Maximum size of a payload.
 */
#define MAX_PAYLOAD_SIZE 32768

struct exception;

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
	uint16_t size;         /**< Size of the payload.           */
	uint8_t  protocol;     /**< Protocol, value of protocol_t. */
	uint8_t  cmd;          /**< Protocol specific command.     */
	uint16_t clientHandle; /**< Handle assigned by the client. */
	uint16_t serverHandle; /**< Handle assigned by the server. */
	uint8_t  payload[0];   /**< Specific payload.              */
};

/**
 * @brief Type for primitives.
 */
typedef struct primitive primitive_t;

/**
 * @brief Type for prim parameter.
 */
typedef const uint8_t prim_param_t;

/**
 * @brief Allocate a new prim.
 * @param payload_size Size of the payload.
 * @param protocol Protocol of the prim.
 * @param cmd Protocol specific command.
 * @param clientHandle Client handle.
 * @param serverHandle Server handle.
 * @param ex Exception structure.
 * @return Pointer to the new prim or NULL, if the payload was too large
 *         or no more memory is available.
 */
static inline primitive_t *new_prim(uint16_t payload_size, protocol_t protocol,
		uint8_t cmd, uint16_t clientHandle, uint16_t serverHandle,
		struct exception *ex)
{
	if (payload_size > MAX_PAYLOAD_SIZE)
		return NULL;
	uint16_t total_size = sizeof(struct primitive) + payload_size;
	primitive_t *prim = mem_alloc(total_size, ex);
	if (prim) {
		prim->size = payload_size;
		prim->protocol = protocol;
		prim->cmd = cmd;
		prim->clientHandle = clientHandle;
		prim->serverHandle = serverHandle;
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

/**
 * @brief Get parameter from primitive.
 * @param prim Prim to investigate.
 * @param i Parameter number.
 * @return Pointer to the parameter or NULL, when no such parameter was
 *         specified.
 */
static inline prim_param_t *get_prim_param(primitive_t *prim, uint16_t i)
{
	uint16_t o = 0, s;
	while (i-- > 0) {
		if (o >= prim->size)
			return NULL;
		s = *((uint16_t*)(&prim->payload[o]));
		o += (s + 2);
	} /* end while */
	return &prim->payload[o];
}

/**
 * @brief Get size of a prim parameter.
 * @param param Pointer to the prim parameter.
 * @return Size of the prim parameter.
 */
static inline uint16_t get_prim_param_size(prim_param_t *param)
{
	if (param)
		return *((uint16_t*)param);
	else
		return 0;
}

/**
 * @brief Get data of a prim parameter.
 * @param param Pointer to the prim parameter.
 * @return Pointer to the data of the prim parameter.
 */
static inline const uint8_t *get_prim_param_data(prim_param_t *param)
{
	if (param)
		return &param[2];
	else
		return NULL;
}

/**
 * @brief Get data of a prim parameter.
 * @param param Pointer to the prim parameter.
 * @param str Pointer to string_t.
 * @return Pointer to const char *c-string.
 */
static inline const char *get_prim_param_cstr(prim_param_t *param,
		string_t *str)
{
	const uint8_t *p = get_prim_param_data(param);
	uint16_t c = get_prim_param_size(param);
	assert(str);
	if (!param)
		return "";
	while (c--)
		string_add_char(str, (char)(*p++));
	return string_c(str);
}

/**
 * @brief Put a param into a prim.
 * @param i Current index in prim.
 * @param pp Pointer to the param.
 * @param cp Size of the param.
 * @return new index in the prim.
 */
static inline int put_prim_param(primitive_t *prim, int i, const uint8_t *pp,
		uint16_t cp)
{
	assert(prim);
	assert(i >= 0);
	assert(pp);
	assert(i + cp + 2 <= prim->size);
	*((uint16_t*)(&prim->payload[i])) = cp;
	i += 2;
	memcpy(&prim->payload[i], pp, cp);
	i += cp;
	return i;
}

#endif /* RUNTIME_PRIMITIVE_H_ */
