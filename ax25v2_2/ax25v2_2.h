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
enum AX25_CMD {
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
	AX25_INVAL = 0xff  /**< Invalid AX25 command.     */
};
typedef enum AX25_CMD AX25_CMD_t;

/**
 * @brief Create a AX25 I frame.
 * @param cH Client Handle.
 * @param sH Server Handle.
 * @param modulo128 Use modulo 128.
 * @param af AddressField.
 * @param nr N(R) variable.
 * @param ns N(S) variable.
 * @param data Pointer to payload data.
 * @param size Size of payload data.
 * @param ex Exception structure.
 * @return New primitive containing a AX25 frame in payload.
 */
extern primitive_t *new_AX25_I(
		uint16_t cH, uint16_t sH,
		bool modulo128,
		struct addressField *af,
		uint8_t nr, uint8_t ns,
		uint8_t *data, size_t size,
		struct exception *ex);

/**
 * @brief Create a AX25 Supervisory frame.
 * @param cH Client Handle.
 * @param sH Server Handle.
 * @param ax25_cmd AX25 Command.
 * @param modulo128 Use modulo 128.
 * @param af AddressField.
 * @param nr N(R) variable.
 * @param ex Exception structure.
 * @return New primitive containing a AX25 frame in payload.
 */
extern primitive_t *new_AX25_Supervisory(
		uint16_t cH, uint16_t sH,
		AX25_CMD_t ax25_cmd,
		bool modulo128,
		struct addressField *af,
		uint8_t nr,
		bool poll,
		struct exception *ex);

/**
 * @brief Create a AX25 RR frame.
 * @param cH Client Handle.
 * @param sH Server Handle.
 * @param modulo128 Use modulo 128.
 * @param af AddressField.
 * @param nr N(R) variable.
 * @param ex Exception structure.
 * @return New primitive containing a AX25 frame in payload.
 */
static inline primitive_t *new_AX25_RR(
		uint16_t cH, uint16_t sH,
		bool modulo128,
		struct addressField *af,
		uint8_t nr, bool poll,
		struct exception *ex)
{
	return new_AX25_Supervisory(cH, sH, AX25_RR, modulo128, af, nr, poll, ex);
}

/**
 * @brief Create a AX25 RNR frame.
 * @param cH Client Handle.
 * @param sH Server Handle.
 * @param modulo128 Use modulo 128.
 * @param af AddressField.
 * @param nr N(R) variable.
 * @param ex Exception structure.
 * @return New primitive containing a AX25 frame in payload.
 */
static inline primitive_t *new_AX25_RNR(
		uint16_t cH, uint16_t sH,
		bool modulo128,
		struct addressField *af,
		uint8_t nr, bool poll,
		struct exception *ex)
{
	return new_AX25_Supervisory(cH, sH, AX25_RNR, modulo128, af, nr, poll, ex);
}

/**
 * @brief Create a AX25 REJ frame.
 * @param cH Client Handle.
 * @param sH Server Handle.
 * @param modulo128 Use modulo 128.
 * @param af AddressField.
 * @param nr N(R) variable.
 * @param ex Exception structure.
 * @return New primitive containing a AX25 frame in payload.
 */
static inline primitive_t *new_AX25_REJ(
		uint16_t cH, uint16_t sH,
		bool modulo128,
		struct addressField *af,
		uint8_t nr, bool poll,
		struct exception *ex)
{
	return new_AX25_Supervisory(cH, sH, AX25_REJ, modulo128, af, nr, poll, ex);
}

/**
 * @brief Create a AX25 SREJ frame.
 * @param cH Client Handle.
 * @param sH Server Handle.
 * @param modulo128 Use modulo 128.
 * @param af AddressField.
 * @param nr N(R) variable.
 * @param ex Exception structure.
 * @return New primitive containing a AX25 frame in payload.
 */
static inline primitive_t *new_AX25_SREJ(
		uint16_t cH, uint16_t sH,
		bool modulo128,
		struct addressField *af,
		uint8_t nr, bool poll,
		struct exception *ex)
{
	return new_AX25_Supervisory(cH, sH, AX25_SREJ, modulo128, af, nr, poll, ex);
}

/**
 * @brief Create a AX25 SREJ frame.
 * @param cH Client Handle.
 * @param sH Server Handle.
 * @param ax25_cmd AX25 Command.
 * @param af AddressField.
 * @param cmd Cmd/Res bit.
 * @param poll Poll/Final bit.
 * @param data Pointer to payload data.
 * @param size Size of payload data.
 * @param ex Exception structure.
 * @return New primitive containing a AX25 frame in payload.
 */
extern primitive_t *new_AX25_Unnumbered(
		uint16_t cH, uint16_t sH,
		AX25_CMD_t ax25_cmd,
		struct addressField *af,
		bool cmd, bool poll,
		uint8_t *data, size_t size,
		struct exception *ex);

/**
 * @brief Create a AX25 SABME frame.
 * @param cH Client Handle.
 * @param sH Server Handle.
 * @param af AddressField.
 * @param ex Exception structure.
 * @return New primitive containing a AX25 frame in payload.
 */
static inline primitive_t *new_AX25_SABME(
		uint16_t cH, uint16_t sH,
		struct addressField *af,
		struct exception *ex)
{
	return new_AX25_Unnumbered(cH, sH, AX25_SABME, af, true, true, NULL, 0, ex);
}

/**
 * @brief Create a AX25 SABM frame.
 * @param cH Client Handle.
 * @param sH Server Handle.
 * @param af AddressField.
 * @param ex Exception structure.
 * @return New primitive containing a AX25 frame in payload.
 */
static inline primitive_t *new_AX25_SABM(
		uint16_t cH, uint16_t sH,
		struct addressField *af,
		struct exception *ex)
{
	return new_AX25_Unnumbered(cH, sH, AX25_SABM, af, true, true, NULL, 0, ex);
}

/**
 * @brief Create a AX25 DISC frame.
 * @param cH Client Handle.
 * @param sH Server Handle.
 * @param af AddressField.
 * @param ex Exception structure.
 * @return New primitive containing a AX25 frame in payload.
 */
static inline primitive_t *new_AX25_DISC(
		uint16_t cH, uint16_t sH,
		struct addressField *af,
		struct exception *ex)
{
	return new_AX25_Unnumbered(cH, sH, AX25_DISC, af, true, true, NULL, 0, ex);
}

/**
 * @brief Create a AX25 DM frame.
 * @param cH Client Handle.
 * @param sH Server Handle.
 * @param af AddressField.
 * @param ex Exception structure.
 * @return New primitive containing a AX25 frame in payload.
 */
static inline primitive_t *new_AX25_DM(
		uint16_t cH, uint16_t sH,
		struct addressField *af,
		struct exception *ex)
{
	return new_AX25_Unnumbered(cH, sH, AX25_DM, af, false, false, NULL, 0, ex);
}

/**
 * @brief Create a AX25 UA frame.
 * @param cH Client Handle.
 * @param sH Server Handle.
 * @param af AddressField.
 * @param ex Exception structure.
 * @return New primitive containing a AX25 frame in payload.
 */
static inline primitive_t *new_AX25_UA(
		uint16_t cH, uint16_t sH,
		struct addressField *af,
		struct exception *ex)
{
	return new_AX25_Unnumbered(cH, sH, AX25_UA, af, false, false, NULL, 0, ex);
}

/**
 * @brief Create a AX25 FRMR frame.
 * @param cH Client Handle.
 * @param sH Server Handle.
 * @param af AddressField.
 * @param ex Exception structure.
 * @return New primitive containing a AX25 frame in payload.
 */
static inline primitive_t *new_AX25_FRMR(
		uint16_t cH, uint16_t sH,
		struct addressField *af,
		struct exception *ex)
{
	return new_AX25_Unnumbered(cH, sH, AX25_FRMR, af, false, false, NULL, 0, ex);
}

/**
 * @brief Create a AX25 UI frame.
 * @param cH Client Handle.
 * @param sH Server Handle.
 * @param af AddressField.
 * @param cmd Cmd/Res bit.
 * @param poll Poll/Final bit.
 * @param data Pointer to payload data.
 * @param size Size of payload data.
 * @param ex Exception structure.
 * @return New primitive containing a AX25 frame in payload.
 */
static inline primitive_t *new_AX25_UI(
		uint16_t cH, uint16_t sH,
		struct addressField *af,
		bool cmd, bool poll,
		uint8_t *data, size_t size,
		struct exception *ex)
{
	return new_AX25_Unnumbered(cH, sH, AX25_UI, af, cmd, poll, data, size, ex);
}

/**
 * @brief Create a AX25 XID frame.
 * @param cH Client Handle.
 * @param sH Server Handle.
 * @param af AddressField.
 * @param cmd Cmd/Res bit.
 * @param poll Poll/Final bit.
 * @param data Pointer to payload data.
 * @param size Size of payload data.
 * @param ex Exception structure.
 * @return New primitive containing a AX25 frame in payload.
 */
static inline primitive_t *new_AX25_XID(
		uint16_t cH, uint16_t sH,
		struct addressField *af,
		bool cmd, bool poll,
		uint8_t *data, size_t size,
		struct exception *ex)
{
	return new_AX25_Unnumbered(cH, sH, AX25_XID, af, cmd, poll, data, size, ex);
}

/**
 * @brief Create a AX25 TEST frame.
 * @param cH Client Handle.
 * @param sH Server Handle.
 * @param af AddressField.
 * @param cmd Cmd/Res bit.
 * @param poll Poll/Final bit.
 * @param data Pointer to payload data.
 * @param size Size of payload data.
 * @param ex Exception structure.
 * @return New primitive containing a AX25 frame in payload.
 */
static inline primitive_t *new_AX25_TEST(
		uint16_t cH, uint16_t sH,
		struct addressField *af,
		bool cmd, bool poll,
		uint8_t *data, size_t size,
		struct exception *ex)
{
	return new_AX25_Unnumbered(cH, sH, AX25_TEST, af, cmd, poll, data, size, ex);
}

/**
 * @brief Check the CRC of a AX25 primitive.
 * @param prim The primitive.
 * @return If the CRC is correct.
 */
extern bool prim_check_AX25_CRC(primitive_t *prim);

/**
 * @brief Get the addressField of a AX25 primitive.
 * @param prim The primitive.
 * @param af AddressField to receive the result.
 * @param ex Exception struct.
 * @return If the address field could be extracted.
 */
extern bool prim_get_AX25_addressField(primitive_t *prim,
		struct addressField *af, struct exception *ex);

/**
 * @brief Get the version of a AX25 primitive.
 * @param prim The primitive.
 * @return If true, this is AX25 V2.x, privious version if false.
 */
extern bool prim_get_AX25_V2(primitive_t *prim);

/**
 * @brief Get the Cmd/Res bit of a AX25 primitive.
 * @param prim The primitive.
 * @return Cmd/Res of the primitive.
 */
extern bool prim_get_AX25_CmdRes(primitive_t *prim);

/**
 * @brief Get the Poll/Final bit of a AX25 primitive.
 * @param prim The primitive.
 * @return Poll/Final of the primitive.
 */
extern bool prim_get_AX25_PollFinal(primitive_t *prim);

/**
 * @brief Get the N(R) variable of a AX25 primitive.
 * @param prim The primitive.
 * @return N(R) variable of the primitive.
 */
extern int8_t prim_get_AX25_NR(primitive_t *prim);

/**
 * @brief Get the N(S) variable of a AX25 primitive.
 * @param prim The primitive.
 * @return N(S) variable of the primitive.
 */
extern int8_t prim_get_AX25_NS(primitive_t *prim);

/**
 * @brief Get data of a AX25 primitive.
 * @param prim The primitive.
 * @param pdata Pointer to the data pointer.
 * @param psize Pointer to the data size.
 * @return True, when data field extraction was successful.
 */
extern bool prim_get_AX25_data(primitive_t *prim,
		uint8_t ** pdata, size_t *psize);

#endif /* AX25V2_2_AX25V2_2_H_ */
