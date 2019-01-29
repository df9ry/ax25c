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

#ifndef AXUDP__INTERNAL_H_
#define AXUDP__INTERNAL_H_

#include "../runtime/dlsap.h"

#define MODULE_NAME "AXUDP"

struct plugin_handle {
	const char  *name;
};

struct instance_handle {
	const char  *name;
	/***/
	dls_t dls;
};

#endif /* AXUDP__INTERNAL_H_ */
