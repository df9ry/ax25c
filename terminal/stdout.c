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
#include "../runtime/ringbuffer.h"
#include "../runtime/dl_prim.h"

#include "_internal.h"

#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <assert.h>

#define S_MONITOR_BUFFER 100
#define L_MONITOR_BUFFER 72
static char mon_line_buffer[S_MONITOR_BUFFER];
static int i_mon_line_buffer = 0;

static volatile bool initialized = false;
struct primbuffer *primbuffer;
static ringbuffer_t  rb_monitor;
static struct plugin_handle *plugin_handle = NULL;
static pthread_mutex_t mutex;
static pthread_cond_t cond;
static pthread_attr_t thread_args;
static pthread_t prim_thread;
static pthread_t monitor_thread;

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

static void *prim_worker(void *id)
{
	primitive_t *prim;

	while (initialized) {
		prim = primbuffer_read_block(primbuffer, NULL);
		if (!initialized) {
			if (prim)
				del_prim(prim);
			return NULL;
		}
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

static inline void out_ch(char ch)
{
	write(STDOUT_FILENO, &ch, 1);
}

static inline void out_ctrl(const char *ctrl)
{
	long int n;
	char *end;

	assert(ctrl);
	while (*ctrl) {
		if (*ctrl == '\\') {
			++ctrl;
			n = strtol(ctrl, &end, 0);
			ctrl = end;
			out_ch((char)n);
		} else {
			out_ch(*ctrl);
			++ctrl;
		}
	} /* end while */
}

static void monitor_write(struct plugin_handle *handle, char *pb, size_t cb)
{
	if (cb == 0)
		return;
	aquire_stdout_lock(MONITOR_thread);
	if (writeLeads) {
		out_ctrl(handle->lead_mon);
		out_ch('\n');
		if (pb[cb-1] == '\n')
			cb--;
	}
	write(STDOUT_FILENO, pb, cb);
	if (writeLeads) {
		out_ctrl(handle->lead_txt);
		out_ch('\n');
	}
	release_stdout_lock();
}

static void *monitor_worker(void *id)
{
	struct plugin_handle *handle = id;
	int n, m;
	void *p;

	while (initialized) {
		n = rb_read_block_ch(&rb_monitor, &mon_line_buffer[i_mon_line_buffer],
				S_MONITOR_BUFFER - i_mon_line_buffer);
		if (!initialized)
			return NULL;
		if (n <= 0)
			continue;
		i_mon_line_buffer += n;
		while (true) {
			p = memchr(mon_line_buffer, '\n', i_mon_line_buffer);
			if (p) {
				m = p - (void*)mon_line_buffer + 1;
				monitor_write(handle, mon_line_buffer, m);
				memmove(mon_line_buffer, &mon_line_buffer[m], m);
				i_mon_line_buffer -= m;
			} else {
				break;
			}
		} /* end while */
		if (i_mon_line_buffer >= L_MONITOR_BUFFER) {
			monitor_write(handle, mon_line_buffer, L_MONITOR_BUFFER);
			memmove(mon_line_buffer, &mon_line_buffer[L_MONITOR_BUFFER],
					L_MONITOR_BUFFER);
			i_mon_line_buffer -= L_MONITOR_BUFFER;
		}
	} /* end while */
	return NULL;
}

void monitor_listener(struct primitive *prim, const char *service, bool tx,
		void *data)
{
	int l = snprintf(mon_line_buffer, S_MONITOR_BUFFER, "%s %s", service,
			(tx ? "TX " : "RX "));
	int m;

	if (l+1 >= S_MONITOR_BUFFER) {
		l = S_MONITOR_BUFFER - 1;
		strcpy(&mon_line_buffer[S_MONITOR_BUFFER-4], "...");
	} else {
		mon_line_buffer[l] = '\0';
	}
	m = monitor(prim, &mon_line_buffer[l], S_MONITOR_BUFFER-l, NULL);
	if (m <= 0)
		return;
	l += m;
	if (l+2 >= S_MONITOR_BUFFER) {
		l = S_MONITOR_BUFFER - 5;
		strcpy(&mon_line_buffer[l], "...");
		l += 3;
	}
	strcpy(&mon_line_buffer[l], "\n");
	l += 1;
	if (rb_get_free(&rb_monitor) >= l)
		rb_write_nonblock_ch(&rb_monitor, mon_line_buffer, l);
	else
		ax25c_log(DEBUG_LEVEL_WARNING, "Lost %i monitor bytes: buffer full", l);
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
	erc = rb_init(&rb_monitor, plugin_handle->mon_size);
	assert(!erc);
	erc = pthread_mutex_init(&mutex, NULL);
	assert(erc == 0);
	erc = pthread_cond_init(&cond, NULL);
	assert(erc == 0);
	pthread_attr_init(&thread_args);
	pthread_attr_setdetachstate(&thread_args, PTHREAD_CREATE_JOINABLE);
	erc = pthread_create(&prim_thread, &thread_args, prim_worker, h);
	assert(erc == 0);
	erc = pthread_create(&monitor_thread, &thread_args, monitor_worker, h);
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
	pthread_kill(prim_thread, SIGINT);
	pthread_kill(monitor_thread, SIGINT);
	rb_destroy(&rb_monitor);
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
