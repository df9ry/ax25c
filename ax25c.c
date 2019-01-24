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

#include "configuration.h"
#include "exception.h"
#include "runtime.h"
#include "runtime/log.h"
#include "runtime/tick.h"
#include "config/ax25c_config.h"

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <time.h>

#include <termios.h>

static void exit_handler(void)
{
	struct termios t;

	tcgetattr(STDIN_FILENO, &t);
	t.c_lflag |= (ICANON | ECHO | ISIG);
	tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

static void handle_signal(int signal) {
	if (signal == SIGINT)
		die();
}

int main(int argc, char *argv[]) {
	struct exception ex;

	ax25c_log_init();
	ax25c_tick_init();

	/* Catch signals */
	atexit(exit_handler);
	signal(SIGINT, handle_signal);

	/* Configure: */
	{
		void *module_handle;
		config_func_t configure;

		if (!load_so("ax25c_config.so", &module_handle, &ex)) {
			return print_ex(&ex);
		}
		if (!getsym_so(module_handle, "configure", (void **)&configure, &ex)) {
			return print_ex(&ex);
		}
		if (!configure(argc, argv, &configuration, &ex)) {
			return print_ex(&ex);
		}
		if (!unload_so(module_handle, &ex)) {
			return print_ex(&ex);
		}
	}

	/* Start: */
	INFO("Startup", "");
	if (!start(&ex)) {
		return print_ex(&ex);
	}

	/* Run: */
	INFO("Run", "");
	ex.erc = EXIT_SUCCESS;
	while (tick(&ex)) {
		usleep(configuration.tick * 1000);
	} /* end while */
	if (ex.erc != EXIT_SUCCESS)
		return print_ex(&ex);

	/* Shutdown: */
	INFO("Shutdown", "");
	stop(&ex);
	ax25c_tick_term();
	ax25c_log_term();

	if (configuration.loglevel >= DEBUG_LEVEL_INFO) {
		return print_ex(&ex);
	} else {
		return ex.erc;
	}
}
