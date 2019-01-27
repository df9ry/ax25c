/*
 *  Project: ax25c - File: dls.h
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

#ifndef RUNTIME_DLS_H_
#define RUNTIME_DLS_H_

#include "primitive.h"

#include <mapc/mapc.h>
#include <stdint.h>

struct dls;
struct exception;
struct string;

typedef struct dls dls_t;

struct dls_stats {
	size_t queue_size;
	size_t queue_free;
};

typedef struct dls_stats dls_stats_t;

struct dls {
	struct mapc_node node;
	const char *name;

	bool (*set_default_local_addr)(dls_t *dls, const char *addr,
			struct string *normal, struct exception *ex);
	bool (*set_default_remote_addr)(dls_t *dls, const char *addr,
			struct string *normal, struct exception *ex);
	bool (*open)(dls_t *dls, dls_t *receiver, struct exception *ex);
	void (*close)(dls_t *dls);
	bool (*on_write)(dls_t *dls, primitive_t *prim, bool expedited,
			struct exception *ex);
	void (*get_queue_stats)(dls_t *dls, dls_stats_t *stats);
	struct dls *peer;
};

#endif /* RUNTIME_DLS_H_ */
