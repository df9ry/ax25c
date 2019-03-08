/*
 *  Project: ax25c - File: unix.c
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

/*
 * Timers and process delays work differently under POSIX.  The entire system
 * is driven on a single select() call, which uses the timeout to detect alarms
 * and the file descriptors to detect input.  An itimer is used to allow
 * keyboard input to continue during lengthy activities --- which I tried to
 * avoid for portability reasons, but it behaves *real* ugly otherwise.
 * Especially when LakeSW.ampr.org lets 350K of SMTP mail pile up...
 */

#include "global.h"
#include "ctype.h"
#ifdef linux
#undef IS_LITTLE_ENDIAN	/* to avoid a re-definition warning */
#endif
#include <sys/stat.h>
#include <signal.h>
#include <sys/time.h>
#include "timer.h"
#include "proc.h"
static void deinit_tick(void);

/* some versions of GNU libc (at least some for Linux) have a bad bug
   that once it happens, a call to __libc_free() never returns. These
   two variables (along with code in ding() and j_free()) are here to
   keep this from disabling a system indefinitely. If a call to free
   takes longer than 60 seconds to return, then TNOS is exited.
 */

//static int within_a_free_call = 0;
//static int time_in_a_free_call = 0;
#define MAX_TIME_INA_FREE_CALL	60

struct io
{
	struct io *next;
	int fd;
	void *event;
};

char Hashtab[256];
volatile int Tick;
int Keyboard;
volatile int32 Clock;
//extern struct timer *Timers;

static int __istate = 1;
//static struct timeval Starttime;
//static struct io *Events;

/*****************************************************************************\
*		  Miscellanous functions not found in Unix		      *
\*****************************************************************************/

char *
strupr(s)
char *s;
{
register char *p = s;

	while (*p)
		*p = (char) toupper (*p), p++;
	return s;
}

char *
strlwr(s)
char *s;
{
register char *p = s;

	while (*p)
		*p = (char) tolower (*p), p++;
	return s;
}

char *
stpcpy(d, s)
register char *d;
register const char *s;
{
	while (*s)
		*d++ = *s++;
	*d = '\0';
	return d;
}

char *
itoa(n, buf, base)
int n, base;
char *buf;
{
#if 0
	if (base != 10)
		tprintf ("WARNING: itoa() passed radix %d\n", base);
#endif
	sprintf (buf, "%d", n);
	return buf;
}

int16
hash_ip(ipaddr)
uint32 ipaddr;
{
int h;

	h = ((ipaddr >> 16) & 0xFFFF) ^ (ipaddr & 0xFFFF);
	return (int16) (Hashtab[((h >> 8) & 0xFF) ^ (h & 0xFF)]);	/*lint !e702 !e571 */
}

/*****************************************************************************\
*			Interrupt management - null			      *
\*****************************************************************************/

int
istate()
{
	return __istate;
}


int
disable ()
{
#if 0
sigset_t s;
int ops;

	if (__istate)	{
		(void) sigemptyset (&s);
		(void) sigaddset (&s, SIGALRM);
		(void) sigprocmask (SIG_BLOCK, &s, (sigset_t *) 0);
	}
	ops = __istate;
	__istate = 0;
	return ops;
#endif
	return 0;
}

int
enable (void)
{
	restore (1);
	return 0;
}

void
restore(prs)
int prs;
{
#if 0
sigset_t s;

	if (__istate != prs)	{
		(void) sigemptyset (&s);
		(void) sigaddset (&s, SIGALRM);
		(void) sigprocmask ((prs? SIG_UNBLOCK: SIG_BLOCK), &s, (sigset_t *) 0);
	}
	__istate = prs;
#endif
}


/*****************************************************************************\
*			      Date and time functions			      *
\*****************************************************************************/

long
secclock()
{
#if 0
static struct timezone tz;
static struct timeval tv;

	(void) gettimeofday (&tv, &tz);
	return tv.tv_sec - Starttime.tv_sec - (Starttime.tv_usec > tv.tv_usec);		/*lint !e514 */
#endif
	return 0;
}


long
msclock()
{
#if 0
struct timezone tz;
struct timeval tv;

	(void) gettimeofday (&tv, &tz);
	if (tv.tv_usec < Starttime.tv_usec)	{
		tv.tv_usec += 1000000;
		tv.tv_sec--;
	}
	return (tv.tv_sec - Starttime.tv_sec) * 1000 +
		(tv.tv_usec - Starttime.tv_usec) / 1000;
#endif
	return 0;
}

#if 0
static void
init_time(void)
{
struct timezone tz;

	(void) gettimeofday (&Starttime, &tz);
}
#endif

void
gettime(tp)
struct time *tp;
{
#if 0
struct tm *tm;
static struct timeval tv;
static struct timezone tz;

	(void) gettimeofday (&tv, &tz);
	tm = localtime ((const time_t *)&tv.tv_sec);
	tp->ti_hund = tv.tv_usec / 10000;
	tp->ti_hour = tm->tm_hour;
	tp->ti_min = tm->tm_min;
	tp->ti_sec = tm->tm_sec;
#endif
}


void
tnos_getdate(dp)
struct date *dp;
{
#if 0
struct tm *tm;
static struct timeval tv;
static struct timezone tz;

	(void) gettimeofday (&tv, &tz);
	tm = localtime ((const time_t *)&tv.tv_sec);
	dp->da_year = tm->tm_year + 1900;
	if (dp->da_year < 1970)
		dp->da_year += 100;
	dp->da_mon = tm->tm_mon + 1;
	dp->da_day = tm->tm_mday;
#endif
}


long
dostounix(dp, tp)
struct date *dp;
struct time *tp;
{
static struct tm tm;
struct tm *tx;
long now;

	tm.tm_year = dp->da_year - 1900;
	tm.tm_mon = dp->da_mon - 1;
	tm.tm_mday = dp->da_day;
	tm.tm_hour = tp->ti_hour;
	tm.tm_min = tp->ti_min;
	tm.tm_sec = tp->ti_sec;
	/* This desperately needs to be fixed.  How? */
	(void) time (&now);
	tx = localtime (&now);
	tm.tm_isdst = tx->tm_isdst;
	return mktime (&tm);
}


/*****************************************************************************\
*			    Timers, I/O and scheduling			      *
\*****************************************************************************/

void
register_io(fd, event)
int fd;
void *event;
{
#if 0
struct io *evp;

	if ((evp = mallocw (sizeof *evp)) == (struct io *)0)	{
		tputs ("register_io: no memory for event\n");
		where_outta_here (1, "register_io");
	}
	evp->fd = fd;
	evp->event = event;
	evp->next = Events;
	Events = evp;
#endif
}


void
unregister_io(fd)
int fd;
{
#if 0
struct io *evp, *evc;

	for (evp = 0, evc = Events; evc->fd != fd; evp = evc, evc = evc->next)
		;
	if (!evc)	{
		tputs ("unregister_io: unknown fd\n");
		return;
	}
	if (evp)
		evp->next = evc->next;
	else
		Events = evc->next;
	j_free (evc);
#endif
}

#if 0
static void
ouch(int sig OPTIONAL)
{
struct sigaction sa;

	sa.sa_handler = SIG_DFL;
	sa.sa_flags = 0;
	(void) sigaction (SIGSEGV, &sa, (struct sigaction *) 0);
	(void) sigaction (SIGBUS, &sa, (struct sigaction *) 0);

	if (shall_we_crash())
		crash_it_already ("SIGSEGV signal");
}
#endif

#if 0
static void
ding(int i)
{
static struct timeval tv;
long oclock;
struct timeval *tvp;		/*lint -esym(550,tvp) */

	/* first a check for a locked up call to free() */
	if (within_a_free_call)	{
		if (++time_in_a_free_call > MAX_TIME_INA_FREE_CALL)	{
			within_a_free_call = 0;
			where_outta_here(3, "a libc free() lockup");
		}
	}

	/* do pending output */
	if (!i)		{
		tflush();
		rflush();
	}
	/* collect input events to wait on */
	/* get time until next timer; if zero, fake a very large one */
	/* if we have a nonzero argument, we're a timer tick; poll, don't block */
	if (i)	{
		tv.tv_sec = tv.tv_usec = 0;
		tvp = &tv;
	} else if (!Timers)
		tvp = 0;
	else {
		/* This section gets improperly optimized in GCC 4.6.x */
		tv.tv_sec = (Timers->expiration - Clock) * MSPTICK;
		if (tv.tv_sec <= 0)
			tv.tv_sec = 0;
		tv.tv_usec = (tv.tv_sec % 1000) * 1000;
		tv.tv_sec /= 1000;
		tvp = &tv;
	}
	/* check for I/O */
	/* run any due timers */
	ksignal ((volatile void *) &Tick, 1);
	/* and update the system time */
	oclock = Clock;
	Clock = msclock() / MSPTICK;
	Tick = Clock - oclock;
}
#endif

#if 0
static void
init_tick(void)
{
struct sigaction sa;
struct itimerval it;

	sa.sa_flags = 0;
	sa.sa_handler = ding;
	(void) sigaction (SIGALRM, &sa, (struct sigaction *) 0);
	it.it_interval.tv_sec = 0;
	it.it_interval.tv_usec = MSPTICK * 1000;
	it.it_value = it.it_interval;
	(void) setitimer (ITIMER_REAL, &it, (struct itimerval *) 0);
}
#endif

static void
deinit_tick(void)
{
#if 0
struct itimerval it;

	it.it_interval.tv_sec = it.it_interval.tv_usec = 0;
	it.it_value = it.it_interval;
	(void) setitimer (ITIMER_REAL, &it, (struct itimerval *) 0);
#endif
}

#if 0
static void
cleanup (int sig OPTIONAL)
{
	where_outta_here (0, "cleanup");
}
#endif

void
init_sys(int no_itimer)
{
#if 0
struct sigaction sa;

	init_time();
	register_io (0, &Keyboard);

	sa.sa_handler = ouch;
	sa.sa_flags = 0;
	(void) sigaction (SIGSEGV, &sa, (struct sigaction *) 0);

	sa.sa_handler = cleanup;
	sa.sa_flags = 0;
	(void) sigaction (SIGTERM, &sa, (struct sigaction *) 0);

	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	(void) sigaction (SIGWINCH, &sa, (struct sigaction *) 0);
	if (!no_itimer)
		init_tick();
#endif
}


void
deinit_sys()
{
	deinit_tick();
	unregister_io(0);
}


void
giveup()
{
#if 0
	/* suspend heartbeat */
	deinit_tick();
	/* block for I/O */
	ding (0);
	/* and reactivate the tick */
	init_tick();
#endif
}



static char **tnos_envp = NULLCHARP;

void setenvfrommain (char **frommain);

void
setenvfrommain (char **frommain)	{
	tnos_envp = frommain;
}
