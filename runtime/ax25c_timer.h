/*
 *  Project: ax25c - File: timer.h
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

#ifndef RUNTIME_AX25C_TIMER_H_
#define RUNTIME_AX25C_TIMER_H_

#include <time.h>
#include <pthread.h>
#include <assert.h>
#include <uki/jiffies.h>
#include <uki/timer.h>
#include <uki/list.h>

struct session;

extern pthread_spinlock_t elapsed_timer_list_lock;
extern struct list_head elapsed_timer_list;

enum TIMER_STATE {
	TIMER_IDLE,
	TIMER_PENDING,
	TIMER_ELAPSED,
	TIMER_SUSPENDED,
	TIMER_DESTROYED
};

struct ax25c_timer {
	struct timer_list  timer;
	struct list_head   node;
	enum TIMER_STATE   state;
	struct session    *session;
	unsigned long      duration;
	unsigned long      rest;
	pthread_spinlock_t lock;
	unsigned long      data;
	void (*function)(unsigned long);
};

typedef struct ax25c_timer ax25c_timer_t;

extern void ax25c_timer_init(ax25c_timer_t *timer, unsigned long data,
		unsigned long duration, struct session *session,
		void (*function)(unsigned long));

extern void ax25c_timer_destroy(ax25c_timer_t *timer);

static inline void ax25c_timer_start(ax25c_timer_t *timer)
{
	assert(timer);
	if (timer->state == TIMER_DESTROYED)
		return;
	pthread_spin_lock(&timer->lock);
	del_timer(&timer->timer);
	pthread_spin_lock(&elapsed_timer_list_lock);
	list_del_init(&timer->node);
	pthread_spin_unlock(&elapsed_timer_list_lock);
	timer->timer.expires = jiffies + timer->duration;
	add_timer(&timer->timer);
	timer->state = TIMER_PENDING;
	pthread_spin_unlock(&timer->lock);
}

static inline void ax25c_timer_stop(ax25c_timer_t *timer)
{
	assert(timer);
	if (timer->state == TIMER_DESTROYED)
		return;
	pthread_spin_lock(&timer->lock);
	del_timer(&timer->timer);
	pthread_spin_lock(&elapsed_timer_list_lock);
	list_del_init(&timer->node);
	pthread_spin_unlock(&elapsed_timer_list_lock);
	timer->state = TIMER_IDLE;
	pthread_spin_unlock(&timer->lock);
}

static inline void ax25c_timer_suspend(ax25c_timer_t *timer)
{
	assert(timer);
	if ((timer->state != TIMER_PENDING) || (timer->state == TIMER_DESTROYED))
		return;
	pthread_spin_lock(&timer->lock);
	del_timer(&timer->timer);
	timer->rest = (long)timer->timer.expires - (long)jiffies;
	timer->state = TIMER_SUSPENDED;
	pthread_spin_unlock(&timer->lock);
}

static inline void ax25c_timer_resume(ax25c_timer_t *timer)
{
	assert(timer);
	if ((timer->state != TIMER_SUSPENDED) || (timer->state == TIMER_DESTROYED))
		return;
	pthread_spin_lock(&timer->lock);
	timer->timer.expires = jiffies + timer->rest;
	timer->state = TIMER_PENDING;
	add_timer(&timer->timer);
	pthread_spin_unlock(&timer->lock);
}

static inline void ax25c_timer_set_duration_ms(ax25c_timer_t *timer,
		unsigned long ms)
{
	assert(timer);
	timer->duration = msecs_to_jiffies(ms);
}

#endif /* RUNTIME_AX25C_TIMER_H_ */
