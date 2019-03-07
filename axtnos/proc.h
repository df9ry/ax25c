/*
 *  Project: ax25c - File: proc.h
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

/** @file proc.h */

#ifndef AXTNOS_PROC_H_
#define AXTNOS_PROC_H_

//#include <ucontext.h>
#define ucontext_t int

#include "mbuf.h"
#include "timer.h"

#if !defined(_SESSION_H) && !defined(_HARDWARE_H) && !defined(_GLOBAL_H)
struct session;			/* forward declaration */
#endif

#define	OUTBUFSIZE	512	/* Size to be malloc'ed for outbuf */


/* Kernel process control block */
#define	PHASH	16		/* Number of wait table hash chains */
struct proc {
	short magic1;		/* for sanity checking */
#define PROC_MAGIC1	0x7388
	struct proc *prev;	/* Process table pointers */
	struct proc *next;

	ucontext_t env;		/* Process register state */

	char i_state;		/* Process interrupt state */

	unsigned short state;
#define	READY	0
#define	WAITING	1
#define	SUSPEND	2
	volatile void *event;	/* Wait event */
	int16 *stack;		/* Process stack */
	unsigned stksize;	/* Size of same */
#ifdef __CYGWIN__
	int16 *stkptr;		/* Process stack pointer */
#endif
	char *name;		/* Arbitrary user-assigned name */
	int retval;		/* Return value from next kwait() */
	struct timer alarm;	/* Alarm clock timer */
	struct mbuf *outbuf;	/* Terminal output buffer */
	int input;		/* standard input socket */
	int output;		/* standard output socket */
	int iarg;		/* Copy of iarg */
	void *parg1;		/* Copy of parg1 */
	void *parg2;		/* Copy of parg2 */
	int freeargs;		/* Free args on termination if set */
	struct session *session; /* for session manager - sigh */
/* #ifdef USE_SETSTACK */
				/* Scratch pointer for process function */
	void (*func)(int, void*, void*);
/* #endif */
#ifdef SCRIPTING
	char *gotolabel;	/* for use w/Command session rmt sysop goto cmd */
	int16 condfalse;	/* used by if/while commands */
#endif
	int ptype;		/* process type */
#define	PTYPE_NORMAL	0	/* regular processes */
#define PTYPE_DAEMON	1	/* one of the daemons in the Daemon list */
#define PTYPE_SERVER	2	/* a server process */
#define PTYPE_IO	3	/* I/O tx/rx process */

	short magic2;		/* for sanity checking */
#define PROC_MAGIC2	0x7373
};
#define NULLPROC (struct proc *)0

extern struct proc *Waittab[];	/* Head of wait list */
extern struct proc *Rdytab;	/* Head of ready list */
extern struct proc *Curproc;	/* Currently running process */
extern struct proc *Susptab;	/* Suspended processes */


#define	SIGQSIZE	300	/* Entries in ksignal queue */
#define RESTART_COUNT	20	/* # of SIGSEGV recoveries to attempt */


struct sigentry {
	volatile void *event;
	int n;
};
struct ksig {
	struct sigentry entry[SIGQSIZE];
	struct sigentry *wp;
	struct sigentry *rp;
	volatile int nentries;	/* modified both by interrupts and main */
	int maxentries;
	int32 duksigs;
	int32 lostsigs;
	uint32 ksigs;		/* Count of ksignal calls */
	uint32 ksigwakes;	/* Processes woken */
	uint32 ksignops;	/* ksignal calls that didn't wake anything */
	uint32 ksigsqueued;	/* ksignal calls queued with ints off */
	uint32 kwaits;		/* Count of kwait calls */
	uint32 kwaitnops;	/* kwait calls that didn't block */
	uint32 kwaitints;	/* kwait calls from interrupt context (error) */
	uint32 krestarts;	/* # times daemon processes restarted from SIGSEGV */
	uint32 kresumes;	/* # times daemon resumed from SIGSEGV */
	uint32 kfreesegvs;	/* # times a free() ignored a SIGSEGV */
};
extern struct ksig Ksig;

/* Prepare for an exception signal and return 0. If after this macro
 * is executed any other process executes alert(pp,val), this will
 * invoke the exception and cause this macro to return a second time,
 * but with the return value 1. This cannot be a function since the stack
 * frame current at the time setjmp is called must still be current
 * at the time the signal is taken. Note use of comma operators to return
 * the value of setjmp as the overall macro expression value.
 */
#define	SETSIG(val)	(Curproc->flags.sset=1,\
	Curproc->signo = (val),setjmp(Curproc->sig))



/* In  kernel.c: */
void alert (struct proc *pp,int val);
void chname (struct proc *pp,const char *newname);
void killproc (struct proc *pp);
void killself (void);
struct proc *mainproc (const char *name);
struct proc *newproc (const char *name,unsigned int stksize,
	void (*pc) (int,void *,void *),
	int iarg,void *parg1,void *parg2,int freeargs);
int ksignal (volatile void *event, int n);
int kwait (volatile void *event);
void resume (struct proc *pp);
void suspend (struct proc *pp);

/* In ksubr.c: */
void kinit (void);
unsigned phash (volatile void *event);
void psetup (struct proc *pp);

/* Stack background fill value for high water mark checking */
#define	STACKPAT	0x55aa

/* Value stashed in location 0 to detect null pointer dereferences */
#define	NULLPAT		0xdead

#endif /* AXTNOS_PROC_H_ */
