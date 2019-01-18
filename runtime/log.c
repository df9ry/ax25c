/*
 *  Project: ax25c - File: log.c
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

#include "../runtime.h"

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <pthread.h>
#include <assert.h>

#define FORMAT_BUFSIZE 128
#define RING_BUFSIZE   1024

static volatile bool initialized = false;
static pthread_spinlock_t lock;
static pthread_mutex_t mutex;
static pthread_cond_t cond;
static pthread_attr_t thread_args;
static pthread_t thread;

static char format_buffer[FORMAT_BUFSIZE];
static char ring_buffer[RING_BUFSIZE];

static size_t head = 0;
static size_t tail = 0;

static inline size_t _avail(void)
{
	if (tail <= head) {
		return RING_BUFSIZE - (head - tail);
	} else {
		return tail - head;
	}
}

static inline void _put(const char *pb, size_t cb)
{
	size_t new_head = head + cb, i;
	assert(cb <= _avail());
	if (new_head < RING_BUFSIZE) {
		memcpy(&ring_buffer[head], pb, cb);
	} else {
		i = RING_BUFSIZE - head;
		memcpy(&ring_buffer[head], pb, i);
		new_head -= RING_BUFSIZE;
		memcpy(ring_buffer, &pb[i], new_head);
	}
	head = new_head;
}

static inline void _get(char **ppb, size_t *pcb, size_t *ptail)
{
	assert(ppb);
	assert(pcb);
	assert(ptail);
	*ppb = &ring_buffer[tail];
	if (tail <= head) {
		*pcb = head - tail;
		*ptail = head;
	} else {
		*pcb = RING_BUFSIZE - tail;
		*ptail = 0;
	}
}

static inline void pump(void)
{
	char *pb;
	size_t cb, new_tail;

	while (true) {
		assert(pthread_spin_lock(&lock) == 0);   /*-->*/
		_get(&pb, &cb, &new_tail);
		assert(pthread_spin_unlock(&lock) == 0); /*<--*/
		if (cb > 0) {
			write(STDERR_FILENO, pb, cb);
			assert(pthread_spin_lock(&lock) == 0);   /*-->*/
			tail = new_tail;
			assert(pthread_spin_unlock(&lock) == 0); /*<--*/
		} else {
			break;
		}
	} /* end while */
}

static const char *ll(enum debug_level_t dl)
{
	switch (dl) {
	case DEBUG_LEVEL_NONE:    return "N:";
	case DEBUG_LEVEL_INFO:    return "I:";
	case DEBUG_LEVEL_WARNING: return "W:";
	case DEBUG_LEVEL_ERROR:   return "E:";
	case DEBUG_LEVEL_DEBUG:   return "D:";
	default:                  return "?:";
	} /* end switch */
}

void ax25c_log(enum debug_level_t dl, const char *fmt, ...)
{
	va_list ap;
	size_t msg_size, avail_size;

	assert(initialized);
	if (configuration.loglevel < dl)
		return;
	assert(pthread_spin_lock(&lock) == 0); /*------------------------>*/
	va_start(ap, fmt);
	msg_size = vsnprintf(format_buffer, FORMAT_BUFSIZE, fmt, ap);
	if (msg_size > FORMAT_BUFSIZE) {
		memcpy(&format_buffer[FORMAT_BUFSIZE-5], "...", 4);
		msg_size = FORMAT_BUFSIZE;
	}
	avail_size = _avail();
	if (avail_size >= msg_size + 3) {
		_put(ll(dl), 2);
		_put(format_buffer, msg_size);
		_put("\n", 1);
	} else if (avail_size >= 6) {
		_put(ll(dl), 2);
		_put("...\n", 4);
	}
	assert(pthread_spin_unlock(&lock) == 0); /*<----------------------*/
	pthread_mutex_lock(&mutex);
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
}

void *worker(void *id)
{
	while (initialized) {
		pump();
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&cond, &mutex);
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

void ax25c_log_init(void)
{
	assert(!initialized);
	head = tail = 0;
	configuration.loglevel = DEBUG_LEVEL_NONE;
	assert(pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE) == 0);
	assert(pthread_mutex_init(&mutex, NULL) == 0);
	assert(pthread_cond_init(&cond, NULL) == 0);
	initialized = true;
	pthread_attr_init(&thread_args);
	pthread_attr_setdetachstate(&thread_args, PTHREAD_CREATE_JOINABLE);
	assert(pthread_create(&thread, &thread_args, worker, NULL) == 0);
	pthread_attr_destroy(&thread_args);
}

void ax25c_log_term(void)
{
	void *status;

	assert(initialized);
	initialized = false;
	pthread_mutex_lock(&mutex);
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
	assert(pthread_join(thread, &status) == 0);
	assert(pthread_cond_destroy(&cond) == 0);
	assert(pthread_mutex_destroy(&mutex) == 0);
	assert(pthread_spin_destroy(&lock) == 0);
}
