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

#include <unistd.h>
#include <signal.h>
#include <assert.h>

struct plugin_handle plugin;

static struct  setting_descriptor plugin_settings_descriptor[] = {
		{ NULL }
};

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
	DBG_DEBUG(MODULE_NAME " start", plugin->name);
	return true;
}

static bool stop_plugin(struct plugin_handle *plugin, struct exception *ex) {
	assert(plugin);
	DBG_DEBUG(MODULE_NAME " stop", plugin->name);
	return true;
}

struct plugin_descriptor plugin_descriptor = {
		get_plugin,	  (start_func)start_plugin,   (stop_func)stop_plugin,
		NULL,         NULL,                       NULL
};
