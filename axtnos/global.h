/*
 *  Project: ax25c - File: global.h
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

/** @file global.h */

#ifndef AXTNOS_GLOBAL_H_
#define AXTNOS_GLOBAL_H_

#ifdef TNOS_MALLOC
#define MALLOC_DEBUG
#endif

#define _HAVE_STRING_ARCH_strdup
#define _HAVE_STRING_ARCH_strndup
#define _HAVE_STRING_ARCH_strcpy
#define _HAVE_STRING_ARCH_strncpy

#include <assert.h>

#ifndef _SYSTEM_H
#include "system.h"
#endif

#if defined(USE_ALT_SYS_ERRLIST)
extern char * sys_errlist (int x);
#define SYS_ERRLIST(x)  	sys_errlist(x)
#endif

#ifndef SYS_ERRLIST
#define SYS_ERRLIST(x)		sys_errlist[x]
#endif

#ifndef STACKPADSIZE
#define STACKPADSIZE 8192
#endif

#ifdef _lint
# define IFLINT(x) x
#else
# define IFLINT(x)
#endif

/* Global definitions used by every source file.
 * Some may be compiler dependent.
 */

#ifndef _STDIO_H
# include <stdio.h>
#endif
#undef  _STDIO_H_
#define _STDIO_H_	1

#if !defined(_STDLIB_H)
# include <stdlib.h>
#endif
#undef  _STDLIB_H_
#define _STDLIB_H_

#ifndef _STRING_H
# include <string.h>
#endif
#undef  _STRING_H_
#define _STRING_H_	1

#define O_BINARY 0
#ifndef _SYS_TYPES_H
# include <sys/types.h>
#endif

#ifndef TNOS_TIMEZONE
#define TNOS_TIMEZONE timezone
#endif

struct ax25_cb;
struct iface;
struct ip;
struct mbx;
struct mbuf;
struct nr4cb;
struct session;

#ifndef __STR
# define __STR(x)	catalog(CAT, x)
#endif

#define	READ_BINARY	    "rb"
#define	WRITE_BINARY	"wb"
#define	APPEND_BINARY	"ab+"
/* DON'T change these! Using ones with 't' make UPDATE_TEXT and
   CREATE_TEXT *NOT* be able to write on some OSs! */
#define	READ_TEXT	"r"
#define	WRITE_TEXT	"w"
#define	APPEND_TEXT	"a+"
#define UPDATE_TEXT	"r+"
#define CREATE_TEXT	"w+"

/* These two lines assume that your compiler's longs are 32 bits and
 * shorts are 16 bits. It is already assumed that chars are 8 bits,
 * but it doesn't matter if they're signed or unsigned.
 */
typedef long int32;		/* 32-bit signed integer */
typedef unsigned long uint32;	/* 32-bit unsigned integer */
typedef unsigned short int16;	/* 16-bit unsigned integer */
typedef unsigned char byte_t;	/*  8-bit unsigned integer */
typedef unsigned char uint8;	/*  8-bit unsigned integer */
#define	uchar(x) ((unsigned char)(x))
#define	MAXINT16 65535		/* Largest 16-bit integer */
#ifdef __GNUC__
# define MAXINT32 4294967295UL    /* Largest 32-bit integer */
#else
# define MAXINT32 4294967295L    /* Largest 32-bit integer */
#endif
#undef NBBY
#define	NBBY	8		/* 8 bits/byte */

#define	HASHMOD	7		/* Modulus used by hash_ip() function */

/* Since not all compilers support structure assignment, the ASSIGN()
 * macro is used. This controls how it's actually implemented.
 */
#ifdef	NOSTRUCTASSIGN	/* Version for old compilers that don't support it */
# define ASSIGN(a,b)	memcpy((char *)&(a),(char *)&(b),sizeof(b));
#else			/* Version for compilers that do */
# define ASSIGN(a,b)	((a) = (b))
#endif

/* Define null object pointer in case stdio.h isn't included */
#ifndef	NULL
/* General purpose NULL pointer */
# define NULL (void *)0
#endif

#define	NULLCHAR	(char *)0	/* Null character pointer */
#define	NULLCHARP	(char **)0	/* Null character pointer pointer */
#define	NULLINT		(int *)0	/* Null integer pointer */
#define NULLFP(x) 	(int (*)x)0
#define NULLVFP(x)	(void (*)x)0
#define	NULLVIFP	(void (*)())0
#define	NULLFILE	(FILE *)0	/* Null file pointer */
#define	NULLFILEPTR	(FILE **)0	/* pointer to Null file pointer */

/* standard boolean constants */
#define FALSE 0
#define TRUE 1
#define NO 0
#define YES 1

#define CTLA 0x1
#define CTLB 0x2
#define CTLC 0x3
#define CTLD 0x4
#define CTLE 0x5
#define CTLF 0x6
#define CTLG 0x7
#define CTLH 0x8
#define CTLI 0x9
#define CTLJ 0xa
#define CTLK 0xb
#define CTLL 0xc
#define CTLM 0xd
#define CTLN 0xe
#define CTLO 0xf
#define CTLP 0x10
#define CTLQ 0x11
#define CTLR 0x12
#define CTLS 0x13
#define CTLT 0x14
#define CTLU 0x15
#define CTLV 0x16
#define CTLW 0x17
#define CTLX 0x18
#define CTLY 0x19
#define CTLZ 0x1a

#define	BS	CTLH
#define	TAB	CTLI
#define	LF	CTLJ
#define	FF	CTLL
#define	CR	CTLM
#define	XON	CTLQ
#define	XOFF	CTLS
#define	ESC	0x1b
#define	DEL	0x7f

#define TNOS_MUTEX_LOCKED	911
#define TNOS_MUTEX_UNLOCKED	7373

void textattr (int color);
void textbackground (int color);
void textcolor (int color);
void textrefresh (void);
#ifdef BSD_RANDOM
# define SRANDOM(n) srandom(n)
# define RANDOM(n) ((int) (random() * (n)))
#else
# ifdef sun
   extern double drand48(void);
# endif
# define SRANDOM(n) srand48(n)
# define RANDOM(n) ((int) (drand48() * (n)))
#endif
  /* !@#$%&* DOS quote-C-unquote dialects! ++bsa */
#define strcmpi strcasecmp
#define stricmp strcasecmp
#define strncmpi strncasecmp
#define strnicmp strncasecmp
  /* and work around a collision which is currently making me drop core... */
#undef tputs
#define tputs j_tputs
  /* some older systems lack strtoul(); we'll just have to hope this is okay */
#ifdef NO_STRTOUL
# define strtoul(s,cp,b) ((unsigned long) strtol((s),(cp),(b)))
#endif

int tputs (const char *s);

#ifndef NO_TNOSFOPEN
# undef fopen
# define fopen tnosfopen
# undef fclose
# define fclose tnosfclose
# undef tmpfile
# define tmpfile tnostmpfile

  FILE *tnosfopen (const char *fname, const char *mode);
  int tnosfclose (FILE *stream);
  FILE *tnostmpfile (void);

#endif

/* string equality shorthand */
#define STREQ(x,y) (strcmp(x,y) == 0)

/* Extract a short from a long */
#ifndef hiword
#define	hiword(x)	((int16)((x) >> 16))
#endif
#ifndef loword
#define	loword(x)	((int16)(x))
#endif

/* Extract a byte from a short */
#ifndef hibyte
#define	hibyte(x)	((unsigned char)((x) >> 8))
#endif
#ifndef lobyte
#define	lobyte(x)	((unsigned char)(x))
#endif

/* Extract nibbles from a byte */
#ifndef hinibble
#define	hinibble(x)	(((x) >> 4) & 0xf)
#endif
#ifndef lonibble
#define	lonibble(x)	((x) & 0xf)
#endif

/* Various low-level and miscellaneous functions */
int kpause (const char *filename, int lineno, int32 ms);
void *callocw (unsigned nelem,unsigned size);
int disable (void);
int enable (void);
int atoip (char *);
int htoi (const char *);
long htol (char *);
int16 hash_ip (uint32 addr);
int istate (void);
void tnoslog (int s,const char *fmt, ...);
void simple_log (int s,const char *str);
void *mallocw (unsigned nb);
void restore (int);
void rflush (void);
void rip (char *);
const char *smsg (const char *msgs[],unsigned nmsgs,unsigned n);
int wildmat (const char *s,char *p,char **argv);
int tprintf (const char *fmt,...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 1, 2)))
#endif
    ;

  /* not sure how NOS will work with the GNU one... */
#define ptol(p) ((long)(p))
#include <unistd.h>

/* General purpose function macros already defined in turbo C */
#ifndef	min
# define min(x,y)	((x)<(y)?(x):(y))	/* Lesser of two args */
#endif

#ifndef max
# define max(x,y)	((x)>(y)?(x):(y))	/* Greater of two args */
#endif

#ifndef _ERRNO_H
#include <errno.h>
#endif

#ifdef BROKE_SPRINTF
# define SPRINTF(x) (int)strlen(sprintf x)
# define VSPRINTF(x) (int)strlen((char *)vsprintf x)
#else
# define SPRINTF(x) sprintf x
# define VSPRINTF(x) vsprintf x
#endif

#if 0
/* Externals used by getopt */
extern int optind;
extern char *optarg;
#endif

/* System clock - count of ticks since startup */
extern volatile int32 Clock;

/* Various useful standard error messages */
extern char Badhost[];
extern char Badinterface[];
extern char Existingiface[];
extern char Nospace[];
extern char Notval[];
extern char Version[];
extern char Nosversion[];
extern char *Hostname;

/* Your system's end-of-line convention */
extern const char Eol[];

  /* PCs have a few pseudo-"standard" library functions used by NOS */
  extern char *stpcpy (char *, const char *);		/*lint !e762 */
  extern char *strlwr (char *);
  extern char *strupr (char *);
  extern char *itoa (int, char *, int);

/* Your system OS - set in files.c */
extern const char System[];

/* I know this is cheating, but it definitely makes sure that
 * each module has config.h included ! - WG7J
 */
#ifndef _CONFIG_H
# include "config.h"
#endif

#ifndef _UNIXTM_H
  /* this is separate so unix.c can load it without global.h */
# include "unixtm.h"
#endif

int getopt (int argc,char * const *argv,const char *opts);

/* in main.c  */
void where_outta_here (int resetme, char const *where);

#ifndef MALLOC_DEBUG
  /* can't do this above, GNU libc defines malloc to gnu_malloc in stdlib.h */
# undef malloc
  /*lint -save -e652 */
# define malloc mallocw
  /*lint -restore */
  /* minimal malloc checking is done, so intercept free() */
# undef free
  /*lint -save -e652 */
# define free j_free
  /*lint -restore */
# ifdef J_FREE_HACK
   void j_free (void *);
# else
   void j_free (const void *);
# endif
#else
# define mallocw malloc
# define callocw calloc
# define j_free free
#endif

#include "rmalloc.h"
#define freeIfSet(x) if (x) free (x);

#endif /* AXTNOS_GLOBAL_H_ */
