/*
 *  Project: ax25c - File: _internal.h
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

#ifndef TERMINAL__INTERNAL_H_
#define TERMINAL__INTERNAL_H_

#include <stdlib.h>
#include <stringc/stringc.h>

struct plugin_handle {
	const char *name;
	size_t line_length;
	string_t   loc_addr;
	string_t   rem_addr;
	const char *lead_txt;
	const char *lead_cmd;
	const char *lead_inf;
	const char *lead_err;
	const char *prompt;
};

extern void stdin_initialize(struct plugin_handle *h);

extern void stdin_terminate(struct plugin_handle *h);

extern void stdout_initialize(struct plugin_handle *h);

extern void stdout_terminate(struct plugin_handle *h);

extern void aquire_stdout_lock(void);

extern void release_stdout_lock(void);

#endif /* TERMINAL__INTERNAL_H_ */
