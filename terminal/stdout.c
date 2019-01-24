/*
 *  Project: ax25c - File: stdout.c
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

#include "_internal.h"

#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>

static bool initialized = false;
static struct plugin_handle *plugin_handle = NULL;
static pthread_spinlock_t spinlock;
static pthread_mutex_t mutex;

void stdout_initialize(struct plugin_handle *h)
{
	assert(h);
	assert(!initialized);
	initialized = true;
	plugin_handle = h;
	assert(pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE) == 0);
	assert(pthread_mutex_init(&mutex, NULL) == 0);
}

void stdout_terminate(struct plugin_handle *h)
{
	assert(h);
	assert(initialized);
	initialized = false;
	plugin_handle = NULL;
	assert(pthread_mutex_destroy(&mutex) == 0);
	assert(pthread_spin_destroy(&spinlock) == 0);
}

void aquire_stdout_lock(void)
{
	pthread_spin_lock(&spinlock);
	pthread_mutex_lock(&mutex);
	pthread_spin_unlock(&spinlock);
}

void release_stdout_lock(void)
{
	pthread_spin_lock(&spinlock);
	pthread_mutex_unlock(&mutex);
	pthread_spin_unlock(&spinlock);
}
