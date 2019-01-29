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

#ifndef RUNTIME__INTERNAL_H_
#define RUNTIME__INTERNAL_H_

#define MODULE_NAME "RUNTIME"

extern struct primbuffer *primbuffer;

extern void ax25c_dlsap_init(void);
extern void ax25c_dlsap_term(void);

#endif /* RUNTIME__INTERNAL_H_ */
