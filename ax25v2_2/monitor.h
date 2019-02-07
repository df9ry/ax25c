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

#ifndef AX25V2_2_MONITOR_H_
#define AX25V2_2_MONITOR_H_

#include <stdbool.h>

struct exception;

extern bool ax25v2_2_monitor_init(struct exception *ex);
extern bool ax25v2_2_monitor_dest(struct exception *ex);

#endif /* AX25V2_2_MONITOR_H_ */
