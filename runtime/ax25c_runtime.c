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

#include "ax25c_runtime.h"

#include <dlfcn.h>
#include <assert.h>

#define MODULE_NAME "RUNTIME"

bool load(char *name, char *ifc, void **handle, struct exception *ex)
{
	assert(name);
	assert(ifc);
	assert(handle);
	*handle = dlopen(name, RTLD_NOW);
	if (!*handle) {
		if (ex) {
			ex->erc = -1;
			ex->message = "Unable to load shared object file";
			ex->param = name;
			ex->module = MODULE_NAME;
			ex->function = "load";
		}
		return false;
	}
	return true;
}

bool unload(void *module, struct exception *ex)
{
	int erc;
	if (module) {
		erc = dlclose(module);
		if (erc) {
			if (ex) {
				ex->erc = erc;
				ex->message = "Unable to unload shared object file";
				ex->param = "";
				ex->module = MODULE_NAME;
				ex->function = "unload";
			}
			return false;
		}
	}
	return true;
}
