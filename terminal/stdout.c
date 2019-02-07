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

#include "../runtime/primbuffer.h"
#include "../runtime/dl_prim.h"

#include "_internal.h"

#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <assert.h>

#define S_MONITOR_BUFFER 72
static char monitor_buffer[S_MONITOR_BUFFER];
#define S_MONITOR_PREFIX 16
static char monitor_prefix[S_MONITOR_PREFIX];

struct primbuffer *primbuffer;

static volatile bool initialized = false;
static struct plugin_handle *plugin_handle = NULL;
static pthread_mutex_t mutex;
static pthread_cond_t cond;
static pthread_attr_t thread_args;
static pthread_t thread;

static inline void out_data(const uint8_t *po, size_t co)
{
	write(STDOUT_FILENO, po, co);
}

static inline void out_txt(const char *pd, size_t cd)
{
	out_data((uint8_t*)pd, cd);
}

static inline void out_str(const char *s)
{
	out_txt(s, strlen(s));
}

static void out_dl_prim(primitive_t *prim)
{
	prim_param_t *param;

	switch (prim->cmd) {
	case DL_UNIT_DATA_INDICATION:
		aquire_stdout_lock(STDOUT_thread);
		param = get_DL_data_param(prim);
		out_str("UI: ");
		out_data(get_prim_param_data(param), get_prim_param_size(param));
		release_stdout_lock();
		break;
	case DL_TEST_REQUEST:
		aquire_stdout_lock(STDOUT_thread);
		param = get_DL_data_param(prim);
		out_str("Test REQU: ");
		out_data(get_prim_param_data(param), get_prim_param_size(param));
		release_stdout_lock();
		break;
	case DL_TEST_INDICATION:
		aquire_stdout_lock(STDOUT_thread);
		param = get_DL_data_param(prim);
		out_str("Test INDI: ");
		out_data(get_prim_param_data(param), get_prim_param_size(param));
		release_stdout_lock();
		break;
	case DL_TEST_CONFIRM:
		aquire_stdout_lock(STDOUT_thread);
		param = get_DL_data_param(prim);
		out_str("Test CONF: ");
		out_data(get_prim_param_data(param), get_prim_param_size(param));
		release_stdout_lock();
		break;
	default:
		break;
	} /* end switch */
}

static void *worker(void *id)
{
	primitive_t *prim;

	while (initialized) {
		prim = primbuffer_read_block(primbuffer, NULL);
		if (!prim)
			continue;
		switch (prim->protocol) {
		case DL:
			out_dl_prim(prim);
			break;
		default:
			break;
		} /* end switch */
		del_prim(prim);
	} /* end while */
	return NULL;
}

void monitor_listener(struct primitive *prim, const char *service, bool tx,
		void *data)
{
	int l = monitor(prim, monitor_buffer, S_MONITOR_BUFFER, NULL);
	if (l <= 0)
		return;
	if (l+1 >= S_MONITOR_BUFFER) {
		l = S_MONITOR_BUFFER - 1;
		strcpy(&monitor_buffer[S_MONITOR_BUFFER-4], "...");
	} else {
		monitor_buffer[l] = '\0';
	}
	l = snprintf(monitor_prefix, S_MONITOR_PREFIX, "M> %s %s", service,
			(tx ? "TX " : "RX "));
	if (l+1 >= S_MONITOR_PREFIX) {
		l = S_MONITOR_PREFIX - 1;
		strcpy(&monitor_prefix[S_MONITOR_PREFIX-4], "...");
	} else {
		monitor_prefix[l] = '\0';
	}
#if 0
	aquire_stdout_lock(STDOUT_thread);
	out_str(monitor_prefix);
	out_str(monitor_buffer);
	out_str("\n");
	release_stdout_lock();
#endif
}

void stdout_initialize(struct plugin_handle *h)
{
	int erc;

	assert(h);
	assert(!initialized);
	initialized = true;
	plugin_handle = h;
	primbuffer = primbuffer_new(plugin_handle->buf_size, NULL);
	assert(primbuffer);
	erc = pthread_mutex_init(&mutex, NULL);
	assert(erc == 0);
	erc = pthread_cond_init(&cond, NULL);
	assert(erc == 0);
	pthread_attr_init(&thread_args);
	pthread_attr_setdetachstate(&thread_args, PTHREAD_CREATE_JOINABLE);
	erc = pthread_create(&thread, &thread_args, worker, NULL);
	assert(erc == 0);
	pthread_attr_destroy(&thread_args);
}

void stdout_terminate(struct plugin_handle *h)
{
	assert(h);
	assert(initialized);
	initialized = false;
	plugin_handle = NULL;
	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&mutex);
	pthread_kill(thread, SIGINT);
	primbuffer_del(primbuffer);
	primbuffer = NULL;
	plugin_handle = NULL;
}

static volatile enum stdout_lock_id locked_id = NO_thread;

void aquire_stdout_lock(enum stdout_lock_id id)
{
	bool have_lock;

	pthread_mutex_lock(&mutex);
	have_lock = (locked_id == id);
	pthread_mutex_unlock(&mutex);
	while (!have_lock) {
		pthread_mutex_lock(&mutex);
		if (locked_id != NO_thread) {
			pthread_cond_wait(&cond, &mutex);
		}
		if (locked_id == NO_thread) {
			have_lock = true;
			locked_id = id;
		} else {
			have_lock = false;
		}
		pthread_mutex_unlock(&mutex);
	} /* end while */
}

void release_stdout_lock(void)
{
	pthread_mutex_lock(&mutex);
	locked_id = NO_thread;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
}
