/*
 *  Project: ax25c - File: ax25c_runtime.c
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

#include "../runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dlfcn.h>
#include <assert.h>

#define MODULE_NAME "RUNTIME"

int print_ex(struct exception *ex)
{
	if (!ex) {
		return EXIT_SUCCESS;
	}
	const char* erm;

	switch (ex->erc) {
	case EXIT_SUCCESS :
		erm = "No error";
		break;
	case EXIT_FAILURE :
		erm = "Application error";
		break;
	default :
		erm = strerror(ex->erc);
		break;
	}
	fprintf(stderr, "Function \"%s\" in module \"%s\""
					" throwed exception \"%s[%s]\""
			        " with error code %i:%s\n",
			(ex->function) ? ex->function : "unknown",
			(ex->module)   ? ex->module   : "unknown",
			(ex->message)  ? ex->message  : "error",
			(ex->param)    ? ex->param    : "",
			ex->erc, erm);
	return ex->erc;
}



bool load_so(const char *name, void **handle, struct exception *ex)
{
	assert(name);
	assert(handle);
	dlerror();
	*handle = dlopen(name, RTLD_NOW);
	if (!*handle) {
		if (ex) {
			ex->erc = EXIT_FAILURE;
			ex->message = dlerror();
			ex->param = name;
			ex->module = MODULE_NAME;
			ex->function = "load_so";
		}
		return false;
	}
	return true;
}

bool getsym_so(void *handle, const char *name, void ** addr, struct exception *ex)
{
	assert(handle);
	assert(name);
	assert(addr);
	dlerror();
	*addr = dlsym(handle, name);
	if (!*addr) {
		if (ex) {
			ex->erc = EXIT_FAILURE;
			ex->message = dlerror();
			ex->param = name;
			ex->module = MODULE_NAME;
			ex->function = "getsym_so";
		}
		return false;
	}
	return true;
}

bool unload_so(void *module, struct exception *ex)
{
	int erc;
	if (module) {
		dlerror();
		erc = dlclose(module);
		if (erc) {
			if (ex) {
				ex->erc = erc;
				ex->message = dlerror();
				ex->param = "";
				ex->module = MODULE_NAME;
				ex->function = "unload_so";
			}
			return false;
		}
	}
	return true;
}
