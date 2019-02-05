/*
 *  Project: ax25c - File: dump.c
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

#include "runtime.h"

#include <pthread.h>
#include <ctype.h>
#include <assert.h>

#define BUFSIZE 72
#define SEGMENT 16

static char buffer[BUFSIZE];

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static inline void _dump_segment(enum debug_level_t loglevel,
		const uint8_t *p, uint32_t a, uint32_t c)
{
	uint16_t i, j;
	char ch;

	i = snprintf(buffer, BUFSIZE, "%08x ", a);
	j = 0;
	for (; j < c; ++j) {
		assert(i+4 < BUFSIZE);
		i += snprintf(&buffer[i], BUFSIZE-i, "%02x ", p[j]);
	} /* end for */
	ax25c_log(loglevel, "%s", buffer);

	i = snprintf(buffer, BUFSIZE, "         ");
	j = 0;
	for (; j < c; ++j) {
		assert(i+4 < BUFSIZE);
		ch = (char)p[j];
		if (!isprint(ch))
			ch = '.';
		i += snprintf(&buffer[i], BUFSIZE-i, " %1c ", ch);
	} /* end for */
	ax25c_log(loglevel, "%s", buffer);
}

void _dump(enum debug_level_t loglevel, const uint8_t *p, uint32_t c)
{
	uint32_t a = 0;

	assert(c <= 0xffffffff - SEGMENT);
	pthread_mutex_lock(&mutex);
	if (!p) {
		strcpy(buffer, "<NULL>");
		ax25c_log(loglevel, "%s", buffer);
		goto exit;
	}
	while (a < c) {
		if (a + SEGMENT < c)
			_dump_segment(loglevel, p, a, SEGMENT);
		else
			_dump_segment(loglevel, p, a, c - a);
		p += SEGMENT;
		a += SEGMENT;
	} /* end while */
exit:
	pthread_mutex_unlock(&mutex);
}
