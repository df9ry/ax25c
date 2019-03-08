/*
 *  Project: ax25c - File: timer.c
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

/* General purpose software timer facilities
 * Copyright 1991 Phil Karn, KA9Q
 */
#include "global.h"
#include "timer.h"
#include "proc.h"

/* Head of running timer chain.
 * The list of running timers is sorted in increasing order of expiration;
 * i.e., the first timer to expire is always at the head of the list.
 */
struct timer *Timers;
int Timer_mutex = TNOS_MUTEX_UNLOCKED;

//static void t_alarm (void *x);
//static int valid_timer (struct timer *t, int exitOnErr);
void stopalltimers (struct proc *pp);
//static void start_timer_internal (const char *filename, int lineno, struct timer *t, struct proc *p);
//static struct proc *curproc = NULLPROC;

struct timerstats {
	uint32 passes;
	uint32 started;
	uint32 stopped;
	uint32 expired;
	uint32 alarms;
};

//static struct timerstats tstats = {0, 0, 0, 0, 0};

/* Process that handles clock ticks */
/* Fixed to solve some timing problems when multiple ticks
 * get handled at once... from Walt Corey, KZ1F
 */
void
timerproc (
int i OPTIONAL,
void *v1 OPTIONAL,
void *v2 OPTIONAL
) {
#if 0
register struct timer *t;
char i_state;
int tmp;
void (**vf) (void);
int32 cur_clock;

	server_disconnect_io ();
	Timer_mutex = TNOS_MUTEX_UNLOCKED;
	for ( ; ; ) {
		kwait (NULL);	/* Let them run before handling more ticks */

		/* Atomic read and decrement of Tick */
		for ( ; ; ) {
			i_state = (char) disable ();
			tmp = Tick;
			if (tmp > 0) {
				Tick--;
				restore (i_state);
				break;
			}
			restore (i_state);
			kwait (&Tick);
		}
		if (!istate ()) {
			restore (1);
		}

		tstats.passes++;

		/* Call the functions listed in config.c */
		for (vf = Cfunc; *vf != NULL; vf++)
			(*vf) ();

		if (Timers == NULLTIMER)
			continue;	/* No active timers, all done */

		cur_clock = rdclock();
		kmutex_lock (&Timer_mutex);

		/* Now go through the list of expired timers, removing each
		 * one and kicking the notify function, if there is one
		 * Note use of subtraction and comparison to zero rather
		 * than the more obvious simple comparison; this avoids
		 * problems when the clock count wraps around.
		 */
		while (Timers != NULLTIMER && (cur_clock - Timers->expiration) >= 0) {
			if (Timers->next == Timers) {
				tcmdprintf ("PANIC: Timer loop at %lx\n", (long) Timers);
				iostop ();
				exit (1);
			}
			/* Save Timers since stop_timer will change it */
			t = Timers;
			Timers = t->next;

			t->state = TIMER_EXPIRE;
			t->next = NULLTIMER;
			freeIfSet (t->filename);
			t->filename = NULLCHAR;
			tstats.expired++;
			if (valid_timer (t, 1) == 1)	{
				if (t->func)	{
					curproc = (struct proc *)t->theproc;
					(*t->func) (t->arg);
					curproc = NULLPROC;
				}
			}
		}
		kmutex_unlock (&Timer_mutex);
	}
#endif
}



/* Start a timer - link to specific process */
#if 0
static void
start_timer_internal (const char *filename, int lineno, struct timer *t, struct proc *p)
{
register struct timer *tnext;
struct timer *tprev = NULLTIMER;

	if (t == NULLTIMER)
		return;

	if (run_timer (t))
		stop_timer (t);

	if (t->duration == 0)
		return;		/* A duration value of 0 disables the timer */

	tstats.started++;
	t->next = NULLTIMER;	/* insure forward chain is NULL */
	t->expiration = rdclock() + t->duration;
	t->state = TIMER_RUN;

	t->magic1 = TIMER_MAGIC1;
	t->magic2 = TIMER_MAGIC2;
	t->filename = strdup (filename);
	t->lineno = lineno;
	if (curproc == NULLPROC)
		t->theproc = (void *) p;
	else
		t->theproc = curproc;

	/* Find right place on list for this guy. Once again, note use
	 * of subtraction and comparison with zero rather than direct
	 * comparison of expiration times.
	 */
	for (tnext = Timers; valid_timer(tnext, 1) == 1; tprev = tnext, tnext = tnext->next) {
		if ((tnext->expiration - t->expiration) >= 0)
			break;
	}


	/* At this point, tprev points to the entry that should go right
	 * before us, and tnext points to the entry just after us. Either or
	 * both may be null.
	 */
	if (tprev == NULLTIMER)
		Timers = t;	/* Put at beginning */
	else
		tprev->next = t;

	t->next = tnext;
}
#endif

/* Start a timer */
void
start_timer (const char *filename, int lineno, struct timer *t)
{
//	start_timer_internal (filename, lineno, t, Curproc);
}


/* Start a timer, detached from the current process */
void
start_detached_timer (const char *filename, int lineno, struct timer *timer)
{
	//start_timer_internal (filename, lineno, timer, Command->proc);
}



/* Stop a timer */
void
stop_timer (struct timer *timer)
{
#if 0
register struct timer *t;
struct timer *tlast = NULLTIMER;

	if (!valid_timer(timer, 0) || !run_timer (timer))
		return;

	/* Verify that timer is really on list */
	for (t = Timers; valid_timer (t, 1) == 1; tlast = t, t = t->next)
		if (t == timer)
			break;

	if (t != NULLTIMER)	{
		tstats.stopped++;

		/* Delete from active timer list */
		if (tlast != NULLTIMER)
			tlast->next = t->next;
		else
			Timers = t->next;	/* Was first on list */

		t->state = TIMER_STOP;
		t->next = NULLTIMER;
		freeIfSet (t->filename);
		t->filename = NULLCHAR;
	}
#endif
}



/* called from killproc to cleanup pending timers */
void
stopalltimers (struct proc *pp)
{
#if 0
struct timer *t, *tmp;

	for (t = Timers; valid_timer (t, 0) == 1; t = tmp)	{
		tmp = t->next;
		if (t->theproc == pp)
			stop_timer (t);
	}
#endif
}

/* Return milliseconds remaining on this timer */
int32
read_timer (struct timer *t)
{
#if 0
int32 remaining;

	if (t == NULLTIMER || !run_timer (t))
		return 0;
	remaining = t->expiration - rdclock();
	if (remaining <= 0)
		return 0;	/* Already expired */
	else
		return remaining * MSPTICK;
#endif
	return 0;
}

void
reset_timer (struct timer *t, int32 interval)
{
#if 0
int32 oldduration;
int32 newduration;
int32 val;
int32 passed = 0;
int resumeIt = 0;
struct proc *owner = NULLPROC;

	if (t == NULLTIMER)
		return;

	if (t->state == TIMER_RUN)	{
		oldduration = t->duration;
		passed = oldduration - (t->expiration - rdclock());
		owner = t->theproc;
		stop_timer (t);
		resumeIt = 1;
	}
	set_timer (t, interval);
	if (resumeIt)	{
		newduration = t->duration;
		val = t->duration - passed;
		if (val <= 0)
			val = 1;
		t->duration = val;
		start_timer_internal (__FILE__, __LINE__, t, owner);
		t->duration = newduration;
	}
#endif
}


void
set_timer (struct timer *t, int32 interval)
{
#if 0
#define FUDGE 0

	if (t == NULLTIMER)
		return;

	/* Round the interval up to the next full tick, and then
	 * add another tick to guarantee that the timeout will not
	 * occur before the interval is up. This is necessary because
	 * we're asynchonous with the system clock.
	 */
	if (interval != 0)
		t->duration = FUDGE + (interval + MSPTICK - 1) / MSPTICK;
	else
		t->duration = 0;
#endif
}

/* Delay process for specified number of milliseconds.
 * Normally returns 0; returns -1 if aborted by alarm.
 */
int
kpause (const char *filename, int lineno, int32 ms)
{
#if 0
int val = 0;

	if (Curproc == NULLPROC || ms == 0)
		return 0;
	kalarm (filename, lineno, ms);
	/* The actual event doesn't matter, since we'll be alerted */
	while (Curproc->alarm.state == TIMER_RUN) {
		if ((val = kwait (Curproc)) != 0)
			break;
	}
	kalarm (filename, lineno, 0L);		/* Make sure it's stopped, in case we were killed */
	return (val == EALARM) ? 0 : -1;
#endif
	return 1;
}

#if 0
static void
t_alarm (void *x)
{
	tstats.alarms++;
	alert ((struct proc *) x, EALARM);
}
#endif

/* Send signal to current process after specified number of milliseconds */
void
kalarm (const char *filename, int lineno, int32 ms)
{
#if 0
	if (Curproc != NULLPROC) {
		set_timer (&Curproc->alarm, ms);
		Curproc->alarm.func = t_alarm;
		Curproc->alarm.arg = (char *) Curproc;
		start_timer (filename, lineno, &Curproc->alarm);
	}
#endif
}

/* Convert time count in seconds to printable days:hr:min:sec format */
char *
tformat (int32 t)
{
static char buf[17];
int days, hrs, mins, secs;
int minus = 0;

	if (t < 0) {
		t = -t;
		minus = 1;
	}

	secs = t % 60;
	t /= 60;
	mins = t % 60;
	t /= 60;
	hrs = t % 24;
	days = t / 24;

	sprintf (buf, "%s%d:%02d:%02d:%02d", (minus) ? "-" : "", days, hrs, mins, secs);
	return buf;
}

/* Read the Clock global variable, with interrupts off to avoid possible
 * inconsistency on 16-bit machines
 */
int32
rdclock(void)
{
#if 0
int i_state;
int32 rval;

	i_state = disable ();
	rval = Clock;
	restore (i_state);
	return rval;
#endif
return 0;
}

#if 0
static int
valid_timer (struct timer *t, int exitOnErr)
{
struct proc *theps;

	if (t == NULLTIMER)
		return 0;
	if (t->magic1 != TIMER_MAGIC1 || t->magic2 != TIMER_MAGIC2)	{
		if (exitOnErr && shall_we_crash())
			crash_it_already ("corrupted timer table 1");
		return -1;
	}

	theps = (struct proc *) t->theproc;
	if (theps == NULLPROC || theps->name == NULLCHAR)	{
		if (exitOnErr && shall_we_crash())
			crash_it_already ("corrupted timer table 2");
		return -1;
	}
	return 1;
}
#endif

/* Display the contents of the timer queue in human-readable format */

int
dotimers (int argc OPTIONAL, char *argv[] OPTIONAL, void *p OPTIONAL)
{
#if 0
struct timer *t;
int sentheader = 0;
struct proc *theps;
int32 exp2;
uint32 should = 0;
uint32 is = 0;

	tprintf ("\nTimer Stats:\n    passes %lu started %lu stopped %lu\n    expired %lu alarms %lu\n",
		tstats.passes, tstats.started, tstats.stopped, tstats.expired, tstats.alarms);
	should = tstats.started - (tstats.stopped + tstats.expired);

	/* in order to make sure that this routine and timerproc are NOT
	   fighting for the timer queue, we put a mutex around it. */
	kmutex_lock (&Timer_mutex);
	for (t = Timers; t != NULLTIMER; t = t->next)	{
		theps = (struct proc *) t->theproc;
		if (!valid_timer (t, 1))	{
			stop_timer (t);
			break;
		}
		if (!sentheader)	{
			sentheader++;
			tprintf ("\n%-33s  %-12s  %-4s  %-8s  %s\n", "Process", "Filename", "Line", "Duration", "  Expires At");
			tprintf (  "%-33s  %-12s  %-4s  %-8s  %s\n", "=======", "========", "====", "========", "==============");
		}
		exp2 = t->expiration * MSPTICK;
		tprintf ("%-33.33s  %-12.12s  %4d  %8ld  %s.%03ld\n", theps->name, t->filename, t->lineno, dur_timer(t),
			tformat (exp2 / 1000), exp2 % 1000);
		is++;
	}
	kmutex_unlock (&Timer_mutex);
	if (should != is)
		tprintf ("\nUnaccounted timers: %lu\n", should - is);
	tputc ('\n');
#endif
	return 0;
}
