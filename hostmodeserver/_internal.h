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

#ifndef HOSTMODESERVER__INTERNAL_H_
#define HOSTMODESERVER__INTERNAL_H_

#include "../serial/serial.h"

#include <pthread.h>

#define MODULE_NAME "HOSTMODESERVER"

#define S_SIO_BUF 257

struct plugin_handle {
	const char  *name;
};

struct instance_handle {
	const char   *name;
	/* Settings */
	const char   *comport;
	unsigned int  baudrate;
	/* State */
	HANDLE        serial;
	char          sio_buf[S_SIO_BUF];
	pthread_t     thread;
	volatile bool alive;
};

#endif /* HOSTMODESERVER__INTERNAL_H_ */
