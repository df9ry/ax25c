/*
 *  Project: ax25c - File: ax25.c
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

/*  Low level AX.25 code:
 *  incoming frame processing (including digipeating)
 *  IP encapsulation
 *  digipeater routing
 *
 *  Copyright 1991 Phil Karn, KA9Q / 1991 Kevin Hil, G1EMM
 */
/* Mods by PA0GRI */

#include "global.h"
//#include "commands.h"
#include "mbuf.h"
#include "iface.h"
//#include "arp.h"
//#include "netrom.h"
//#include "trace.h"
//#include "pktdrvr.h"

//static int axsend (struct iface * iface, const char *dest, char *source,
//			   int cmdrsp, int ctl, struct mbuf * data);

//static void incCounter (struct iface *iface);

extern int AXSmartRoute;
/* List of AX.25 multicast addresses in network format (shifted ascii).
 * Only the first entry is used for transmissions, but any incoming
 * packet with any one of these destination addresses is recognized
 * as a multicast
 */
/* NOTE: IF you CHANGE the order of these, also change the codes in ax25.h !!!
 * mailfor.c, nr3, and ax25cmd.c depend on this to get broadcast addresses !!!
 * 920306 - WG7J
 */
char Ax25multi[][AXALEN] =
{
     {'Q' << 1, 'S' << 1, 'T' << 1, ' ' << 1, ' ' << 1, ' ' << 1, '0' << 1},	/* QST */
     {'N' << 1, 'O' << 1, 'D' << 1, 'E' << 1, 'S' << 1, ' ' << 1, '0' << 1},	/* NODES */
     {'M' << 1, 'A' << 1, 'I' << 1, 'L' << 1, ' ' << 1, ' ' << 1, '0' << 1},	/* MAIL */
     {'I' << 1, 'D' << 1, ' ' << 1, ' ' << 1, ' ' << 1, ' ' << 1, '0' << 1},	/* ID */
     {'O' << 1, 'P' << 1, 'E' << 1, 'N' << 1, ' ' << 1, ' ' << 1, '0' << 1},	/* OPEN */
     {'C' << 1, 'Q' << 1, ' ' << 1, ' ' << 1, ' ' << 1, ' ' << 1, '0' << 1},	/* CQ */
     {'B' << 1, 'E' << 1, 'A' << 1, 'C' << 1, 'O' << 1, 'N' << 1, '0' << 1},	/* BEACON */
     {'R' << 1, 'M' << 1, 'N' << 1, 'C' << 1, ' ' << 1, ' ' << 1, '0' << 1},	/* RMNC */
     {'A' << 1, 'L' << 1, 'L' << 1, ' ' << 1, ' ' << 1, ' ' << 1, '0' << 1},	/* ALL */
	{'\0'},
};

char NOCALL[] = {'N' << 1, 'O' << 1, 'C' << 1, 'A' << 1, 'L' << 1, 'L' << 1, '0' << 1};

/*lint -restore */

char Mycall[AXALEN];
char AXuser[AXALEN];
char MyBBS[AXALEN];
char Myalias[AXALEN];		/* the NETROM alias in 'call' form */

char Tcall[AXALEN];
char Icall[AXALEN];
char Ncall[AXALEN];

struct ax_route *Ax_routes;		/* Routing table header */
struct ax_route *Ax_setups;	/* Routing table header for setting up connections */
static int Ax_setups_counter = 0;

int16 fcstab[256] =
{
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
	0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
	0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
	0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
	0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
	0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
	0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
	0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
	0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
	0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
	0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
	0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
	0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
	0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
	0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
	0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
	0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
	0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
	0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
	0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
	0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
	0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
	0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
	0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
	0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
	0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
	0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
	0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
	0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
	0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
	0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
	0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

#if 0
static void
incCounter (struct iface *iface)
{
	/* update the counters for messages out */
	if (iface)
		iface->axcnt.msgout++;
}
#endif

/* Send IP datagrams across an AX.25 link */
int
ax_send (struct mbuf *bp, struct iface *iface, uint32 gateway, int prec OPTIONAL,
	int del, int tput OPTIONAL, int rel)
{
#if 0
	char *hw_addr;
	struct ax25_cb *axp;
	struct mbuf *tbp;
	struct ax_route *axr;
	char mode = AX_DEFMODE;	/* default to interface mode */

	if (gateway == iface->broadcast)	/* This is a broadcast IP datagram */
		return (*iface->output) (iface, Ax25multi[0], iface->ipcall, PID_IP, bp);

	if ((hw_addr = res_arp (iface, ARP_AX25, gateway, bp)) == NULLCHAR)
		return 0;	/* Wait for address resolution */

	/* If there's a defined route, get it */
	axr = ax_lookup (NULLCHAR, hw_addr, iface, AX_NONSETUP);

	if (axr == NULLAXR) {
		if (iface->flags & CONNECT_MODE)
			mode = AX_VC_MODE;
		else
			mode = AX_DATMODE;
	} else {
		mode = axr->mode;
		if (mode == AX_DEFMODE) {
			if (iface->flags & CONNECT_MODE)
				mode = AX_VC_MODE;
			else
				mode = AX_DATMODE;
		}
	}

	/* UI frames are used for any one of the following three conditions:
	 * 1. The "low delay" bit is set in the type-of-service field.
	 * 2. The "reliability" TOS bit is NOT set and the interface is in
	 *    datagram mode.
	 * 3. The destination is the broadcast address (this is helpful
	 *    when broadcasting on an interface that's in connected mode).
	 */
	if (del || (!rel && (mode == AX_DATMODE)) || addreq (hw_addr, Ax25multi[0]))
		/* Use UI frame */
		return (*iface->output) (iface, hw_addr, iface->ipcall, PID_IP, bp);

	/* Reliability is needed; use I-frames in AX.25 connection */
	if ((axp = find_ax25 (iface->ipcall, hw_addr, iface)) == NULLAX25) {
		/* Open a new connection */
		axp = open_ax25 (iface, iface->ipcall, hw_addr,
		     AX_ACTIVE, Axwindow, s_arcall, s_atcall, s_ascall, -1);
		if (axp == NULLAX25) {
			free_p (bp);
			return -1;
		}
	}
	if (axp->state == LAPB_DISCONNECTED) {
		est_link (axp);
		lapbstate (axp, LAPB_SETUP);
	}
	/* Insert the PID */
	if ((tbp = pushdown (bp, 1)) == NULLBUF) {
		free_p (bp);
		return -1;
	}
	bp = tbp;
	bp->data[0] = PID_IP;
	if ((tbp = segmenter (bp, axp->paclen, axp)) == NULLBUF) {
		free_p (bp);
		return -1;
	}
	return send_ax25 (axp, tbp, -1);
#endif
	return 0;
}

/* Add header and send connectionless (UI) AX.25 packet.
 * Note that the calling order here must match enet_output
 * since ARP also uses it.
 */
int
ax_output (iface, dest, source, pid, data)
struct iface *iface;		/* Interface to use; overrides routing table */
const char *dest;		/* Destination AX.25 address (7 bytes, shifted) */
char *source;			/* Source AX.25 address (7 bytes, shifted) */
int16 pid;			/* Protocol ID */
struct mbuf *data;		/* Data field (follows PID) */
{
#if 0
struct mbuf *bp;
int retval;

	/* Prepend pid to data */
	bp = pushdown (data, 1);
	if (bp == NULLBUF) {
		free_p (data);
		return -1;
	}
	bp->data[0] = uchar(pid);

	retval = axsend (iface, dest, source, LAPB_COMMAND, UI, bp);

	/* update the counters for messages out */
	if (retval != -1)
		incCounter (iface);

	return retval;
#endif
	return 0;
}

/* Common subroutine for sendframe() and ax_output() */
#if 0
static int
axsend (iface, dest, source, cmdrsp, ctl, data)
struct iface *iface;		/* Interface to use; overrides routing table */
const char *dest;		/* Destination AX.25 address (7 bytes, shifted) */
char *source;			/* Source AX.25 address (7 bytes, shifted) */
int cmdrsp;			/* Command/response indication */
int ctl;			/* Control field */
struct mbuf *data;		/* Data field (includes PID) */
{
struct mbuf *cbp;
struct ax25 addr;
struct ax_route *axr;
char const *idest;
int rval;

	/* If the source addr is unspecified, use the interface address */
	if (source[0] == '\0')
		source = iface->hwaddr;

	/* If there's a digipeater route, get it */
	axr = ax_lookup (source, dest, iface, AX_NONSETUP);

	memcpy (addr.dest, dest, AXALEN);
	memcpy (addr.source, source, AXALEN);
	addr.cmdrsp = cmdrsp;

	if (axr != NULLAXR) {
		memcpy (addr.digis, axr->digis, (size_t) axr->ndigis * AXALEN);
		addr.ndigis = axr->ndigis;
		idest = addr.digis[0];
	} else {
		addr.ndigis = 0;
		idest = dest;
	}

	addr.nextdigi = 0;

	/* Allocate mbuf for control field, and fill in */
	if ((cbp = pushdown (data, 1)) == NULLBUF) {
		free_p (data);
		return -1;
	}
	cbp->data[0] = uchar(ctl);

	if ((data = htonax25 (&addr, cbp)) == NULLBUF) {
		free_p (cbp);	/* Also frees data */
		return -1;
	}

	/* This shouldn't be necessary because redirection has already been
	 * done at the IP router layer, but just to be safe...
	 */
	if (iface->forw != NULLIF) {
		logsrc (iface->forw, source);
		logdest (iface->forw, idest);
		rval = (*iface->forw->raw) (iface->forw, data);
	} else {
		logsrc (iface, source);
		logdest (iface, idest);
		rval = (*iface->raw) (iface, data);
	}
	return rval;
}
#endif

/* Process incoming AX.25 packets.
 * After optional tracing, the address field is examined. If it is
 * directed to us as a digipeater, repeat it.  If it is addressed to
 * us or to QST-0, kick it upstairs depending on the protocol ID.
 */
extern int32 CT4init;

void
ax_recv (struct iface *iface, struct mbuf *bp)
{
#if 0
struct mbuf *hbp;
unsigned char control;
struct ax25 hdr;
struct ax25_cb *axp = NULLAX25;
struct ax_route *axr;
struct iface *cross;
int nomark = 0;
char (*mpp)[AXALEN];
int mcast;
char *isrc, *idest;	/* "immediate" source and destination */
int To_us = 0;		/* Is this a link to us ? */
int send2axui = 0;
	/* Pull header off packet and convert to host structure */
	if (ntohax25 (&hdr, &bp) < 0) {
		/* Something wrong with the header */
		free_p (bp);
		return;
	}
	if (iface->flags & LOG_IPHEARD) {
		struct mbuf *nbp;
		int len;

		len = len_p (bp);
		if (dup_p (&nbp, bp, 0, (int16) len) == len) {
			/* find higher proto, if any */
#if 0
			(void) PULLCHAR (&nbp);		/*lint !e506 * skip control byte */
			if (PULLCHAR (&nbp) == PID_IP) {	/*lint !e506 */
				(void) atohip (&ip, &nbp);
			}
#endif
			free_p (nbp);
		}
	}

	/* If there were digis in this packet and at least one has
	 * been passed, then the last passed digi is the immediate source.
	 * Otherwise it is the original source.
	 */
	if (hdr.ndigis != 0 && hdr.nextdigi != 0)
		isrc = hdr.digis[hdr.nextdigi - 1];
	else
		isrc = hdr.source;

	/* If there are digis in this packet and not all have been passed,
	 * then the immediate destination is the next digi. Otherwise it
	 * is the final destination.
	 */
	cross = NULLIF;
	if (hdr.ndigis != 0 && hdr.nextdigi != hdr.ndigis) {
		idest = hdr.digis[hdr.nextdigi];
		if (!addreq (idest, iface->hwaddr)) {
			/* Check if digi matches callsign of any other
			 * interface for crossband digipeating.
			 */
			for (cross = Ifaces; cross != NULLIF; cross = cross->next) {
#if 0
				if (cross->type == CL_AX25 && addreq (idest, cross->hwaddr)) {
					/* Swap callsigns so that the reply
					 * can be crossband digipeated in
					 * the other direction.
					 */
					memcpy (idest, iface->hwaddr, AXALEN);
					break;
				}
#endif
			}
		}
	} else
		idest = hdr.dest;

	/* Don't log our own packets if we overhear them, as they're
	 * already logged by axsend() and by the digipeater code.
	 */
	if (!addreq (isrc, iface->hwaddr) && !addreq (isrc, iface->ipcall)) {

			logsrc (iface, isrc);
			logdest (iface, idest);
	}
	/* Examine immediate destination for a multicast address */
	mcast = 0;
	for (mpp = Ax25multi; (*mpp)[0] != '\0'; mpp++) {
		if (addreq (idest, *mpp)) {
			mcast = 1;
			break;
		}
	}


	/* Now check for any connection already in the AX.25 cb list
	 * This allows netrom user connects (with inverted ssid's) ,
	 * connections already established etc.. to pass.
	 * There should be no more digis needed!
	 * Added 11/15/91 WG7J
	 */
	if (hdr.nextdigi == hdr.ndigis)
		/* See if the source and destination address are in hash table */
		if ((axp = find_ax25 (hdr.dest, hdr.source, iface)) != NULLAX25)
			To_us = KNOWN_LINK;

	if (addreq (idest, iface->ipcall)) {
		To_us |= IP_LINK;
		if (To_us & KNOWN_LINK && axp != NULLAX25)
			axp->jumpstarted = To_us;
	}
	if (AXSmartRoute && !To_us && !mcast && !cross) {
		nomark = 1;
		for (cross = Ifaces; cross != NULLIF; cross = cross->next) {
			if (iface == cross)	/* skip same interface */
				continue;
#if 0
			if (cross->type == CL_AX25) {
				/* this one checks to see if it is an axip dstaddr */
				if (cross->rmtaddr && (addreq (hdr.dest, cross->rmtaddr) || addreq (idest, cross->rmtaddr)))
					break;

				/* this one handles a neighbor in dest if no digis or all digis done */
				if ((!hdr.ndigis || (hdr.nextdigi == hdr.ndigis)) && ((ax_lookup (NULLCHAR, hdr.dest, cross, AX_NONSETUP) != NULLAXR) || ad_lookup (cross, hdr.dest, 1)))
					break;

				/* this one handles a neighbor in the digi list as the next digi */
				if (ax_lookup (NULLCHAR, idest, cross, AX_NONSETUP) != NULLAXR || al_lookup (cross, idest, 1))
					break;
			}
#endif
		}
		nomark = (int) cross;	/* if not found here, then == 0 */
	}

	if (!To_us && !mcast && !cross) {
		/* Not a broadcast, and not addressed to us. Inhibit
		 * transmitter to avoid colliding with addressed station's
		 * response, and discard packet.
		 */
		free_p (bp);
		return;
	}
	/* At this point, packet is either addressed to us, or is
	 * a multicast.
	 */
	if (addreq (hdr.source, NOCALL)) {	/* don't even acknowledge NOCALL's */
		free_p (bp);
		return;
	}
	if (nomark || hdr.nextdigi < hdr.ndigis) {	/* Packet requests digipeating. See if we can repeat it. */
		if ((iface->flags & AX25_DIGI) && !mcast) {
			/* Yes, kick it back out. htonax25 will set the
			 * repeated bit.
			 */
			if (!nomark)
				hdr.nextdigi++;

			if (cross)	/* Crossband digipeat */
				iface = cross;
			if (iface->flags & AX25_DIGI) {
				if ((hbp = htonax25 (&hdr, bp)) != NULLBUF) {
					logsrc (iface, iface->hwaddr);
					if (iface->forw != NULLIF) {
						logdest (iface->forw, hdr.digis[hdr.nextdigi]);
						(void) (*iface->forw->raw) (iface->forw, hbp);
					} else {
						logdest (iface, hdr.digis[hdr.nextdigi]);
						(void) (*iface->raw) (iface, hbp);
					}
					bp = NULLBUF;
				}
			}
		}
		free_p (bp);	/* Dispose if not forwarded */
		return;
	}
	/* If we reach this point, then the packet has passed all digis,
	 * and is either addressed to us or is a multicast
	 */
	if (bp == NULLBUF)
		return;		/* Nothing left */

	/* If there's no locally-set entry in the routing table and
	 * this packet has digipeaters, create or update it. Leave
	 * local routes alone.
	 */
	if (((axr = ax_lookup (NULLCHAR, hdr.source, iface, AX_NONSETUP)) == NULLAXR || axr->type == AX_AUTO) && hdr.ndigis > 0) {
		char digis[MAXDIGIS][AXALEN];
		int i, j;

		/* Construct reverse digipeater path */
		for (i = hdr.ndigis - 1, j = 0; i >= 0; i--, j++) {
			memcpy (digis[j], hdr.digis[i], AXALEN);
			digis[j][ALEN] &= (char)(~(E | REPEATED));
		}
		(void) ax_add (hdr.source, AX_AUTO, digis, hdr.ndigis, iface);
	}
	/* Sneak a peek at the control field. This kludge is necessary because
	 * AX.25 lacks a proper protocol ID field between the address and LAPB
	 * sublayers; a control value of UI indicates that LAPB is to be
	 * bypassed.
	 */
	control = (*bp->data & ~PF);

	if (control == UI) {
		int pid;
		struct axlink *ipp;

		(void) PULLCHAR (&bp);			/*lint !e506 */
		if ((pid = PULLCHAR (&bp)) == -1)	/*lint !e506 */
			return;	/* No PID */
		/* Find network level protocol and hand it off */
		for (ipp = Axlink; ipp->funct != NULL; ipp++) {
			if (ipp->pid == pid)
				break;
		}
		if (ipp->funct != NULL)
			(*ipp->funct) (iface, NULLAX25, hdr.source, hdr.dest, bp, (mcast + send2axui));
		else
			free_p (bp);
		return;
	}
	/* Everything from here down is connected-mode LAPB, so ignore
	 * multicasts
	 */
	if ((mcast + send2axui) != 0) {
		free_p (bp);
		return;
	}
	/* At this point, if we already have a connection on its way,
	 * we already have found the control block !
	 * 11/15/91 WG7J/PA3DIS
	 */
	if (!(To_us & KNOWN_LINK)) {
		/* This is a new connection to either the interface call,
		 * or to the BBS call or the BBS alias or the INFO call
		 * or the NEWS call or the TUTOR call or the IPCALL,
		 * or to the netrom-interface call or system alias.
		 * Create a new ax25 entry for this guy,
		 * insert into hash table keyed on his address,
		 * and initialize table entries
		 */
		if ((axp = cr_ax25 (hdr.dest, hdr.source, iface)) == NULLAX25) {
			free_p (bp);
			return;
		}
#if 1
		/* add route to newly created connection block */
		axp->route = (struct ax_route *) callocw (1, sizeof (struct ax_route));
		hdr.source[AXALEN - 1] &= SSID;
		memcpy (axp->route->target, hdr.source, AXALEN);
		axp->route->ndigis = hdr.ndigis;
		axp->route->iface = iface;
		axp->route->mode = AX_DEFMODE;	/* set mode to default */
		{
		int i, j;

			for (i = hdr.ndigis - 1, j = 0; i >= 0; i--, j++) {
				memcpy (axp->route->digis[j], hdr.digis[i], AXALEN);
				axp->route->digis[j][ALEN] &= (char)(~(E | REPEATED));
			}
		}
#endif
		axp->jumpstarted = To_us;
		if (hdr.cmdrsp == LAPB_UNKNOWN)
			axp->proto = V1;	/* Old protocol in use */
	}
	(void) lapb_input (axp, hdr.cmdrsp, bp);
#endif
}



/* General purpose AX.25 frame output */
int
sendframe (struct ax25_cb *axp, int cmdrsp, int ctl, struct mbuf *data)
{
#if 0
int retval;

	retval = axsend (axp->iface, axp->remote, axp->local, cmdrsp, ctl, data);

	/* update the counters for messages out */
	if (retval != -1)
		incCounter(axp->iface);

	return retval;
#endif
	return 0;
}

/* Find a route for an AX.25 address
 * Code to remove SSID field C/R- and E-bits in ax_lookup(), ax_add() and
 * ax_drop() added by vk6zjm 4/5/92. This eliminates duplicate AX25 routes.
 * 1992-05-28 - Added interface -- sm6rpz
 */

struct ax_route *
ax_lookup (const char *source, const char *target, struct iface *ifp, int type)
{
#if 0
register struct ax_route *axr;
struct ax_route **routetbl = &Ax_routes;
struct ax_route *axlast = NULLAXR;
char xtarget[AXALEN];
char xsource[AXALEN];
struct ax25_cb *axp;
int onlysetup = 0;

	if (type == AX_SETUP)
		routetbl = &Ax_setups;

	/* Remove C/R and E bits in local copy only */
	memcpy (xtarget, target, AXALEN);
	xtarget[AXALEN - 1] &= SSID;

	if (source == (char *)-1)	{
		onlysetup = 1;
		source = NULLCHAR;
	}

	/* if source address is present, then we set it up too, then
	   see if this is part of an active connection. If so, we return
	   the digis used to set up the connection, instead of the current
	   ones */
	if (source)	{
		/* Remove C/R and E bits in local copy only */
		memcpy (xsource, source, AXALEN);
		xsource[AXALEN - 1] &= SSID;
		if ((axp = find_ax25 (source, target, ifp)) != NULLAX25)
			return axp->route;
	}

	do	{
		for (axr = *routetbl; axr != NULLAXR; axlast = axr, axr = axr->next) {
			if (addreq (axr->target, xtarget) && axr->iface == ifp) {
				if (axr != *routetbl && axlast != NULLAXR) {
					/* Move entry to top of list to speed
					 * future searches
					 */
					axlast->next = axr->next;
					axr->next = *routetbl;
					*routetbl = axr;

				}
				return axr;
			}
		}
		if (routetbl == &Ax_setups && !onlysetup)
			routetbl = &Ax_routes;
		else
			routetbl = (struct ax_route **)0;
	} while (routetbl != (struct ax_route **)0);
	return axr;
#endif
	return NULL;
}



/* Add an entry to the AX.25 routing table */
struct ax_route *
ax_add (char *target, int type, char digis[][AXALEN], int ndigis, struct iface *ifp)
{
#if 0
register struct ax_route *axr;
struct ax_route **routetbl = &Ax_routes;
char xtarget[AXALEN];

	if (ndigis < 0 || ndigis > MAXDIGIS || addreq (target, ifp->hwaddr) || addreq (target, ifp->ipcall))
		return NULLAXR;

	if (type == AX_SETUP)
		routetbl = &Ax_setups;

	/* Remove C/R and E bits in local copy only */
	memcpy (xtarget, target, AXALEN);
	xtarget[AXALEN - 1] &= SSID;

	if ((axr = ax_lookup ((char *)-1, xtarget, ifp, type)) == NULLAXR) {
		axr = (struct ax_route *) callocw (1, sizeof (struct ax_route));

		axr->next = *routetbl;
		*routetbl = axr;
		memcpy (axr->target, xtarget, AXALEN);
		axr->ndigis = ndigis;
		axr->iface = ifp;
		axr->mode = AX_DEFMODE;	/* set mode to default */
		if (type == AX_SETUP)
			Ax_setups_counter = 1100;
	} else
		/* don't allow an AUTO to override a LOCAL entry */
		if ((axr->type == AX_LOCAL) && (type == AX_AUTO))
			return axr;

	axr->type = (char) type;
	if (axr->ndigis != ndigis)
		axr->ndigis = ndigis;

	memcpy (axr->digis, digis[0], (size_t) ndigis * AXALEN);
	return axr;
#endif
	return NULL;
}

int
ax_drop (char *target, struct iface *ifp, int type)
{
#if 0
struct ax_route **routetbl = &Ax_routes;
register struct ax_route *axr;
struct ax_route *axlast = NULLAXR;
char xtarget[AXALEN];

	if (type == AX_SETUP)
		routetbl = &Ax_setups;

	/* Remove C/R and E bits in local copy only */
	memcpy (xtarget, target, AXALEN);
	xtarget[AXALEN - 1] &= SSID;

	for (axr = *routetbl; axr != NULLAXR; axlast = axr, axr = axr->next)
		if (addreq (axr->target, xtarget) && axr->iface == ifp)
			break;
	if (axr == NULLAXR)
		return -1;	/* Not in table! */
	if (axlast != NULLAXR)
		axlast->next = axr->next;
	else
		*routetbl = axr->next;

	free ((char *) axr);
#endif
	return 0;
}

/* This function gets called every clock tick, to elegantly remove
   stale ax25 route in the Ax_setups queue */
void
axsetupflush (void)
{
struct ax_route *axr, *next = NULLAXR;

	if (Ax_setups_counter)	{
		if (--Ax_setups_counter == 0)	{
			for (axr = Ax_setups; axr != NULLAXR; axr = next)	{
				next = axr->next;
				free (axr);
			}
			Ax_setups = NULLAXR;
		}
	}
}

/* Handle ordinary incoming data (no network protocol) */
void
axnl3 (struct iface *iface OPTIONAL, struct ax25_cb *axp, char *src OPTIONAL,
	char *dest OPTIONAL, struct mbuf *bp, int mcast OPTIONAL)
{
#if 0
	if (axp == NULLAX25) {
		/* beac_input(iface,src,bp); */
		free_p (bp);
	} else {
		append (&axp->rxq, bp);
		if (axp->r_upcall != NULLVFP ((struct ax25_cb *, int)))
			(*axp->r_upcall) (axp, len_p (axp->rxq));
	}
#endif
}
