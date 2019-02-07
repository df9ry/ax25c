/*
 *  Project: ax25c - File: monitor.h
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

#ifndef RUNTIME_MONITOR_H_
#define RUNTIME_MONITOR_H_

#include "primitive.h"

typedef int (monitor_function)(struct primitive *prim, char *pb, size_t cb,
		struct exception *ex);

extern bool register_monitor_provider(protocol_t protocol,
		monitor_function *func, struct exception *ex);

extern bool unregister_monitor_provider(protocol_t protocol,
		monitor_function *func, struct exception *ex);

extern void monitor_init(void);

extern void monitor_destroy(void);

#endif /* RUNTIME_MONITOR_H_ */
