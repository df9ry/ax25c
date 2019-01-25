/*
 *  Project: ax25c - File: ax25v2_2.c
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

#include "../runtime.h"
#include "../dlsap.h"

#include "_internal.h"

#include <errno.h>
#include <assert.h>

static bool set_default_local_addr(dls_t *_dls, const char *addr,
		string_t *norm, exception_t *ex);

static bool set_default_remote_addr(dls_t *_dls, const char *addr,
		string_t *norm, exception_t *ex);

static bool dls_open(dls_t *_dls, dls_t *receiver, struct exception *ex);

static void dls_close(dls_t *_dls);

static bool on_write(dls_t *_dls, primitive_t *prim, bool expedited,
		struct exception *ex);

static void dls_queue_stats(dls_t *_dls, dls_stats_t *stats);

static dls_t dls = {
		.set_default_local_addr  = set_default_local_addr,
		.set_default_remote_addr = set_default_remote_addr,
		.open                    = dls_open,
		.close                   = dls_close,
		.on_write                = on_write,
		.get_queue_stats         = dls_queue_stats,
		.peer                    = NULL
};

static bool set_default_local_addr(dls_t *_dls, const char *addr,
		string_t *norm, exception_t *ex)
{
	const char *next;

	if (_dls != &dls) {
		exception_fill(ex, EINVAL, MODULE_NAME,
				"set_default_local_addr", "Channel disruption", "");
		return false;
	}

	callsign call = callsignFromString(addr, &next, ex);
	if (!call)
		return false;
	if (*next) {
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME,
				"set_default_local_addr", "Exceeding chars after callsign",
				addr);
		return false;
	}
	plugin.default_addr.source = call;
	if (norm) {
		char buf[20];
		if (callsignToString(call, buf, 20, ex) < 0)
			return false;
		string_set_c(norm, buf);
	}
	return true;
}

static bool set_default_remote_addr(dls_t *_dls, const char *addr,
		string_t *norm, exception_t *ex)
{
	addressField_t af;

	if (_dls != &dls) {
		exception_fill(ex, EINVAL, MODULE_NAME,
				"set_default_remote_addr", "Channel disruption", "");
		return false;
	}

	if (!addressFieldFromString(plugin.default_addr.source, addr, &af, ex))
		return false;
	memcpy(&plugin.default_addr, &addr, sizeof(plugin.default_addr));
	if (norm) {
		char buf[60];
		if (!addressFieldToString(&af, buf, 60, ex))
			return false;
		string_set_c(norm, buf);
	}
	return true;
}

static bool dls_open(dls_t *_dls, dls_t *receiver, struct exception *ex)
{
	if (_dls != &dls) {
		exception_fill(ex, EINVAL, MODULE_NAME,
				"dls_open", "Channel disruption", "");
		return false;
	}
	if (receiver && dls.peer) {
		exception_fill(ex, EEXIST, MODULE_NAME,
				"dls_open", "Channel already connected", "");
		return false;
	}
	dls.peer = receiver;
	return true;
}

static void dls_close(dls_t *_dls)
{
	if (_dls != &dls)
		return;
	dls.peer = NULL;
}

static bool on_write(dls_t *_dls, primitive_t *prim, bool expedited,
		struct exception *ex)
{
	if (_dls != &dls) {
		exception_fill(ex, EINVAL, MODULE_NAME,
				"on_write", "Channel disruption", "");
		return false;
	}
	return true;
}

static void dls_queue_stats(dls_t *_dls, dls_stats_t *stats)
{
	if (_dls != &dls)
		return;
}

bool ax25v2_2_initialize(struct plugin_handle *h, struct exception *ex)
{
	dls.name = h->name;
	INFO("Register Service Access Point", h->name);
	return dlsap_register_dls(&dls, ex);
}

bool ax25v2_2_start(struct plugin_handle *h, struct exception *ex)
{
	return true;
}

bool ax25v2_2_stop(struct plugin_handle *h, struct exception *ex)
{
	return true;
}
