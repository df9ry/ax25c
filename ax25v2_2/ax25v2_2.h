/*
 *  Project: ax25c - File: ax25v2_2.h
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

#ifndef AX25V2_2_AX25V2_2_H_
#define AX25V2_2_AX25V2_2_H_

#include "../runtime/primitive.h"

struct exception;
struct addressField;

/**
 * @brief AX25 commands.
 */
enum {
	AX25_I     = 0x00, /**< Information Command Frame */
	AX25_RR    = 0x01, /**< Receive Ready.            */
	AX25_RNR   = 0x05, /**< Receive Not Ready.        */
	AX25_REJ   = 0x09, /**< Reject.                   */
	AX25_SREJ  = 0x0d, /**< Selective Reject.         */
	AX25_SABME = 0x6f, /**< Set Async Balanced Mode.  */
	AX25_SABM  = 0x2f, /**< Set Async Balanced Mode.  */
	AX25_DISC  = 0x43, /**< Disconnect.               */
	AX25_DM    = 0x0f, /**< Disconnect Mode.          */
	AX25_UA    = 0x63, /**< Unnumbered Acknowledge.   */
	AX25_FRMR  = 0x87, /**< Frame Reject.             */
	AX25_UI    = 0x03, /**< Unnumbered Acknowledge.   */
	AX25_XID   = 0xaf, /**< Exchange Identification.  */
	AX25_TEST  = 0xe3, /**< Test.                     */
} AX25_CMD;
typedef enum AX25_CMD AX25_CMD_t;

/**
 * @brief Create a AX25 I frame.
 * @param af AddressField.
 * @param nr N(R) variable.
 * @param ns N(S) variable.
 * @param data Pointer to payload data.
 * @param size Size of payload data.
 * @param ex Exception structure.
 * @return New primitive containing a AX25 frame in payload.
 */
extern primitive_t *new_AX25_I(
		struct addressField *af,
		uint8_t nr, uint8_t ns,
		uint8_t *data, size_t size,
		struct exception *ex);

/**
 * @brief Create a AX25 I frame.
 * @param af AddressField.
 * @param nr N(R) variable.
 * @param ns N(S) variable.
 * @param data Pointer to payload data.
 * @param size Size of payload data.
 * @param ex Exception structure.
 * @return New primitive containing a AX25 frame in payload.
 */
extern primitive_t *new_AX25_Supervisory(
		AX25_CMD_t cmd,
		struct addressField *af,
		uint8_t nr,
		bool poll,
		uint8_t *data, size_t size,
		struct exception *ex);

static inline primitive_t *new_AX25_RR(
		struct addressField *af,
		uint8_t nr, bool poll,
		struct exception *ex)
{
	return new_AX25_Supervisory(AX25_RR, af, nr, poll, ex);
}

static inline primitive_t *new_AX25_RNR(
		struct addressField *af,
		uint8_t nr, bool poll,
		struct exception *ex)
{
	return new_AX25_Supervisory(AX25_RNR, af, nr, poll, ex);
}

static inline primitive_t *new_AX25_REJ(
		struct addressField *af,
		uint8_t nr, bool poll,
		struct exception *ex)
{
	return new_AX25_Supervisory(AX25_REJ, af, nr, poll, ex);
}

static inline primitive_t *new_AX25_SREJ(
		struct addressField *af,
		uint8_t nr, bool poll,
		struct exception *ex)
{
	return new_AX25_Supervisory(AX25_SREJ, af, nr, poll, ex);
}

extern primitive_t *new_AX25_Unnumbered(
		AX25_CMD_t cmd,
		struct addressField *af,
		bool poll,
		uint8_t *data, size_t size,
		struct exception *ex);

static inline primitive_t *new_AX25_SABME(
		struct addressField *af,
		struct exception *ex)
{
	return new_AX25_Supervisory(AX25_SABME, af, true, NULL, 0, ex);
}

static inline primitive_t *new_AX25_SABM(
		struct addressField *af,
		struct exception *ex)
{
	return new_AX25_Supervisory(AX25_SABM, af, true, NULL, 0, ex);
}

static inline primitive_t *new_AX25_DISC(
		struct addressField *af,
		struct exception *ex)
{
	return new_AX25_Supervisory(AX25_DISC, af, true, NULL, 0, ex);
}

static inline primitive_t *new_AX25_DM(
		struct addressField *af,
		struct exception *ex)
{
	return new_AX25_Supervisory(AX25_DM, af, false, NULL, 0, ex);
}

static inline primitive_t *new_AX25_UA(
		struct addressField *af,
		struct exception *ex)
{
	return new_AX25_Supervisory(AX25_UA, af, false, NULL, 0, ex);
}

static inline primitive_t *new_AX25_FRMR(
		struct addressField *af,
		struct exception *ex)
{
	return new_AX25_Supervisory(AX25_FRMR, af, false, NULL, 0, ex);
}

static inline primitive_t *new_AX25_UI(
		struct addressField *af,
		bool poll,
		uint8_t *data, size_t size,
		struct exception *ex)
{
	return new_AX25_Supervisory(AX25_UI, af, poll, data, size, ex);
}

static inline primitive_t *new_AX25_XID(
		struct addressField *af,
		bool poll,
		uint8_t *data, size_t size,
		struct exception *ex)
{
	return new_AX25_Supervisory(AX25_XID, af, poll, data, size, ex);
}

static inline primitive_t *new_AX25_TEST(
		struct addressField *af,
		bool poll,
		uint8_t *data, size_t size,
		struct exception *ex)
{
	return new_AX25_Supervisory(AX25_TEST, af, poll, data, size, ex);
}




#endif /* AX25V2_2_AX25V2_2_H_ */
