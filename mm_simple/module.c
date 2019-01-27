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
#include "../runtime/memory.h"

#include <uki/kernel.h>

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>

#define HEAD 0x3247
#define TAIL 0xe6a3
#define ALIGN 8

#define PLUGIN_NAME "mm_simple"

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

struct mem {
	uint16_t head;
	uint16_t c_locks;
	uint32_t size;
	uint64_t data[0];
};

static inline struct mem *getContainer(void *mem) {
	struct mem *p = container_of(mem, struct mem, data);
	void *addr_tail;

	assert(p->head == HEAD);
	addr_tail = (&p->data) + p->size;
	assert(*((uint16_t*)addr_tail) == TAIL);
	return p;
}

static void *mem_alloc_impl(uint32_t cb, struct exception *ex)
{
	uint32_t size = (cb > 0) ? (((cb - 1) / ALIGN) + 1) * ALIGN : 0;
	struct mem *mem;
	void *addr_tail;

	assert(pthread_mutex_lock(&lock) == 0);
	mem = malloc(sizeof(struct mem) + size + sizeof(uint16_t));
	assert(pthread_mutex_unlock(&lock) == 0);
	if ((!mem) && ex) {
		ex->erc = ENOMEM;
		ex->function = "mem_alloc";
		ex->module = PLUGIN_NAME;
		ex->param = "";
		return NULL;
	}
	mem->head = HEAD;
	mem->c_locks = 1;
	mem->size = size;
	memset(&mem->data, 0x00, size);
	addr_tail = (&mem->data) + size;
	*((uint16_t*)addr_tail) = TAIL;
	return &mem->data;
}

static uint32_t mem_size_impl(void *mem)
{
	struct mem *p;
	uint32_t size;

	if (!mem)
		return 0;
	assert(pthread_mutex_lock(&lock) == 0);
	p = getContainer(mem);
	size = p->size;
	assert(pthread_mutex_unlock(&lock) == 0);
	return size;
}

static void mem_lock_impl(void *mem)
{
	struct mem *p;

	if (!mem)
		return;
	assert(pthread_mutex_lock(&lock) == 0);
	p = getContainer(mem);
	p->c_locks += 1;
	assert(pthread_mutex_unlock(&lock) == 0);
}

static void mem_free_impl(void *mem) {
	struct mem *p;

	if (!mem)
		return;
	assert(pthread_mutex_lock(&lock) == 0);
	p = getContainer(mem);
	assert(p->c_locks--);
	if (!p->c_locks)
		free(p);
	assert(pthread_mutex_unlock(&lock) == 0);
}

static struct mm_interface mmi = {
		.mem_alloc = mem_alloc_impl,
		.mem_free  = mem_free_impl,
		.mem_lock  = mem_lock_impl,
		.mem_size  = mem_size_impl
};

static struct plugin_handle {
	const char *name;
} plugin;

static struct setting_descriptor plugin_settings_descriptor[] = {
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
	registerMemoryManager(&mmi);
	return &plugin;
}

static bool start_plugin(struct plugin_handle *plugin, struct exception *ex) {
	assert(plugin);
	assert(ex);
	DEBUG("mm_simple start", plugin->name);
	return true;
}

static bool stop_plugin(struct plugin_handle *plugin, struct exception *ex) {
	assert(plugin);
	assert(ex);
	DEBUG("mm_simple stop", plugin->name);
	return true;
}

struct plugin_descriptor plugin_descriptor = {
		get_plugin,	  (start_func)start_plugin, (stop_func)stop_plugin,
		NULL,         NULL,                     NULL
};

