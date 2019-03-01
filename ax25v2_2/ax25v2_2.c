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

#include "../runtime/runtime.h"
#include "../runtime/dlsap.h"
#include "../runtime/dl_prim.h"

#include "_internal.h"
#include "callsign.h"
#include "session.h"
#include "ax25v2_2.h"

#include <stringc/stringc.h>

#include <errno.h>
#include <pthread.h>
#include <assert.h>

static bool set_client_local_addr(dls_t *_dls, const char *addr,
		string_t *norm, exception_t *ex);

static bool set_client_remote_addr(dls_t *_dls, const char *addr,
		string_t *norm, exception_t *ex);

static bool client_dls_open(dls_t *_dls, dls_t *receiver, struct exception *ex);

static void client_dls_close(dls_t *_dls);

static bool on_client_write(dls_t *_dls, primitive_t *prim, bool expedited,
		struct exception *ex);

static void client_dls_queue_stats(dls_t *_dls, dls_stats_t *stats);

static bool on_server_write(dls_t *_dls, primitive_t *prim, bool expedited,
		struct exception *ex);

static dls_t client_dls = {
		.set_default_local_addr  = set_client_local_addr,
		.set_default_remote_addr = set_client_remote_addr,
		.open                    = client_dls_open,
		.close                   = client_dls_close,
		.on_write                = on_client_write,
		.get_queue_stats         = client_dls_queue_stats,
		.peer                    = NULL
};

static dls_t server_dls = {
		.set_default_local_addr  = NULL,
		.set_default_remote_addr = NULL,
		.open                    = NULL,
		.close                   = NULL,
		.on_write                = on_server_write,
		.get_queue_stats         = NULL,
		.peer                    = NULL
};

static bool set_client_local_addr(dls_t *_dls, const char *addr,
		string_t *norm, exception_t *ex)
{
	const char *next;

	if (_dls != &client_dls) {
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

static bool set_client_remote_addr(dls_t *_dls, const char *addr,
		string_t *norm, exception_t *ex)
{
	addressField_t af;

	if (_dls != &client_dls) {
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

static bool client_dls_open(dls_t *_dls, dls_t *receiver, struct exception *ex)
{
	if (_dls != &client_dls) {
		exception_fill(ex, EINVAL, MODULE_NAME,
				"dls_open", "Channel disruption", "");
		return false;
	}
	if (receiver && client_dls.peer) {
		exception_fill(ex, EEXIST, MODULE_NAME,
				"dls_open", "Channel already connected", "");
		return false;
	}
	client_dls.peer = receiver;
	return true;
}

static void client_dls_close(dls_t *_dls)
{
	if (_dls != &client_dls)
		return;
	client_dls.peer = NULL;
}

static bool on_write_dl_connect_request(dls_t *_dls, primitive_t *prim,
		struct exception *ex)
{
	struct session *session;
	int i, erc;

	assert(&client_dls == _dls);
	if (!server_dls.peer) {
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME,
				"on_write_dl_connect_request", "Channel closed", client_dls.name);
		return false;
	}
	assert(prim);
	session = NULL;
	erc = pthread_spin_lock(&plugin.session_lock); /* ===v */
	assert(erc == 0);
	for (i = 0; i < plugin.n_sessions; ++i) {
		if (!plugin.sessions[i].is_active) {
			session = &plugin.sessions[i];
			session->is_active = true;
			break;
		}
	} /* end for */
	erc = pthread_spin_unlock(&plugin.session_lock); /* =^ */
	assert(erc == 0);
	if (!session) {
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME,
				"on_write_dl_connect_request",
				"No session available", client_dls.name);
		return false;
	}
	session->client_id = prim->clientHandle;
	prim->serverHandle = session->server_id;
	return true;
}

static bool on_write_dl(dls_t *_dls, primitive_t *prim, struct exception *ex)
{
	bool res = false;

	switch (prim->cmd) {
	case DL_CONNECT_REQUEST:
		res = on_write_dl_connect_request(_dls, prim, ex);
		break;
	default:
		break;
	} /* end switch */
	return res;
}

static bool on_client_write(dls_t *_dls, primitive_t *prim, bool expedited,
		struct exception *ex)
{
	if (_dls != &client_dls) {
		exception_fill(ex, EINVAL, MODULE_NAME,
				"on_write", "Channel disruption", "");
		return false;
	}
	if (!prim) {
		exception_fill(ex, EINVAL, MODULE_NAME,
				"on_write", "Primitive is NULL", "");
		return false;
	}

	switch (prim->protocol) {
	case DL:
		if (!on_write_dl(_dls, prim, ex))
			return false;
		break;
	default:
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME,
				"on_write", "Unhandled Protocol", "");
		return false;
	} /* end switch */

	monitor_put(prim, _dls->name, true);
	primbuffer_write_nonblock(&plugin.tx_buffer, prim, expedited);
	return true;
}

static void client_dls_queue_stats(dls_t *_dls, dls_stats_t *stats)
{
	if (_dls != &client_dls)
		return;
}

static bool on_server_write(dls_t *_dls, primitive_t *prim, bool expedited,
		struct exception *ex)
{
	return true;
}

bool ax25v2_2_initialize(struct plugin_handle *h, struct exception *ex)
{
	assert(h);
	client_dls.name = h->name;
	DBG_INFO("Register Service Access Point", h->name);
	return dlsap_register_dls(&client_dls, ex);
}

bool ax25v2_2_start(struct plugin_handle *h, struct exception *ex)
{
	int erc;

	assert(h);
	erc = pthread_spin_init(&h->session_lock, PTHREAD_PROCESS_PRIVATE);
	assert(erc == 0);
	primbuffer_init(&h->rx_buffer);
	primbuffer_init(&h->tx_buffer);
	server_dls.peer = dlsap_lookup_dls(h->peer);
	if (!server_dls.peer) {
		exception_fill(ex, ENOENT, MODULE_NAME, "ax25v2_2_start",
				"SAP not found", h->peer);
		return false;
	}
	if (!dlsap_open(server_dls.peer, &server_dls, ex))
		return false;
	return true;
}

bool ax25v2_2_stop(struct plugin_handle *h, struct exception *ex)
{
	assert(h);
	pthread_spin_destroy(&h->session_lock);
	dlsap_close(server_dls.peer);
	server_dls.peer = NULL;
	primbuffer_destroy(&h->rx_buffer);
	primbuffer_destroy(&h->tx_buffer);
	return true;
}
