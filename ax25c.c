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

#include "runtime/ax25c_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static int print_ex(struct exception *ex) {
	assert(ex);
	fprintf(stderr, "Function \"%s\" in module \"%s\""
					" throwed exception \"%s[%s]\""
			        " with error code %i\n",
			(ex->function) ? ex->function : "unknown",
			(ex->module)   ? ex->module   : "unknown",
			(ex->message)  ? ex->message  : "error",
			(ex->param)    ? ex->param    : "",
			ex->erc);
	return ex->erc;
}

int main(int argc, char *argv[]) {
	void *module;
	struct exception ex = {};

	if (!load("ax25c_config", "configuration", &module, &ex))
		return (print_ex(&ex));
	if (!unload(&module, &ex))
		return (print_ex(&ex));
	return EXIT_SUCCESS;
}
