/*
 *  Project: ax25c - File: internet.h
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

/** @file internet.h */

#ifndef AXTNOS_INTERNET_H_
#define AXTNOS_INTERNET_H_

#include "global.h"

/* Global structures and constants pertaining to the interface between IP and
 * 	higher level protocols
 */

/* IP protocol field values */
#define	ICMP_PTCL	1	/* Internet Control Message Protocol */
#define ENCAP_PTCL	4	/* new IP encap value */
#define	TCP_PTCL	6	/* Transmission Control Protocol */
#define	UDP_PTCL	17	/* User Datagram Protocol */
#define	RSPF_PTCL	73	/* Radio Shortest Path First Protocol */
#define	AX25_PTCL	93	/* AX25 inside IP according to RFC-1226 */
#define	IP_PTCL		94	/* IP inside IP */

#define	MAXTTL		255	/* Maximum possible IP time-to-live value */

/* DoD-style precedences */
#define	IP_PRECEDENCE	0xe0	/* Mask for precedence field */
#define	ROUTINE		0x00
#define	PRIORITY	0x20
#define	IMMEDIATE	0x40
#define	FLASH		0x60
#define	FLASH_OVER	0x80
#define	CRITIC		0xa0
#define	INET_CTL	0xc0
#define	NET_CTL		0xe0

/* Amateur-style precedences */
#define	AM_ROUTINE	0x00
#define	AM_WELFARE	0x20
#define	AM_PRIORITY	0x40
#define	AM_EMERGENCY	0x60

/* Class-of-service bits */
#define	IP_COS		0x1c	/* Mask for class-of-service bits */
#define	LOW_DELAY	0x10
#define	THROUGHPUT	0x08
#define	RELIABILITY	0x04

/* IP TOS fields */
#define	PREC(x)		(((x)>>5) & 0x7)
#undef DELAY
#define	DELAY		0x10
#define	THRUPUT		0x8
#define	RELIABLITY	0x4

/* Pseudo-header for TCP and UDP checksumming */
struct pseudo_header {
	uint32 source;		/* IP source */
	uint32 dest;		/* IP destination */
	char protocol;		/* Protocol */
	int16 length;		/* Data field length */
};
#define	NULLHEADER	(struct pseudo_header *)0

/* Format of a MIB entry for statistics gathering */
struct mib_entry {
	const char *name;
	struct {
		uint32 integer;
	} value;
};

#endif /* AXTNOS_INTERNET_H_ */
