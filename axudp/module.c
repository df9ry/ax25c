/*
 *  Project: ax25c - File: module.c
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

#include "../config/configuration.h"
#include "../runtime/primbuffer.h"
#include "../runtime/runtime.h"
#include "../runtime/dlsap.h"

#include "_internal.h"

#include <errno.h>
#include <signal.h>
#include <assert.h>

struct plugin_handle plugin;

static struct setting_descriptor plugin_settings_descriptor[] = {
		{ NULL }
};

static struct setting_descriptor instance_settings_descriptor[] = {
		{ "host",        CSTR_T, offsetof(struct instance_handle, host),        "localhost" },
		{ "port",        CSTR_T, offsetof(struct instance_handle, port),        "9300"      },
		{ "tx_buf_size", UINT_T, offsetof(struct instance_handle, tx_buf_size), "64"        },
		{ "rx_buf_size", UINT_T, offsetof(struct instance_handle, rx_buf_size), "256"       },
		{ "mode",        CSTR_T, offsetof(struct instance_handle, mode),        "client"    },
		{ "ip_version",  CSTR_T, offsetof(struct instance_handle, ip_version),  "ax_v4"     },
		{ NULL }
};

static void *rx_worker(void *id)
{
	int n;
	primitive_t *prim;
	EXCEPTION(ex);

	struct instance_handle *instance = id;
	assert(instance);
	while (instance->alive) {
		if (instance->server_mode) {
			instance->peer_addr_len = sizeof(struct sockaddr_storage);
			n = recvfrom(instance->sockfd, instance->rx_buf,
					instance->rx_buf_size, 0,
					(struct sockaddr*) &instance->peer_addr,
					&instance->peer_addr_len);
			if (n < 0) {
				if (configuration.loglevel >= DEBUG_LEVEL_ERROR)
					ax25c_log(DEBUG_LEVEL_ERROR,
							"AXUDP:rx_worker:recvfrom() error %i:%s",
							n, gai_strerror(n));
				continue;
			}
			if (configuration.loglevel >= DEBUG_LEVEL_INFO) {
				char host[NI_MAXHOST], service[NI_MAXSERV];
				int s = getnameinfo((struct sockaddr*) &instance->peer_addr,
                        instance->peer_addr_len, host, NI_MAXHOST, service,
						NI_MAXSERV, NI_NUMERICSERV);
				if (s == 0) {
					ax25c_log(DEBUG_LEVEL_INFO,
							"AXUDP:rx_worker Got %i octets from %s:%s",
							s, host, service);
				} else if (configuration.loglevel >= DEBUG_LEVEL_ERROR) {
					ax25c_log(DEBUG_LEVEL_ERROR,
							"AXUDP:rx_worker:getnameinfo() error %i:%s",
							s, gai_strerror(s));
					continue;
				}
			}
		} else {
			n = read(instance->sockfd, instance->rx_buf, instance->rx_buf_size);
			if (n < 0) {
				if (configuration.loglevel >= DEBUG_LEVEL_ERROR)
					ax25c_log(DEBUG_LEVEL_ERROR,
							"AXUDP:rx_worker:read() error %i:%s",
							errno, strerror(errno));
				continue;
			}
		}
		if (configuration.loglevel >= DEBUG_LEVEL_DEBUG) {
			ax25c_log(DEBUG_LEVEL_DEBUG, "Received UDP packet on %s",
					instance->name);
			dump(DEBUG_LEVEL_DEBUG, instance->rx_buf, (uint16_t)n);
		}
		if (!instance->dls.peer)
			continue;
		prim = new_prim(n, AX25, -1, 0, 0, &ex);
		if (!prim) {
			if (configuration.loglevel >= DEBUG_LEVEL_ERROR)
				ax25c_log(DEBUG_LEVEL_ERROR,
						"AXUDP:rx_worker:new_prim: Error no %i[%s] in %s:%s: %s[%s]",
						ex.erc, strerror(ex.erc),
						STRING_C(ex.module), STRING_C(ex.function),
						STRING_C(ex.message), STRING_C(ex.param));
			continue;
		}
		memcpy(prim->payload, instance->rx_buf, prim->size);
		if (!dlsap_write(instance->dls.peer, prim, false, &ex)) {
			ax25c_log(DEBUG_LEVEL_ERROR,
					"AXUDP:rx_worker:dlsap_write: Error no %i[%s] in %s:%s: %s[%s]",
					ex.erc, strerror(ex.erc),
					STRING_C(ex.module), STRING_C(ex.function),
					STRING_C(ex.message), STRING_C(ex.param));
			continue;
		}
	} /* end while */
	return NULL;
}

static void *tx_worker(void *id)
{
	struct instance_handle *instance = id;
	primitive_t *prim;
	int n;

	assert(instance);
	while (instance->alive) {
		prim = primbuffer_read_block(instance->primbuf, NULL);
		if (!(prim && instance->alive))
			continue;
		if (prim->protocol != AX25) {
			ERROR("AXUDP:tx_worker", "Protocol != AX.25");
			continue;
		}
		if (configuration.loglevel >= DEBUG_LEVEL_DEBUG) {
			ax25c_log(DEBUG_LEVEL_DEBUG, "Send UDP packet on %s",
					instance->name);
			dump(DEBUG_LEVEL_DEBUG, prim->payload, prim->size);
		}
		if (instance->server_mode) {
			n = sendto(instance->sockfd, prim->payload, prim->size, 0,
			                    (struct sockaddr*) &instance->peer_addr,
			                    instance->peer_addr_len);
			if ((n < 0) &&
					(configuration.loglevel >= DEBUG_LEVEL_ERROR))
			{
				ax25c_log(DEBUG_LEVEL_ERROR,
						"AXUDP:tx_worker:sendto() error %i:%s",
						n, gai_strerror(n));
			} else if ((n != prim->size) &&
					(configuration.loglevel >= DEBUG_LEVEL_ERROR))
			{
				ax25c_log(DEBUG_LEVEL_ERROR,
						"AXUDP:tx_worker:sendto(): partial:%i <> %i",
						prim->size, n);
			}
		} else {
			n = write(instance->sockfd, prim->payload, prim->size);
			if ((n < 0) &&
					(configuration.loglevel >= DEBUG_LEVEL_ERROR))
			{
				ax25c_log(DEBUG_LEVEL_ERROR,
						"AXUDP:tx_worker:write() error %i:%s",
						errno, strerror(errno));
			} else if ((n != prim->size) &&
					(configuration.loglevel >= DEBUG_LEVEL_ERROR))
			{
				ax25c_log(DEBUG_LEVEL_ERROR,
						"AXUDP:tx_worker:write(): partial:%i <> %i",
						prim->size, n);
			}
		}
		del_prim(prim);
	} /* end while */
	return NULL;
}

static bool dls_open(dls_t *_dls, dls_t *receiver, struct exception *ex);
static void dls_close(dls_t *_dls);
static bool on_write(dls_t *_dls, primitive_t *prim, bool expedited,
		struct exception *ex);
static void dls_queue_stats(dls_t *_dls, dls_stats_t *stats);

static dls_t dls_template = {
		.set_default_local_addr  = NULL,
		.set_default_remote_addr = NULL,
		.open                    = dls_open,
		.close                   = dls_close,
		.on_write                = on_write,
		.get_queue_stats         = dls_queue_stats,
		.peer                    = NULL,
		.session                 = NULL,
};

static bool dls_open(dls_t *dls, dls_t *receiver, struct exception *ex)
{
	if (!dls) {
		exception_fill(ex, EINVAL, MODULE_NAME,
				"dls_open", "DLS is NULL", "");
		return false;
	}
	struct instance_handle *instance = dls->session;
	if (!instance) {
		exception_fill(ex, EINVAL, MODULE_NAME,
				"dls_open", "Session is NULL", "");
		return false;
	}
	if (receiver && instance->dls.peer) {
		exception_fill(ex, EEXIST, MODULE_NAME,
				"dls_open", "Session already connected", "");
		return false;
	}
	instance->dls.peer = receiver;
	return true;
}

static void dls_close(dls_t *dls)
{
	if (!dls)
		return;
	struct instance_handle *instance = dls->session;
	if (!instance)
		return;
	instance->dls.peer = NULL;
}

static bool on_write(dls_t *dls, primitive_t *prim, bool expedited,
		struct exception *ex)
{
	if (!dls) {
		exception_fill(ex, EINVAL, MODULE_NAME,
				"on_write", "DLS is NULL", "");
		return false;
	}
	struct instance_handle *instance = dls->session;
	if (!instance) {
		exception_fill(ex, EINVAL, MODULE_NAME, "on_write", "Session is NULL",
				"");
		return false;
	}
	if (!primbuffer_write_nonblock(instance->primbuf, prim, expedited)) {
		exception_fill(ex, ENOMEM, MODULE_NAME, "on_write", "Buffer overflow",
				"");
		return false;
	}
	return true;
}

static void dls_queue_stats(dls_t *dls, dls_stats_t *stats)
{
	if (!dls)
		return;
	struct instance_handle *instance = dls->session;
	if (!instance)
		return;
}

static void *get_plugin(const char *name,
		configurator_func configurator, void *context, struct exception *ex)
{
	assert(name);
	assert(configurator);
	memset(&plugin, 0x00, sizeof(struct plugin_handle));
	plugin.name = name;
	if (!configurator(&plugin, plugin_settings_descriptor, context, ex)) {
		return NULL;
	}
	return &plugin;
}

static bool start_plugin(struct plugin_handle *plugin, struct exception *ex) {
	assert(plugin);
	DEBUG("axudp start", plugin->name);
	return true;
}

static bool stop_plugin(struct plugin_handle *plugin, struct exception *ex) {
	assert(plugin);
	DEBUG("axudp stop", plugin->name);
	return true;
}

static void *get_instance(const char *name,
		configurator_func configurator, void *context, struct exception *ex)
{
	struct instance_handle *instance;
	assert(name);
	assert(configurator);
	DEBUG("axudp instance greate", name);
	instance = (struct instance_handle*)malloc(sizeof(struct instance_handle));
	assert(instance);
	memset(instance, 0x00, sizeof(struct instance_handle));
	instance->name = name;
	if (!configurator(instance, instance_settings_descriptor, context, ex)) {
		free(instance);
		return NULL;
	}
	instance->name = name;
	memcpy(&instance->dls, &dls_template, sizeof(struct dls));
	instance->dls.name = name;
	instance->dls.session = instance;
	INFO("Register Service Access Point", instance->name);
	if (!dlsap_register_dls(&instance->dls, ex))
		return false;
	return instance;
}

static bool start_instance(struct instance_handle *instance, exception_t *ex)
{
	pthread_attr_t thread_args;
	int erc, ip_v;
	struct addrinfo  hints;
	struct addrinfo *addrinfo, *rp;

	DEBUG("axudp instance start", instance->name);

	instance->rx_thread_running = false;
	instance->tx_thread_running = false;

	/* Determine mode */
	if (strcmp(instance->mode, "client") == 0) {
		instance->server_mode = false;
	} else if (strcmp(instance->mode, "server") == 0) {
		instance->server_mode = true;
		WARNING("Server mode is partially implemented only!", "");
		instance->peer_addr_len = 0;
	} else {
		exception_fill(ex, EINVAL, MODULE_NAME, "start_instance",
				"Invalid mode (server|client)", instance->mode);
		return false;
	}

	/* Determine ip_v */
	if (strcmp(instance->ip_version, "any") == 0) {
		ip_v = 0;
	} else if (strcmp(instance->ip_version, "ip_v4") == 0) {
		ip_v = 4;
	} else if (strcmp(instance->ip_version, "ip_v6") == 0) {
		ip_v = 6;
	} else {
		exception_fill(ex, EINVAL, MODULE_NAME, "start_instance",
				"Invalid ip_version (ip_v4|ip_v6|any)", instance->ip_version);
		return false;
	}

	/* Allocate buffers */
	instance->primbuf = primbuffer_new(instance->tx_buf_size, ex);
	if (!instance->primbuf)
		return false;
	instance->rx_buf = malloc(instance->rx_buf_size);
	if (!instance->rx_buf) {
		exception_fill(ex, ENOMEM, MODULE_NAME, "start_instance",
				"Unable to allocate rx buffer", "");
		primbuffer_del(instance->primbuf);
		instance->primbuf = NULL;
		return false;
	}

	/* Open connection */
	if (configuration.loglevel >= DEBUG_LEVEL_DEBUG)
		ax25c_log(DEBUG_LEVEL_DEBUG, "Resolving host \"%s:%s\"",
				instance->host, instance->port);
	memset(&hints, 0x00, sizeof(hints));
	switch (ip_v) {
	case 4:
		hints.ai_family   = AF_INET;
		break;
	case 6:
		hints.ai_family   = AF_INET6;
		break;
	default:
		hints.ai_family   = AF_UNSPEC;
		break;
	}
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = 0;
	hints.ai_flags    = AI_CANONNAME | AI_ADDRCONFIG;
	erc = getaddrinfo(
			(strlen(instance->host) == 0) ? NULL : instance->host,
			instance->port, &hints, &addrinfo);
	if (erc != 0) {
		exception_fill(ex, erc, MODULE_NAME, "start_instance:getaddrinfo",
				gai_strerror(erc), instance->host);
		return false;
	}
	for (rp = addrinfo; rp != NULL; rp = rp->ai_next) {
		instance->sockfd = socket(rp->ai_family, rp->ai_socktype,
				rp->ai_protocol);
		if (instance->sockfd == -1)
			continue;
		if (instance->server_mode) {
			if (bind(instance->sockfd, rp->ai_addr, rp->ai_addrlen) == 0)
				break; /* Success */
		} else {
			if (connect(instance->sockfd, rp->ai_addr, rp->ai_addrlen) != -1)
				break; /* Success */
		}
	   close(instance->sockfd);
	} /* end for */
	if (!rp) {
		exception_fill(ex, ENOENT, MODULE_NAME, "start_instance",
				instance->server_mode ? "bind" : "connect", instance->host);
		return false;
	}
	if (configuration.loglevel >= DEBUG_LEVEL_DEBUG)
		ax25c_log(DEBUG_LEVEL_DEBUG, "Host \"%s:%s\" resolved to %s %s \"%s\"",
				instance->host, instance->port,
				(instance->server_mode ? "server" : "client"),
				((rp->ai_family == AF_INET6) ? "AF_INET6" : "AF_INET"),
				rp->ai_canonname);
	freeaddrinfo(addrinfo);

	/* Start threads */
	instance->alive = true;
	pthread_attr_init(&thread_args);
	pthread_attr_setdetachstate(&thread_args, PTHREAD_CREATE_JOINABLE);
	erc = pthread_create(&instance->rx_thread, &thread_args, rx_worker, instance);
	if (erc != 0) {
		exception_fill(ex, erc, MODULE_NAME, "start_instance",
				"Error creating rx_thread", instance->name);
		instance->rx_thread_running = true;
		instance->alive = false;
	}
	erc = pthread_create(&instance->tx_thread, &thread_args, tx_worker, instance);
	if (erc != 0) {
		exception_fill(ex, erc, MODULE_NAME, "start_instance",
				"Error creating tx_thread", instance->name);
		instance->tx_thread_running = true;
		instance->alive = false;
	}
	pthread_attr_destroy(&thread_args);
	return instance->alive;
}

static bool stop_instance(struct instance_handle *instance, exception_t *ex) {
	assert(instance);

	DEBUG("axudp instance stop", instance->name);
	instance->alive = false;
	if (instance->rx_thread_running) {
		pthread_kill(instance->rx_thread, SIGINT);
		instance->rx_thread_running = false;
	}
	if (instance->tx_thread_running) {
		pthread_kill(instance->tx_thread, SIGINT);
		instance->rx_thread_running = false;
	}
	if (instance->primbuf) {
		primbuffer_del(instance->primbuf);
		instance->primbuf = NULL;
	}
	if (instance->rx_buf) {
		free(instance->rx_buf);
		instance->rx_buf = NULL;
	}
	if (instance->sockfd != -1) {
		close(instance->sockfd);
		instance->sockfd = -1;
	}
	return true;
}

struct plugin_descriptor plugin_descriptor = {
		get_plugin,	  (start_func)start_plugin,   (stop_func)stop_plugin,
		get_instance, (start_func)start_instance, (stop_func)stop_instance
};
