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
#include "../runtime/runtime.h"
#include "../runtime/exception.h"
#include "../runtime/dlsap.h"
#include "../runtime/primitive.h"

#include "_internal.h"
#include "ax25c_timer.h"
#include "monitor.h"
#include "session.h"

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>

struct plugin_handle plugin;

static struct setting_descriptor plugin_settings_descriptor[] = {
		{ "peer",       CSTR_T,  offsetof(struct plugin_handle, peer),       "ROUTER" },
		{ "n_sessions", NSIZE_T, offsetof(struct plugin_handle, n_sessions), "1"      },
		{ NULL }
};

static bool onTick(void *user_data, struct exception *ex)
{
	int erc;

	bool busy = true;
	assert(user_data == &plugin);
	while (busy) {
		/* Handle RX */
		{
			bool expedited;
			struct primitive *prim = primbuffer_read_nonblock(&plugin.rx_buffer, &expedited);
			if (prim) {
				if (prim->clientHandle < plugin.n_sessions) {
					if (!session_rx(&plugin.sessions[prim->clientHandle], prim, ex))
							return false;
				}
				del_prim(prim);
				continue;
			}
		}
		/* Handle TX */
		{
			bool expedited;
			struct primitive *prim = primbuffer_read_nonblock(&plugin.tx_buffer, &expedited);
			if (prim) {
				if (prim->serverHandle < plugin.n_sessions) {
					if (!session_tx(&plugin.sessions[prim->serverHandle], prim, ex))
							return false;
				}
				del_prim(prim);
				continue;
			}
		}
		/* Handle timer */
		{
			struct ax25c_timer *timer;

			erc = pthread_spin_lock(&elapsed_timer_list_lock); /* ===v */
			assert(erc == 0);
			timer = list_first_entry_or_null(&elapsed_timer_list,
					struct ax25c_timer, node);
			if (timer)
				list_del_init(&timer->node);
			erc = pthread_spin_unlock(&elapsed_timer_list_lock); /* =^ */
			assert(erc == 0);
			if (timer) {
				assert(timer->state == TIMER_ELAPSED);
				timer->state = TIMER_IDLE;
				assert(timer->function);
				timer->function(timer->data);
				continue;
			}
		}
	} /* end while */
	return true;
}

static struct tick_listener tick_listener = {
		.onTick    = onTick,
		.user_data = NULL
};

static void *get_plugin(const char *name,
		configurator_func configurator, void *context, struct exception *ex)
{
	assert(name);
	assert(configurator);
	memset(&plugin, 0x00, sizeof(struct plugin_handle));
	plugin.name = name;
	if (!configurator(&plugin, plugin_settings_descriptor, context, ex))
		return NULL;
	if (!ax25v2_2_initialize(&plugin, ex))
		return NULL;
	return &plugin;
}

static bool start_plugin(struct plugin_handle *plugin, struct exception *ex) {
	int i;

	assert(plugin);
	DBG_DEBUG("Start", plugin->name);
	init_ax25c_timer();
	plugin->sessions = malloc(sizeof(struct session) * plugin->n_sessions);
	if (!plugin->sessions) {
		exception_fill(ex, ENOMEM, MODULE_NAME, "start_plugin",
				"Unable to allocate sessions", NULL);
		return false;
	}
	for (i = 0; i < plugin->n_sessions; ++i) {
		plugin->sessions[i].server_id = i;
		if (!init_session(&plugin->sessions[i], ex))
			return false;
	} /* end for */
	if (!ax25v2_2_monitor_init(ex))
		return false;
	if (!ax25v2_2_start(plugin, ex))
		return false;
	tick_listener.user_data = plugin;
	registerTickListener(&tick_listener);
	return true;
}

static bool stop_plugin(struct plugin_handle *plugin, struct exception *ex) {
	int i;

	assert(plugin);
	assert(ex);
	DBG_DEBUG("Stop", plugin->name);
	unregisterTickListener(&tick_listener);
	ax25v2_2_stop(plugin, ex);
	ax25v2_2_monitor_dest(ex);
	for (i = 0; i < plugin->n_sessions; ++i)
		term_session(&plugin->sessions[i]);
	free(plugin->sessions);
	plugin->sessions = NULL;
	term_ax25c_timer();
	return true;
}

struct plugin_descriptor plugin_descriptor = {
		get_plugin,	  (start_func)start_plugin, (stop_func)stop_plugin,
		NULL,         NULL,                     NULL
};
