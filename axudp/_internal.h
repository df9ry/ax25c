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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#define MODULE_NAME "AXUDP"

struct primbuffer;

struct plugin_handle {
	const char  *name;
};

struct instance_handle {
	const char  *name;
	dls_t                   dls;
	/***/
	const char             *host;
	const char             *port;
	size_t                  rx_buf_size;
	const char             *mode;
	const char             *ip_version;
	/***/
	volatile bool           alive;
	struct primbuffer       primbuffer;
	uint8_t                *rx_buf;
	bool                    rx_thread_running;
	pthread_t               rx_thread;
	bool                    tx_thread_running;
	pthread_t               tx_thread;
	int                     sockfd;
	bool                    server_mode;
	/* Server mode: Only support one client! */
	struct sockaddr_storage peer_addr;
	socklen_t               peer_addr_len;
};

#endif /* AXUDP__INTERNAL_H_ */
