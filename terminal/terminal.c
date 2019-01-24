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

#include <assert.h>

#define S_INBUF 64
#define MODULE_NAME "Terminal"

static volatile bool initialized = false;

void initialize(struct plugin_handle *h)
{
	assert(!initialized);
	assert(h);
	initialized = true;
	stdout_initialize(h);
	stdin_initialize(h);
}

void terminate(struct plugin_handle *h)
{
	assert(initialized);
	assert(h);
	initialized = false;
	stdin_terminate(h);
	stdout_terminate(h);
}
