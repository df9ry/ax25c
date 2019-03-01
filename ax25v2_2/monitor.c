/*
 *  Project: ax25c - File: monitor.c
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

#include "../runtime/monitor.h"
#include "../runtime/primitive.h"
#include "../runtime/runtime.h"

#include "callsign.h"
#include "monitor.h"
#include "ax25v2_2.h"
#include "_internal.h"

#include <ctype.h>
#include <errno.h>

static int _ax25_monitor_provider(struct primitive *prim, char *pb, size_t cb)
{
	int i, _i, j = 0, k, _cb = cb, nr, ns;
	callsign call;
	bool x_bit, h_bit;
	uint8_t o;

	assert(prim);
	assert(pb);
	assert(prim->protocol == AX25);
	pb[0] = '\0';
	i = monitor_put_str("AX25: ", pb, _cb);
	if (!prim_check_AX25_CRC(prim)) {
		i += snprintf(&pb[i], _cb-i, "CRC invalid (%i byte): ", prim->size);
		goto data;
	}
	if (_cb < 17) {
		i += snprintf(&pb[i], _cb-i, "Frame too short (%i byte): ", prim->size);
		i += monitor_put_dump(prim->payload, prim->size, &pb[i], _cb-i);
		return i;
	}
	call = callsignFromFrame(&prim->payload[7]);
	x_bit = getXBit(call);
	_i = callsignToString(call, &pb[i], _cb-i, NULL);
	if (_i < 0) {
		i += snprintf(&pb[i], _cb-i, "...");
		return i;
	}
	i += _i;
	i += monitor_put_str("->", &pb[i], _cb-1);
	call = callsignFromFrame(&prim->payload[0]);
	_i = callsignToString(call, &pb[i], _cb-i, NULL);
	if (_i < 0) {
		i += snprintf(&pb[i], _cb-i, "...");
		return i;
	}
	i += _i;
	if (!x_bit) {
		i += monitor_put_str(" via ", &pb[i], _cb-1);
		if (i >= _cb)
			return i;
	}
	j = 14;
	while (!x_bit) {
		if (j+9 >= prim->size) {
			i += snprintf(&pb[i], _cb-i, "Too many digipeaters");
			return i;
		}
		call = callsignFromFrame(&prim->payload[j]);
		x_bit = getXBit(call);
		h_bit = getHBit(call);
		_i = callsignToString(call, &pb[i], _cb-i, NULL);
		if (_i < 0) {
			i += snprintf(&pb[i], _cb-i, "...");
			return i;
		}
		j += 7;
		if (h_bit)
			i += monitor_put_str("* ", &pb[i], _cb-i);
		else
			i += monitor_put_str(" ", &pb[i], _cb-1);
		if (_i < 0) {
			i += snprintf(&pb[i], _cb-i, "...");
			return i;
		}
	} /* end while */
	if (j >= prim->size)
		return i;
	o = prim->payload[j++];
	if (j >= prim->size)
		return i;
	if ((o & 0x01) == 0x00) { /* I frame */
		i += monitor_put_str(" I ", &pb[i], _cb-i);
		if (prim->flags & AX25_MODULO_KNOWN) {
			if (prim->flags & AX25_MODULO_128) {
				ns = (o >> 1) & 0x7f;
				if (j >= prim->size)
					return i;
				o = prim->payload[j++];
				nr = (o >> 1) & 0x7f;
				if (o & 0x01)
					i += monitor_put_str("P", &pb[i], _cb-i);
				else
					i += monitor_put_str("F", &pb[i], _cb-i);
				if (prim_get_AX25_C(prim))
					i += monitor_put_str("+ ", &pb[i], _cb-i);
				else
					i += monitor_put_str("- ", &pb[i], _cb-i);
				i += snprintf(&pb[i], _cb-i, "(%03i/%03i)", nr, ns);
			} else {
				if (o & 0x10)
					i += monitor_put_str("P", &pb[i], _cb-i);
				else
					i += monitor_put_str("F", &pb[i], _cb-i);
				nr = (o >> 5) & 0x07;
				ns = (o >> 1) & 0x07;
				if (prim_get_AX25_C(prim))
					i += monitor_put_str("+ ", &pb[i], _cb-i);
				else
					i += monitor_put_str("- ", &pb[i], _cb-i);
				i += snprintf(&pb[i], _cb-i, "(%01i/%01i)", nr, ns);
			}
			if (j >= prim->size)
				return i;
			o = prim->payload[j++];
			i += snprintf(&pb[i], _cb-i, "[%02x] ", o);
			i += monitor_put_info(&prim->payload[j], prim->size-j-2, &pb[i], cb-i);
			i += monitor_put_str(" ", &pb[i], _cb-i);
			goto data;
		} else {
			i += monitor_put_str("?", &pb[i], _cb-i);
			if (prim_get_AX25_C(prim))
				i += monitor_put_str("+ ", &pb[i], _cb-i);
			else
				i += monitor_put_str("- ", &pb[i], _cb-i);
			goto data;
		}
	} else if ((o & 0x03) == 0x01) { /* S frame */
		switch (o & 0x0f) {
		case 0x01:
			i += monitor_put_str(" RR ", &pb[i], _cb-i);
			break;
		case 0x05:
			i += monitor_put_str(" RNR ", &pb[i], _cb-i);
			break;
		case 0x09:
			i += monitor_put_str(" REJ ", &pb[i], _cb-i);
			break;
		case 0x0d:
			i += monitor_put_str(" SREJ ", &pb[i], _cb-i);
			break;
		default:
			i += monitor_put_str(" S? ", &pb[i], _cb-i);
			break;
		} /* end switch */
		if (prim->flags & AX25_MODULO_KNOWN) {
			if (prim->flags & AX25_MODULO_128) {
				if (j >= prim->size)
					return i;
				o = prim->payload[j++];
				nr = (o >> 1) & 0x7f;
				if (o & 0x01)
					i += monitor_put_str("P", &pb[i], _cb-i);
				else
					i += monitor_put_str("F", &pb[i], _cb-i);
				if (prim_get_AX25_C(prim))
					i += monitor_put_str("+ ", &pb[i], _cb-i);
				else
					i += monitor_put_str("- ", &pb[i], _cb-i);
				i += snprintf(&pb[i], _cb-i, "(%03i)", nr);
			} else {
				if (o & 0x10)
					i += monitor_put_str("P", &pb[i], _cb-i);
				else
					i += monitor_put_str("F", &pb[i], _cb-i);
				nr = (o >> 5) & 0x07;
				if (prim_get_AX25_C(prim))
					i += monitor_put_str("+ ", &pb[i], _cb-i);
				else
					i += monitor_put_str("- ", &pb[i], _cb-i);
				i += snprintf(&pb[i], _cb-i, "(%01i)", nr);
			}
		} else {
			i += monitor_put_str("?", &pb[i], _cb-i);
			if (prim_get_AX25_C(prim))
				i += monitor_put_str("+ ", &pb[i], _cb-i);
			else
				i += monitor_put_str("- ", &pb[i], _cb-i);
		}
	} else if ((o & 0x03) == 0x03) { /* U frame */
		switch (o & 0xef) {
		case 0x6f:
			i += monitor_put_str(" SABME ", &pb[i], _cb-i);
			break;
		case 0x2f:
			i += monitor_put_str(" SABM ", &pb[i], _cb-i);
			break;
		case 0x43:
			i += monitor_put_str(" DISC ", &pb[i], _cb-i);
			break;
		case 0x0f:
			i += monitor_put_str(" DM ", &pb[i], _cb-i);
			break;
		case 0x63:
			i += monitor_put_str(" UA ", &pb[i], _cb-i);
			break;
		case 0x87:
			i += monitor_put_str(" FRMR ", &pb[i], _cb-i);
			break;
		case 0x03:
			i += monitor_put_str(" UI ", &pb[i], _cb-i);
			break;
		case 0xaf:
			i += monitor_put_str(" XID ", &pb[i], _cb-i);
			break;
		case 0xe3:
			i += monitor_put_str(" TEST ", &pb[i], _cb-i);
			break;
		default:
			i += monitor_put_str(" U? ", &pb[i], _cb-i);
			break;
		} /* end switch */
		if (o & 0x10)
			i += monitor_put_str("P", &pb[i], _cb-i);
		else
			i += monitor_put_str("F", &pb[i], _cb-i);
		if (prim_get_AX25_C(prim))
			i += monitor_put_str("+ ", &pb[i], _cb-i);
		else
			i += monitor_put_str("- ", &pb[i], _cb-i);
		if ((o & 0xef) == 0x03) { /* UI */
			if (j >= prim->size)
				return i;
			o = prim->payload[j++];
			i += snprintf(&pb[i], _cb-i, "[%02x] ", o);
		}
	}
data:
	k = prim->size - j - 2;
	if (k > 0) {
		i += snprintf(&pb[i], _cb-i, "(%i byte) ", k);
		i += monitor_put_info(&prim->payload[j], prim->size-j-2, &pb[i], _cb-i);
		i += monitor_put_str(" ", &pb[i], _cb-i);
		i += monitor_put_dump(&prim->payload[j], prim->size-j-2, &pb[i], _cb-i);
	}
	return i;
}

static int ax25_monitor_provider(struct primitive *prim, char *pb, size_t cb,
		struct exception *ex)
{
	int res;

	if (cb < 5) {
		exception_fill(ex, ENOMEM, MODULE_NAME, "ax25_monitor_provider",
				"buffer too small (< 5)", "");
		return -ENOMEM;
	}
	res = _ax25_monitor_provider(prim, pb, cb);
	if (res < 0) {
		exception_fill(ex, EINVAL, MODULE_NAME, "ax25_monitor_provider",
				"internal error", "");
		return -EINVAL;
	}
	if (res+4 >= cb) {
		memcpy(&pb[cb-5], "...", 4);
		return cb-1;
	} else {
		pb[res] = '\0';
		return res;
	}
}

bool ax25v2_2_monitor_init(struct exception *ex)
{
	return register_monitor_provider(AX25, ax25_monitor_provider, ex);
}

bool ax25v2_2_monitor_dest(struct exception *ex)
{
	return unregister_monitor_provider(AX25, ax25_monitor_provider, ex);
}
