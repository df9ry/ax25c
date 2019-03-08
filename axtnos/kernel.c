/*
 *  Project: ax25c - File: kernel.c
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

/* Non pre-empting synchronization kernel, machine-independent portion
 * Copyright 1991 Phil Karn, KA9Q
 */

#include "global.h"
#include "timer.h"
#include "mbuf.h"
#include "proc.h"

struct proc *Curproc;		/* Currently running process */
struct proc *Rdytab;		/* Processes ready to run (not including curproc) */
struct proc *Waittab[PHASH];	/* Waiting process list */
struct proc *Susptab;		/* Suspended processes */
//static struct proc *Rdytail;
static struct proc *Waittail[PHASH];
//static struct proc *Susptail;
//static struct mbuf *Killq;

extern void stopalltimers (struct proc *pp);
extern void tnosfcloseall (struct proc *p);
int valid_proc (struct proc *pp);
void kill_all_procs (void);
//static void addproc (struct proc * entry);
//static void delproc (struct proc * entry);
//static int procsigs (void);
//static void ksig (volatile void *event, int n);
struct ksig Ksig;
void volatile *lastkernelevent = (void *)0;

/* Create a process descriptor for the main function. Must be actually
 * called from the main function!
 * Note that standard I/O is NOT set up here.
 */
struct proc *
mainproc (const char *name)
{
register struct proc *pp;
int i;

	for (i = 0; i < PHASH; i++)	{
		Waittab[i] = NULLPROC;
		Waittail[i] = NULLPROC;
	}
	/* Create process descriptor */
	pp = (struct proc *) callocw (1, sizeof (struct proc));

	/* Create name */
	pp->name = strdup (name);

	/* Initialize the magic numbers */
	pp->magic1 = PROC_MAGIC1;
	pp->magic2 = PROC_MAGIC2;

	pp->stksize = STACKBASE;
	pp->stack = (void *) STACKBASE;
	/* Make current */
	pp->state = READY;
	Curproc = pp;
	return pp;
}

/* Create a new, ready process and return pointer to descriptor.
 * The general registers are not initialized, but optional args are pushed
 * on the stack so they can be seen by a C function.
 */
struct proc *
newproc (
const char *name,			/* Arbitrary user-assigned name string */
unsigned int stksize,			/* Stack size in words to allocate */
void (*pc) (int, void *, void *),	/* Initial execution address */
int iarg,				/* Integer argument (argc) */
void *parg1,				/* Generic pointer argument #1 (argv) */
void *parg2,				/* Generic pointer argument #2 (session ptr) */
int freeargs				/* If set, free arg list on parg1 at termination */
) {
#if 0
register struct proc *pp;
int i;

	/* Create process descriptor */
	pp = (struct proc *) callocw (1, sizeof (struct proc));

	/* Create name */
	pp->name = strdup (name);

	/* Initialize the magic numbers */
	pp->magic1 = PROC_MAGIC1;
	pp->magic2 = PROC_MAGIC2;

	/* Allocate stack */
	if (stksize) {
		stksize *= 2;
		stksize = (stksize < 1024) ? 1024 : stksize;
	}
	stksize += STACKPADSIZE;

	stksize = (stksize + 3) & ~3;
	pp->stksize = stksize;
	if ((pp->stack = (int16 *) mallocw (sizeof (int16) * pp->stksize)) == NULL) {
		free (pp->name);
		free ((char *) pp);
		return NULLPROC;
	}
	/* Initialize stack for high-water check */
	for (i = 0; (unsigned) i < stksize; i++)
		pp->stack[i] = STACKPAT;

	pp->func = pc;
	pp->freeargs = freeargs;
	pp->iarg = iarg;
	pp->parg1 = parg1;
	pp->parg2 = parg2;

	/* Task initially runs with interrupts on */
	pp->i_state = 1;

	/* Do machine-dependent initialization of stack */
	psetup (pp);

	/* Inherit creator's input and output sockets */
	(void) usesock (Curproc->input);
	pp->input = Curproc->input;
	(void) usesock (Curproc->output);
	pp->output = Curproc->output;

	/*
	 * The old "curses" tty driver faked this, occasionally getting it
	 * not quire right (IMHO, but the DOS version did the same implicitly).
	 * The new one uses this pointer to get it "right".
	 *
	 * The session manager uses this because multiple sessions (potentially
	 * all of them!) can be simultaneously "current" (e.g. "xterm" session
	 * manager).  This is even more important with external sessions, which
	 * are *always* "current".
	 */
	pp->session = Curproc->session;

	/* Add to ready process table */
	pp->state = READY;
	addproc (pp);
	return pp;
#endif
	return NULL;
}

/* Free resources allocated to specified process. If a process wants to kill
 * itself, the reaper is called to do the dirty work. This avoids some
 * messy situations that would otherwise occur, like freeing your own stack.
 */
void
killproc (register struct proc *pp)
{
#if 0
char **argv;

	if (!valid_proc (pp))
		return;

	/* Don't check the stack here! Will cause infinite recursion if
	 * called from a stack error
	 */

	if (pp == Curproc)
		killself ();	/* Doesn't return */

	/* Stop alarm clock in case it's running (also done in killself() */
	stop_timer (&pp->alarm);
	stopalltimers (pp);

	/* Close any open sockets and files */
	freesock (pp);
	tnosfcloseall (pp);

	close_s (pp->input);
	close_s (pp->output);

	/* Alert everyone waiting for this proc to die */
	ksignal (pp, 0);

	/* Remove from appropriate table */
	delproc (pp);

	/* Free allocated memory resources */
	if (pp->freeargs == 1) {
		argv = pp->parg1;
		while (pp->iarg-- != 0)
			free (*argv++);
		free (pp->parg1);
	}

	if (pp->freeargs == 2)
		free (pp->parg2);
	free (pp->name);
	free (pp->stack);
	if (pp->outbuf)
		free (pp->outbuf);
	free ((char *) pp);
#endif
}

/* Terminate current process by sending a request to the killer process.
 * Automatically called when a process function returns. Does not return.
 */
void
killself ()
{
#if 0
register struct mbuf *bp;
char *cp;

	if (Curproc != NULLPROC) {
		bp = pushdown (NULLBUF, sizeof (Curproc));
		memcpy (bp->data, (char *) &Curproc, sizeof (Curproc));
		enqueue (&Killq, bp);
	}

	cp = (char *) mallocw (strlen (Curproc->name) + 1 + strlen (" (zombie)"));
	if (cp)	{
		sprintf (cp, "%s (zombie)", Curproc->name);
		chname (Curproc, cp);
		free (cp);
	}

	/* "Wait for me; I will be merciful and quick." */
	for (;;)
		kwait (NULL);
}

/* Process used by processes that want to kill themselves */
void
killer (int i OPTIONAL, void *v1 OPTIONAL, void *v2 OPTIONAL)
{
struct proc *pp;
struct mbuf *bp;

	server_disconnect_io ();
	for ( ; ; ) {
		while ((volatile struct mbuf *) Killq == NULLBUF)
			kwait (&Killq);
		bp = dequeue (&Killq);
		(void) pullup (&bp, (unsigned char *) &pp, sizeof (pp));
		free_p (bp);
		if (pp != Curproc)	/* We're immortal */
			killproc (pp);
	}
#endif
}

/* Inhibit a process from running */
void
suspend (struct proc *pp)
{
#if 0
	if (!valid_proc (pp))
		return;

	if (pp != Curproc)
		delproc (pp);	/* Running process isn't on any list */
	pp->state |= SUSPEND;
	if (pp != Curproc)
		addproc (pp);	/* kwait will do it for us */
	else
		kwait (NULL);
#endif
}

/* Restart suspended process */
void
resume (struct proc *pp)
{
#if 0
	if (!valid_proc (pp))
		return;

	if (pp != Curproc)
		delproc (pp);		/* Can't be Curproc! */
	pp->state &= ~SUSPEND;
	if (pp != Curproc)
		addproc (pp);	/* kwait will do it for us */
	else
		kwait (NULL);
#endif
}

/* Wakeup waiting process, regardless of event it's waiting for. The process
 * will see a return value of "val" from its kwait() call.
 */
void
alert (struct proc *pp, int val)
{
#if 0
	if (!valid_proc (pp))
		return;

#ifdef	PROCTRACE
	tcmdprintf ("alert(%lx,%u) [%s]\n", ptol (pp), val, pp->name);
#endif
	if (pp != Curproc)
		delproc (pp);
	pp->state &= ~WAITING;
	pp->retval = val;
	pp->event = 0;
	if (pp != Curproc)
		addproc (pp);
	else
		kwait (NULL);
#endif
}

/* Post a wait on a specified event and give up the CPU until it happens. The
 * null event is special: it means "I don't want to block on an event, but let
 * somebody else run for a while". It can also mean that the present process
 * is terminating; in this case the wait never returns.
 *
 * Kwait() returns 0 if the event was signaled; otherwise it returns the
 * arg in an alert() call. Kwait must not be called from interrupt level.
 *
 * Before waiting and after giving up the CPU, kwait() processes the signal
 * queue containing events signaled when interrupts were off. This means
 * the process queues are no longer modified by interrupt handlers,
 * so it is no longer necessary to run with interrupts disabled here. This
 * greatly improves interrupt latencies.
 */
int
kwait (volatile void *event)
{
#if 0
struct proc *oldprocptr;
int tmp;
int i_state;

	Ksig.kwaits++;

	/* Enable interrupts, after saving the current state.
	 * This minimizes interrupt latency since we may have a lot
	 * of work to do. This seems safe, since care has been taken
	 * here to ensure that signals from interrupt level are not lost, e.g.,
	 * if we're waiting on an event, we post it before we scan the
	 * signal queue.
	 */
	i_state = istate();
	if (!i_state)
		Ksig.kwaitints++;
	(void) enable ();

	if(event != NULL){
		/* Post a wait for the specified event */
		Curproc->event = event;
		Curproc->state = WAITING;
		addproc(Curproc);	/* Put us on the wait list */
	}
	/* If the signal queue contains a signal for the event that we're
	 * waiting for, this will wake us back up
	 */
	(void) procsigs();
	if (event == NULL){
		/* We remain runnable */
		if (Rdytab == NULL) {
			/* Nothing else is ready, so just return */
			Ksig.kwaitnops++;
			restore(i_state);
			return 0;
		}
		addproc(Curproc); /* Put us on the end of the ready list */
	}
	/* Look for a ready process and run it. If there are none,
	 * loop or halt until an interrupt makes something ready.
	 */
	while(Rdytab == NULL){
		/* Give system back to upper-level multitasker, if any.
		 * Note that this function enables interrupts internally
		 * to prevent deadlock, but it restores our state
		 * before returning.
		 */
		giveup();
		/* Process signals that occurred during the giveup() */
		(void) procsigs();
	}
	/* Remove first entry from ready list */
	oldprocptr = Curproc;
	Curproc = Rdytab;
	delproc(Curproc);

	/* Now do the context switch.
	 * This technique was inspired by Rob, PE1CHL, and is a bit tricky.
	 *
	 * First save the current process's state. Then if
	 * this is still the old process, load the new environment. Since the
	 * new task will "think" it's returning from the setjmp() with a return
	 * value of 1, the comparison with 0 will bypass the longjmp(), which
	 * would otherwise cause an infinite loop.
	 */
	/* Save old state */
	if (oldprocptr)	{
		oldprocptr->i_state = 0;
		if(i_state)
			oldprocptr->i_state = 1;
	}

	if (oldprocptr == NULLPROC)
		setcontext(&Curproc->env);
	else
		swapcontext(&oldprocptr->env, &Curproc->env);

	/* At this point, we're running in the newly dispatched task */
	tmp = Curproc->retval;
	Curproc->retval = 0;

	/* Also restore the true interrupt state here, in case the longjmp
	 * DOES restore the interrupt state saved at the time of the setjmp().
	 * This is the case with Turbo-C's setjmp/longjmp.
	 */
	restore(Curproc->i_state);

	/* Otherwise return normally to the new task */
	return tmp;
#endif
	return 0;
}

int
ksignal (volatile void *event, int n)
{
#if 0
int cnt;

	if(istate())	{
		/* Interrupts are on, just call ksig directly after
		 * processing the previously queued signals
		 */
		cnt = procsigs ();
		ksig (event, n);
		return cnt;
	}

	/* Interrupts are off, so quickly queue event */
	Ksig.ksigsqueued++;

 	/* Ignore duplicate signals to protect against a mad device driver
	 * overflowing the signal queue
	 */
	if(event == lastkernelevent && Ksig.nentries != 0){
		Ksig.duksigs++;
		return 0;
	}

	if(Ksig.nentries == SIGQSIZE){
		/* It's hard to handle this gracefully */
		Ksig.lostsigs++;
		return 0;
	}

	lastkernelevent = Ksig.wp->event = event;
	Ksig.wp->n = n;
	if (++Ksig.wp > &Ksig.entry[SIGQSIZE - 1])
		Ksig.wp = Ksig.entry;
	Ksig.nentries++;
#endif
	return 0;
}

#if 0
static int
procsigs(void)
{
int cnt = 0;
int tmp;
int i_state;
	for ( ; ; )	{
		/* Atomic read and decrement of entry count */
		i_state = disable ();
		tmp = Ksig.nentries;
		if (tmp != 0)
			Ksig.nentries--;
		restore(i_state);

		if (tmp == 0)
			break;
		ksig (Ksig.rp->event, Ksig.rp->n);
		if (++Ksig.rp > &Ksig.entry[SIGQSIZE - 1])
			Ksig.rp = Ksig.entry;
		cnt++;
	}

	if (cnt > Ksig.maxentries)
		Ksig.maxentries = cnt;	/* Record high water mark */

	return cnt;
}
#endif

/* Make ready the first 'n' processes waiting for a given event. The ready
 * processes will see a return value of 0 from kwait().  Note that they don't
 * actually get control until we explicitly give up the CPU ourselves through
 * a kwait(). ksig is now called from pwait, which is never called at
 * interrupt time, so it is no longer necessary to protect the proc queues
 * against interrupts. This also helps interrupt latencies considerably.
 */
#if 0
static void
ksig(
volatile void *event,	/* Event to signal */
int n		/* Max number of processes to wake up */
){
struct proc *pp;
struct proc *pnext;
unsigned int hashval;
int cnt = 0;

	Ksig.ksigs++;

	if(event == NULL){
		Ksig.ksignops++;
		return;		/* Null events are invalid */
	}

	/* n == 0 means "signal everybody waiting for this event" */
	if(n == 0)
		n = 65535;

	hashval = phash(event);
	for (pp = Waittab[hashval]; n != 0 && pp != NULLPROC; pp = pnext)	{
		pnext = pp->next;
		if(pp->event == event){
			delproc (pp);
			pp->state &= ~WAITING;
			pp->retval = 0;
			pp->event = NULL;
			addproc (pp);
			n--;
			cnt++;
		}
	}
	for (pp = Susptab; n != 0 && pp != NULLPROC; pp = pnext)	{
		pnext = pp->next;
		if(pp->event == event){
			delproc (pp);
			pp->state &= ~WAITING;
			pp->event = 0;
			pp->retval = 0;
			addproc (pp);
			n--;
			cnt++;
		}
	}
	if(cnt == 0)
		Ksig.ksignops++;
	else
		Ksig.ksigwakes += (unsigned long) cnt;
}
#endif

/* Rename a process */
void
chname (struct proc *pp, const char *newname)
{
	free (pp->name);
	pp->name = strdup (newname);
}

int
valid_proc (struct proc *pp)
{
	if (pp == NULLPROC)
		return 0;
	if (pp->magic1 != PROC_MAGIC1 || pp->magic2 != PROC_MAGIC2)
		return 0;
	if (pp->state > (SUSPEND + 1))
		return 0;
	return 1;
}

/* Remove a process entry from the appropriate table */
#if 0
static void
delproc (entry)
register struct proc *entry;	/* Pointer to entry */
{
int i_state;
struct proc *next = NULLPROC;
struct proc *prev = NULLPROC;

	if (!valid_proc (entry))
		return;

	i_state = disable ();

	if (valid_proc (entry->prev))
		prev = entry->prev;

	if (valid_proc (entry->next))	{
		entry->next->prev = prev;
		next = entry->next;
	}

	if (prev != NULLPROC)
		prev->next = next;
	else {
		switch (entry->state) {
			case READY:
				Rdytab = next;
				break;
			case WAITING:
				Waittab[phash (entry->event)] = next;
				break;
			case SUSPEND:
			case SUSPEND | WAITING:
				Susptab = next;
				break;
			default:
				break;
		}
	}
	entry->next = entry->prev = NULLPROC;

	switch (entry->state) {
		case READY:
			if (entry == Rdytail)
				Rdytail = prev;
			break;
		case WAITING:
			if (entry == Waittail[phash (entry->event)])
				Waittail[phash (entry->event)] = prev;
			break;
		case SUSPEND:
		case SUSPEND | WAITING:
			if (entry == Susptail)
				Susptail = prev;
			break;
		default:
			break;
	}

	restore (i_state);
}
#endif

/* Append proc entry to end of appropriate list */
#if 0
static void
addproc (entry)
register struct proc *entry;	/* Pointer to entry */
{
struct proc **head, **tail;
int i_state;

	if (!valid_proc (entry))
		return;

	switch (entry->state) {
		case READY:
			head = &Rdytab;
			tail = &Rdytail;
			break;
		case WAITING:
			head = &Waittab[phash (entry->event)];
			tail = &Waittail[phash (entry->event)];
			break;
		case SUSPEND:
		case SUSPEND | WAITING:
			head = &Susptab;
			tail = &Susptail;
			break;
		default:
			head = 0;	/* silence warning */
			tail = 0;
			break;
	}
	if (head) {
		entry->next = NULLPROC;
		i_state = disable ();
		if (!valid_proc (*head)) {
//			if (*head && shall_we_crash())
//				crash_it_already ("corrupted process table");
			/* Empty list, stick at beginning */
			entry->prev = NULLPROC;
			*head = entry;
			if (tail)		/* should always be true */
				*tail = entry;
		} else {
			if (tail)	{	/* should always be true */
				if (!valid_proc(*tail))
					crash_it_already ("corrupted process table");
				(*tail)->next = entry;
				entry->prev = *tail;
				*tail = entry;
			}
		}
		restore (i_state);
	}
}
#endif
