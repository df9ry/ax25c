/*
 *  Project: ax25c - File: callsign.c
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

#include "../exception.h"

#include "callsign.h"
#include "_internal.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>

union _callsign {
	callsign encoded;
	uint8_t octets[7];
};

static void fillEx(struct exception *ex, const char *fn, const char *msg,
					const char *par, int erc)
{
	assert(fn);
	assert(msg);
	if (ex) {
		ex->erc = erc;
		ex->module = MODULE_NAME;
		ex->function = fn;
		ex->message = msg;
		ex->param = par ? par : "";
	}
}

static uint8_t getOctetOfChar(char ch)
{

	if ((ch >= 'a') && (ch <= 'z'))
		ch -= 0x20;
	if ( ((ch >= '0') && (ch <= '9')) || ((ch >= 'A') && (ch <= 'Z')) )
		return ch << 1;
	return 0;
}

static const char *__skipWhitespace(const char *str)
{
	while ((*str) && isspace(*str))
		++str;
	return str;
}

callsign callsignFromString(const char *str, const char **next,
							struct exception *ex)
{
	size_t i = 0;
	uint8_t octet;
	long int _ssid;
	char *_call;
	const char *_str = str;
	union _callsign c;
	bool with_uuid = false;

	assert(str);
	c.encoded = 0;
	while (*str != '\0') {
		if (isspace(*str)) {
			break;
		}
		if (*str == '-') {
			++str;
			with_uuid = true;
			break;
		}
		if (i > 5) {
			fillEx(ex, "callsignFromString",
					"Callsign too long (max. 6 characters)", _str,
					EXIT_FAILURE);
			return 0;
		}
		octet = getOctetOfChar(*str);
		if (octet == 0) {
			fillEx(ex, "callsignFromString",
					"Invalid callsign character", _str,
					EXIT_FAILURE);
			return 0;
		}
		c.octets[i] = octet;
		++str;
		++i;
	} /* end while */
	if (i == 0) {
		fillEx(ex, "callsignFromString",
				"Callsign too short (min. 1 character)", _str,
				EXIT_FAILURE);
		return 0;
	}
	while (i < 6) {
		c.octets[i] = 0x40;
		++i;
	} /* end while */
	if (with_uuid) {
		_ssid = strtol(str, &_call, 0);
		if ((_ssid < 0) || (_ssid >= 16)) {
			fillEx(ex, "callsignFromString",
					"SSID is out of range (0..15)", _str,
					EXIT_FAILURE);
			return 0;
		}
	} else {
		_call = (char*)str;
		_ssid = 0;
	}
	c.octets[6] = 0x60 | (_ssid << 1);
	if (next)
		*next = __skipWhitespace(_call);
	return c.encoded;
}

static int __tooShort(const char *func, struct exception *ex)
{
	assert(func);
	fillEx(ex, func, "Buffer too short", NULL, EXIT_FAILURE);
	return -1;
}

int callsignToString(callsign call, char *pb, size_t cb, struct exception *ex)
{
	union _callsign c;
	char ch;
	size_t i, cb1;
	int ssid;

	if (!call) {
		if (cb < 7)
			return __tooShort("callsignToString", ex);
		strcpy(pb, "<NULL>");
		return 6;
	}
	assert(call);
	assert(pb);
	assert(cb);
	cb1 = cb;
	c.encoded = call;
	for (i = 0; i < 6; ++i) {
		ch = (c.octets[i] & 0xfe) >> 1;
		if (ch != ' ') {
			if (cb-- == 0)
				return __tooShort("callsignToString", ex);
			*pb++ = ch;
		}
	} /* end for */
	if (cb-- == 0)
		return __tooShort("callsignToString", ex);
	*pb++ = '-';
	ssid = (c.octets[6] & 0x1e) >> 1;
	i = snprintf(pb, cb, "%i", ssid);
	if (i >= cb)
		return __tooShort("callsignToString", ex);
	cb -= i;
	pb += i;
	if (getHBit(call)) {
		if (cb-- == 0)
			return __tooShort("callsignToString", ex);
		*pb++ = '*';
	}
	if (cb-- == 0)
		return __tooShort("callsignToString", ex);
	*pb++ = '\0';
	return cb1 - cb - 1;
}

bool addressFieldFromString(callsign source, const char *dest,
							struct addressField *af, struct exception *ex)
{
	const char *next;

	assert(dest);
	assert(af);
	memset(af, 0x00, sizeof(struct addressField));
	af->source = source;
	dest = __skipWhitespace(dest);

	/* Read destination */
	af->destination = callsignFromString(dest, &next, ex);
	if (!af->destination)
		return false;
	dest = __skipWhitespace(next);
	if (!(*dest)) {
		setXBit(&af->source, true);
		goto done;
	}

	/* Read repeater 1 */
	af->repeaters[0] = callsignFromString(dest, &next, ex);
	if (!af->repeaters[0])
		return false;
	dest = __skipWhitespace(next);
	if (!(*dest)) {
		setXBit(&af->repeaters[0], true);
		goto done;
	}

	/* Read repeater 2 */
	af->repeaters[1] = callsignFromString(dest, &next, ex);
	if (!af->repeaters[1])
		return false;
	dest = __skipWhitespace(next);
	if (!(*dest)) {
		setXBit(&af->repeaters[1], true);
		goto done;
	}

	/* Error */
	if (ex) {
		ex->erc = EXIT_FAILURE;
		ex->module = MODULE_NAME;
		ex->function = "addressFieldFromString";
		ex->message = "Too many repeaters (max. 2)";
		ex->param = dest;
	}
	return false;

done:
	if (next[0]) {
		/* Error */
		if (ex) {
			ex->erc = EXIT_FAILURE;
			ex->module = MODULE_NAME;
			ex->function = "addressFieldFromString";
			ex->message = "Exceeding characters after callsign";
			ex->param = dest;
		}
		return false;

	}
	return true;
}

bool addressFieldToString(const struct addressField *af,
									char *pb, size_t cb,
									struct exception *ex)
{
	int cb1;

	assert(af);
	assert(pb);
	assert(cb);

	cb1 = callsignToString(af->destination, pb, cb, ex);
	if (cb1 < 0)
		return false;
	cb -= cb1;
	pb += cb1;
	if (cb++ == 0)
		return (__tooShort("addressFieldToString", ex) == 0);
	if (getXBit(af->source)) {
		*pb = '\0';
		return true;
	} else {
		*pb++ = ' ';
	}

	cb1 = callsignToString(af->repeaters[0], pb, cb, ex);
	if (cb1 < 0)
		return false;
	cb -= cb1;
	pb += cb1;
	if (cb++ == 0)
		return (__tooShort("addressFieldToString", ex) == 0);
	if (getXBit(af->repeaters[0])) {
		*pb++ = '\0';
		return true;
	} else {
		*pb++ = ' ';
	}

	cb1 = callsignToString(af->repeaters[1], pb, cb, ex);
	if (cb1 < 0)
		return false;
	cb -= cb1;
	pb += cb1;
	if (cb++ == 0)
		return (__tooShort("addressFieldToString", ex) == 0);
	if (getXBit(af->repeaters[1])) {
		*pb++ = '\0';
		return true;
	}
	if (cb < 5)
		return (__tooShort("addressFieldToString", ex) == 0);

	strncpy(pb, " ...", cb);
	return true;
}
