/*
 *  Project: ax25c - File: ax25.h
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

/** @file ax25.h */

#ifndef AXTNOS_AX25_H_
#define AXTNOS_AX25_H_

#ifndef	_GLOBAL_H
#include "global.h"
#endif

#include "ip.h"

#ifndef	_MBUF_H
#include "mbuf.h"
#endif

#ifndef	_IFACE_H
#include "iface.h"
#endif

#ifndef	_SOCKADDR_H
#include "sockaddr.h"
#endif


/* AX.25 datagram (address) sub-layer definitions */

#define	MAXDIGIS	7	/* Maximum number of digipeaters */
#define	ALEN		6	/* Number of chars in callsign field */
#define	AXALEN		7	/* Total AX.25 address length, including SSID */
#define	AXBUF		10	/* Buffer size for maximum-length ascii call */


#ifndef _LAPB_H
#include "lapb.h"
#endif

/* Bits within SSID field of AX.25 address */
#define	SSID		0x1e	/* Sub station ID */
#define	REPEATED	0x80	/* Has-been-repeated bit in repeater field */
#define	E		0x01	/* Address extension bit */
#define	C		0x80	/* Command/response designation */
#define AXN		0x20	/* Supports AX25 L2 negotiations */
#define Q		0x40	/* AX25 negotiation request/response */

#define DIGI_IDS	"+?#-"

/* Our AX.25 address */
extern char Mycall[AXALEN];
extern char AXuser[AXALEN];
extern char NOCALL[AXALEN];


/* List of AX.25 multicast addresses, e.g., "QST   -0" in shifted ASCII */
extern char Ax25multi[][AXALEN];
#define QSTCALL 0
#define NODESCALL 1
#define MAILCALL 2
#define IDCALL 3
/*currently not used in this code*/
#define OPENCALL 4
#define CQCALL 5
#define BEACONCALL 6
#define RMNCCALL 7
#define ALLCALL 8


/* Number of chars in interface field. The involved definition takes possible
 * alignment requirements into account, since ax25_addr is of an odd size.
 */
#define	ILEN	(sizeof(struct sockaddr) - sizeof(short) - AXALEN)

/* Socket address, AX.25 style */
struct sockaddr_ax {
	short sax_family;		/* 2 bytes */
	char ax25_addr[AXALEN];
	char iface[ILEN];		/* Interface name */
};

/* Internal representation of an AX.25 header */
struct ax25 {
	char dest[AXALEN];		/* Destination address */
	char source[AXALEN];		/* Source address */
	char digis[MAXDIGIS][AXALEN];	/* Digi string */
	int ndigis;			/* Number of digipeaters */
	int nextdigi;			/* Index to next digi in chain */
	int cmdrsp;			/* Command/response */
};

/* C-bit stuff */
#define	LAPB_UNKNOWN		0
#define	LAPB_COMMAND		1
#define	LAPB_RESPONSE		2

/* AX.25 routing table entry */
struct ax_route {
	struct ax_route *next;		/* Linked list pointer */
	char target[AXALEN];
    struct iface *iface;
	char digis[MAXDIGIS][AXALEN];
	int ndigis;
    char type;
#define AX_NONSETUP	0		/* used to request a non-setup route */
#define	AX_LOCAL	1		/* Set by local ax25 route command */
#define	AX_AUTO		2		/* Set by incoming packet */
#define AX_SETUP	4		/* Used for overriding auto entries */
	char mode;			/* Connection mode (G1EMM) */
#define	AX_DEFMODE	0		/* Use default interface mode */
#define	AX_VC_MODE	1		/* Try to use Virtual circuit */
#define	AX_DATMODE	2		/* Try to use Datagrams only */
};
#define NULLAXR	((struct ax_route *)0)

extern struct ax_route *Ax_routes;
extern struct ax_route *Ax_setups;

/* AX.25 Level 3 Protocol IDs (PIDs) */
#define PID_X25		0x01	/* CCITT X.25 PLP */
#define	PID_SEGMENT	0x08	/* Segmentation fragment */
#define PID_TEXNET	0xc3	/* TEXNET datagram protocol */
#define	PID_LQ		0xc4	/* Link quality protocol */
#define	PID_APPLETALK	0xca	/* Appletalk */
#define	PID_APPLEARP	0xcb	/* Appletalk ARP */
#define	PID_IP		0xcc	/* ARPA Internet Protocol */
#define	PID_ARP		0xcd	/* ARPA Address Resolution Protocol */
#define	PID_RARP	0xce	/* ARPA Reverse Address Resolution Protocol */
#define	PID_NETROM	0xcf	/* NET/ROM */
#define	PID_NO_L3	0xf0	/* No level 3 protocol */

#define	SEG_FIRST	0x80	/* First segment of a sequence */
#define	SEG_REM		0x7f	/* Mask for # segments remaining */

#define	AX_EOL		"\r"	/* AX.25 end-of-line convention */

#if 0
/* Link quality report packet header, internal format */
struct lqhdr {
	int16 version;		/* Version number of protocol */
#define	LINKVERS	1
	uint32	ip_addr;	/* Sending station's IP address */
};
#define	LQHDR	6
/* Link quality entry, internal format */
struct lqentry {
	char addr[AXALEN];	/* Address of heard station */
	int32 count;		/* Count of packets heard from that station */
};
#define	LQENTRY	11
#endif

/* Link quality database record format
 * Currently used only by AX.25 interfaces
 */
struct lq {
	struct lq *next;
	char addr[AXALEN];	/* Hardware address of station heard */
	struct iface *iface;	/* Interface address was heard on */
	int32 time;		/* Time station was last heard */
	int32 currxcnt;	/* Current # of packets heard from this station */

#ifdef	notdef		/* Not yet implemented */
	/* # of packets heard from this station as of his last update */
	int32 lastrxcnt;

	/* # packets reported as transmitted by station as of his last update */
	int32 lasttxcnt;

	int16 hisqual;	/* Fraction (0-1000) of station's packets heard
			 * as of last update
			 */
	int16 myqual;	/* Fraction (0-1000) of our packets heard by station
			 * as of last update
			 */
#endif
};
#define	NULLLQ	(struct lq *)0

extern struct lq *Lq;	/* Link quality record headers */

/* Structure used to keep track of monitored destination addresses */
struct ld {
	struct ld *next;	/* Linked list pointers */
	char addr[AXALEN];/* Hardware address of destination overheard */
	struct iface *iface;	/* Interface address was heard on */
	int32 time;		/* Time station was last mentioned */
	int32 currxcnt;	/* Current # of packets destined to this station */
};
#define	NULLLD	(struct ld *)0

extern struct ld *Ld;	/* Destination address record headers */

#ifdef __GNUC__
struct ax25_cb;			/* forward declaration */
#endif

/* Linkage to network protocols atop ax25 */
struct axlink {
	int pid;
	void (*funct) (struct iface *,struct ax25_cb *,char *, char *,
	 struct mbuf *,int);
};
extern struct axlink Axlink[];

/* Codes for the open_ax25 call */
#define	AX_PASSIVE	0
#define	AX_ACTIVE	1
#define	AX_SERVER	2	/* Passive, clone on opening */

/* Max number of fake AX.25 interfaces for RFC-1226 encapsulation */
#define NAX25       16

#define	AXHEARD_PASS	0	/* Log both src and dest callsigns */
#define	AXHEARD_NOSRC	1	/* do not log source callsign */
#define	AXHEARD_NODST	2	/* do not log destination callsign */
#define	AXHEARD_NONE	3	/* do not log any callsign */

/* an call that should not be jump-started */
struct no_js {
    struct no_js *next;
    char call[AXALEN];
};

/* AX.25 protocol data kept per interface */
struct ifax25 {
	int paclen;
	int lapbtimertype;
	long irtt;
	int version;
	long t3;
	long t4;
	int n2;
	int maxframe;
	int pthresh;
	int window;
	long blimit;
	long maxwait;   /* maximum backoff time */
	char *bctext;
};

/* In ax25.c: */
struct ax_route *ax_add (char *,int,char digis[][AXALEN],int,struct iface *);
int ax_drop (char *,struct iface *, int);
struct ax_route *ax_lookup (const char *source, const char *dest,struct iface *ifp, int type);
void axip_input (struct iface *iface,struct ip *ip,struct mbuf *bp,int rxbroadcast);
void ax_recv (struct iface *,struct mbuf *);
int ax_send (struct mbuf *bp,struct iface *iface,uint32 gateway,int prec,
	int del,int tput,int rel);
int ax_output (struct iface *iface,const char *dest,char *source,int16 pid,
	struct mbuf *data);
int sendframe (struct ax25_cb *axp,int cmdrsp,int ctl,struct mbuf *data);
void axnl3 (struct iface *iface,struct ax25_cb *axp,char *src,
	char *dest,struct mbuf *bp,int mcast);
void axsetupflush (void);

/* In ax25cmd.c: */
void st_ax25 (struct ax25_cb *axp);
int axheard (struct iface *ifp);
void init_ifax25 (struct ifax25 *);
int connect_filt (int argc,char *argv[],char target[],struct iface *ifp, int mode);

/* In axhdr.c: */
struct mbuf *htonax25 (struct ax25 *hdr,struct mbuf *data);
int ntohax25 (struct ax25 *hdr,struct mbuf **bpp);

/* In axlink.c: */
void logsrc (struct iface *iface,char *addr);
void logdest (struct iface *iface,const char *addr);
#if 0
void getlqentry (struct lqentry *ep,struct mbuf **bpp);
void getlqhdr (struct lqhdr *hp,struct mbuf **bpp);
char *putlqentry (char *cp,char *addr,int32 count);
char *putlqhdr (char *cp,int16 version,uint32 ip_addr);
#endif
struct lq *al_lookup (struct iface *ifp,char *addr,int sort);
struct ld *ad_lookup (struct iface *ifp,const char *addr,int sort);

/* In ax25user.c: */
int ax25val (struct ax25_cb *axp);
int disc_ax25 (struct ax25_cb *axp);
int kick_ax25 (struct ax25_cb *axp);
struct ax25_cb *open_ax25 (struct iface *,char *,char *,
	int,int16,
	void (*) (struct ax25_cb *,int),
	void (*) (struct ax25_cb *,int),
	void (*) (struct ax25_cb *,int,int),
	int user);
struct mbuf *recv_ax25 (struct ax25_cb *axp,int16 cnt);
int reset_ax25 (struct ax25_cb *axp);
int send_ax25 (struct ax25_cb *axp,struct mbuf *bp,int pid);

/* In ax25subr.c: */
int addreq (const char *a,const char *b);
struct ax25_cb *cr_ax25 (char *local,char *remote,struct iface *iface);
void del_ax25 (struct ax25_cb *axp);
struct ax25_cb *find_ax25 (const char *local, const char *remote,struct iface *iface);
char *pax25 (char *e,char *addr);
int setcall (char *out,const char *call);

/* In axui.c: */
#ifdef AXUI
int doaxui (int argc, char *argv[],void *p);
void axui_input (struct iface *iface,struct ax25_cb *axp,char *src,char *dest,struct mbuf *bp,int mcast);
#endif

/* In socket.c: */
#if 0
void beac_input (struct iface *iface,char *src,struct mbuf *bp);
#endif
void s_arcall (struct ax25_cb *axp,int cnt);
void s_ascall (struct ax25_cb *axp,int old,int new);
void s_atcall (struct ax25_cb *axp,int cnt);

#endif /* AXTNOS_AX25_H_ */
