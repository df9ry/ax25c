/*
 *  Project: ax25c - File: ip.h
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

/** @file ip.h */

#ifndef AXTNOS_IP_H_
#define AXTNOS_IP_H_

#include "global.h"
#include "mbuf.h"

#ifdef __GNUC__
struct ip;			/* forward declaration for gcc */
#endif

#include "iface.h"
#include "internet.h"
#include "timer.h"

#define TLB		30	/* Default reassembly timeout, sec */
#define	IPVERSION	4
#define IP_CS_OLD	1	/* use saved checksum */
#define IP_CS_NEW	0	/* calculate checksum */

extern char Hashtab[];	/* Modulus lookup table */

/* SNMP MIB variables, used for statistics and control. See RFC 1066 */
extern struct mib_entry Ip_mib[];
#define	ipForwarding		Ip_mib[1].value.integer
#define	ipDefaultTTL		Ip_mib[2].value.integer
#define	ipInReceives		Ip_mib[3].value.integer
#define	ipInHdrErrors		Ip_mib[4].value.integer
#define	ipInAddrErrors		Ip_mib[5].value.integer
#define	ipForwDatagrams		Ip_mib[6].value.integer
#define	ipInUnknownProtos	Ip_mib[7].value.integer
#define	ipInDiscards		Ip_mib[8].value.integer
#define	ipInDelivers		Ip_mib[9].value.integer
#define	ipOutRequests		Ip_mib[10].value.integer
#define	ipOutDiscards		Ip_mib[11].value.integer
#define	ipOutNoRoutes		Ip_mib[12].value.integer
#define	ipReasmTimeout		Ip_mib[13].value.integer
#define	ipReasmReqds		Ip_mib[14].value.integer
#define	ipReasmOKs		Ip_mib[15].value.integer
#define	ipReasmFails		Ip_mib[16].value.integer
#define	ipFragOKs		Ip_mib[17].value.integer
#define	ipFragFails		Ip_mib[18].value.integer
#define	ipFragCreates		Ip_mib[19].value.integer

#define	NUMIPMIB	19

/* IP header, INTERNAL representation */
#define IPLEN		20	/* Length of standard IP header */
#define IP_MAXOPT	40	/* Largest option field, bytes */
struct ip {
	uint32 source;		/* Source address */
	uint32 dest;		/* Destination address */
	int16 length;		/* Total length */
	int16 id;		/* Identification */
	int16 offset;		/* Fragment offset in bytes */
	int16 checksum;		/* Header checksum */

	struct {
		char congest;	/* Congestion experienced bit (exp) */
		char df;	/* Don't fragment flag */
		char mf;	/* More Fragments flag */
	} flags;

	char version;		/* IP version number */
	char tos;		/* Type of service */
	char ttl;		/* Time to live */
	char protocol;		/* Protocol */
	char optlen;		/* Length of options field, bytes */
	char options[IP_MAXOPT];/* Options field */
};
#define	NULLIP	(struct ip *)0

/* Fields in option type byte */
#define	OPT_COPIED	0x80	/* Copied-on-fragmentation flag */
#define	OPT_CLASS	0x60	/* Option class */
#define	OPT_NUMBER	0x1f	/* Option number */

/* IP option numbers */
#define	IP_EOL		0	/* End of options list */
#define	IP_NOOP		1	/* No Operation */
#define	IP_SECURITY	2	/* Security parameters */
#define	IP_LSROUTE	3	/* Loose Source Routing */
#define	IP_TIMESTAMP	4	/* Internet Timestamp */
#define	IP_RROUTE	7	/* Record Route */
#define	IP_STREAMID	8	/* Stream ID */
#define	IP_SSROUTE	9	/* Strict Source Routing */

/* Timestamp option flags */
#define	TS_ONLY		0	/* Time stamps only */
#define	TS_ADDRESS	1	/* Addresses + Time stamps */
#define	TS_PRESPEC	3	/* Prespecified addresses only */

/* IP routing table entry */
struct route {
	struct route *prev;	/* Linked list pointers */
	struct route *next;
	uint32 target;		/* Target IP address */
	unsigned int bits;	/* Number of significant bits in target */
	uint32 gateway;		/* IP address of local gateway for this target */
	int32 metric;		/* Hop count or whatever */
	struct iface *iface;	/* Device interface structure */
	int flags;
#define	RTPRIVATE	0x1	/* Should the world be told of this route ? */
#define	RTTRIG	0x2		/* Trigger is pending for this route */
	struct timer timer;	/* Time until aging of this entry */
	int32 uses;		/* Usage count */
#ifdef RIP
	int16 route_tag;    /* Tag used by RIP-2, N0POY 4/93 */
#endif
};
#define	NULLROUTE	(struct route *)0
extern struct route *Routes[32][HASHMOD];	/* Routing table */
extern struct route R_default;			/* Default route entry */

/* IP access header entry  (WFM) */
struct rtaccess{
	struct rtaccess *nxtiface;	/* Linked list pointer */
	struct rtaccess *nxtbits;
	int16 status;		/* 0=permit, 1=deny */
	int16 protocol;		/* 0=any, otherwise IP protocol # */
	uint32 source;		/* Source IP address */
	unsigned int sbits;	/* Number of significant bits in source */
	uint32 target;		/* Target IP address */
	unsigned int bits;	/* Number of significant bits in target */
	struct iface *iface;	/* Device interface structure */
	int16 lowport;		/* tcp & udp port range. low=0 implies all */
	int16 highport;
};
#define NULLACCESS	(struct rtaccess *)0
extern struct rtaccess *IPaccess;
/* end WFM access control */

/* Cache for the last-used routing entry, speeds up the common case where
 * we handle a burst of packets to the same destination
 */
struct rt_cache {
	uint32 target;
	struct route *route;
};


/* Reassembly descriptor */
struct reasm {
	struct reasm *next;	/* Linked list pointer */
	struct timer timer;	/* Reassembly timeout timer */
	struct frag *fraglist;	/* Head of data fragment chain */
	int16 length;		/* Entire datagram length, if known */
	uint32 source;		/* src/dest/id/protocol uniquely describe a datagram */
	uint32 dest;
	int16 id;
	char protocol;
};
#define	NULLREASM	(struct reasm *)0

/* Fragment descriptor in a reassembly list */
struct frag {
	struct frag *prev;	/* Previous fragment on list */
	struct frag *next;	/* Next fragment */
	struct mbuf *buf;	/* Actual fragment data */
	int16 offset;		/* Starting offset of fragment */
	int16 last;		/* Ending offset of fragment */
};
#define	NULLFRAG	(struct frag *)0

extern struct reasm *Reasmq;	/* The list of reassembly descriptors */

/* Structure for handling raw IP user sockets */
struct raw_ip {
	struct raw_ip *next;	/* Linked list pointer */

	struct mbuf *rcvq;	/* receive queue */
	void (*r_upcall) (struct raw_ip *);
	int protocol;		/* Protocol */
	int user;		/* User linkage */
};
#define	NULLRIP	((struct raw_ip *)0)

/* Transport protocol link table */
struct iplink {
	char proto;
	void (*funct) (struct iface *,struct ip *,struct mbuf *,int);
};
extern struct iplink Iplink[];

/* IP heard structure */
struct iph {
    struct iph *next;
    uint32 addr;
    struct iface *iface;
    long count;
    long time;
};
#define NULLIPH (struct iph *)0
#define MAXIPHEARD 16

/* In ip.c: */
void ip_recv (struct iface *iface,struct ip *ip,struct mbuf *bp,
	int rxbroadcast);
void ipip_recv (struct iface *iface,struct ip *ip,struct mbuf *bp,
	int rxbroadcast);
int ip_send (uint32 source,uint32 dest,char protocol,char tos,char ttl,
	struct mbuf *bp,int16 length,int16 id,char df);
struct raw_ip *raw_ip (int protocol,void (*r_upcall) (struct raw_ip *));
void del_ip (struct raw_ip *rrp);

/* In iproute.c: */
void ipinit (void);
int16 ip_mtu (uint32 addr);
int ip_encap (struct mbuf *bp,struct iface *iface,uint32 gateway,
	int prec,int del,int tput,int rel);
int ip_route (struct iface *i_iface,struct mbuf *bp,int rxbroadcast);
uint32 locaddr (uint32 addr);
void rt_merge (int trace);
struct route *rt_add (uint32 target,unsigned int bits,uint32 gateway,
	struct iface *iface,int32 metric,int32 ttl,char private);
int rt_drop (uint32 target,unsigned int bits);
struct route *rt_lookup (uint32 target);
struct route *rt_blookup (uint32 target,unsigned int bits);
void addaccess (int16 protocol,uint32 source,unsigned int sbits,
	uint32 target,unsigned int tbits,struct iface *ifp,
	int16 low,int16 high,int16 permit);

/* In iphdr.c: */
int16 cksum (struct pseudo_header *ph,struct mbuf *m,int16 len);
int16 eac (int32 sum);
struct mbuf *htonip (struct ip *ip,struct mbuf *data,int cflag);
int ntohip (struct ip *ip,struct mbuf **bpp);

/* In either lcsum.c or pcgen.asm: */
int16 lcsum (int16 *wp,int16 len);

/* In ipcmd.c */
void log_ipheard (uint32 addr, struct iface *iface);

#endif /* AXTNOS_IP_H_ */
