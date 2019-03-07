/*
 *  Project: ax25c - File: lapb.h
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

/** @file lapb.h */

#ifndef AXTNOS_LAPB_H_
#define AXTNOS_LAPB_H_

#include "global.h"
#include "mbuf.h"
#include "iface.h"
#include "timer.h"
#include "ax25.h"

/* Upper sub-layer (LAPB) definitions */

/* Control field templates */
#define	I	0x00	/* Information frames */
#define	S	0x01	/* Supervisory frames */
#define	RR	0x01	/* Receiver ready */
#define	RNR	0x05	/* Receiver not ready */
#define	REJ	0x09	/* Reject */
#define	U	0x03	/* Unnumbered frames */
#define	SABM	0x2f	/* Set Asynchronous Balanced Mode */
#define	DISC	0x43	/* Disconnect */
#define	DM	0x0f	/* Disconnected mode */
#define	UA	0x63	/* Unnumbered acknowledge */
#define	FRMR	0x87	/* Frame reject */
#define	UI	0x03	/* Unnumbered information */
#define	PF	0x10	/* Poll/final bit */

#define	MMASK	7	/* Mask for modulo-8 sequence numbers */

/* FRMR reason bits */
#define	W	1	/* Invalid control field */
#define	X	2	/* Unallowed I-field */
#define	Y	4	/* Too-long I-field */
#define	Z	8	/* Invalid sequence number */

/* Per-connection link control block
 * These are created and destroyed dynamically,
 * and are indexed through a hash table.
 * One exists for each logical AX.25 Level 2 connection
 */
struct ax25_cb {
	struct ax25_cb *next;		/* Linked list pointers */

	struct iface *iface;		/* Interface */

	struct mbuf *txq;		/* Transmit queue */
	struct mbuf *rxasm;		/* Receive reassembly buffer */
	struct mbuf *rxq;		/* Receive queue */

	char local[AXALEN];		/* Addresses */
	char remote[AXALEN];

	struct {
		char rejsent;		/* REJ frame has been sent */
		char remotebusy;	/* Remote sent RNR */
		char rtt_run;		/* Round trip "timer" is running */
		char retrans;		/* A retransmission has occurred */
        char clone;         /* Server-type cb, will be cloned */
        char rxd_I_frame;   /* I-frame received */
    } flags;

	char reason;			/* Reason for connection closing */
#define	LB_NORMAL	0		/* Normal close */
#define	LB_DM		1		/* Received DM from other end */
#define	LB_TIMEOUT	2		/* Excessive retries */
#define LB_UNUSED	3		/* Link is redundant - unused */

	char response;			/* Response owed to other end */
	char vs;			/* Our send state variable */
	char vr;			/* Our receive state variable */
	char unack;			/* Number of unacked frames */
	int maxframe;			/* Transmit flow control level, frames */
	int16 paclen;			/* Maximum outbound packet size, bytes */
	int16 window;			/* Local flow control limit, bytes */
	char proto;			/* Protocol version */
#define	V1	1			/* AX.25 Version 1 */
#define	V2	2			/* AX.25 Version 2 */
	int16 pthresh;			/* Poll threshold, bytes */
	unsigned retries;		/* Retry counter */
	unsigned n2;			/* Retry limit */
	int state;			/* Link state */
#define	LAPB_DISCONNECTED	1
#define LAPB_LISTEN		2
#define	LAPB_SETUP		3
#define	LAPB_DISCPENDING	4
#define	LAPB_CONNECTED		5
#define	LAPB_RECOVERY		6
	struct timer t1;		/* Retry timer */
	struct timer t3;		/* Keep-alive poll timer */
	struct timer t4;		/* Link redundancy timer */
	int32 rtt_time;			/* Stored clock values for RTT, ticks */
	int rtt_seq;			/* Sequence number being timed */
	int32 srt;			/* Smoothed round-trip time, ms */
	int32 mdev;			/* Mean rtt deviation, ms */

	void (*r_upcall) (struct ax25_cb *,int);	/* Receiver upcall */
	void (*t_upcall) (struct ax25_cb *,int);	/* Transmit upcall */
	void (*s_upcall) (struct ax25_cb *,int,int);	/* State change upcall */

	int user;			/* User pointer */
	int segremain;			/* Segmenter state */
    int jumpstarted;    /* Was this one jumpstarted ? */
#define KNOWN_LINK  1       /* incoming conn. is already known in cb list */
#define NETROM_LINK 2       /* new connection to netrom interface call */
#define ALIAS_LINK  4       /* new connection to alias call */
#define CONF_LINK   8       /* new connection to conference call */
#define IFACE_LINK  16      /* new connection to the interface call */
#define IP_LINK     32      /* new connection to the IP call (VC) */
#define NR4_LINK    64
#define TELNET_LINK 128
#define JUMPSTARTED 256     /* jumpstart was used */
#define TUTOR_LINK  512     /* new connection to tutorial call */
#define INFO_LINK   1024    /* new connection to info call */
#define NEWS_LINK   2048    /* new connection to news call */
#define RLOGIN_LINK 4096    /* sysop rlogin connection */
#define TIP_LINK    8192
#define TTY_LINK    16384   /* new connection to the ttylink call */
#define NODE_LINK   32768   /* new connection to the node call */
	struct ax_route *route;	/* route connection was created with */
};
#define	NULLAX25	((struct ax25_cb *)0)
extern struct ax25_cb *Ax25_cb;
extern const char *Ax25states[],*Axreasons[];
extern int32 Axirtt,T3init,T4init,Blimit;
extern int16 N2,Maxframe,Paclen,Pthresh,Axwindow,Axversion;

/* In lapb.c: */
void est_link (struct ax25_cb *axp);
void lapbstate (struct ax25_cb *axp,int s);
int lapb_input (struct ax25_cb *axp,int cmdrsp,struct mbuf *bp);
int lapb_output (struct ax25_cb *axp);
struct mbuf *segmenter (struct mbuf *bp,int16 ssize, struct ax25_cb *axp);
int sendctl (struct ax25_cb *axp,int cmdrsp,int cmd);

/* In lapbtimer.c: */
void pollthem (void *p);
void recover (void *p);
void redundant (void *p);

/* In ax25subr.c: */
int16 ftype (int control);

/* In nr3.c: */
void nr_derate ( struct ax25_cb *axp);

#endif /* AXTNOS_LAPB_H_ */
