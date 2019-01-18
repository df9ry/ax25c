/*
 *  Project: ax25c - File: ax25c_runtime.c
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

#define _POSIX_SOURCE 1

#include "../runtime.h"

#include <uki/kernel.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dlfcn.h>
#include <assert.h>

#define MODULE_NAME "RUNTIME"

struct configuration configuration = {
		.name = NULL,
		.loglevel = DEBUG_LEVEL_NONE
};

int print_ex(struct exception *ex)
{
	if (!ex) {
		return EXIT_SUCCESS;
	}
	const char* erm;

	switch (ex->erc) {
	case EXIT_SUCCESS :
		erm = "No error";
		break;
	case EXIT_FAILURE :
		erm = "Application error";
		break;
	default :
		erm = strerror(ex->erc);
		break;
	}
	fprintf(stderr, "Function \"%s\" in module \"%s\""
					" throwed exception \"%s[%s]\""
			        " with error code %i[%s]\n",
			(ex->function) ? ex->function : "unknown",
			(ex->module)   ? ex->module   : "unknown",
			(ex->message)  ? ex->message  : "error",
			(ex->param)    ? ex->param    : "",
			ex->erc, erm);
	return ex->erc;
}

bool load_so(const char *name, void **handle, struct exception *ex)
{
	assert(name);
	assert(handle);
	dlerror();
	*handle = dlopen(name, RTLD_NOW);
	if (!*handle) {
		if (ex) {
			ex->erc = EXIT_FAILURE;
			ex->message = dlerror();
			ex->param = name;
			ex->module = MODULE_NAME;
			ex->function = "load_so";
		}
		return false;
	}
	return true;
}

bool getsym_so(void *handle, const char *name, void ** addr, struct exception *ex)
{
	assert(handle);
	assert(name);
	assert(addr);
	dlerror();
	*addr = dlsym(handle, name);
	if (!*addr) {
		if (ex) {
			ex->erc = EXIT_FAILURE;
			ex->message = dlerror();
			ex->param = name;
			ex->module = MODULE_NAME;
			ex->function = "getsym_so";
		}
		return false;
	}
	return true;
}

bool unload_so(void *module, struct exception *ex)
{
	int erc;
	if (module) {
		dlerror();
		erc = dlclose(module);
		if (erc) {
			if (ex) {
				ex->erc = erc;
				ex->message = dlerror();
				ex->param = "";
				ex->module = MODULE_NAME;
				ex->function = "unload_so";
			}
			return false;
		}
	}
	return true;
}

struct cb_info {
	struct exception *ex;
	struct plugin_descriptor *pd;
};

static void start_instance(struct mapc_node *elem, void *user_data)
{
	struct cb_info *pcbi = user_data;
	struct instance *inst;
	assert(pcbi);
	assert(pcbi->ex);
	if (pcbi->ex->erc != EXIT_SUCCESS)
		return;
	inst = container_of(elem, struct instance, node);
	assert(inst);
	assert(pcbi->pd->start_instance);
	INFO("START INST", inst->name);
	pcbi->pd->start_instance(inst->handle, pcbi->ex);
}

static void stop_instance(struct mapc_node *elem, void *user_data)
{
	struct cb_info *pcbi = user_data;
	struct instance *inst;
	assert(pcbi);
	assert(pcbi->ex);
	if (pcbi->ex->erc != EXIT_SUCCESS)
		return;
	inst = container_of(elem, struct instance, node);
	assert(inst);
	assert(pcbi->pd->stop_instance);
	INFO("STOP INST", inst->name);
	pcbi->pd->stop_instance(inst->handle, pcbi->ex);
}

static void start_plugin(struct mapc_node *elem, void *user_data)
{
	struct cb_info cbi = { user_data, NULL };
	struct plugin *plugin;
	assert(cbi.ex);
	if (cbi.ex->erc != EXIT_SUCCESS)
		return;
	plugin = container_of(elem, struct plugin, node);
	assert(plugin);
	cbi.pd = plugin->plugin_descriptor;
	assert(cbi.pd);
	assert(cbi.pd->start_plugin);
	INFO("START PLUG", plugin->name);
	if (!cbi.pd->start_plugin(plugin->handle, cbi.ex))
		return;
	mapc_foreach(&plugin->instances, start_instance, &cbi);
}

static void stop_plugin(struct mapc_node *elem, void *user_data)
{
	struct cb_info cbi = { user_data, NULL };
	struct plugin *plugin;
	assert(cbi.ex);
	if (cbi.ex->erc != EXIT_SUCCESS)
		return;
	plugin = container_of(elem, struct plugin, node);
	assert(plugin);
	cbi.pd = plugin->plugin_descriptor;
	assert(cbi.pd);
	mapc_foreach_reverse(&plugin->instances, stop_instance, &cbi);
	if (cbi.ex->erc != EXIT_SUCCESS)
		return;
	INFO("STOP PLUG", plugin->name);
	assert(cbi.pd->stop_plugin);
	cbi.pd->stop_plugin(plugin->handle, cbi.ex);
}

static volatile bool alive = false;

void die(void) {
	alive = false;
}

bool stop(struct exception *ex) {
	assert(ex);
	ex->erc = EXIT_SUCCESS;
	die();
	DEBUG("alive", "false");
	mapc_foreach_reverse(&configuration.plugins, stop_plugin, ex);
	return (ex->erc == EXIT_SUCCESS);
}

bool start(struct exception *ex)
{
	assert(ex);
	ex->erc = EXIT_SUCCESS;
	alive = true;
	DEBUG("alive", "true");
	mapc_foreach(&configuration.plugins, start_plugin, ex);
	if (ex->erc != EXIT_SUCCESS) {
		struct exception ex1;
		stop(&ex1);
		print_ex(&ex1);
		return false;
	}
	return (ex->erc == EXIT_SUCCESS);
}

bool tick(struct exception *ex)
{
	assert(ex);
	if (!alive) {
		ex->erc = EXIT_SUCCESS;
		ex->message = "Normal shutdown";
		ex->param = "";
		ex->module = MODULE_NAME;
		ex->function = "tick";
		return false;
	}
	return true;
}
