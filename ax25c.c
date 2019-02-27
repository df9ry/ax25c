/*
 *  Project: ax25c - File: ax25.c
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

#include "config/configuration.h"
#include "config/ax25c_config.h"
#include "runtime/exception.h"
#include "runtime/runtime.h"
#include "runtime/log.h"
#include "runtime/tick.h"

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <time.h>

#ifdef __MINGW32__
#include <windows.h>
#include <winsock2.h>
#else
#include <termios.h>
#endif

static void exit_handler(void)
{
#ifndef __MINGW32__
	struct termios t;

	tcgetattr(STDIN_FILENO, &t);
	t.c_lflag |= (ICANON | ECHO | ISIG);
	tcsetattr(STDIN_FILENO, TCSANOW, &t);
#endif
}

static void handle_signal(int signal) {
	if (signal == SIGINT)
		die();
}

#ifdef __MINGW32__
#include <windows.h>
void _usleep(__int64 usec)
{
    HANDLE timer;
    LARGE_INTEGER ft;

    ft.QuadPart = -(10*usec); // Convert to 100 nanosecond interval, negative value indicates relative time

    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
}
#else
#define _usleep usleep
#endif

int main(int argc, char *argv[]) {
	EXCEPTION(ex);

	runtime_initialize();

	/* Catch signals */
	atexit(exit_handler);
	signal(SIGINT, handle_signal);

	/* Configure: */
	{
		void *module_handle;
		config_func_t configure;

		if (!load_so("ax25c_config.so", &module_handle, &ex))
			return print_ex(&ex);
		if (!getsym_so(module_handle, "configure", (void **)&configure, &ex))
			return print_ex(&ex);
		if (!configure(argc, argv, &configuration, &ex))
			return print_ex(&ex);
		if (!unload_so(module_handle, &ex))
			return print_ex(&ex);
	}

	/* Start: */
	DBG_INFO("Startup", "");
	if (!start(&ex)) {
		return print_ex(&ex);
	}

	/* Run: */
	DBG_INFO("Run", "");
	ex.erc = EXIT_SUCCESS;
	while (tick(&ex)) {
		_usleep(configuration.tick * 1000);
	} /* end while */
	if (ex.erc != EXIT_SUCCESS)
		return print_ex(&ex);

	/* Shutdown: */
	DBG_INFO("Shutdown", "");
	stop(&ex);
	runtime_terminate();

	if (configuration.loglevel >= DEBUG_LEVEL_INFO) {
		return print_ex(&ex);
	} else {
		return ex.erc;
	}
}
