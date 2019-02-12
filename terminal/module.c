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

#include "terminal.h"
#include "_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>

struct plugin_handle plugin;

static struct setting_descriptor plugin_settings_descriptor[] = {
		{ "line_length",  SIZE_T, offsetof(struct plugin_handle, line_length),  "256"    },
		{ "loc_addr",     STR_T,  offsetof(struct plugin_handle, loc_addr),     "NOCALL" },
		{ "rem_addr",     STR_T,  offsetof(struct plugin_handle, rem_addr),     "NOCALL" },
		{ "mon_size",     SIZE_T, offsetof(struct plugin_handle, mon_size),     "1024"   },
		{ "peer",         CSTR_T, offsetof(struct plugin_handle, peer),         "AX25"   },
		{ "lead_txt",     CSTR_T, offsetof(struct plugin_handle, lead_txt),     ":"      },
		{ "lead_cmd",     CSTR_T, offsetof(struct plugin_handle, lead_cmd),     ">"      },
		{ "lead_inf",     CSTR_T, offsetof(struct plugin_handle, lead_inf),     "+"      },
		{ "lead_err",     CSTR_T, offsetof(struct plugin_handle, lead_err),     "!"      },
		{ "lead_mon",     CSTR_T, offsetof(struct plugin_handle, lead_mon),     "#"      },
		{ "prompt",       CSTR_T, offsetof(struct plugin_handle, prompt),       "cmd>"   },
		{ NULL }
};

static void *get_plugin(const char *name,
		configurator_func configurator, void *context, struct exception *ex)
{
	assert(name);
	assert(configurator);
	memset(&plugin, 0x00, sizeof(struct plugin_handle));
	plugin.name = name;
	STRING_INIT(plugin.loc_addr);
	STRING_INIT(plugin.rem_addr);
	if (!configurator(&plugin, plugin_settings_descriptor, context, ex)) {
		return NULL;
	}
	return &plugin;
}

static bool start_plugin(struct plugin_handle *plugin, struct exception *ex) {
	assert(plugin);
	DEBUG("terminal start", plugin->name);
	return terminal_start(plugin, ex);
}

static bool stop_plugin(struct plugin_handle *plugin, struct exception *ex) {
	assert(plugin);
	assert(ex);
	DEBUG("terminal stop", plugin->name);
	return terminal_stop(plugin, ex);
}

struct plugin_descriptor plugin_descriptor = {
		get_plugin,	  (start_func)start_plugin, (stop_func)stop_plugin,
		NULL,         NULL,                     NULL
};
