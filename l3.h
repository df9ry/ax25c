/*
 *  Project: ax25c - File: l3.h
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

#ifndef L3_H_
#define L3_H_

/**
 * @brief L3 protocols.
 */
enum L3_PROTOCOL {
	L3_IMPL1 = 0x10, /**< AX.25 layer 3 implemented .                         */
	L3_IMPL2 = 0x20, /**< AX.25 layer 3 implemented.                          */
	L3_ISO   = 0x01, /**< ISO 8208/CCITT X.25 PLP.                            */
	L3_CTCP  = 0x06, /**< Compressed TCP/IP packet. Van Jacobson (RFC 1144)   */
	L3_UTCP  = 0x07, /**< Uncompressed TCP/IP packet. Van Jacobson (RFC 1144) */
	L3_SEGF  = 0x08, /**< Segmentation fragment.                              */
	L3_TEXN  = 0xc3, /**< TEXNET datagram protocol.                           */
	L3_LQP   = 0xc4, /**< Link Quality Protocol.                              */
	L3_APPLE = 0xca, /**< Appletalk.                                          */
	L3_APARP = 0xcb, /**< Appletalk ARP.                                      */
	L3_ARPAI = 0xcc, /**< ARPA Internet Protocol.                             */
	L3_ARPAA = 0xcd, /**< ARPA Address resolution.                            */
	L3_FLEXN = 0xce, /**< FlexNet.                                            */
	L3_NETRM = 0xcf, /**< NET/ROM.                                            */
	L3_NPROT = 0xf0, /**< No layer 3 protocol implemented.                    */
	L3_ESC   = 0xff  /**< Escape character. Next octet contains more info.    */
};

#endif /* L3_H_ */
