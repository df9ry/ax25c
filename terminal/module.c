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

#include "../configuration.h"

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>

static struct plugin_handle {
	const char *name;
} plugin;

static struct setting_descriptor plugin_settings_descriptor[] = {
		{ NULL }
};

struct instance_handle {
	const char  *name;
	size_t       line_length;
	unsigned int port;
};

static struct setting_descriptor instance_settings_descriptor[] = {
		{ "line_length", SIZE_T, offsetof(struct instance_handle, line_length), "256" },
		{ "port",        UINT_T, offsetof(struct instance_handle, port),        NULL  },
		{ NULL }
};

static void *get_plugin(const char *name,
		configurator_func configurator, void *context, struct exception *ex)
{
	assert(name);
	assert(configurator);
	plugin.name = name;
	if (!configurator(&plugin, plugin_settings_descriptor, context, ex)) {
		return NULL;
	}
	return &plugin;
}

static bool start_plugin(struct plugin_handle *plugin) {
	return true;
}

static void *get_instance(const char *name,
		configurator_func configurator, void *context, struct exception *ex)
{
	struct instance *instance;
	assert(name);
	assert(configurator);
	instance = (struct instance*)malloc(sizeof(struct instance));
	assert(instance);
	instance->name = name;
	if (!configurator(instance, instance_settings_descriptor, context, ex)) {
		free(instance);
		return NULL;
	}
	return instance;
}

static bool start_instance(struct instance_handle *plugin) {
	return true;
}

struct plugin_descriptor plugin_descriptor = {
		get_plugin,	  (start_func)start_plugin,
		get_instance, (start_func)start_instance
};
