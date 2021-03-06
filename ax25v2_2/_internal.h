/*
 *  Project: ax25c - File: _internal.h
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

#ifndef AX25V2_2__INTERNAL_H_
#define AX25V2_2__INTERNAL_H_

#define MODULE_NAME "AX25V2_2"

#include "callsign.h"

#include "../runtime/primbuffer.h"

#include <uki/list.h>
#include <stdbool.h>
#include <pthread.h>

struct exception;
struct session;

struct plugin_handle {
	const char        *name;
	addressField_t     default_addr;
	const char        *peer;
	size_t             n_sessions;
	pthread_spinlock_t session_lock;
	struct session    *sessions;
	primbuffer_t       rx_buffer;
	primbuffer_t       tx_buffer;
};

extern struct plugin_handle plugin;

extern bool ax25v2_2_initialize(struct plugin_handle *h, struct exception *ex);
extern bool ax25v2_2_start(struct plugin_handle *h, struct exception *ex);
extern bool ax25v2_2_stop(struct plugin_handle *h, struct exception *ex);

#endif /* AX25V2_2__INTERNAL_H_ */
