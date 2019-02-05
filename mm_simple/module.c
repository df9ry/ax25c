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
	uint8_t  data[0];
};

static inline struct mem *getContainer(void *ptr) {
	struct mem *mem = container_of(ptr, struct mem, data);

	assert(mem->head == HEAD);
	assert(*((uint16_t*)(&mem->data[mem->size])) == TAIL);
	return mem;
}

static void *mem_alloc_impl(uint32_t cb, struct exception *ex)
{
	uint32_t size = (cb > 0) ? (((cb - 1) / ALIGN) + 1) * ALIGN : 0;
	struct mem *mem;

	assert(pthread_mutex_lock(&lock) == 0);
	mem = malloc(sizeof(struct mem) + size + sizeof(uint16_t));
	assert(pthread_mutex_unlock(&lock) == 0);
	if (!mem) {
		exception_fill(ex, ENOMEM, PLUGIN_NAME, "mem_alloc", "Out of memory", "");
		return NULL;
	}
	mem->head = HEAD;
	mem->c_locks = 1;
	mem->size = size;
	memset(&mem->data, 0x00, size);
	*((uint16_t*)(&mem->data[mem->size])) = TAIL;
	return &mem->data;
}

static uint32_t mem_size_impl(void *ptr)
{
	struct mem *mem;
	uint32_t size;

	if (!ptr)
		return 0;
	assert(pthread_mutex_lock(&lock) == 0);
	mem = getContainer(ptr);
	size = mem->size;
	assert(pthread_mutex_unlock(&lock) == 0);
	return size;
}

static void mem_lock_impl(void *ptr)
{
	struct mem *mem;

	if (!ptr)
		return;
	assert(pthread_mutex_lock(&lock) == 0);
	mem = getContainer(ptr);
	mem->c_locks += 1;
	assert(pthread_mutex_unlock(&lock) == 0);
}

static void mem_free_impl(void *ptr) {
	struct mem *mem;

	if (!ptr)
		return;
	pthread_mutex_lock(&lock);
	mem = getContainer(ptr);
	assert(mem->c_locks--);
	if (!mem->c_locks) {
		memset(mem, 0x55, sizeof(struct mem) + mem->size + sizeof(uint16_t));
		free(mem);
	}
	pthread_mutex_unlock(&lock);
}

static void mem_chck_impl(void *ptr) {
	assert(ptr);
	pthread_mutex_lock(&lock);
	assert(getContainer(ptr));
	pthread_mutex_unlock(&lock);
}

static struct mm_interface mmi = {
		.mem_alloc = mem_alloc_impl,
		.mem_free  = mem_free_impl,
		.mem_lock  = mem_lock_impl,
		.mem_size  = mem_size_impl,
		.mem_chck  = mem_chck_impl
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

