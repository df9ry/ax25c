/*
 *  Project: ax25c - File: session.c
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

#include "session.h"
#include "_internal.h"

#include "../runtime/exception.h"

#include <errno.h>
#include <assert.h>

bool init_session(struct session *session, struct exception *ex)
{
	assert(session);
	session->is_active = false;
	return true;
}

void term_session(struct session *session)
{
	assert(session);
	session->is_active = false;
}

bool session_tx(struct session *session, struct primitive *prim, struct exception *ex)
{
	assert(session);
	assert(prim);
	if (!session->is_active) {
		exception_fill(ex, EPERM, MODULE_NAME,
				"session_tx", "Session not active", "");
		return false;
	}
	return true;
}

bool session_rx(struct session *session, struct primitive *prim, struct exception *ex)
{
	assert(session);
	assert(prim);
	if (!session->is_active) {
		exception_fill(ex, EPERM, MODULE_NAME,
				"session_rx", "Session not active", "");
		return false;
	}
	return true;
}
