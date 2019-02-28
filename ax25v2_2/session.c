/*
 * session.c
 *
 *  Created on: 28.02.2019
 *      Author: tania
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
