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

struct plugin_handle {
	const char *name;
	size_t      line_length;
	size_t      rx_bufsize;
	size_t      rx_threshold;
	size_t      tx_threshold;
	size_t      s_out_buf;
	const char *mycall;
	const char *lead_txt;
	const char *lead_cmd;
	const char *lead_inf;
	const char *lead_err;
	const char *prompt;
};

#endif /* TERMINAL__INTERNAL_H_ */
