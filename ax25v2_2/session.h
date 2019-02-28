/*
 *  Project: ax25c - File: session.h
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

#ifndef AX25V2_2_SESSION_H_
#define AX25V2_2_SESSION_H_

#include <stdint.h>
#include <stdbool.h>

struct exception;
struct primitive;

struct session {
	uint16_t server_id;
	uint16_t client_id;
	bool     is_active;
};

extern bool init_session(struct session *session, struct exception *ex);

extern void term_session(struct session *session);

extern bool session_tx(struct session *session, struct primitive *prim, struct exception *ex);

extern bool session_rx(struct session *session, struct primitive *prim, struct exception *ex);

#endif /* AX25V2_2_SESSION_H_ */
