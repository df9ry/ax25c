/*
 *  Project: ax25c - File: tick.c
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

#include "tick.h"
#include "../exception.h"
#include "../runtime.h"

#include <assert.h>
#include <pthread.h>
#include <uki/list.h>

#define MODULE_NAME "Tick"

static volatile bool initialized = false;
static struct list_head list;
static pthread_spinlock_t lock;

void ax25c_tick_init(void)
{
	assert(!initialized);
	assert(pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE) == 0);
	INIT_LIST_HEAD(&list);
	initialized = true;
}

void ax25c_tick_term(void)
{
	assert(initialized);
	while (!list_empty(&list))
		list_del(&list_first_entry(&list, struct tick_listener, node)->node);
	assert(pthread_spin_destroy(&lock) == 0);
	initialized = false;
}

void registerTickListener(struct tick_listener *tl)
{
	assert(tl);
	assert(initialized);
	INIT_LIST_HEAD(&tl->node);
	assert(pthread_spin_lock(&lock) == 0); /*---->*/
	list_add(&tl->node, &list);
	assert(pthread_spin_unlock(&lock) == 0); /*<--*/
}

void unregisterTickListener(struct tick_listener *tl)
{
	assert(tl);
	assert(initialized);
	assert(pthread_spin_lock(&lock) == 0); /*---->*/
	list_del(&tl->node);
	assert(pthread_spin_unlock(&lock) == 0); /*<--*/
}

bool tick(struct exception *ex)
{
	struct list_head *cursor, *temp;
	struct tick_listener *tl;

	assert(initialized);
	assert(ex);
	if (!isAlive()) {
		ex->erc = EXIT_SUCCESS;
		ex->message = "Normal shutdown";
		ex->param = "";
		ex->module = MODULE_NAME;
		ex->function = "tick";
		return false;
	}
	list_for_each_safe(cursor, temp, &list) {
		assert(cursor);
		tl = list_entry(cursor, struct tick_listener, node);
		assert(tl);
		assert(tl->onTick);
		if (!tl->onTick(tl->user_data, ex))
			return false;
	} /* end list_for_each_safe */
	return true;
}
