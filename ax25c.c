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

#define _POSIX_SOURCE 1

#include "configuration.h"
#include "exception.h"
#include "runtime.h"
#include "config/ax25c_config.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	struct exception ex;

	{ /* Load the configuration */
		void *module_handle;
		config_func_t configure;

		if (!load_so("ax25c_config.so", &module_handle, &ex))
			return print_ex(&ex);
		if (!getsym_so(module_handle, "configure", (void **)&configure, &ex))
			return print_ex(&ex);
		if (!configure(argc, argv, &configuration, &ex))
			return print_ex(&ex);
		if (!start(&ex))
			return print_ex(&ex);
		if (!unload_so(module_handle, &ex))
			return print_ex(&ex);
	}

	return EXIT_SUCCESS;
}
