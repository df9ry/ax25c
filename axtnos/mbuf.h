/*
 *  Project: ax25c - File: mbuf.h
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

/** @file mbuf.h */

#ifndef AXTNOS_MBUF_H_
#define AXTNOS_MBUF_H_

#include <stdio.h>

#include "global.h"

/* Basic message buffer structure */
struct mbuf {
	struct mbuf *next;	/* Links mbufs belonging to single packets */
	struct mbuf *anext;	/* Links packets on queues */
	int16 size;		/* Size of associated data buffer */
	int refcnt;		/* Reference count */
	struct mbuf *dup;	/* Pointer to duplicated mbuf */
	unsigned char *data;	/* Active working pointers */
	int16 cnt;
};
#define	NULLBUF	(struct mbuf *)0
#define	NULLBUFP (struct mbuf **)0

#define	PULLCHAR(bpp)\
 ((bpp) != NULL && (*bpp) != NULLBUF && (*bpp)->cnt > 1 ? \
 ((*bpp)->cnt--,(unsigned char)*(*bpp)->data++) : pullchar(bpp))

/* In mbuf.c: */
struct mbuf *alloc_mbuf (int16 size);
struct mbuf *free_mbuf (struct mbuf *bp);

struct mbuf *ambufw (int16 size);
struct mbuf *copy_p (struct mbuf *bp,int16 cnt);
int16 dup_p (struct mbuf **hp,struct mbuf *bp,int16 offset,int16 cnt);
struct mbuf *free_p (struct mbuf *bp);
int16 len_p (struct mbuf *bp);
void trim_mbuf (struct mbuf **bpp,int16 length);
int write_p (FILE *fp,struct mbuf *bp);

struct mbuf *dequeue (struct mbuf **q);
void enqueue (struct mbuf **q,struct mbuf *bp);
void free_q (struct mbuf **q);
int16 len_q (struct mbuf *bp);

struct mbuf *qdata (const unsigned char *data,int16 cnt);
int16 dqdata (struct mbuf *bp,unsigned char *buf,unsigned cnt);

void append (struct mbuf **bph,struct mbuf *bp);
struct mbuf *pushdown (struct mbuf *bp,int16 size);
int16 pullup (struct mbuf **bph,unsigned char *buf,int16 cnt);

int pullchar (struct mbuf **bpp);       /* returns -1 if nothing */
int16 pull16 (struct mbuf **bpp);	/* returns -1 if nothing */
uint32 pull32 (struct mbuf **bpp);	/* returns  0 if nothing */

int16 get16 (char *cp);
uint32 get32 (char *cp);
unsigned char *put16 (unsigned char *cp,int16 x);
unsigned char *put32 (unsigned char *cp,uint32 x);

#define AUDIT(bp)       audit(bp,__FILE__,__LINE__)

#endif /* AXTNOS_MBUF_H_ */
