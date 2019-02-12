/*
 *  Project: ax25c - File: terminal.c
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

#include "../runtime/runtime.h"
#include "../runtime/dlsap.h"
#include "../runtime/primbuffer.h"
#include "terminal.h"
#include "_internal.h"

#include <errno.h>
#include <assert.h>

#define S_INBUF 64

static volatile bool initialized = false;

static bool on_write(dls_t *dls, primitive_t *prim, bool expedited,
			struct exception *ex)
{
	primbuffer_write_nonblock(&primbuffer, prim, false);
	return true;
}

struct dls local_dls = {
		.set_default_local_addr  = NULL,
		.set_default_remote_addr = NULL,
		.open                    = NULL,
		.close                   = NULL,
		.on_write                = on_write,
		.get_queue_stats         = NULL,
		.name                    = NULL,
		.peer                    = NULL,
};

struct dls *peerDLS(void)
{
	return local_dls.peer;
}

bool terminal_start(struct plugin_handle *h, struct exception *ex)
{
	assert(!initialized);
	assert(h);
	local_dls.name = h->name;

	local_dls.peer = dlsap_lookup_dls(h->peer);
	if (!local_dls.peer) {
		exception_fill(ex, ENOENT, MODULE_NAME, "terminal_start",
				"SAP not found", h->peer);
		return false;
	}

	if (!dlsap_set_default_local_addr(local_dls.peer, string_c(&h->loc_addr),
			&h->loc_addr, ex))
	{
		return false;
	}

	if (!dlsap_set_default_remote_addr(local_dls.peer, string_c(&h->rem_addr),
			&h->rem_addr, ex))
	{
		return false;
	}

	if (!dlsap_open(local_dls.peer, &local_dls, ex))
	{
		return false;
	}

	stdout_initialize(h);
	stdin_initialize(h);
	initialized = true;
	return true;
}

bool terminal_stop(struct plugin_handle *h, struct exception *ex)
{
	assert(h);
	if (!initialized)
		return false;
	initialized = false;
	stdin_terminate(h);
	stdout_terminate(h);
	dlsap_close(local_dls.peer);
	local_dls.peer = NULL;
	return true;
}
