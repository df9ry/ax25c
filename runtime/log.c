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
#include <ringbuffer.h>

#define FORMAT_BUFSIZE 72
static char format_buffer[FORMAT_BUFSIZE];

#define PUMP_BUFSIZE 256
static char pump_buffer[PUMP_BUFSIZE];

#define LOGBUF_SIZE 4096

static volatile bool initialized = false;
static ringbuffer_t ring_buffer;
static pthread_attr_t thread_args;
static pthread_t thread;

void ax25c_log(enum debug_level_t dl, const char *fmt, ...)
{
	va_list ap;
	size_t msg_size;
	int written;

	assert(initialized);
	if (configuration.loglevel < dl)
		return;
	va_start(ap, fmt);
	switch (dl) {
	case DEBUG_LEVEL_NONE:
		format_buffer[0] = 'N';
		break;
	case DEBUG_LEVEL_ERROR:
		format_buffer[0] = 'E';
		break;
	case DEBUG_LEVEL_WARNING:
		format_buffer[0] = 'W';
		break;
	case DEBUG_LEVEL_INFO:
		format_buffer[0] = 'I';
		break;
	case DEBUG_LEVEL_DEBUG:
		format_buffer[0] = 'D';
		break;
	default:
		format_buffer[0] = '?';
		break;
	} /* end witch */
	format_buffer[1] = ':';
	msg_size = vsnprintf(&format_buffer[2], FORMAT_BUFSIZE-2, fmt, ap) + 2;
	if (msg_size > FORMAT_BUFSIZE - 3) {
		strcpy(&format_buffer[FORMAT_BUFSIZE-6], "...");
		msg_size = FORMAT_BUFSIZE - 3;
	}
	strcpy(&format_buffer[msg_size++], "\n");
	written = rb_write_asy_ch(&ring_buffer, format_buffer, strlen(format_buffer));
	if (written < 0)
		rb_loose(&ring_buffer, msg_size);
}

void *worker(void *id)
{
	int res;

	while (initialized) {
		res = rb_clear_lost(&ring_buffer);
		if (res > 0) {
			if (configuration.loglevel >= DEBUG_LEVEL_WARNING) {
				fprintf(stderr, "W:Debug lost: %i characters\n", res);
			}
		}
		res = rb_read_syn_ch(&ring_buffer, pump_buffer, PUMP_BUFSIZE);
		if (res > 0)
			write(STDERR_FILENO, pump_buffer, res);
	} /* end while */
	return NULL;
}

void ax25c_log_init(void)
{
	assert(!initialized);
	configuration.loglevel = DEBUG_LEVEL_NONE;
	assert(rb_init(&ring_buffer, LOGBUF_SIZE) == 0);
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
	assert(pthread_join(thread, &status) == 0);
	assert(rb_destroy(&ring_buffer) == 0);
}
