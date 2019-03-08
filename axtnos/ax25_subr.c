/*
 *  Project: ax25c - File: ax25_subr.c
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

/* low level AX25 routines:
 * callsign conversion
 * control block management
 *
 * Copyright 1991 Phil Karn, KA9Q
 */
/* Mods by G1EMM */
#include "global.h"
#include "lapb.h"
#include "ctype.h"

struct ax25_cb *Ax25_cb;

/* Default AX.25 parameters */
int32 T3init = 0;		/* No keep-alive polling */
int32 T4init = 300;		/* 5 Minutes of no I frame tx or rx => redundant link */
int16 Maxframe = 1;		/* Stop and wait */
int16 N2 = 10;			/* 10 retries */
int16 Axwindow = 2048;		/* 2K incoming text before RNR'ing */
int16 Paclen = 256;		/* 256-byte I fields */
int16 Pthresh = 128;		/* Send polls for packets larger than this */
int32 Axirtt = 5000;		/* Initial round trip estimate, ms */
int16 Axversion = V2;		/* Protocol version */
int32 Blimit = 30;		/* Retransmission backoff limit */

/* Look up entry in connection table
 * Check BOTH the source AND destination address
 * Added 11/15/91, WG7J/PA3DIS
 */
struct ax25_cb *
find_ax25 (local, theremote, iface)
const char *local;
const char *theremote;
struct iface *iface;
{
#if 0
register struct ax25_cb *axp;
struct ax25_cb *axlast = NULLAX25;

	/* Search list */
	for (axp = Ax25_cb; axp != NULLAX25; axlast = axp, axp = axp->next) {
		if (addreq (axp->remote, theremote) && addreq (axp->local, local) \
		    &&axp->iface == iface) {
			if (axlast != NULLAX25) {
				/* Move entry to top of list to speed
				 * future searches
				 */
				axlast->next = axp->next;
				axp->next = Ax25_cb;
				Ax25_cb = axp;
			}
			return axp;
		}
	}
#endif
	return NULLAX25;
}


/* Remove address entry from connection table */
void
del_ax25 (conn)
struct ax25_cb *conn;
{
#if 0
register struct ax25_cb *axp;
struct ax25_cb *axlast = NULLAX25;

	for (axp = Ax25_cb; axp != NULLAX25; axlast = axp, axp = axp->next) {
		if (axp == conn)
			break;
	}

	if (axp == NULLAX25)
		return;		/* Not found */

	/* Remove from list */
	if (axlast != NULLAX25)
		axlast->next = axp->next;
	else
		Ax25_cb = axp->next;

	/* Timers should already be stopped, but just in case... */
	stop_timer (&axp->t1);
	stop_timer (&axp->t3);
	stop_timer (&axp->t4);

	axp->r_upcall = NULLVFP ((struct ax25_cb *, int));
	axp->s_upcall = NULLVFP ((struct ax25_cb *, int, int));
	axp->t_upcall = NULLVFP ((struct ax25_cb *, int));

	/* Free allocated resources */
	free_q (&axp->txq);
	free_q (&axp->rxasm);
	free_q (&axp->rxq);
	if (axp->route)
		free (axp->route);
	free ((char *) axp);
#endif
}


/* Create an ax25 control block. Allocate a new structure, if necessary,
 * and fill it with all the defaults.
 *
 * This takes BOTH source and destination address.
 * 11/15/91, WG7J/PA3DIS
 *
 * Modified to take the Irtt from the interface - 11/22/93 - WG7J
 */
struct ax25_cb *
cr_ax25 (local, theremote, iface)
char *local;
char *theremote;
struct iface *iface;
{
#if 0
register struct ax25_cb *axp;
struct ifax25 *ifax;
struct ax_route *axr;

	if ((theremote == NULLCHAR) || (local == NULLCHAR))
		return NULLAX25;

	if ((axp = find_ax25 (local, theremote, iface)) != NULLAX25)
		return axp;

	/* Create an entry and insert it at the head of the chain */
	axp = (struct ax25_cb *) callocw (1, sizeof (struct ax25_cb));

	axp->next = Ax25_cb;
	Ax25_cb = axp;

	/*fill in 'defaults'*/
	memcpy (axp->local, local, AXALEN);
	memcpy (axp->remote, theremote, AXALEN);

	if ((axr = ax_lookup (NULLCHAR, theremote, iface, AX_SETUP)) != NULLAXR)	{
		axp->route = (struct ax_route *) callocw (1, sizeof (struct ax_route));
		memcpy (axp->route, axr, sizeof (struct ax_route));
		if (axr->type == AX_SETUP)
			(void) ax_drop (theremote, iface, AX_SETUP);
	}

	axp->user = -1;
	axp->state = LAPB_DISCONNECTED;

	if (iface && (ifax = iface->ax25) != NULL) {
		axp->maxframe = ifax->maxframe;
		axp->window = (int16) ifax->window;
		axp->iface = iface;
#if 0
		axp->paclen = iface->paclen;
#endif
		axp->paclen = (int16) ifax->paclen;
		axp->proto = (char) ifax->version;	/* Default, can be changed by other end */
		axp->pthresh = (int16) ifax->pthresh;
		axp->n2 = (unsigned) ifax->n2;
		axp->srt = ifax->irtt;
		set_timer (&axp->t1, 2 * axp->srt);
		set_timer (&axp->t3, ifax->t3);
		set_timer (&axp->t4, ifax->t4 * 1000L);
	}
	axp->t1.func = recover;
	axp->t1.arg = axp;

	axp->t3.func = pollthem;
	axp->t3.arg = axp;

	axp->t4.func = redundant;
	axp->t4.arg = axp;

	/* Always to a receive and state upcall as default */
	/* Also bung in a default transmit upcall - in case */
	axp->r_upcall = s_arcall;
	axp->s_upcall = s_ascall;
	axp->t_upcall = s_atcall;

	return axp;
#endif
	return NULL;
}


/*
 * setcall - convert callsign plus substation ID of the form
 * "KA9Q-0" to AX.25 (shifted) address format
 *   Address extension bit is left clear
 *   Return -1 on error, 0 if OK
 */
int
setcall (out, call)
char *out;
const char *call;
{
int csize;
unsigned ssid;
register int i;
register char *dp;
char c;

	if (out == NULLCHAR || call == NULLCHAR || *call == '\0')
		return -1;

	/* Find dash, if any, separating callsign from ssid
	 * Then compute length of callsign field and make sure
	 * it isn't excessive
	 */
	dp = strchr (call, '-');
	if (dp == NULLCHAR)
		csize = (int) strlen (call);
	else
		csize = (const char *) dp - call;
	if (csize > ALEN)
		return -1;
	/* Now find and convert ssid, if any */
	if (dp != NULLCHAR) {
		dp++;		/* skip dash */
		ssid = (unsigned) atoi (dp);
		if (ssid > 15)
			return -1;
	} else
		ssid = 0;
	/* Copy upper-case callsign, left shifted one bit */
	for (i = 0; i < csize; i++) {
		c = *call++;
		if (islower (c))
			c = (char) toupper (c);
		*out++ = (char) (c << 1);	/*lint !e701 */
	}
	/* Pad with shifted spaces if necessary */
	for (; i < ALEN; i++)
		*out++ = ' ' << 1;

	/* Insert substation ID field and set reserved bits */
	*out = (char) (0x60 | (ssid << 1));
	return 0;
}


int
addreq (a, b)
register const char *a, *b;
{
	if (memcmp (a, b, ALEN) != 0 || ((a[ALEN] ^ b[ALEN]) & SSID) != 0)
		return 0;
	else
		return 1;
}


/* Convert encoded AX.25 address to printable string */
char *
pax25 (e, addr)
char *e;
char *addr;
{
register int i;
char c;
char *cp;
char extension;

	cp = e;
	for (i = ALEN; i != 0; i--) {
		c = (*addr++ >> 1) & 0x7f;	/*lint !e702 */
		if (c != ' ')
			*cp++ = c;
	}
	extension = '-';	/*lint !e539 */

	if ((*addr & SSID) != 0)
		sprintf (cp, "%c%d", extension, (*addr >> 1) & 0xf);	/*lint !e702 * ssid */
	else
		*cp = '\0';

	return e;
}


/* Figure out the frame type from the control field
 * This is done by masking out any sequence numbers and the
 * poll/final bit after determining the general class (I/S/U) of the frame
 */
int16
ftype (control)
register int control;
{
	if ((control & 1) == 0)	/* An I-frame is an I-frame... */
		return I;
	if (control & 2)	/* U-frames use all except P/F bit for type */
		return (int16) (uchar (control) & ~PF);
	else			/* S-frames use low order 4 bits for type */
		return (int16) (uchar (control) & 0xf);
}

