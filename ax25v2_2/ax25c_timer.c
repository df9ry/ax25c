/*
 *  Project: ax25c - File: timer.c
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

#include "ax25c_timer.h"

pthread_spinlock_t elapsed_timer_list_lock;
LIST_HEAD(elapsed_timer_list);

void init_ax25c_timer(void)
{
	int erc = pthread_spin_init(&elapsed_timer_list_lock, PTHREAD_PROCESS_PRIVATE);
	assert(erc == 0);
}

void term_ax25c_timer(void)
{
	while (!list_empty(&elapsed_timer_list))
		list_del(elapsed_timer_list.next);
	pthread_spin_destroy(&elapsed_timer_list_lock);
}

static void callback(unsigned long data)
{
	ax25c_timer_t *timer = (ax25c_timer_t*)data;
	assert(timer);
	if (timer->state != TIMER_PENDING)
		return;
	pthread_spin_lock(&timer->lock);
	pthread_spin_lock(&elapsed_timer_list_lock);
	timer->state = TIMER_ELAPSED;
	list_add(&timer->node, &elapsed_timer_list);
	pthread_spin_unlock(&elapsed_timer_list_lock);
	pthread_spin_unlock(&timer->lock);
}

void ax25c_timer_init(ax25c_timer_t *timer, unsigned long data,
		unsigned long duration, struct session *session,
		void (*function)(unsigned long))
{
	assert(timer);
	init_timer(&timer->timer);
	INIT_LIST_HEAD(&timer->node);
	timer->timer.data = (unsigned long)timer;
	timer->timer.function = callback;
	timer->state = TIMER_IDLE;
	timer->session = session;
	timer->duration = msecs_to_jiffies(duration);
	timer->rest = 0;
	timer->data = data;
	timer->function = function;
	pthread_spin_init(&timer->lock, PTHREAD_PROCESS_PRIVATE);
}

void ax25c_timer_destroy(ax25c_timer_t *timer)
{
	assert(timer);
	pthread_spin_lock(&timer->lock);
	del_timer(&timer->timer);
	list_del(&timer->node);
	timer->state = TIMER_DESTROYED;
	pthread_spin_unlock(&timer->lock);
	pthread_spin_destroy(&timer->lock);
}
