/*
 *  Project: ax25c - File: dlsap.h
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

#ifndef RUNTIME_DLSAP_H_
#define RUNTIME_DLSAP_H_

#include "dls.h"
#include "exception.h"
#include "primitive.h"
#include <stringc/stringc.h>
#include <stdbool.h>

/**
 * @brief Register a Data Link Service.
 * @param dls Pointer to the Data Link Service to register.
 * @param ex Exception structure. Optional.
 * @return True, if registration was successful. In the case of an error
 *         the result is false and ex contains specific information.
 */
extern bool dlsap_register_dls(dls_t *dls, exception_t *ex);

/**
 * @brief Unregister a Data Link Service.
 * @param dls Pointer to the Data Link Service to register.
 * @param ex Exception structure. Optional.
 * @return True, if the unregistration was successful. In the case of an error
 *         the result is false and ex contains specific information.
 */
extern bool dlsap_unregister_dls(dls_t *dls, exception_t *ex);

/**
 * @brief Lookup a Data Link Service.
 * @param name Name of the Data Link Service to lookup.
 * @return Pointer to Data Link Service with name 'name' or NULL, if no Data
 *         Link Service registered with this name.
 */
extern dls_t *dlsap_lookup_dls(const char *name);

/**
 * @brief Set default local address.
 * @param dls Pointer to Data Link Service to use.
 * @param addr Local Address.
 * @param normal Pointer to string for normalized address. Optional.
 * @param ex Exception structure. Optional.
 * @return True, when call was successful.
 */
extern bool dlsap_set_default_local_addr(dls_t *dls, const char *addr,
		string_t *normal, exception_t *ex);

/**
 * @brief Set default remote address.
 * @param dls Pointer to Data Link Service to use.
 * @param addr Remote Address.
 * @param normal Pointer to string for normalized address. Optional.
 * @param ex Exception structure. Optional.
 * @return Normalized form of the Address or NULL, when error.
 */
extern bool dlsap_set_default_remote_addr(dls_t *dls, const char *addr,
		string_t *normal, exception_t *ex);

/**
 * @brief Open the connection to the peer, optionally provide a back
 *        channel.
 * @param dls Pointer to Data Link Service to use.
 * @param back Back channel, optional.
 * @param ex Exception structure. Optional.
 * @return True, when call was successful.
 */
extern bool dlsap_open(dls_t *dls, dls_t *back, exception_t *ex);

/**
 * @brief Close the connection to the peer. After this call nothing will be
 *        received on the back channel.
 * @param dls Pointer to Data Link Service to use.
 */
extern void dlsap_close(dls_t *dls);

/**
 * @brief Write a prim to the peer. This write is non blocking, so it can be
 *        used insite of a tick or timer routine.
 * @param dls Pointer to Data Link Service to use.
 * @param prim The primitive to write.
 * @bool expedited. This prim is expedited.
 * @param ex Exception structure. Optional.
 * @return True, when call was successful.
 */
extern bool dlsap_write(dls_t *dls, primitive_t *prim, bool expedited,
		exception_t *ex);

/**
 * @brief get queue status from the peer.
 * @param dls Pointer to Data Link Service to use.
 * @param stats Pointer to the stats.
 */
extern void get_queue_stats(dls_t *dls, dls_stats_t *stats);

#endif /* RUNTIME_DLSAP_H_ */
