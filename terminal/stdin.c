/*
 *  Project: ax25c - File: stdin.c
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

#include "../runtime/runtime.h"
#include "../runtime/dlsap.h"
#include "../runtime/dl_prim.h"
#include "_internal.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <ctype.h>
#include <signal.h>
#include <assert.h>

#include <termios.h>

#define S_IOBUF 256

#define STOP   3
#define BEL    7
#define BS     8
#define LF    10
#define ESC   27
#define DEL  127

static volatile bool initialized = false;

static struct plugin_handle *plugin_handle = NULL;
static pthread_attr_t thread_args;
static pthread_t thread;

static enum state {
	S_TXT,
	S_CMD, S_CMD_I, S_CMD_R, S_CMD_C, S_CMD_T, S_CMD_U, S_CMD_M, S_CMD_L,
	S_INF,
	S_ERR,
	S_MON
} state;
int substate;

static const char *help =
		"\tC [call] [digi] [digi] - Connect modulo7 to remote\n"
		"\tD                      - Disconnect\n"
		"\tE [call] [digi] [digi] - Connect modulo128 to remote\n"
		"\tH                      - Display this help\n"
		"\tI [call]               - Set or get local call\n"
		"\tL [N|I|W|E|D]          - Set or get log level\n"
		"\tM [+|-]                - Set or get monitor on/off\n"
		"\tQ                      - Quit program\n"
		"\tR [call] [digi] [digi] - Set or get default remote call\n"
		"\tT [call] [digi] [digi] - Send TEST frame\n"
		"\tU [call] [digi] [digi] - Send UI frame\n"
		"\tX                      - Negotiate with remote";

static char read_buf[S_IOBUF];
static size_t i_read_buf = 0;

static bool monitor_flag = false;
static void *monitor_handle = NULL;

static void send_line(const char *pb, size_t cb)
{
	i_read_buf = 0;
	state = S_TXT;
}

static void setMonitor(bool f)
{
	if (f != monitor_flag) {
		monitor_flag = f;
		if (f) {
			assert(!monitor_handle);
			monitor_handle = register_monitor_listener(monitor_listener, NULL);
		} else {
			unregister_monitor_listener(monitor_handle);
			monitor_handle = NULL;
		}
	}
}

static void out_str(const char *str)
{
	aquire_stdout_lock(STDIN_thread);
	write(STDOUT_FILENO, str, strlen(str));
}

static void out_ch(char ch)
{
	aquire_stdout_lock(STDIN_thread);
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
}

static void out_lead(void)
{
	if (writeLeads) {
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
		case S_MON:
			out_ctrl(plugin_handle->lead_mon);
			break;
		default:
			break;
		} /* end switch */
	}
}

static void new_line(void)
{
	out_lead();
	out_ch('\n');
	i_read_buf = 0;
	if (state == S_TXT)
		release_stdout_lock();
}

static const char *getStr(bool add_nl)
{
	const char *p;

	assert(i_read_buf + 2 < S_IOBUF);
	if (add_nl)
		read_buf[i_read_buf++] = '\n';
	read_buf[i_read_buf] = '\0';
	p = read_buf;
	while ((*p) && isspace(*p))
		++p;
	return p;
}

static void inputCh(char ch)
{
	if (i_read_buf + 1 >= S_IOBUF) {
		out_ch(BEL);
		return;
	}
	read_buf[i_read_buf] = ch;
	out_ch(ch);
	++i_read_buf;
}

static void onDel(void)
{
	if (i_read_buf == 0) {
		out_ch(BEL);
		return;
	}
	out_ch(BS);
	out_ch(' ');
	out_ch(BS);
	--i_read_buf;
}

static void onEsc(void)
{
	if (i_read_buf > 0) {
		send_line(read_buf, i_read_buf);
	}
	if (state == S_CMD) {
		state = S_TXT;
		substate = 0;
		new_line();
	} else {
		state = S_CMD;
		substate = 0;
		new_line();
		out_str(plugin_handle->prompt);
	}
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

static int16_t cConnect = 0;

static void connect(void)
{
	EXCEPTION(ex);
	const char *dstAddr = string_c(&plugin.rem_addr);
	const char *srcAddr = string_c(&plugin.loc_addr);
	if (configuration.loglevel >= DEBUG_LEVEL_DEBUG)
		ax25c_log(DEBUG_LEVEL_DEBUG, "TX CONNECT: %s -> %s", srcAddr, dstAddr);
	primitive_t *prim = new_DL_CONNECT_Request(++cConnect,
			(uint8_t*)dstAddr, strlen(dstAddr),
			(uint8_t*)srcAddr, strlen(srcAddr), &ex);
	if (!prim) {
		state = S_ERR;
		new_line();
		out_str(STRING_C(ex.message));
		goto done;
	}
	if (!dlsap_write(peerDLS(), prim, false, &ex)) {
		state = S_ERR;
		new_line();
		out_str(STRING_C(ex.message));
	}
	del_prim(prim);
done:
	EXCEPTION_RESET(ex);
	state = S_TXT;
	new_line();
	i_read_buf = 0;
}

static int16_t cTest = 0;

static void test(void)
{
	EXCEPTION(ex);
	const char *pc = getStr(true);
	const char *dstAddr = string_c(&plugin.rem_addr);
	const char *srcAddr = string_c(&plugin.loc_addr);
	if (configuration.loglevel >= DEBUG_LEVEL_DEBUG)
		ax25c_log(DEBUG_LEVEL_DEBUG, "TX TEST: %s -> %s: %s", srcAddr, dstAddr, pc);
	primitive_t *prim = new_DL_TEST_Request(++cTest,
			(uint8_t*)dstAddr, strlen(dstAddr),
			(uint8_t*)srcAddr, strlen(srcAddr),
			(uint8_t*)pc, strlen(pc), &ex);
	if (!prim) {
		state = S_ERR;
		new_line();
		out_str(STRING_C(ex.message));
		goto done;
	}
	if (!dlsap_write(peerDLS(), prim, false, &ex)) {
		state = S_ERR;
		new_line();
		out_str(STRING_C(ex.message));
	}
	del_prim(prim);
done:
	EXCEPTION_RESET(ex);
	state = S_TXT;
	new_line();
	i_read_buf = 0;
}

static int16_t cUI = 0;

static void ui(void)
{
	EXCEPTION(ex);
	const char *pc = getStr(true);
	const char *dstAddr = string_c(&plugin.rem_addr);
	const char *srcAddr = string_c(&plugin.loc_addr);
	if (configuration.loglevel >= DEBUG_LEVEL_DEBUG)
		ax25c_log(DEBUG_LEVEL_DEBUG, "TX UI: %s -> %s: %s", srcAddr, dstAddr, pc);
	primitive_t *prim = new_DL_UNIT_DATA_Request(++cUI,
			(uint8_t*)dstAddr, strlen(dstAddr),
			(uint8_t*)srcAddr, strlen(srcAddr),
			(uint8_t*)pc, strlen(pc), &ex);
	if (!prim) {
		state = S_ERR;
		new_line();
		out_str(STRING_C(ex.message));
		goto done;
	}
	if (!dlsap_write(peerDLS(), prim, false, &ex)) {
		state = S_ERR;
		new_line();
		out_str(STRING_C(ex.message));
	}
	del_prim(prim);
done:
	EXCEPTION_RESET(ex);
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

static void onHelp(void)
{
	out_str("Help");
	state = S_INF;
	new_line();
	out_str(help);
	state = S_TXT;
	new_line();
}

static void onCmdI(void)
{
	const char *pc = getStr(true);

	assert(plugin_handle);
	assert(pc);
	if (!(*pc)) {
		pc = string_c(&plugin_handle->loc_addr);
		state = S_INF;
		new_line();
		out_str("I ");
		out_str(pc);
	} else {
		EXCEPTION(ex);
		if (dlsap_set_default_local_addr(peerDLS(), pc, &plugin.loc_addr, &ex))
		{
			state = S_INF;
			new_line();
			out_str("I ");
			out_str(string_c(&plugin.loc_addr));
		} else {
			state = S_ERR;
			new_line();
			out_str(STRING_C(ex.message));
		}
		EXCEPTION_RESET(ex);
	}
	i_read_buf = 0;
	state = S_TXT;
	new_line();
}

static void onCmdR(void)
{
	const char *pc = getStr(false);

	assert(plugin_handle);
	assert(pc);
	if (!(*pc)) {
		pc = string_c(&plugin_handle->rem_addr);
		state = S_INF;
		new_line();
		out_str("Remote ");
		out_str(pc);
	} else {
		EXCEPTION(ex);
		if (dlsap_set_default_remote_addr(peerDLS(), pc, &plugin.rem_addr, &ex))
		{
			state = S_INF;
			new_line();
			out_str("Remote ");
			out_str(string_c(&plugin.rem_addr));
		} else {
			state = S_ERR;
			new_line();
			out_str(STRING_C(ex.message));
		}
		EXCEPTION_RESET(ex);
	}
	i_read_buf = 0;
	state = S_TXT;
	new_line();
}

static void onCmdC(void)
{
	const char *pc = getStr(false);

	assert(plugin_handle);
	assert(pc);
	if (!(*pc)) {
		pc = string_c(&plugin_handle->rem_addr);
		state = S_INF;
		new_line();
		out_str("Connect ");
		out_str(pc);
	} else {
		EXCEPTION(ex);
		if (dlsap_set_default_remote_addr(peerDLS(), pc, &plugin.rem_addr, &ex))
		{
			state = S_INF;
			new_line();
			out_str("Remote ");
			out_str(string_c(&plugin.rem_addr));
		} else {
			state = S_ERR;
			new_line();
			out_str(STRING_C(ex.message));
		}
		EXCEPTION_RESET(ex);
	}
	connect();
	i_read_buf = 0;
	state = S_TXT;
	new_line();
}

static void onCmdT(void)
{
	const char *pc = getStr(true);

	assert(plugin_handle);
	assert(pc);
	if (!(*pc)) {
		pc = string_c(&plugin_handle->rem_addr);
		state = S_INF;
		new_line();
		out_str("Test ");
		out_str(pc);
	} else {
		EXCEPTION(ex);
		if (dlsap_set_default_remote_addr(peerDLS(), pc, &plugin.rem_addr, &ex))
		{
			state = S_INF;
			new_line();
			out_str("Test ");
			out_str(string_c(&plugin.rem_addr));
		} else {
			state = S_ERR;
			new_line();
			out_str(STRING_C(ex.message));
			state = S_TXT;
			new_line();
			return;
		}
		EXCEPTION_RESET(ex);
	}
	state = S_INF;
	new_line();
	out_str("> ");
	state = S_CMD_T;
	substate = 2;
	i_read_buf = 0;
}

static void onCmdU(void)
{
	const char *pc = getStr(true);

	assert(plugin_handle);
	assert(pc);
	if (!(*pc)) {
		pc = string_c(&plugin_handle->rem_addr);
		state = S_INF;
		new_line();
		out_str("UI ");
		out_str(pc);
	} else {
		EXCEPTION(ex);
		if (dlsap_set_default_remote_addr(peerDLS(), pc, &plugin.rem_addr, &ex))
		{
			state = S_INF;
			new_line();
			out_str("UI ");
			out_str(STRING_C(plugin.rem_addr));
		} else {
			state = S_ERR;
			new_line();
			out_str(STRING_C(ex.message));
			state = S_TXT;
			new_line();
			return;
		}
		EXCEPTION_RESET(ex);
	}
	state = S_INF;
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
		out_str(monitor_flag ? "on" : "off");
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

static void onCmdL(char ch)
{
	switch(ch) {
	case ' ':
		state = S_INF;
		new_line();
		out_str("Loglevel is ");
		switch (configuration.loglevel) {
		case DEBUG_LEVEL_DEBUG:
			out_str("DEBUG");
			break;
		case DEBUG_LEVEL_ERROR:
			out_str("ERROR");
			break;
		case DEBUG_LEVEL_WARNING:
			out_str("WARNING");
			break;
		case DEBUG_LEVEL_INFO:
			out_str("INFO");
			break;
		case DEBUG_LEVEL_NONE:
			out_str("NONE");
			break;
		default:
			out_str("<INVALID>");
			break;
		} /* end switch */
		state = S_TXT;
		new_line();
		break;
	case 'D':
		state = S_INF;
		new_line();
		configuration.loglevel = DEBUG_LEVEL_DEBUG;
		out_str("Loglevel is DEBUG");
		state = S_TXT;
		new_line();
		break;
	case 'E':
		state = S_INF;
		new_line();
		configuration.loglevel = DEBUG_LEVEL_ERROR;
		out_str("Loglevel is ERROR");
		state = S_TXT;
		new_line();
		break;
	case 'W':
		state = S_INF;
		new_line();
		configuration.loglevel = DEBUG_LEVEL_WARNING;
		out_str("Loglevel is WARNING");
		state = S_TXT;
		new_line();
		break;
	case 'I':
		state = S_INF;
		new_line();
		configuration.loglevel = DEBUG_LEVEL_INFO;
		out_str("Loglevel is INFO");
		state = S_TXT;
		new_line();
		break;
	case 'N':
		state = S_INF;
		new_line();
		configuration.loglevel = DEBUG_LEVEL_NONE;
		out_str("Loglevel is NONE");
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
	case DEL:
		onDel();
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
	case DEL:
		onDel();
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
	case DEL:
		onDel();
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
	case DEL:
		onDel();
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

static void inputCmdT2(char ch)
{
	switch (ch) {
	case DEL:
		onDel();
		break;
	case LF:
		test();
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
	case DEL:
		onDel();
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
	case DEL:
		onDel();
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
	case DEL:
		onDel();
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
	case '+': case 'y': case 'Y': case 'j': case 'J': case 't': case 'T':
		out_str("on");
		onCmdM('+');
		break;
	case '-': case 'n': case 'N': case 'f': case 'F':
		out_str("off");
		onCmdM('-');
		break;
	default :
		break;
	} /* end switch */
}

static void inputCmdL1(char ch)
{
	switch (ch) {
	case DEL:
		onDel();
		break;
	case LF:
		onCmdL(' ');
		break;
	case ESC:
		onEsc();
		break;
	case STOP:
		onQuit();
		break;
	case 'n': case 'N':
		out_str("NONE");
		onCmdL('N');
		break;
	case 'i': case 'I':
		out_str("INFO");
		onCmdL('I');
		break;
	case 'w': case 'W':
		out_str("WARNING");
		onCmdL('W');
		break;
	case 'e': case 'E':
		out_str("ERROR");
		onCmdL('E');
		break;
	case 'd': case 'D':
		out_str("DEBUG");
		onCmdL('D');
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
	case 2:
		inputCmdT2(ch);
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

static void inputCmdL(char ch)
{
	switch (substate) {
	case 0:
		out_str("Loglevel ");
		i_read_buf = 0;
		substate = 1;
		break;
	case 1:
		inputCmdL1(ch);
		break;
	default:
		break;
	} /* end switch */
}

static void inputTxt(char ch)
{
	switch (ch) {
	case DEL:
		onDel();
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
	if (ch == escapeChar) {
		onEsc();
		return;
	}
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
	case 'h': case 'H': case '?':
		onHelp();
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
	case 'l': case 'L':
		state = S_CMD_L;
		substate = 0;
		inputCmdL(ch);
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
	if (ch == escapeChar) {
		onEsc();
		return;
	}
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
	case S_CMD_L:
		inputCmdL(ch);
		break;
	default :
		break;
	} /* end switch */
}

static void *worker(void *id)
{
	while (initialized) {
		uint8_t ch;
		int res = read(STDIN_FILENO, &ch, 1);
		if (res > 0)
			input((char)ch);
	} /* end while */
	return NULL;
}

void stdin_initialize(struct plugin_handle *h)
{
	struct termios t;
	int erc;

	assert(h);
	assert(!initialized);

	tcgetattr(STDIN_FILENO, &t);
	t.c_lflag &= ~(ICANON | ECHO | ISIG);
	tcsetattr(STDIN_FILENO, TCSANOW, &t);

	plugin_handle = h;
	initialized = true;
	state = S_INF;
	substate = 0;
	new_line();
	out_str("Tania's AX.25 Terminal Ready");
	state = S_TXT;
	new_line();
	pthread_attr_init(&thread_args);
	pthread_attr_setdetachstate(&thread_args, PTHREAD_CREATE_JOINABLE);
	erc = pthread_create(&thread, &thread_args, worker, NULL);
	assert(erc == 0);
	pthread_attr_destroy(&thread_args);
}

extern void stdin_terminate(struct plugin_handle *h)
{
	struct termios t;
	int erc;

	assert(h);
	state = S_INF;
	substate = 0;
	new_line();
	out_str("Terminal Quit");
	state = S_TXT;
	new_line();
	tcgetattr(STDIN_FILENO, &t);
	t.c_lflag |= (ICANON | ECHO | ISIG);
	tcsetattr(STDIN_FILENO, TCSANOW, &t);
	if (!initialized)
		return;
	assert(initialized);
	initialized = false;
	erc = pthread_kill(thread, SIGINT);
	assert(erc == 0);
	plugin_handle = NULL;
}
