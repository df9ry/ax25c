/*
 *  Project: ax25c - File: timer.h
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

/** @file timer.h */

#ifndef AXTNOS_TIMER_H_
#define AXTNOS_TIMER_H_

#include "global.h"

/* Software timers
 * There is one of these structures for each simulated timer.
 * Whenever the timer is running, it is on a linked list
 * pointed to by "Timers". The list is sorted in ascending order of
 * expiration, with the first timer to expire at the head. This
 * allows the timer process to avoid having to scan the entire list
 * on every clock tick; once it finds an unexpired timer, it can
 * stop searching.
 *
 * Stopping a timer or letting it expire causes it to be removed
 * from the list. Starting a timer puts it on the list at the right
 * place.
 */
struct timer {
	short magic1;		/* for sanity checking */
#define TIMER_MAGIC1	0x1234
	struct timer *next;	/* Linked-list pointer */
	int32 duration;		/* Duration of timer, in ticks */
	int32 expiration;	/* Clock time at expiration */
	void (*func) (void *);	/* Function to call at expiration */
	void *arg;		/* Arg to pass function */
	char state;		/* Timer state */
#define	TIMER_STOP	0
#define	TIMER_RUN	1
#define	TIMER_EXPIRE	2
	void  *theproc;		/* Process owning the timer
				   (actually a struct proc *) */
	char *filename;
	int lineno;
	short magic2;		/* for sanity checking */
#define TIMER_MAGIC2	0x4321
};
#define	NULLTIMER	(struct timer *)0

#ifndef	MSPTICK
#define	MSPTICK		20		/* Milliseconds per tick */
#endif

/* Useful user macros that hide the timer structure internals */
#define	dur_timer(t)	((t)->duration*MSPTICK)
#define	run_timer(t)	((t)->state == TIMER_RUN)

extern volatile int Tick;
extern void (*Cfunc[]) (void);	/* List of clock tick functions */

/* In timer.c: */
void kalarm (const char *filename, int lineno, int32 ms);
int32 read_timer (struct timer *t);
void reset_timer (struct timer *t,int32 x);
void set_timer (struct timer *t,int32 x);
void start_timer (const char *filename, int lineno, struct timer *t);
void start_detached_timer (const char *filename, int lineno, struct timer *t);
void stop_timer (struct timer *timer);
char *tformat (int32 t);
int32 rdclock (void);

#endif /* AXTNOS_TIMER_H_ */
