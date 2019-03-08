/*
 *  Project: ax25c - File: mbuf.c
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

/* mbuf (message buffer) primitives
 * Copyright 1991 Phil Karn, KA9Q
 *
 * Mods by G1EMM
 */

#include "global.h"
#include "mbuf.h"
#include "proc.h"

/* Allocate mbuf with associated buffer of 'size' bytes. If interrupts
 * are enabled, use the regular heap. If they're off, use the special
 * interrupt buffer pool.
 */
struct mbuf *
alloc_mbuf (register int16 size)
{
register struct mbuf *bp;

	/* Interrupts are enabled, use the heap normally */
	bp = (struct mbuf *) mallocw ((unsigned) (size + sizeof (struct mbuf)));

	if (bp == NULLBUF)
		return NULLBUF;
	/* Clear just the header portion */
	memset ((char *) bp, 0, sizeof (struct mbuf));

	if ((bp->size = size) != 0)
		bp->data = (unsigned char *) (bp + 1);
	bp->refcnt++;
	return bp;
}


/* Allocate mbuf, waiting if memory is unavailable */
struct mbuf *
ambufw (int16 size)
{
register struct mbuf *bp;

	bp = (struct mbuf *) mallocw ((unsigned) (size + sizeof (struct mbuf)));

	/* Clear just the header portion */
	memset ((char *) bp, 0, sizeof (struct mbuf));

	if ((bp->size = size) != 0)
		bp->data = (unsigned char *) (bp + 1);
	bp->refcnt++;
	return bp;
}

/* Decrement the reference pointer in an mbuf. If it goes to zero,
 * free all resources associated with mbuf.
 * Return pointer to next mbuf in packet chain
 */
struct mbuf *
free_mbuf (register struct mbuf *bp)
{
struct mbuf *bpnext;

	if (bp == NULLBUF)
		return NULLBUF;

	bpnext = bp->next;
	if (bp->dup != NULLBUF) {
		(void) free_mbuf (bp->dup);	/* Follow indirection */
		bp->dup = NULLBUF;
	}
	/* Decrement reference count. If it has gone to zero, free it. */
	if (--bp->refcnt <= 0)
		free ((char *) bp);
	return bpnext;
}

/* Free packet (a chain of mbufs). Return pointer to next packet on queue,
 * if any
 */
struct mbuf *
free_p (register struct mbuf *bp)
{
register struct mbuf *abp;

	if (bp == NULLBUF)
		return NULLBUF;
	abp = bp->anext;
	while (bp != NULLBUF)
		bp = free_mbuf (bp);
	return abp;
}

/* Free entire queue of packets (of mbufs) */
void
free_q (struct mbuf **q)
{
register struct mbuf *bp;

	while ((bp = dequeue (q)) != NULLBUF)
		free_p (bp);
}

/* Count up the total number of bytes in a packet */
int16
len_p (register struct mbuf *bp)
{
register int16 cnt = 0;

	while (bp != NULLBUF) {
		cnt += bp->cnt;
		bp = bp->next;
	}
	return cnt;
}

/* Count up the number of packets in a queue */
int16
len_q (register struct mbuf *bp)
{
register int16 cnt;

	for (cnt = 0; bp != NULLBUF; cnt++, bp = bp->anext) ;
	return cnt;
}

/* Trim mbuf to specified length by lopping off end */
void
trim_mbuf (struct mbuf **bpp, int16 length)
{
register int16 tot = 0;
register struct mbuf *bp;

	if (bpp == NULLBUFP || *bpp == NULLBUF)
		return;		/* Nothing to trim */

	if (length == 0) {
		/* Toss the whole thing */
		free_p (*bpp);
		*bpp = NULLBUF;
		return;
	}
	/* Find the point at which to trim. If length is greater than
	 * the packet, we'll just fall through without doing anything
	 */
	for (bp = *bpp; bp != NULLBUF; bp = bp->next) {
		if (tot + bp->cnt < length) {
			tot += bp->cnt;
		} else {
			/* Cut here */
			bp->cnt = length - tot;
			free_p (bp->next);
			bp->next = NULLBUF;
			break;
		}
	}
}

/* Duplicate/enqueue/dequeue operations based on mbufs */

/* Duplicate first 'cnt' bytes of packet starting at 'offset'.
 * This is done without copying data; only the headers are duplicated,
 * but without data segments of their own. The pointers are set up to
 * share the data segments of the original copy. The return pointer is
 * passed back through the first argument, and the return value is the
 * number of bytes actually duplicated.
 */
int16
dup_p (struct mbuf **hp, register struct mbuf *bp, register int16 offset, register int16 cnt)
{
register struct mbuf *cp;
int16 tot;

	if (cnt == 0 || bp == NULLBUF || hp == NULLBUFP) {
		if (hp != NULLBUFP)
			*hp = NULLBUF;
		return 0;
	}
	if ((*hp = cp = alloc_mbuf (0)) == NULLBUF)
		return 0;

	/* Skip over leading mbufs that are smaller than the offset */
	while (bp != NULLBUF && bp->cnt <= offset) {
		offset -= bp->cnt;
		bp = bp->next;
	}
	if (bp == NULLBUF) {
		(void) free_mbuf (cp);
		*hp = NULLBUF;
		return 0;	/* Offset was too big */
	}
	tot = 0;
	for (;;) {
		/* Make sure we get the original, "real" buffer (i.e. handle the
		 * case of duping a dupe)
		 */
		if (bp->dup != NULLBUF)
			cp->dup = bp->dup;
		else
			cp->dup = bp;

		/* Increment the duplicated buffer's reference count */
		cp->dup->refcnt++;

		cp->data = bp->data + offset;
		cp->cnt = min (cnt, bp->cnt - offset);
		offset = 0;
		cnt -= cp->cnt;
		tot += cp->cnt;
		bp = bp->next;
		if (cnt == 0 || bp == NULLBUF || (cp->next = alloc_mbuf (0)) == NULLBUF)
			break;
		cp = cp->next;
	}
	return tot;
}

/* Copy first 'cnt' bytes of packet into a new, single mbuf */
struct mbuf *
copy_p (register struct mbuf *bp, register int16 cnt)
{
register struct mbuf *cp;
register unsigned char *wp;
register int16 n;

	if (bp == NULLBUF || cnt == 0 || (cp = alloc_mbuf (cnt)) == NULLBUF)
		return NULLBUF;
	wp = cp->data;
	while (cnt != 0 && bp != NULLBUF) {
		n = min (cnt, bp->cnt);
		memcpy (wp, bp->data, (size_t) n);
		wp += n;
		cp->cnt += n;
		cnt -= n;
		bp = bp->next;
	}
	return cp;
}

/* Copy and delete "cnt" bytes from beginning of packet. Return number of
 * bytes actually pulled off
 */
int16
pullup (struct mbuf **bph, unsigned char *buf, int16 cnt)
{
register struct mbuf *bp;
int16 n, tot;

	tot = 0;
	if (bph == NULLBUFP)
		return 0;
	while (cnt != 0 && (bp = *bph) != NULLBUF) {
		n = min (cnt, bp->cnt);
		if (buf != (unsigned char *) 0) {
			if (n == 1)	/* Common case optimization */
				*buf = *bp->data;
			else if (n > 1)
				memcpy (buf, bp->data, (size_t) n);
			buf += n;
		}
		tot += n;
		cnt -= n;
		bp->data += n;
		bp->cnt -= n;
		if (bp->cnt == 0) {
			/* If this is the last mbuf of a packet but there
			 * are others on the queue, return a pointer to
			 * the next on the queue. This allows pullups to
			 * to work on a packet queue
			 */
			if (bp->next == NULLBUF && bp->anext != NULLBUF) {
				*bph = bp->anext;
				(void) free_mbuf (bp);
			} else
				*bph = free_mbuf (bp);
		}
	}
	return tot;
}

/* Append mbuf to end of mbuf chain */
void
append (struct mbuf **bph, struct mbuf *bp)
{
#if 0
register struct mbuf *p;
char i_state;

	if (bph == NULLBUFP || bp == NULLBUF)
		return;

	i_state = (char) disable ();
	if (*bph == NULLBUF) {
		/* First one on chain */
		*bph = bp;
	} else {
		for (p = *bph; p->next != NULLBUF; p = p->next) ;
		p->next = bp;
	}
	restore (i_state);
	ksignal (bph, 1);
#endif
}

/* Insert specified amount of contiguous new space at the beginning of an
 * mbuf chain. If enough space is available in the first mbuf, no new space
 * is allocated. Otherwise a mbuf of the appropriate size is allocated and
 * tacked on the front of the chain.
 *
 * This operation is the logical inverse of pullup(), hence the name.
 */
struct mbuf *
pushdown (register struct mbuf *bp, int16 size)
{
register struct mbuf *nbp;

	/* Check that bp is real, that it hasn't been duplicated, and
	 * that it itself isn't a duplicate before checking to see if
	 * there's enough space at its front.
	 */
	if (bp != NULLBUF && bp->refcnt == 1 && bp->dup == NULLBUF
	    && bp->data - (unsigned char *) (bp + 1) >= size) {
		/* No need to alloc new mbuf, just adjust this one */
		bp->data -= size;
		bp->cnt += size;
	} else {
		nbp = ambufw (size);
		nbp->next = bp;
		nbp->cnt = size;
		bp = nbp;
	}
	return bp;
}

/* Append packet to end of packet queue */
void
enqueue (struct mbuf **q, struct mbuf *bp)
{
#if 0
register struct mbuf *p;
char i_state;

	if (q == NULLBUFP || bp == NULLBUF)
		return;
	i_state = (char) disable ();
	if (*q == NULLBUF) {
		/* List is empty, stick at front */
		*q = bp;
	} else {
		for (p = *q; p->anext != NULLBUF; p = p->anext) ;
		p->anext = bp;
	}
	restore (i_state);
	ksignal (q, 1);
#endif
}

/* Unlink a packet from the head of the queue */
struct mbuf *
dequeue (register struct mbuf **q)
{
#if 0
register struct mbuf *bp;
char i_state;

	if (q == NULLBUFP)
		return NULLBUF;
	i_state = (char) disable ();
	if ((bp = *q) != NULLBUF) {
		*q = bp->anext;
		bp->anext = NULLBUF;
	}
	restore (i_state);
	return bp;
#endif
	return NULL;
}

/* Copy user data into an mbuf */
struct mbuf *
qdata (const unsigned char *data, int16 cnt)
{
register struct mbuf *bp;

	bp = ambufw (cnt);
	memcpy (bp->data, data, (size_t) cnt);
	bp->cnt = cnt;
	return bp;
}

/* Copy mbuf data into user buffer */
int16
dqdata (struct mbuf *bp, unsigned char *buf, unsigned cnt)
{
int16 tot;
unsigned n;
struct mbuf *bp1;

	if (buf == (unsigned char *) 0)
		return 0;

	tot = 0;
	for (bp1 = bp; bp1 != NULLBUF; bp1 = bp1->next) {
		n = min (bp1->cnt, cnt);
		memcpy (buf, bp1->data, n);
		cnt -= n;
		buf += n;
		tot += (int16) n;
	}
	free_p (bp);
	return tot;
}

/* Pull a 32-bit integer in host order from buffer in network byte order.
 * On error, return 0. Note that this is indistinguishable from a normal
 * return.
 */
uint32
pull32 (struct mbuf **bpp)
{
char buf[4];

	if (pullup (bpp, (unsigned char *) buf, 4) != 4)
		/* Return zero if insufficient buffer */
		return 0;

	return get32 (buf);
}

/* Pull a 16-bit integer in host order from buffer in network byte order.
 * Return -1 on error
 */
int16
pull16 (struct mbuf **bpp)
{
char buf[2];

	if (pullup (bpp, (unsigned char *) buf, 2) != 2)
		return ((int16) - 1);	/* Nothing left */

	return get16 (buf);
}

/* Pull single character from mbuf */
int
pullchar (struct mbuf **bpp)
{
char c;

	if (pullup (bpp, (unsigned char *) &c, 1) != 1)
		return -1;	/* Nothing left */
	return (int) uchar (c);
}

int
write_p (FILE *fp, struct mbuf *bp)
{
	while (bp != NULLBUF) {
		if (fwrite (bp->data, 1, (size_t) bp->cnt, fp) != bp->cnt)
			return -1;
		bp = bp->next;
	}
	return 0;
}
