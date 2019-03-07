/*
 *  Project: ax25c - File: iface.h
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

/** @file iface.h */

#ifndef AXTNOS_IFACE_H_
#define AXTNOS_IFACE_H_

#include "global.h"
#include "mbuf.h"
#include "proc.h"

#ifdef __GNUC__
struct iface;			/* forward declaration for gcc */
#endif

#include "tcp.h"

#ifdef AX25
#ifndef _AX25_H
#include "ax25.h"
#endif
#endif

/* Interface encapsulation mode table entry. An array of these structures
 * are initialized in config.c with all of the information necessary
 * to attach a device.
 */

#if !defined(send) && !defined(_SOCKET_H)
/* ugly hack to avoid both libc collisions and jnos misconnections */
#include "socket.h"
#endif


struct iftype {
	const char *name;	/* Name of encapsulation technique */
    int (*send) (struct mbuf *,struct iface *,uint32,int,int,int,int);
				/* Routine to send an IP datagram */
	int (*output) (struct iface *,const char *,char *,int16,struct mbuf *);
				/* Routine to send link packet */
	char *(*format) (char *,char *);
				/* Function that formats addresses */
	int (*scan) (char *,const char *);
				/* Reverse of format */
	int type;		/* Type field for network process */
	int hwalen;		/* Length of hardware address, if any */
};
#define	NULLIFT	(struct iftype *)0
extern struct iftype Iftypes[];


#ifdef AX25	/* placed here to prevent interdependency problems w/header files */
struct ax25_counters	{
	int32	msgin;
	int32	msgout;
	int32	datain;
	int32	dataout;
	int32	segin;
	int32	segout;
	int32	segerr;
	int32	frmerr;
	int32	rnrin;
	int32	rejin;
	int32	rnrout;
	int32	rejout;
	int32	retries;
};
#endif


/* Interface control structure */
struct iface {
	struct iface *next;	/* Linked list pointer */
	const char *name;	/* Ascii string with interface name */
	char *descr;		/* Description of interface */

	uint32 addr;		/* IP address */
	uint32 broadcast;	/* Broadcast address */
	uint32 netmask;		/* Network mask */

	int16 mtu;		/* Maximum transmission unit size */
        /*
        **      Interface Metric - this value is used by RIP
        **      to determine what value to increment the RIP_TTL
        **      by.  This is a simple hack to allow specifying that
        **      some interfaces are of higher quality than others.
        **
        */
	int iface_metric;

	int32 flags;		/* Configuration flags */
#define DATAGRAM_MODE   0L	/* Send datagrams in raw link frames */
#define CONNECT_MODE    1L	/* Send datagrams in connected mode */
#define IS_NR_IFACE     2L	/* Activated for NET/ROM - WG7J */
#define NR_VERBOSE      4L	/* NET/ROM broadcast is verbose - WG7J */
#define IS_CONV_IFACE   8L	/* Activated for conference call access - WG7J */
#define AX25_BEACON     16L	/* Broadcast AX.25 beacons */
#define MAIL_BEACON     32L	/* Send MAIL beacons */
#define HIDE_PORT       64L	/* Don't show port in PBBS P command */
#define AX25_DIGI       128L	/* Allow digipeating */
#define ARP_EAVESDROP   256L	/* Listen to ARP replies */
#define ARP_KEEPALIVE   512L	/* Keep arp entries alive after timeout */
#define LOG_AXHEARD    1024L	/* Do ax.25 heard logging on this interface */
#define LOG_IPHEARD    2048L	/* Do IP heard logging on this interface */
#define NO_AX25        4096L	/* No ax.25 PBBS connections on this port */
#define BBS_ONLY       8192L	/* BBS's only in PBBS via this port */
#define USERS_ONLY     16384L	/* Users only on this port */
#define SYSOP_ONLY     32768L	/* Sysops only on this port */
#define LOOPBACK_AX25  65536L	/* Used for AX25 loopback interface */

#ifdef NETROM
	int quality;            /* Netrom interface quality */
#endif

	int16 trace;		/* Trace flags */
#define	IF_TRACE_OUT	0x01	/* Output packets */
#define	IF_TRACE_IN	0x10	/* Packets to me except broadcast */
#define	IF_TRACE_ASCII	0x100	/* Dump packets in ascii */
#define	IF_TRACE_HEX	0x200	/* Dump packets in hex/ascii */
#define	IF_TRACE_NOBC	0x1000	/* Suppress broadcasts */
#define	IF_TRACE_RAW	0x2000	/* Raw dump, if supported */
#define	IF_TRACE_COLOR	0x4000	/* Color _ve1ttl */
	char *trfile;		/* Trace file name, if any */
	FILE *trfp;		/* Stream to trace to */
	int trsock;		/* Socket to trace to */

	struct iface *forw;	/* Forwarding interface for output, if rx only */
#ifdef RXECHO
	struct iface *rxecho;	/* Echo received packets here - WG7J */
#endif

	struct proc *rxproc;	/* Receiver process, if any */
	struct proc *txproc;	/* Transmitter process, if any */
	struct proc *supv;	/* Supervisory process, if any */

	/* Device dependant */
	int dev;		/* Subdevice number to pass to send */
				/* To device -- control */
	int32 (*ioctl) (struct iface *,int cmd,int set,int32 val);
				/* From device -- when status changes */
	int (*iostatus) (struct iface *,int cmd,int32 val);
				/* Call before detaching */
	int (*stop) (struct iface *);
	char *hwaddr;		/* Device hardware address, if any */
	char *ipcall;		/* Device IP call sign, if any */
	char *rmtaddr;		/* AXIP remote address, if any */

	/* Encapsulation dependant */
#ifdef AX25
	struct ifax25 *ax25;    /* Pointer to ax.25 protocol structure */
	struct ax25_counters axcnt; /* AX25 counters */
#endif
	struct iftcp *tcp;      /* Tcp protocol variables */
	void *edv;		/* Pointer to protocol extension block, if any */
	int type;		/* Link header type for phdr */
	int xdev;		/* Associated Slip or Nrs channel, if any */
	int port;		/* Sub port for multy port kiss */
	struct iftype *iftype;	/* Pointer to appropriate iftype entry */

				/* Encapsulate an IP datagram */
	int (*send) (struct mbuf *,struct iface *,uint32,int,int,int,int);
				/* Encapsulate any link packet */
	int (*output) (struct iface *,const char *,char *,int16,struct mbuf *);
				/* Send raw packet */
	int (*raw)		(struct iface *,struct mbuf *);
				/* Display status */
	void (*show)		(struct iface *);

	int (*discard)		(struct iface *,struct mbuf *);
	int (*echo)		(struct iface *,struct mbuf *);

	/* Counters */
	int32 ipsndcnt; 	/* IP datagrams sent */
	int32 rawsndcnt;	/* Raw packets sent */
	int32 iprecvcnt;	/* IP datagrams received */
	int32 rawrecvcnt;	/* Raw packets received */
	int32 lastsent;		/* Clock time of last send */
	int32 lastrecv;		/* Clock time of last receive */
};
#define	NULLIF	(struct iface *)0
extern struct iface *Ifaces;	/* Head of interface list */
extern struct iface  Loopback;	/* Optional loopback interface */
extern struct iface  Encap;	/* IP-in-IP pseudo interface */

/* Header put on front of each packet in input queue */
struct phdr {
	struct iface *iface;
	unsigned short type;	/* Use pktdrvr "class" values */
};

#if 0
/* Header put on front of each packet sent to an interface */
struct qhdr {
	char tos;
	uint32 gateway;
};
#endif

extern struct mbuf *Hopper;

/* In iface.c: */
struct iface *if_lookup (const char *name);
struct iface *ismyaddr (uint32 addr);
int if_detach (struct iface *ifp);
int setencap (struct iface *ifp,const char *mode);
char *if_name (struct iface *ifp,const char *comment);
int bitbucket (struct iface *ifp,struct mbuf *bp);
int dosetflag (int argc,char *argv[],void *p,int flagtoset, int AX25only);
int mask2width (uint32 mask);         /* Added N0POY, for rip code */

/* In config.c: */
int net_route (struct iface *ifp,int type,struct mbuf *bp);



#endif /* AXTNOS_IFACE_H_ */
