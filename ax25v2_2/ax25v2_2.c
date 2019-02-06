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
#include "ax25v2_2.h"

#include <stringc/stringc.h>

#include <errno.h>
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

static bool on_write_dl_unit_data_request(dls_t *_dls, primitive_t *prim,
		struct exception *ex)
{
	bool res = false;
	primitive_t *resp;
	prim_param_t *dstAddr, *srcAddr, *data;

	assert(prim);
	if (_dls->peer) {
		dstAddr = get_DL_dst_param(prim);
		assert(dstAddr);
		srcAddr = get_DL_src_param(prim);
		assert(srcAddr);
		data = get_DL_data_param(prim);
		assert(data);
		resp = new_DL_UNIT_DATA_Indication(0,
				get_prim_param_data(srcAddr), get_prim_param_size(srcAddr),
				get_prim_param_data(dstAddr), get_prim_param_size(dstAddr),
				get_prim_param_data(data),    get_prim_param_size(data), ex);
		if (resp)
			res = dlsap_write(client_dls.peer, resp, false, ex);
		else
			res = false;
	}
	del_prim(resp);
	return res;
}

static bool on_write_dl_test_request(dls_t *_dls, primitive_t *prim,
		struct exception *ex)
{
	bool res = false;
	primitive_t *frame;
	prim_param_t *dstAddr, *srcAddr, *data;
	STRING(dst);
	STRING(src);
	callsign source;
	addressField_t af;
	const char *next;

	if (server_dls.peer) {
		assert(prim);
		dstAddr = get_DL_dst_param(prim);
		assert(dstAddr);
		get_prim_param_cstr(dstAddr, &dst);
		srcAddr = get_DL_src_param(prim);
		assert(srcAddr);
		get_prim_param_cstr(srcAddr, &src);
		data = get_DL_data_param(prim);
		assert(data);
		source = callsignFromString(STRING_C(src), &next, ex);
		if (!source)
			goto exit;
		if (*next) {
			exception_fill(ex, EXIT_FAILURE, MODULE_NAME,
					"on_write_dl_test_request",
					"Exceeding characters in source call", STRING_C(src));
			goto exit;
		}
		if (!addressFieldFromString(source, STRING_C(dst), &af, ex))
			return false;
		frame = new_AX25_TEST(prim->clientHandle, 0, &af, true, true,
				get_prim_param_data(data), get_prim_param_size(data), ex);
		if (frame) {
			res = dlsap_write(server_dls.peer, frame, false, ex);
			del_prim(frame);
			res = true;
		}
	}
exit:
	STRING_RESET(dst);
	STRING_RESET(src);
	return res;
}

static bool on_write_dl_connect_request(dls_t *_dls, primitive_t *prim,
		struct exception *ex)
{
	bool res = false;
	primitive_t *frame;
	prim_param_t *dstAddr, *srcAddr;
	STRING(dst);
	STRING(src);
	callsign source;
	addressField_t af;
	const char *next;

	if (server_dls.peer) {
		assert(prim);
		dstAddr = get_DL_dst_param(prim);
		assert(dstAddr);
		get_prim_param_cstr(dstAddr, &dst);
		srcAddr = get_DL_src_param(prim);
		assert(srcAddr);
		get_prim_param_cstr(srcAddr, &src);
		source = callsignFromString(STRING_C(src), &next, ex);
		if (!source)
			goto exit;
		if (*next) {
			exception_fill(ex, EXIT_FAILURE, MODULE_NAME,
					"on_write_dl_connect_request",
					"Exceeding characters in source call", STRING_C(src));
			goto exit;
		}
		if (!addressFieldFromString(source, STRING_C(dst), &af, ex))
			return false;
		frame = new_AX25_SABM(prim->clientHandle, 0, &af, ex);
		if (frame) {
			res = dlsap_write(server_dls.peer, frame, false, ex);
			del_prim(frame);
			res = true;
		}
	}
exit:
	STRING_RESET(dst);
	STRING_RESET(src);
	return res;
}

static bool on_write_dl(dls_t *_dls, primitive_t *prim, struct exception *ex)
{
	bool res = false;

	switch (prim->cmd) {
	case DL_UNIT_DATA_REQUEST:
		res = on_write_dl_unit_data_request(_dls, prim, ex);
		break;
	case DL_TEST_REQUEST:
		res = on_write_dl_test_request(_dls, prim, ex);
		break;
	case DL_CONNECT_REQUEST:
		res = on_write_dl_connect_request(_dls, prim, ex);
		break;
	default:
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME,
				"on_write_dl", "Unimplemented Command", "");
		res = false;
		break;
	} /* end switch */
	return res;
}

static bool on_client_write(dls_t *_dls, primitive_t *prim, bool expedited,
		struct exception *ex)
{
	bool res = false;

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
		res = on_write_dl(_dls, prim, ex);
		break;
	default:
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME,
				"on_write", "Unhandled Protocol", "");
		res = false;
		break;
	} /* end switch */
	return res;
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
	client_dls.name = h->name;
	INFO("Register Service Access Point", h->name);
	return dlsap_register_dls(&client_dls, ex);
}

bool ax25v2_2_start(struct plugin_handle *h, struct exception *ex)
{
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
	dlsap_close(server_dls.peer);
	server_dls.peer = NULL;
	return true;
}
