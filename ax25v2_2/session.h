/*
 * session.h
 *
 *  Created on: 28.02.2019
 *      Author: tania
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
