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
#include "../runtime/dlsap.h"

#include "_internal.h"

#include <errno.h>
#include <assert.h>

struct plugin_handle plugin;

static struct setting_descriptor plugin_settings_descriptor[] = {
		{ NULL }
};

static struct setting_descriptor instance_settings_descriptor[] = {
		{ NULL }
};

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
		exception_fill(ex, EINVAL, MODULE_NAME,
				"on_write", "Session is NULL", "");
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
	assert(ex);
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

static bool start_instance(struct instance_handle *instance,
		struct exception *ex)
{
	DEBUG("axudp instance start", instance->name);
	return true;
}

static bool stop_instance(struct instance_handle *instance, struct exception *ex) {
	assert(instance);
	assert(ex);
	DEBUG("axudp instance stop", instance->name);
	return true;
}

struct plugin_descriptor plugin_descriptor = {
		get_plugin,	  (start_func)start_plugin,   (stop_func)stop_plugin,
		get_instance, (start_func)start_instance, (stop_func)stop_instance
};
