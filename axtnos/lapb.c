/*
 *  Project: ax25c - File: lapb.c
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

/* Link Access Procedures Balanced (LAPB), the upper sublayer of
 * AX.25 Level 2.
 * Copyright 1991 Phil Karn, KA9Q
 *
 * 92/02/07 WG7J
 * Modified to drop ax.25 route records in cases where routes
 * were added by the connections. Inspired by K4TQL
 */
#include "lapb.h"

/* This forces data sent by jumpstarting a mailbox connect to
 * resend the header etc. when the first UA reply is missed, and
 * a second SABM is received for the same connection
 * added by Ron Murray VK6ZJM, Murray_RJ@cc.curtin.edu.au
 */
#define SABM_HOLDOFF

static void handleit (struct ax25_cb * axp, int pid, struct mbuf * bp);
static void procdata (struct ax25_cb * axp, struct mbuf * bp);
static int ackours (struct ax25_cb * axp, int16 n);
static void clr_ex (struct ax25_cb * axp);
static void enq_resp (struct ax25_cb * axp);
static void inv_rex (struct ax25_cb * axp);
static void drop_axr (struct ax25_cb * axp);

/*If we have an AX.25 AUTO route record, drop it.*/
static void
drop_axr (struct ax25_cb *axp)
{
struct ax_route *axr;

	axr = ax_lookup (NULLCHAR, axp->remote, axp->iface, AX_NONSETUP);
	if (axr != NULLAXR && axr->type != AX_LOCAL)
		(void) ax_drop (axp->remote, axp->iface, AX_NONSETUP);
}



/* Process incoming frames */
int
lapb_input (axp, cmdrsp, bp)
struct ax25_cb *axp;		/* Link control structure */
int cmdrsp;			/* Command/response flag */
struct mbuf *bp;		/* Rest of frame, starting with ctl */
{
int control;
int class;		/* General class (I/S/U) of frame */
int16 type;		/* Specific type (I/RR/RNR/etc) of frame */
char pf;		/* extracted poll/final bit */
char poll = 0;
char final = 0;
int16 nr;		/* ACK number of incoming frame */
int16 ns = 0;		/* Seq number of incoming frame */
int16 tmp;
int recovery = 0;

	if (bp == NULLBUF || axp == NULLAX25 || axp->iface == NULLIF) {
		free_p (bp);
		return -1;
	}
	/* Extract the various parts of the control field for easy use */
	if ((control = PULLCHAR (&bp)) == -1) {		/*lint !e506 */
		free_p (bp);	/* Probably not necessary */
		return -1;
	}

	/* update the counters for messages in */
	axp->iface->axcnt.msgin++;

	type = ftype (control);
	class = type & 0x3;
	pf = control & PF;
	/* Check for polls and finals */
	if (pf) {
		switch (cmdrsp) {
			case LAPB_COMMAND:
				poll = YES;
				break;
			case LAPB_RESPONSE:
				final = YES;
				break;
			default:
				break;
		}
	}
	/* Extract sequence numbers, if present */
	switch (class) {
		case I:
		case I + 2:
			ns = (control >> 1) & MMASK;	/*lint !e702 !e616 */
		case S:	/* Note fall-thru */
		default:
			nr = (control >> 5) & MMASK;	/*lint !e702 */
			break;
	}
	/* This section follows the SDL diagrams by K3NA fairly closely */
	switch (axp->state) {
		case LAPB_DISCONNECTED:
			switch (type) {
				case SABM:	/* Initialize or reset link */
					/* This a new incoming connection. */
					(void) sendctl (axp, LAPB_RESPONSE, UA | pf);	/* Always accept */
					clr_ex (axp);
					axp->unack = axp->vr = axp->vs = 0;
					lapbstate (axp, LAPB_CONNECTED);	/* Resets state counters */
					axp->srt = axp->iface->ax25->irtt;
					axp->mdev = 0;
					set_timer (&axp->t1, 2 * axp->srt);
					start_timer (__FILE__, __LINE__, &axp->t3);
					start_timer (__FILE__, __LINE__, &axp->t4);
#ifdef SABM_HOLDOFF
					axp->flags.rxd_I_frame = 0;	/* nothing received yet */
#endif
					/*Jump-start the mailbox. This causes the [NET-H$] prompt to
					 *be sent Immediately, instead of after the first data packet
					 *Check for connections from known netrom calls,
					 *calls from the 'jumpstart exclude' list and check if interface
					 *is in vc mode...
					 *we don't want to start the mailbox in these cases,
					 *since these could be attempts to do level 3 stuff.
					 *12/09/91 WG7J
					 */
					break;
				case DISC:	/* Always answer a DISC with a DM */
					(void) sendctl (axp, LAPB_RESPONSE, DM | pf);
					break;
				case DM:	/* Ignore to avoid infinite loops */
					break;
				default:	/* All others get DM */
					if (poll)
						(void) sendctl (axp, LAPB_RESPONSE, DM | pf);
					break;
			}
			if (axp->state == LAPB_DISCONNECTED) {	/* we can close connection */
#if 0
				stop_timer (&axp->t1);	/* waste all the timers */
				stop_timer (&axp->t3);
				stop_timer (&axp->t4);
				free_q (&axp->txq);	/* lose transmit queue */
#endif
				drop_axr (axp);	/* drop ax25 route */
				del_ax25 (axp);	/* clean out the trash */
				free_p (bp);
				return 0;
			}
			break;
		case LAPB_SETUP:
			switch (type) {
				case SABM:	/* Simultaneous open */
					(void) sendctl (axp, LAPB_RESPONSE, UA | pf);
					break;
				case DISC:
					(void) sendctl (axp, LAPB_RESPONSE, DM | pf);
					drop_axr (axp);	/* drop ax25 route */
					free_q (&axp->txq);
					stop_timer (&axp->t1);
					axp->reason = LB_DM;
					lapbstate (axp, LAPB_DISCONNECTED);
					free_p (bp);
					return 0;

				case UA:	/* Connection accepted */
					/* Note: xmit queue not cleared */
					stop_timer (&axp->t1);
					start_timer (__FILE__, __LINE__, &axp->t3);
					axp->unack = axp->vr = axp->vs = 0;
					lapbstate (axp, LAPB_CONNECTED);
					start_timer (__FILE__, __LINE__, &axp->t4);
					break;
				case DM:	/* Connection refused */
					free_q (&axp->txq);
					stop_timer (&axp->t1);
					axp->reason = LB_DM;
					lapbstate (axp, LAPB_DISCONNECTED);
					free_p (bp);
					return 0;

				default:	/* Respond with DM only to command polls */
					if (poll)
						(void) sendctl (axp, LAPB_RESPONSE, DM | pf);
					break;
			}
			break;
		case LAPB_DISCPENDING:
			switch (type) {
				case SABM:
					(void) sendctl (axp, LAPB_RESPONSE, DM | pf);
					drop_axr (axp);	/* drop ax25 route */
					break;
				case DISC:
					(void) sendctl (axp, LAPB_RESPONSE, UA | pf);
					drop_axr (axp);	/* drop ax25 route */
					break;
				case UA:
				case DM:
					stop_timer (&axp->t1);
					lapbstate (axp, LAPB_DISCONNECTED);
					free_p (bp);
					return 0;

				default:	/* Respond with DM only to command polls */
					if (poll)
						(void) sendctl (axp, LAPB_RESPONSE, DM | pf);
					drop_axr (axp);	/* drop ax25 route */
					break;
			}
			break;
		case LAPB_RECOVERY:	/* folded these two cases together */
			recovery = 1;	/* fall through */
		case LAPB_CONNECTED:
			switch (type) {
				case SABM:
					(void) sendctl (axp, LAPB_RESPONSE, UA | pf);
#ifdef SABM_HOLDOFF
					if (axp->flags.rxd_I_frame) {
						/* only reset if we've had a */
						/* valid I-frame. Otherwise he */
						/* may just not have got our UA */
#endif
						clr_ex (axp);
						if (!recovery)
							free_q (&axp->txq);
						stop_timer (&axp->t1);
						start_timer (__FILE__, __LINE__, &axp->t3);
						axp->unack = axp->vr = axp->vs = 0;
						lapbstate (axp, LAPB_CONNECTED);	/* Purge queues */
						if (recovery && !run_timer (&axp->t4))
							start_timer (__FILE__, __LINE__, &axp->t4);
#ifdef SABM_HOLDOFF
					}
#endif
					break;
				case DISC:
					free_q (&axp->txq);
					(void) sendctl (axp, LAPB_RESPONSE, UA | pf);
					stop_timer (&axp->t1);
					stop_timer (&axp->t3);
					axp->reason = LB_NORMAL;
					lapbstate (axp, LAPB_DISCONNECTED);
					free_p (bp);
					return 0;

				case DM:
					axp->reason = LB_DM;
					lapbstate (axp, LAPB_DISCONNECTED);
					free_p (bp);
					return 0;

				case UA:
					est_link (axp);
					lapbstate (axp, LAPB_SETUP);	/* Re-establish */
					break;
				case FRMR:
					/* update the counters for FRMR in */
					if (axp && axp->iface)
						axp->iface->axcnt.frmerr++;

					est_link (axp);
					lapbstate (axp, LAPB_SETUP);	/* Re-establish link */
					break;
				case RR:
				case RNR:
					axp->flags.remotebusy = (type == RNR) ? YES : NO;

					if (type == RNR)
						/* update the counters for RNR in */
						axp->iface->axcnt.rnrin++;

					if (recovery && (axp->proto == V1 || final)) {
						stop_timer (&axp->t1);
						(void) ackours (axp, nr);
						if (axp->unack != 0) {
							if (control != RNR || axp->pthresh != 65535)
								inv_rex (axp);
							else {
								stop_timer (&axp->t1);
								start_timer (__FILE__, __LINE__, &axp->t3);
							}
						} else {
							start_timer (__FILE__, __LINE__, &axp->t3);
							lapbstate (axp, LAPB_CONNECTED);
							if (!run_timer (&axp->t4))
								start_timer (__FILE__, __LINE__, &axp->t4);
						}
					} else {
						if (poll)
							enq_resp (axp);
						(void) ackours (axp, nr);
						/* Keep timer running even if all frames
						 * were acked, since we must see a Final
						 */
#if 1
						if ((type == RNR) && axp->pthresh == 65535) {
							stop_timer (&axp->t1);
							start_timer (__FILE__, __LINE__, &axp->t3);
						} else if (recovery) {
#endif
							if (!run_timer (&axp->t1))
								start_timer (__FILE__, __LINE__, &axp->t1);
						}
					}
					break;
				case REJ:
					axp->flags.remotebusy = NO;

					/* update the counters for rej in */
					axp->iface->axcnt.rejin++;

					if (recovery) {
						/* Don't insist on a Final response from the old proto */
						if (axp->proto == V1 || final) {
							stop_timer (&axp->t1);
							(void) ackours (axp, nr);
							if (axp->unack != 0) {
								inv_rex (axp);
							} else {
								start_timer (__FILE__, __LINE__, &axp->t3);
								lapbstate (axp, LAPB_CONNECTED);
								if (!run_timer (&axp->t4))
									start_timer (__FILE__, __LINE__, &axp->t4);
							}
						} else {
							if (poll)
								enq_resp (axp);
							(void) ackours (axp, nr);
							if (axp->unack != 0) {
								/* This is certain to trigger output */
								inv_rex (axp);
							}
							/* A REJ that acks everything but doesn't
							 * have the F bit set can cause a deadlock.
							 * So make sure the timer is running.
							 */
							if (!run_timer (&axp->t1))
								start_timer (__FILE__, __LINE__, &axp->t1);
						}
					} else {
						if (poll)
							enq_resp (axp);
						(void) ackours (axp, nr);
						stop_timer (&axp->t1);
						start_timer (__FILE__, __LINE__, &axp->t3);
						/* This may or may not actually invoke transmission,
						 * depending on whether this REJ was caused by
						 * our losing his prior ACK.
						 */
						inv_rex (axp);
					}
					break;
				case I:
					(void) ackours (axp, nr);	/** == -1) */

					/* update the counters for data in */
					axp->iface->axcnt.datain++;
#ifdef SABM_HOLDOFF
					axp->flags.rxd_I_frame = 1;	/* we got something */
#endif
					if (recovery) {
						/* Make sure timer is running, since an I frame
						 * cannot satisfy a poll
						 */
						if (!run_timer (&axp->t1))
							start_timer (__FILE__, __LINE__, &axp->t1);
					} else
						start_timer (__FILE__, __LINE__, &axp->t4);
					if (len_p (axp->rxq) >= axp->window) {
						/* Too bad he didn't listen to us; he'll
						 * have to resend the frame later. This
						 * drastic action is necessary to avoid
						 * deadlock.
						 */
						if (recovery || poll)	{
							(void) sendctl (axp, LAPB_RESPONSE, RNR | pf);

							/* update the counters for rnr out */
							axp->iface->axcnt.rnrout++;
						}
						free_p (bp);
						bp = NULLBUF;
						break;
					}
					/* Reject or ignore I-frames with receive sequence number errors */
					if ((char) ns != axp->vr) {
						if (axp->proto == V1 || !axp->flags.rejsent) {
							axp->flags.rejsent = YES;
							(void) sendctl (axp, LAPB_RESPONSE, REJ | pf);

							/* update the counters for rej out */
							axp->iface->axcnt.rejout++;
						} else if (poll)
							enq_resp (axp);
						axp->response = 0;
						break;
					}
					axp->flags.rejsent = NO;
					axp->vr = (axp->vr + 1) & MMASK;
					tmp = len_p (axp->rxq) >= axp->window ? RNR : RR;
					if (poll)	{
						(void) sendctl (axp, LAPB_RESPONSE, tmp | PF);

						if (tmp == RNR)
							/* update the counters for rnr out */
							axp->iface->axcnt.rnrout++;
					} else
						axp->response = (char) tmp;

					procdata (axp, bp);
					bp = NULLBUF;
					break;
				default:	/* All others ignored */
					break;
			}
			break;
		default:
			break;
	}
	free_p (bp);		/* In case anything's left */

	/* See if we can send some data, perhaps piggybacking an ack.
	 * If successful, lapb_output will clear axp->response.
	 */
	(void) lapb_output (axp);

	if (axp && axp->response != 0) {
		(void) sendctl (axp, LAPB_RESPONSE, axp->response);

		if (axp->response == RNR)
			/* update the counters for rnr out */
			axp->iface->axcnt.rnrout++;

		axp->response = 0;
	}
	return 0;
}



/* Handle incoming acknowledgements for frames we've sent.
 * Free frames being acknowledged.
 * Return -1 to cause a frame reject if number is bad, 0 otherwise
 */
static int
ackours (struct ax25_cb *axp, int16 n)
{
struct mbuf *bp;
int acked = 0;		/* Count of frames acked by this ACK */
int16 oldest;		/* Seq number of oldest unacked I-frame */
int32 rtt, abserr;
int32 waittime, maxwait;

	/* Free up acknowledged frames by purging frames from the I-frame
	 * transmit queue. Start at the remote end's last reported V(r)
	 * and keep going until we reach the new sequence number.
	 * If we try to free a null pointer,
	 * then we have a frame reject condition.
	 */
	oldest = (axp->vs - axp->unack) & MMASK;
	while (axp->unack != 0 && oldest != n) {
		if ((bp = dequeue (&axp->txq)) == NULLBUF) {
			/* Acking unsent frame */
			return -1;
		}
		free_p (bp);
		axp->unack--;
		acked++;
		if (axp->flags.rtt_run && axp->rtt_seq == oldest) {
			/* A frame being timed has been acked */
			axp->flags.rtt_run = 0;
			/* Update only if frame wasn't retransmitted */
			if (!axp->flags.retrans) {
				rtt = msclock () - axp->rtt_time;
				abserr = (rtt > axp->srt) ? rtt - axp->srt :
					axp->srt - rtt;

				/* Run SRT and mdev integrators */
				axp->srt = ((axp->srt * 7) + rtt + 4) >> 3;		/*lint !e704 */
				axp->mdev = ((axp->mdev * 3) + abserr + 2) >> 2;	/*lint !e704 */
				/* Update timeout */
				waittime = 4 * axp->mdev + axp->srt;
				maxwait = axp->iface->ax25->maxwait;

				if (maxwait && (waittime > maxwait))
					waittime = maxwait;

				set_timer (&axp->t1, waittime);
			}
		}
		axp->flags.retrans = 0;
		axp->retries = 0;
		oldest = (oldest + 1) & MMASK;
	}
	if (axp->unack == 0) {
		/* All frames acked, stop timeout */
		stop_timer (&axp->t1);
		start_timer (__FILE__, __LINE__, &axp->t3);
	} else if (acked != 0) 		/* Partial ACK; restart timer */
		start_timer (__FILE__, __LINE__, &axp->t1);

	if (acked != 0) {
		/* If user has set a transmit upcall, indicate how many frames
		 * may be queued
		 */
		if (axp->t_upcall != NULLVFP ((struct ax25_cb *, int)))
			(*axp->t_upcall) (axp, axp->paclen * (axp->maxframe - axp->unack));
	}
	return 0;
}



/* Establish data link */
void
est_link (struct ax25_cb *axp)
{
	clr_ex (axp);
	axp->retries = 0;
	(void) sendctl (axp, LAPB_COMMAND, SABM | PF);
	stop_timer (&axp->t3);
	start_timer (__FILE__, __LINE__, &axp->t1);
}



/* Clear exception conditions */
static void
clr_ex (struct ax25_cb *axp)
{
	axp->flags.remotebusy = NO;
	axp->flags.rejsent = NO;
	axp->response = 0;
	stop_timer (&axp->t3);
}



/* Enquiry response */
static void
enq_resp (struct ax25_cb *axp)
{
char ctl;

	ctl = len_p (axp->rxq) >= axp->window ? RNR | PF : RR | PF;
	(void) sendctl (axp, LAPB_RESPONSE, ctl);

	if (ctl == (RNR | PF))
		/* update the counters for rnr out */
		axp->iface->axcnt.rnrout++;

	axp->response = 0;
	stop_timer (&axp->t3);
}



/* Invoke retransmission */
static void
inv_rex (struct ax25_cb *axp)
{
	axp->vs -= axp->unack;
	axp->vs &= MMASK;
	axp->unack = 0;
}



/* Send S or U frame to currently connected station */
int
sendctl (struct ax25_cb *axp, int cmdrsp, int cmd)
{
	if ((ftype ((char) cmd) & 0x3) == S)	/* Insert V(R) if S frame */
		cmd |= (axp->vr << 5);		/*lint !e701 */

	return sendframe (axp, cmdrsp, cmd, NULLBUF);
}



/* Start data transmission on link, if possible
 * Return number of frames sent
 */
int
lapb_output (register struct ax25_cb *axp)
{
register struct mbuf *bp;
struct mbuf *tbp;
char control;
int sent = 0;
int i;

	if (axp == NULLAX25 || axp->iface == NULLIF
	    || (axp->state != LAPB_RECOVERY && axp->state != LAPB_CONNECTED)
	    || axp->flags.remotebusy)
		return 0;

	/* Dig into the send queue for the first unsent frame */
	bp = axp->txq;
	for (i = 0; i < axp->unack; i++) {
		if (bp == NULLBUF)
			break;	/* Nothing to do */
		bp = bp->anext;
	}
	/* Start at first unsent I-frame, stop when either the
	 * number of unacknowledged frames reaches the maxframe limit,
	 * or when there are no more frames to send
	 */
	while (bp != NULLBUF && axp->unack < axp->maxframe) {
		control = (char) (I | (axp->vs++ << 1) | (axp->vr << 5));	/*lint !e701 */
		axp->vs &= MMASK;
		(void) dup_p (&tbp, bp, 0, len_p (bp));
		if (tbp == NULLBUF)
			return sent;	/* Probably out of memory */
		(void) sendframe (axp, LAPB_COMMAND, control, tbp);
		start_timer (__FILE__, __LINE__, &axp->t4);
		axp->unack++;
		/* We're implicitly acking any data he's sent, so stop any
		 * delayed ack
		 */
		axp->response = 0;
		if (!run_timer (&axp->t1)) {
			stop_timer (&axp->t3);
			start_timer (__FILE__, __LINE__, &axp->t1);
		}
		sent++;

		/* update the counters for data out */
		axp->iface->axcnt.dataout++;

		bp = bp->anext;
		if (!axp->flags.rtt_run) {
			/* Start round trip timer */
			axp->rtt_seq = (control >> 1) & MMASK;		/*lint !e702 */
			axp->rtt_time = msclock ();
			axp->flags.rtt_run = 1;
		}
	}
	return sent;
}



/* Set new link state */
void
lapbstate (struct ax25_cb *axp, int s)
{
int oldstate;

	oldstate = axp->state;
	axp->state = s;
	if (s == LAPB_DISCONNECTED) {
		stop_timer (&axp->t1);
		stop_timer (&axp->t3);
		stop_timer (&axp->t4);
		free_q (&axp->txq);
		drop_axr (axp);	/* any ax25 route that hasn't been dropped yet*/
	}
	/* Don't bother the client unless the state is really changing */
	if ((oldstate != s) && (axp->s_upcall != NULLVFP ((struct ax25_cb *, int, int))))
		(*axp->s_upcall) (axp, oldstate, s);
}



/* Process a valid incoming I frame */
static void
procdata (struct ax25_cb *axp, struct mbuf *bp)
{
int pid;
int seq;

	/* Extract level 3 PID */
	if ((pid = PULLCHAR (&bp)) == -1)		/*lint !e506 */
		return;		/* No PID */

	if (axp->segremain != 0) {
		/* Reassembly in progress; continue */
		seq = PULLCHAR (&bp);			/*lint !e506 */
		if (pid == PID_SEGMENT && (seq & SEG_REM) == axp->segremain - 1) {
			/* Correct, in-order segment */
			append (&axp->rxasm, bp);

			/* update the counters for segments in */
			axp->iface->axcnt.segin++;

			if ((axp->segremain = (seq & SEG_REM)) == 0) {

				/* update the counters for segment errors */
				axp->iface->axcnt.segerr++;

				/* Done; kick it upstairs */
				bp = axp->rxasm;
				axp->rxasm = NULLBUF;
				pid = PULLCHAR (&bp);	/*lint !e506 */
				handleit (axp, pid, bp);
			}
		} else {
			/* Error! */
			free_p (axp->rxasm);
			axp->rxasm = NULLBUF;
			axp->segremain = 0;
			free_p (bp);
		}
	} else {
		/* No reassembly in progress */
		if (pid == PID_SEGMENT) {
			/* update the counters for segments in */
			axp->iface->axcnt.segin++;

			/* Start reassembly */
			seq = PULLCHAR (&bp);		/*lint !e506 */
			if (!(seq & SEG_FIRST))
				free_p (bp);	/* not first seg - error! */
			else {
				/* Put first segment on list */
				axp->segremain = seq & SEG_REM;
				axp->rxasm = bp;
			}
		} else 		/* Normal frame; send upstairs */
			handleit (axp, pid, bp);
	}
}



/* New-style frame segmenter. Returns queue of segmented fragments, or
 * original packet if small enough
 */
struct mbuf *
segmenter (bp, ssize, axp)
struct mbuf *bp;		/* Complete packet */
int16 ssize;			/* Max size of frame segments */
struct ax25_cb *axp;
{
struct mbuf *result = NULLBUF;
struct mbuf *bptmp, *bp1;
int16 len, offset;
int segments;

	/* See if packet is too small to segment. Note 1-byte grace factor
	 * so the PID will not cause segmentation of a 256-byte IP datagram.
	 */
	len = len_p (bp);
	if (len <= ssize + 1)
		return bp;	/* Too small to segment */

	ssize -= 2;		/* ssize now equal to data portion size */
	segments = 1 + (len - 1) / ssize;	/* # segments  */
	offset = 0;

	/* update the counters for seqments out */
	if (axp && axp->iface)
		axp->iface->axcnt.segout += segments;

	while (segments != 0) {
		offset += dup_p (&bptmp, bp, offset, ssize);
		if (bptmp == NULLBUF) {
			free_q (&result);
			break;
		}
		/* Make room for segmentation header */
		if ((bp1 = pushdown (bptmp, 2)) == NULLBUF) {
			free_p (bptmp);
			free_q (&result);
			break;
		}
		bp1->data[0] = PID_SEGMENT;
		bp1->data[1] = uchar(--segments);
		if (offset == ssize)
			bp1->data[1] |= SEG_FIRST;
		enqueue (&result, bp1);
	}
	free_p (bp);
	return result;
}



static void
handleit (struct ax25_cb *axp, int pid, struct mbuf *bp)
{
#if 0
struct axlink *ipp;
	for (ipp = Axlink; ipp->funct != NULL; ipp++) {
		if (ipp->pid == pid)
			break;
	}
	if (ipp->funct != NULL)
		(*ipp->funct) (axp->iface, axp, NULLCHAR, NULLCHAR, bp, 0);
	else
		free_p (bp);
#endif
}
