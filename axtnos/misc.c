/*
 *  Project: ax25c - File: misc.c
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

/* Miscellaneous machine independent utilities
 * Copyright 1991 Phil Karn, KA9Q
 */
#include "global.h"

static int htob (char c);
const char *tcp_port_name (int portnum);

extern int Mprunning;

char Whitespace[] = " \t\r\n";

struct port_table {
	const char *name;
	int port;
};

#if 0
static struct port_table tcp_port_table[] =
{
	{ "*",		0 },
	{ "bootpcli",	IPPORT_BOOTPC },
	{ "bootpsvr",	IPPORT_BOOTPS },
	{ "callbook",	IPPORT_CALLDB },
	{ "cbs",	IPPORT_CBS },
	{ "convers",	IPPORT_CONVERS },
	{ "daytime",	IPPORT_DAYTIME },
	{ "discard",	IPPORT_DISCARD },	/* ARPA discard protocol */
	{ "domain",	IPPORT_DOMAIN },	/* ARPA domain nameserver */
	{ "echo",	IPPORT_ECHO },		/* ARPA echo protocol */
	{ "finger",	IPPORT_FINGER },	/* ARPA finger protocol */
	{ "fbbtelnet",	IPPORT_FTELNET },
	{ "ftp",	IPPORT_FTP },		/* ARPA file transfer protocol (cmd) */
	{ "ftp-data",	IPPORT_FTPD },		/* ARPA file transfer protocol (data) */
	{ "http",	IPPORT_HTTP },
	{ "info",	IPPORT_INFO },
	{ "netupds",	4715 },
	{ "news",	IPPORT_NEWS },
	{ "nntp",	IPPORT_NNTP },
	{ "pop2",	IPPORT_POP2 },		/* Post Office Prot. v2 */
	{ "pop3",	IPPORT_POP3 },		/* Post Office Prot. v3 */
	{ "quote",	IPPORT_QUOTE },
	{ "remote",	IPPORT_REMOTE },
	{ "rip",	IPPORT_RIP },
	{ "rlogin",	IPPORT_RLOGIN },
	{ "rwho",	IPPORT_RWHO },
	{ "smtp",	IPPORT_SMTP },		/* ARPA simple mail transfer protocol */
	{ "telnet",	IPPORT_TELNET },	/* ARPA virtual terminal protocol */
	{ "term",	IPPORT_TERM },		/* Serial interface server port */
	{ "tftpd",	IPPORT_TFTPD },
	{ "time",	IPPORT_TIME },
	{ "timed",	IPPORT_TIMED },
	{ "trace",	IPPORT_TRACE },
	{ "ttylink",	IPPORT_TTYLINK },
	{ "tutor",	IPPORT_TUTOR },
	{ "xconvers",	IPPORT_XCONVERS },
	{ "xwindows",	IPPORT_X },
	{ NULLCHAR,	0 }
};
#endif

#if 0
/* convert a tcp port number or name to integer - WG7J */
int
atoip (s)
char *s;
{
struct port_table *port = tcp_port_table;
int p;
size_t n;

	if (!s)
		return 0;
	if ((p = atoi (s)) == 0) {
		n = strlen (s);
		for (; port->name; port++) {
			if (!strncmp (s, port->name, n)) {
				p = port->port;
				break;
			}
		}
	}
	return p;
}
#endif

#if 0
const char *
tcp_port_name (int portnum)
{
struct port_table *port = tcp_port_table;
static char buf[11];

	for (; port->name; port++) {
		if (port->port == portnum)
			return (port->name);
	}
	sprintf (buf, "%u", portnum);
	return (buf);
}
#endif


/* Select from an array of strings, or return ascii number if out of range */
const char *
smsg (msgs, nmsgs, n)
const char *msgs[];
unsigned nmsgs, n;
{
static char buf[16];

	if (n < nmsgs && msgs[n] != NULLCHAR)
		return msgs[n];
	sprintf (buf, "%u", n);
	return buf;
}

/* Convert hex-ascii to integer */
int
htoi (s)
const char *s;
{
int i = 0;
char c;

	while ((c = *s++) != '\0') {
		if (c == 'x')
			continue;	/* allow 0x notation */
		if ('0' <= c && c <= '9')
			i = (i * 16) + (c - '0');
		else if ('a' <= c && c <= 'f')
			i = (i * 16) + (c - 'a' + 10);
		else if ('A' <= c && c <= 'F')
			i = (i * 16) + (c - 'A' + 10);
		else
			break;
	}
	return i;
}

/* Convert single hex-ascii character to binary */
static int
htob (c)
char c;
{
	if ('0' <= c && c <= '9')
		return c - '0';
	else if ('a' <= c && c <= 'f')
		return c - 'a' + 10;
	else if ('A' <= c && c <= 'F')
		return c - 'A' + 10;
	else
		return -1;
}

/* Read an ascii-encoded hex string, convert to binary and store in
 * output buffer. Return number of bytes converted
 */
int
readhex (out, in, size)
char *out, *in;
int size;
{
int c, count;

	if (in == NULLCHAR)
		return 0;
	for (count = 0; count < size; count++) {
		while (*in == ' ' || *in == '\t')
			in++;			/* Skip white space */
		if ((c = htob (*in++)) == -1)
			break;			/* Hit non-hex character */
		out[count] = (char) (c << 4);	/*lint !e701 * First nybble */
		while (*in == ' ' || *in == '\t')
			in++;			/* Skip white space */
		if ((c = htob (*in++)) == -1)
			break;			/* Hit non-hex character */
		out[count] |= (char) c;		/* Second nybble */
	}
	return count;
}

/* replace terminating end of line marker(s) with null */
void
rip (s)
register char *s;
{
register char *cp;

	while ((cp = strchr (s, '\n')) != NULLCHAR || (cp = strchr (s, '\r')) != NULLCHAR)
		*cp = '\0';
}

/* Host-network conversion routines, replaced on the x86 with
 * assembler code in pcgen.asm
 */
/* Put a long in host order into a char array in network order */
unsigned char *
put32 (cp, x)
register unsigned char *cp;
uint32 x;
{
	*cp++ = uchar((x >> 24) & 0xff);
	*cp++ = uchar((x >> 16) & 0xff);
	*cp++ = uchar((x >> 8) & 0xff);
	*cp++ = uchar(x & 0xff);
	return cp;
}

/* Put a short in host order into a char array in network order */
unsigned char *
put16 (cp, x)
register unsigned char *cp;
int16 x;
{
	*cp++ = uchar((x >> 8) & 0xff);
	*cp++ = uchar(x & 0xff);
	return cp;
}

int16
get16 (cp)
register char *cp;
{
register int16 x;

	x = uchar (*cp++);
	x <<= 8;
	x |= uchar (*cp);
	return x;
}

/* Machine-independent, alignment insensitive network-to-host long conversion */
uint32
get32 (cp)
register char *cp;
{
uint32 rval;

	rval = uchar (*cp++);
	rval <<= 8;
	rval |= uchar (*cp++);
	rval <<= 8;
	rval |= uchar (*cp++);
	rval <<= 8;
	rval |= uchar (*cp);

	return rval;
}

void
kmutex_lock (int *key)
{
#if 0
	while (Mprunning == 1 && *key == TNOS_MUTEX_LOCKED)
		kwait (key);
	*key = TNOS_MUTEX_LOCKED;
#endif
}

void
kmutex_unlock (int *key)
{
#if 0
	int errnosave = errno;

	*key = TNOS_MUTEX_UNLOCKED;
	if (Mprunning == 1)	{
		ksignal (key, 1);
		kwait (NULL);
	}
	errno = errnosave;
#endif
}
