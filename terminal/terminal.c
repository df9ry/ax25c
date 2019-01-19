/*
 *  Project: ax25c - File: terminal.c
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

#include "terminal.h"
#include "_internal.h"
#include "../runtime.h"

#include <sys/types.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>

#define S_IOBUF 256
#define S_INBUF 64
#define MODULE_NAME "Terminal"
#define ESC 27
#define BS 127
#define LF 10
#define STOP 3
#define BEL 7

static volatile bool initialized = false;
static struct termios termios_save;
static bool hasTerminal;
static char *ringbuffer = NULL;
static size_t head, tail = 0;
static struct tick_listener tl;

static char read_buf[S_IOBUF];
static size_t i_read_buf = 0;
static enum state {	S_TXT, S_CMD, S_INF, S_ERR } state;
static struct plugin_handle *plugin_handle = NULL;

static void send_line(const char *pb, size_t cb)
{
	i_read_buf = 0;
	state = S_TXT;
}

static void out_ch(char ch)
{
	write(STDOUT_FILENO, &ch, 1);
}

static void out_ctrl(const char *ctrl)
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
	write(STDOUT_FILENO, ctrl, strlen(ctrl));
}

static void out_lead(void)
{
	switch (state) {
	case S_TXT : out_ctrl(plugin_handle->lead_txt); break;
	case S_CMD : out_ctrl(plugin_handle->lead_cmd); break;
	case S_INF : out_ctrl(plugin_handle->lead_inf); break;
	case S_ERR : out_ctrl(plugin_handle->lead_err); break;
	default    : assert(false);                     break;
	} /* end switch */
}

static void new_line(void)
{
	out_lead();
	out_ch('\n');
}

static void onPrint(char ch)
{
	if (i_read_buf >= S_IOBUF) {
		out_ch(7);
		return;
	}
	read_buf[i_read_buf] = ch;
	out_ch(ch);
	++i_read_buf;
}

static void onBs(void)
{
	if (i_read_buf == 0) {
		out_ch(7);
		return;
	}
	out_ch(8);
	out_ch(' ');
	out_ch(8);
	--i_read_buf;
}

static void onEsc(void)
{
	if (i_read_buf > 0) {
		send_line(read_buf, i_read_buf);
	}
	state = S_CMD;
	new_line();
	write(STDIN_FILENO, plugin_handle->prompt, strlen(plugin_handle->prompt));
}

static void onLf(void)
{
	send_line(read_buf, i_read_buf);
	new_line();
}

static void input(char ch)
{
	if (state == S_CMD) {
		switch (ch) {
		case BS   : onBs();   break;
		case LF   : onLf();   break;
		case STOP : die();    break;
		default :
			if (isprint(ch))
				onPrint(ch);
		} /* end case */
	} else {
		if (ch == ESC) {
			onEsc();
		}
	}
}

static bool onTick(void *user_data, struct exception *ex)
{
	int i, n;
	char buf[S_INBUF];

	assert(initialized);
	assert(ex);
	n = read(STDIN_FILENO, buf, S_INBUF);
	if (n > 0) {
		for (i = 0; i < n; ++i)
			input(buf[i]);
	}
	return true;
}

void initialize(struct plugin_handle *h)
{
	struct termios t;
	int erc, flags;

	assert(!initialized);
	assert(h);
	plugin_handle = h;
	/* Switch off echo and line processing: */
	hasTerminal = (tcgetattr(STDIN_FILENO, &t) == 0);
	if (hasTerminal) {
		termios_save = t;
		t.c_lflag &= ~(ICANON | ECHO | ISIG);
		assert(tcsetattr(STDIN_FILENO, TCSANOW, &t) == 0);
	} else {
		erc = errno;
		if (configuration.loglevel >= DEBUG_LEVEL_WARNING) {
			ax25c_log(DEBUG_LEVEL_WARNING,
					"Unable to get terminal. Error %i: %s",
					erc, strerror(erc));
		}
	}
	ringbuffer = malloc(h->s_out_buf);
	assert(ringbuffer);
	head = tail = 0;

	i_read_buf = 0;

	flags = fcntl(STDIN_FILENO, F_GETFL);
	assert(flags != -1);
	flags |= O_NONBLOCK;
	assert(fcntl(STDIN_FILENO, F_SETFL, flags) != -1);
	initialized = true;
	state = S_TXT;
	new_line();
	tl.onTick = onTick;
	tl.user_data = NULL;
	registerTickListener(&tl);
}

void terminate(struct plugin_handle *h)
{
	int flags;

	assert(initialized);
	assert(h);
	plugin_handle = NULL;
	flags = fcntl(STDIN_FILENO, F_GETFL);
	assert(flags != -1);
	flags &= ~O_NONBLOCK;
	assert(fcntl(STDIN_FILENO, F_SETFL, flags) != -1);
	if (hasTerminal) {
		/* Restore terminal settings */
		assert(tcsetattr(STDIN_FILENO, TCSANOW, &termios_save) == 0);
	}
	initialized = false;
	assert(ringbuffer);
	free(ringbuffer);
	unregisterTickListener(&tl);
}
