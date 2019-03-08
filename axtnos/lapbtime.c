/*
 *  Project: ax25c - File: lapbtime.c
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

/* LAPB (AX.25) timer recovery routines
 * Copyright 1991 Phil Karn, KA9Q
 *
 * Mods by G1EMM
 */
#include "global.h"
#include "mbuf.h"
#include "ax25.h"

static void tx_enq (struct ax25_cb * axp);

int lapbtimertype = 0;		/* default to binary exponential */

/* Called whenever timer T1 expires */
void
recover (void *p)
{
register struct ax25_cb *axp = (struct ax25_cb *) p;
long waittime = dur_timer (&axp->t1);
long maxwait;

	axp->flags.retrans = 1;
	axp->retries++;

	/* update the counters for retries out */
	if (axp && axp->iface)
		axp->iface->axcnt.retries++;
	if (!axp || !axp->iface || !axp->iface->ax25)
		return;

	switch (axp->iface->ax25->lapbtimertype) {
		case 2:	/* original backoff mode*/
			set_timer (&axp->t1, axp->srt * 2);
			break;
		case 1:	/* linear backoff mode */
			if ((1L << axp->retries) < axp->iface->ax25->blimit)
				set_timer (&axp->t1, dur_timer (&axp->t1) + axp->srt);
			break;
		case 0:	/* exponential backoff mode */
			if ((1L << axp->retries) < axp->iface->ax25->blimit)
				set_timer (&axp->t1, dur_timer (&axp->t1) * 2);
			break;
		default:
			break;
	}
	/* IF a maximum is set, and we surpass it, use the maximum */
	maxwait = axp->iface->ax25->maxwait;

	if (maxwait && (waittime > maxwait))
		waittime = maxwait;

	set_timer (&axp->t1, waittime);

	switch (axp->state) {
		case LAPB_SETUP:
			if (axp->n2 != 0 && axp->retries > axp->n2) {
				free_q (&axp->txq);
				axp->reason = LB_TIMEOUT;
				lapbstate (axp, LAPB_DISCONNECTED);
			} else {
				(void) sendctl (axp, LAPB_COMMAND, SABM | PF);
				start_timer (__FILE__, __LINE__, &axp->t1);
			}
			break;
		case LAPB_DISCPENDING:
			if (axp->n2 != 0 && axp->retries > axp->n2) {
				axp->reason = LB_TIMEOUT;
				lapbstate (axp, LAPB_DISCONNECTED);
			} else {
				(void) sendctl (axp, LAPB_COMMAND, DISC | PF);
				start_timer (__FILE__, __LINE__, &axp->t1);
			}
			break;
		case LAPB_CONNECTED:
		case LAPB_RECOVERY:
			if (axp->n2 != 0 && axp->retries > axp->n2) {
				/* Give up */
				(void) sendctl (axp, LAPB_RESPONSE, DM | PF);
				free_q (&axp->txq);
				axp->reason = LB_TIMEOUT;
				lapbstate (axp, LAPB_DISCONNECTED);
			} else {
				/* Transmit poll */
				tx_enq (axp);
				lapbstate (axp, LAPB_RECOVERY);
			}
			break;
		default:
			break;
	}
}

/* Send a poll (S-frame command with the poll bit set) */
void
pollthem (void *p)
{
register struct ax25_cb *axp;

	axp = (struct ax25_cb *) p;
	if (axp->proto == V1)
		return;		/* Not supported in the old protocol */
	switch (axp->state) {
		case LAPB_RECOVERY:
		case LAPB_CONNECTED:
			axp->retries = 0;
			tx_enq (axp);
			lapbstate (axp, LAPB_RECOVERY);
			break;
		default:
			break;
	}
}

/* Called whenever timer T4 (link rudundancy timer) expires */
void
redundant (void *p)
{
register struct ax25_cb *axp;

	axp = (struct ax25_cb *) p;
	switch (axp->state) {
		case LAPB_CONNECTED:
		case LAPB_RECOVERY:
			axp->retries = 0;
			(void) sendctl (axp, LAPB_COMMAND, DISC | PF);
			start_timer (__FILE__, __LINE__, &axp->t1);
			free_q (&axp->txq);
			lapbstate (axp, LAPB_DISCPENDING);
			break;
		default:
			break;
	}
}

/* Transmit query */
static void
tx_enq (register struct ax25_cb *axp)
{
char ctl;
struct mbuf *bp;

	/* I believe that retransmitting the oldest unacked
	 * I-frame tends to give better performance than polling,
	 * as long as the frame isn't too "large", because
	 * chances are that the I frame got lost anyway.
	 * This is an option in LAPB, but not in the official AX.25.
	 */
	if (axp->txq != NULLBUF && axp->pthresh != 65535 && (len_p (axp->txq) < axp->pthresh || axp->proto == V1)) {
		/* Retransmit oldest unacked I-frame */
		(void) dup_p (&bp, axp->txq, 0, len_p (axp->txq));
		ctl = PF | I | (((axp->vs - axp->unack) & MMASK) << 1) | (axp->vr << 5);	/*lint !e701 !e734 */
		(void) sendframe (axp, LAPB_COMMAND, ctl, bp);
	} else {
		ctl = len_p (axp->rxq) >= axp->window ? RNR | PF : RR | PF;
		(void) sendctl (axp, LAPB_COMMAND, ctl);
	}
	axp->response = 0;
	stop_timer (&axp->t3);
	start_timer (__FILE__, __LINE__, &axp->t1);
}
