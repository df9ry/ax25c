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
//#include <termios.h>
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
//static struct termios termios_save;
//static bool hasTerminal;
static char *ringbuffer = NULL;
static size_t head, tail = 0;
static struct tick_listener tl;

static char read_buf[S_IOBUF];
static size_t i_read_buf = 0;
static enum state {
	S_TXT,
	S_CMD, S_CMD_I, S_CMD_R, S_CMD_C, S_CMD_T, S_CMD_U, S_CMD_M,
	S_INF,
	S_ERR
} state;
static int substate;
static struct plugin_handle *plugin_handle = NULL;
static bool monitor = false;

static void setMonitor(bool f)
{
	monitor = f;
}

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

static void out_str(const char *str)
{
	assert(str);
	write(STDOUT_FILENO, str, strlen(str));
}

static void out_lead(void)
{
	switch (state) {
	case S_TXT:
		out_ctrl(plugin_handle->lead_txt);
		break;
	case S_CMD:
		out_ctrl(plugin_handle->lead_cmd);
		break;
	case S_INF:
		out_ctrl(plugin_handle->lead_inf);
		break;
	case S_ERR:
		out_ctrl(plugin_handle->lead_err);
		break;
	default:
		break;
	} /* end switch */
}

static void new_line(void)
{
	out_lead();
	out_ch('\n');
}

static const char *getStr(void)
{
	const char *p;

	assert(i_read_buf + 1 < S_IOBUF);
	read_buf[i_read_buf] = '\0';
	p = read_buf;
	while ((*p) && isspace(*p))
		++p;
	return p;
}

static void inputCh(char ch)
{
	if (i_read_buf + 1 >= S_IOBUF) {
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
	substate = 0;
	new_line();
	write(STDIN_FILENO, plugin_handle->prompt, strlen(plugin_handle->prompt));
}

static void onLf(void)
{
	send_line(read_buf, i_read_buf);
	new_line();
}

static void onDisconnect(void)
{
	out_str("Disconnect");
	state = S_ERR;
	new_line();
	out_str("Not connected!");
	state = S_TXT;
	new_line();
	i_read_buf = 0;
}

static void connect(void)
{
	state = S_ERR;
	new_line();
	out_str("Connect not implemented!");
	state = S_TXT;
	new_line();
	i_read_buf = 0;
}

static void test(void)
{
	state = S_ERR;
	new_line();
	out_str("Test not implemented!");
	state = S_TXT;
	new_line();
	i_read_buf = 0;
}

static void ui(void)
{
	state = S_ERR;
	new_line();
	out_str("UI not implemented!");
	state = S_TXT;
	new_line();
	i_read_buf = 0;
}

static void onQuit(void)
{
	out_str("Quit");
	state = S_TXT;
	new_line();
	i_read_buf = 0;
	die();
}

static void onCmdI(void)
{
	char buf[20];
	const char *pc = getStr();

	assert(plugin_handle);
	assert(pc);
	if (!(*pc)) {
		state = S_INF;
		assert(callsignToString(plugin_handle->mycall, buf, 20, NULL) >= 0);
		new_line();
		out_str("I ");
		out_str(buf);
	} else {
		struct exception ex;
		const char *next;
		callsign call = callsignFromString(pc, &next, &ex);
		if (call) {
			if (*next == '\0') {
				plugin_handle->mycall = call;
				assert(callsignToString(plugin_handle->mycall, buf, 20, NULL) >= 0);
				state = S_INF;
				new_line();
				out_str("I ");
				out_str(buf);
			} else {
				state = S_ERR;
				new_line();
				out_str("Extra characters after callsign: ");
				out_str(pc);
			}
		} else {
			state = S_ERR;
			new_line();
			out_str(ex.message);
			out_str("[");
			out_str(ex.param);
			out_str("]");
		}
	}
	i_read_buf = 0;
	state = S_TXT;
	new_line();
}

static void onCmdR(void)
{
	char buf[60];
	const char *pc = getStr();

	assert(plugin_handle);
	assert(pc);
	if (!(*pc)) {
		state = S_INF;
		assert(addressFieldToString(&plugin_handle->addr, buf, 60, NULL));
		new_line();
		out_str("Remote ");
		out_str(buf);
	} else {
		struct exception ex;
		struct addressField af;
		if (addressFieldFromString(plugin_handle->mycall, pc, &af, &ex)) {
			memcpy(&plugin_handle->addr, &af, sizeof(struct addressField));
			assert(addressFieldToString(&plugin_handle->addr, buf, 60, NULL));
			state = S_INF;
			new_line();
			out_str("Remote ");
			out_str(buf);
		} else {
			state = S_ERR;
			new_line();
			out_str(ex.message);
			out_str("[");
			out_str(ex.param);
			out_str("]");
		}
	}
	i_read_buf = 0;
	state = S_TXT;
	new_line();
}

static void onCmdC(void)
{
	char buf[60];
	const char *pc = getStr();

	assert(plugin_handle);
	assert(pc);
	if (!(*pc)) {
		state = S_INF;
		assert(addressFieldToString(&plugin_handle->addr, buf, 60, NULL));
		new_line();
		out_str("Connect ");
		out_str(buf);
		connect();
	} else {
		struct exception ex;
		struct addressField af;
		if (addressFieldFromString(plugin_handle->mycall, pc, &af, &ex)) {
			memcpy(&plugin_handle->addr, &af, sizeof(struct addressField));
			assert(addressFieldToString(&plugin_handle->addr, buf, 60, NULL));
			state = S_INF;
			new_line();
			out_str("Connect ");
			out_str(buf);
			connect();
		} else {
			state = S_ERR;
			new_line();
			out_str(ex.message);
			out_str("[");
			out_str(ex.param);
			out_str("]");
		}
	}
	i_read_buf = 0;
	state = S_TXT;
	new_line();
}

static void onCmdT(void)
{
	char buf[60];
	const char *pc = getStr();

	assert(plugin_handle);
	assert(pc);
	if (!(*pc)) {
		state = S_INF;
		assert(addressFieldToString(&plugin_handle->addr, buf, 60, NULL));
		new_line();
		out_str("Test ");
		out_str(buf);
		test();
	} else {
		struct exception ex;
		struct addressField af;
		if (addressFieldFromString(plugin_handle->mycall, pc, &af, &ex)) {
			memcpy(&plugin_handle->addr, &af, sizeof(struct addressField));
			assert(addressFieldToString(&plugin_handle->addr, buf, 60, NULL));
			state = S_INF;
			new_line();
			out_str("Test ");
			out_str(buf);
			test();
		} else {
			state = S_ERR;
			new_line();
			out_str(ex.message);
			out_str("[");
			out_str(ex.param);
			out_str("]");
		}
	}
	i_read_buf = 0;
	state = S_TXT;
	new_line();
}

static void onCmdU(void)
{
	char buf[60];
	const char *pc = getStr();

	assert(plugin_handle);
	assert(pc);
	if (!(*pc)) {
		assert(addressFieldToString(&plugin_handle->addr, buf, 60, NULL));
	} else {
		struct exception ex;
		struct addressField af;
		if (addressFieldFromString(plugin_handle->mycall, pc, &af, &ex)) {
			memcpy(&plugin_handle->addr, &af, sizeof(struct addressField));
			assert(addressFieldToString(&plugin_handle->addr, buf, 60, NULL));
		} else {
			state = S_ERR;
			new_line();
			out_str(ex.message);
			out_str("[");
			out_str(ex.param);
			out_str("]");
		}
	}
	state = S_INF;
	new_line();
	out_str("UI ");
	out_str(buf);
	new_line();
	out_str("> ");
	state = S_CMD_U;
	substate = 2;
	i_read_buf = 0;
}

static void onCmdM(char ch)
{
	switch(ch) {
	case ' ':
		state = S_INF;
		new_line();
		out_str("Monitor is ");
		out_str(monitor ? "on" : "off");
		state = S_TXT;
		new_line();
		break;
	case '+':
		state = S_INF;
		new_line();
		setMonitor(true);
		out_str("Monitor is on");
		state = S_TXT;
		new_line();
		break;
	case '-':
		state = S_INF;
		new_line();
		setMonitor(false);
		out_str("Monitor is off");
		state = S_TXT;
		new_line();
		break;
	default:
		break;
	} /* end switch */
}

static void inputCmdI1(char ch)
{
	switch (ch) {
	case BS:
		onBs();
		break;
	case LF:
		onCmdI();
		break;
	case ESC:
		onEsc();
		break;
	case STOP:
		onQuit();
		break;
	default :
		if (isprint(ch))
			inputCh(ch);
		break;
	} /* end switch */
}

static void inputCmdR1(char ch)
{
	switch (ch) {
	case BS:
		onBs();
		break;
	case LF:
		onCmdR();
		break;
	case ESC:
		onEsc();
		break;
	case STOP:
		onQuit();
		break;
	default :
		if (isprint(ch))
			inputCh(ch);
		break;
	} /* end switch */
}

static void inputCmdC1(char ch)
{
	switch (ch) {
	case BS:
		onBs();
		break;
	case LF:
		onCmdC();
		break;
	case ESC:
		onEsc();
		break;
	case STOP:
		onQuit();
		break;
	default :
		if (isprint(ch))
			inputCh(ch);
		break;
	} /* end switch */
}

static void inputCmdT1(char ch)
{
	switch (ch) {
	case BS:
		onBs();
		break;
	case LF:
		onCmdT();
		break;
	case ESC:
		onEsc();
		break;
	case STOP:
		onQuit();
		break;
	default :
		if (isprint(ch))
			inputCh(ch);
		break;
	} /* end switch */
}

static void inputCmdU1(char ch)
{
	switch (ch) {
	case BS:
		onBs();
		break;
	case LF:
		onCmdU();
		break;
	case ESC:
		onEsc();
		break;
	case STOP:
		onQuit();
		break;
	default :
		if (isprint(ch))
			inputCh(ch);
		break;
	} /* end switch */
}

static void inputCmdU2(char ch)
{
	switch (ch) {
	case BS:
		onBs();
		break;
	case LF:
		ui();
		break;
	case ESC:
		onEsc();
		break;
	case STOP:
		onQuit();
		break;
	default :
		if (isprint(ch))
			inputCh(ch);
		break;
	} /* end switch */
}

static void inputCmdM1(char ch)
{
	switch (ch) {
	case BS:
		onBs();
		break;
	case LF:
		onCmdM(' ');
		break;
	case ESC:
		onEsc();
		break;
	case STOP:
		onQuit();
		break;
	case '+':
		out_str("+");
		onCmdM('+');
		break;
	case '-':
		out_str("-");
		onCmdM('-');
		break;
	default :
		break;
	} /* end switch */
}

static void inputCmdI(char ch)
{
	switch (substate) {
	case 0:
		out_str("I ");
		i_read_buf = 0;
		substate = 1;
		break;
	case 1:
		inputCmdI1(ch);
		break;
	default:
		break;
	} /* end switch */
}

static void inputCmdR(char ch)
{
	switch (substate) {
	case 0:
		out_str("Remote ");
		i_read_buf = 0;
		substate = 1;
		break;
	case 1:
		inputCmdR1(ch);
		break;
	default:
		break;
	} /* end switch */
}

static void inputCmdC(char ch)
{
	switch (substate) {
	case 0:
		out_str("Connect ");
		i_read_buf = 0;
		substate = 1;
		break;
	case 1:
		inputCmdC1(ch);
		break;
	default:
		break;
	} /* end switch */
}

static void inputCmdT(char ch)
{
	switch (substate) {
	case 0:
		out_str("Test ");
		i_read_buf = 0;
		substate = 1;
		break;
	case 1:
		inputCmdT1(ch);
		break;
	default:
		break;
	} /* end switch */
}

static void inputCmdU(char ch)
{
	switch (substate) {
	case 0:
		out_str("UI ");
		i_read_buf = 0;
		substate = 1;
		break;
	case 1:
		inputCmdU1(ch);
		break;
	case 2:
		inputCmdU2(ch);
		break;
	default:
		break;
	} /* end switch */
}

static void inputCmdM(char ch)
{
	switch (substate) {
	case 0:
		out_str("Monitor ");
		i_read_buf = 0;
		substate = 1;
		break;
	case 1:
		inputCmdM1(ch);
		break;
	default:
		break;
	} /* end switch */
}

static void inputTxt(char ch)
{
	switch (ch) {
	case BS:
		onBs();
		break;
	case LF:
		onLf();
		break;
	case ESC:
		onEsc();
		break;
	case STOP:
		state = S_CMD;
		new_line();
		out_str(plugin_handle->prompt);
		onQuit();
		break;
	default :
		if (isprint(ch))
			inputCh(ch);
		break;
	} /* end switch */
}

static void inputCmd0(char ch)
{
	switch (ch) {
	case ESC:
		onEsc();
		break;
	case STOP:
		onQuit();
		break;
	case 'i': case 'I':
		state = S_CMD_I;
		substate = 0;
		inputCmdI(ch);
		break;
	case 'r': case 'R':
		state = S_CMD_R;
		substate = 0;
		inputCmdR(ch);
		break;
	case 'c': case 'C':
		state = S_CMD_C;
		substate = 0;
		inputCmdC(ch);
		break;
	case 'd': case 'D':
		onDisconnect();
		break;
	case 'q': case 'Q':
		onQuit();
		break;
	case 't': case 'T':
		state = S_CMD_T;
		substate = 0;
		inputCmdT(ch);
		break;
	case 'u': case 'U':
		state = S_CMD_U;
		substate = 0;
		inputCmdU(ch);
		break;
	case 'm': case 'M':
		state = S_CMD_M;
		substate = 0;
		inputCmdM(ch);
		break;
	default:
		break;
	} /* end switch */
}

static void inputCmd(char ch)
{
	switch (substate) {
	case 0:
		inputCmd0(ch);
		break;
	default:
		break;
	} /* end switch */
}

static void input(char ch)
{
	switch (state) {
	case S_TXT :
		inputTxt(ch);
		break;
	case S_CMD :
		inputCmd(ch);
		break;
	case S_CMD_I:
		inputCmdI(ch);
		break;
	case S_CMD_R:
		inputCmdR(ch);
		break;
	case S_CMD_C:
		inputCmdC(ch);
		break;
	case S_CMD_T:
		inputCmdT(ch);
		break;
	case S_CMD_U:
		inputCmdU(ch);
		break;
	case S_CMD_M:
		inputCmdM(ch);
		break;
	default :
		break;
	} /* end switch */
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
//	struct termios t;
//	int erc, flags;

	assert(!initialized);
	assert(h);
	plugin_handle = h;
	/* Switch off echo and line processing: */
//	hasTerminal = (tcgetattr(STDIN_FILENO, &t) == 0);
//	if (hasTerminal) {
//		termios_save = t;
//		t.c_lflag &= ~(ICANON | ECHO | ISIG);
//		assert(tcsetattr(STDIN_FILENO, TCSANOW, &t) == 0);
//	} else {
//		erc = errno;
//		if (configuration.loglevel >= DEBUG_LEVEL_WARNING) {
//			ax25c_log(DEBUG_LEVEL_WARNING,
//					"Unable to get terminal. Error %i: %s",
//					erc, strerror(erc));
//		}
//	}
	ringbuffer = malloc(h->s_out_buf);
	assert(ringbuffer);
	head = tail = 0;

	i_read_buf = 0;

//	flags = fcntl(STDIN_FILENO, F_GETFL);
//	assert(flags != -1);
//	flags |= O_NONBLOCK;
//	assert(fcntl(STDIN_FILENO, F_SETFL, flags) != -1);
	initialized = true;
	state = S_INF;
	new_line();
	out_str("AX.25 Terminal Ready");
	state = S_TXT;
	new_line();
	tl.onTick = onTick;
	tl.user_data = NULL;
	registerTickListener(&tl);
}

void terminate(struct plugin_handle *h)
{
//	int flags;

	assert(initialized);
	assert(h);
	plugin_handle = NULL;
//	flags = fcntl(STDIN_FILENO, F_GETFL);
//	assert(flags != -1);
//	flags &= ~O_NONBLOCK;
//	assert(fcntl(STDIN_FILENO, F_SETFL, flags) != -1);
//	if (hasTerminal) {
//		/* Restore terminal settings */
//		assert(tcsetattr(STDIN_FILENO, TCSANOW, &termios_save) == 0);
//	}
	initialized = false;
	assert(ringbuffer);
	free(ringbuffer);
	unregisterTickListener(&tl);
}
