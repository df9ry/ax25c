/*
 *  Project: ax25c - File: monitor.c
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

#include "monitor.h"
#include "runtime.h"
#include "primitive.h"
#include "_internal.h"
#include "exception.h"

#include <uki/list.h>
#include <uki/kernel.h>

#include <pthread.h>
#include <errno.h>
#include <assert.h>

static pthread_spinlock_t monitor_lock;
static pthread_spinlock_t listener_lock;

static monitor_function* monitor_providers[PROTOCOL_UPPER];

bool register_monitor_provider(protocol_t protocol, monitor_function *func,
		struct exception *ex)
{
	bool res = false;

	assert(protocol < PROTOCOL_UPPER);
	pthread_spin_lock(&monitor_lock);
	if (monitor_providers[protocol]) {
		exception_fill(ex, EEXIST, MODULE_NAME, "register_monitor_provider",
				"Protocol already registered", "");
		goto exit;
	}
	monitor_providers[protocol] = func;
	res = true;
exit:
	pthread_spin_unlock(&monitor_lock);
	return res;
}

bool unregister_monitor_provider(protocol_t protocol, monitor_function *func,
		struct exception *ex)
{
	bool res = false;

	assert(protocol < PROTOCOL_UPPER);
	pthread_spin_lock(&monitor_lock);
	if (monitor_providers[protocol] != func) {
		exception_fill(ex, EPERM, MODULE_NAME, "unregister_monitor_provider",
				"Invalid monitor function", "");
		goto exit;
	}
	monitor_providers[protocol] = NULL;
	res = true;
exit:
	pthread_spin_unlock(&monitor_lock);
	return res;
}

int monitor(struct primitive *prim, char *pb, size_t cb, struct exception *ex)
{
	int res;
	monitor_function *f;

	assert(prim);
	assert(pb);
	assert(prim->protocol < PROTOCOL_UPPER);
	pthread_spin_lock(&monitor_lock);
	f = monitor_providers[prim->protocol];
	if (f) {
		res = f(prim, pb, cb, ex);
	} else {
		exception_fill(ex, ENOENT, MODULE_NAME, "monitor",
				"No monitor provider registered", "");
		res = -ENOENT;
	}
	pthread_spin_unlock(&monitor_lock);
	return res;
}

static LIST_HEAD(monitor_listener_list);

struct monitor_listener {
	struct list_head           node;
	monitor_listener_function *listener;
	void                      *data;
};

void *register_monitor_listener(monitor_listener_function *listener, void *data)
{
	struct monitor_listener *l;

	assert(listener);
	l = malloc(sizeof(struct monitor_listener));
	if (!l)
		return NULL;
	INIT_LIST_HEAD(&l->node);
	l->listener = listener;
	l->data = data;
	pthread_spin_lock(&listener_lock);
	list_add_tail(&l->node, &monitor_listener_list);
	pthread_spin_unlock(&listener_lock);
	return l;
}

bool unregister_monitor_listener(void *handle)
{
	bool res = false;
	struct monitor_listener *cursor;

	pthread_spin_lock(&listener_lock);
	if (handle) {
		list_for_each_entry(cursor, &monitor_listener_list, node) {
			if (cursor == handle) {
				res = true;
				list_del(&cursor->node);
				break;
			}
		} /* end list_for_each_entry */
	}
	pthread_spin_unlock(&listener_lock);
	if (res)
		free(handle);
	return res;
}

void monitor_put(primitive_t *prim, const char *service, bool tx)
{
	struct monitor_listener *cursor;

	assert(service);
	if (!prim)
		return;
	pthread_spin_lock(&listener_lock);
	list_for_each_entry(cursor, &monitor_listener_list, node) {
		cursor->listener(prim, service, tx, cursor->data);
	} /* end list_for_each_entry */
	pthread_spin_unlock(&listener_lock);
}

void monitor_init(void)
{
	memset(&monitor_providers, 0x00, sizeof(monitor_providers));
	pthread_spin_init(&monitor_lock, PTHREAD_PROCESS_PRIVATE);
	pthread_spin_init(&listener_lock, PTHREAD_PROCESS_PRIVATE);
}

void monitor_destroy(void)
{
	pthread_spin_destroy(&listener_lock);
	pthread_spin_destroy(&monitor_lock);
}
