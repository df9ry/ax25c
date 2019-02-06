/*
 *  Project: ax25c - File: callsign.h
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

/**
 * @file callsign.h
 * @brief Support for managing callsigns.
 */
#ifndef AX25V2_2_CALLSIGN_H_
#define AX25V2_2_CALLSIGN_H_

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

struct exception;

/**
 * @brief Internal representation of callsigns.
 */
typedef uint64_t callsign;

struct addressField {
	callsign destination;
	callsign source;
	callsign repeaters[2];
};

union _callsign {
	callsign encoded;
	uint8_t octets[7];
};

typedef struct addressField addressField_t;

/**
 * @brief Encode a callsign from string notation.
 * @param str String notation to encode into a callsign.
 * @param next Pointer to next char after the callsign. May be NULL.
 * @param ex Exception struct, optional.
 * @return Encoded callsign or 0, in the case of an error. Param ex will
 *         contain detailed information about the problem otherwise.
 */
extern callsign callsignFromString(const char *str, const char **next,
		struct exception *ex);

/**
 * @brief Decode a callsign to a string buffer.
 * @param call Internal callsign representation to decode.
 * @param pb Pointer to target char buffer.
 * @param cb Size of the target char buffer.
 * @param ex Exception struct, optional.
 * @return Number of bytes used from buffer. When -1, an error happened.
 *         Param ex will contain detailed Information about the problem.
 */
extern int callsignToString(callsign call, char *pb, size_t cb,
								struct exception *ex);

/**
 * @brief Encode an addressField from string notation.
 * @param source Source callsign.
 * @param dest String notation to encode destination into an addressField.
 * @param af Pointer to addressField to encode to.
 * @param ex Exception struct, optional.
 * @return True when encoding was successful. Param ex will
 *         contain detailed information about the problem otherwise.
 */
extern bool addressFieldFromString(callsign source, const char *dest,
		struct addressField *af, struct exception *ex);

/**
 * @brief Decode an adressField into string notation.
 * @param af AddressField to decode.
 * @param pb Pointer to string buffer.
 * @param cb Size of string buffer.
 * @param ex Exception struct, optional.
 * @return True when decoding was successful. Param ex will
 *         contain detailed information about the problem otherwise.
 */
extern bool addressFieldToString(const struct addressField *af,
									char *pb, size_t cb,
									struct exception *ex);

/**
 * @brief Copy addressField.
 * @param dst Pointer to destination addressField.
 * @param src Pointer to source addressField.
 */
static inline void addressFieldCopy(struct addressField *dst,
		const struct addressField *src)
{
	assert(src);
	assert(dst);
	memcpy(dst, src, sizeof(struct addressField));
}

/**
 * @brief Has Repeated bit value.
 */
#define H_BIT 0x080

/**
 * @brief Get the H-Bit of a callsign.
 * @param call The callsign to investigate.
 * @return H-Bit.
 */
static inline bool getHBit(callsign call)
{
	union _callsign c;
	c.encoded = call;
	return (c.octets[6] & H_BIT);
}

/**
 * @brief Set the H-Bit of a callsign.
 * @param call The callsign to modify.
 * @param hbit Value of H-Bit to set.
 * @return Modified callsign.
 */
static inline void setHBit(callsign *call, bool hbit)
{
	union _callsign *c = (union _callsign*)call;
	if (hbit)
		c->octets[6] |= H_BIT;
	else
		c->octets[6] &= ~H_BIT;
}

/**
 * @brief Address field extension bit.
 */
#define X_BIT 0x01

/**
 * @brief Get the X-Bit of a callsign.
 * @param call The callsign to investigate.
 * @return X-Bit.
 */
static inline bool getXBit(callsign call)
{
	union _callsign c;
	c.encoded = call;
	return (c.octets[6] & X_BIT);
}

/**
 * @brief Set the X-Bit of a callsign.
 * @param call The callsign to modify.
 * @param xbit Value of X-Bit to set.
 * @return Modified callsign.
 */
static inline void setXBit(callsign *call, bool xbit)
{
	union _callsign *c = (union _callsign*)call;
	if (xbit)
		c->octets[6] |= X_BIT;
	else
		c->octets[6] &= ~X_BIT;
}

/**
 * @brief Command bit.
 */
#define C_BIT 0x80

/**
 * @brief Get the C-Bit of a callsign.
 * @param call The callsign to investigate.
 * @return C-Bit.
 */
static inline bool getCBit(callsign call)
{
	union _callsign c;
	c.encoded = call;
	return (c.octets[6] & C_BIT);
}

/**
 * @brief Set the C-Bit of a callsign.
 * @param call The callsign to modify.
 * @param cbit Value of C-Bit to set.
 * @return Modified callsign.
 */
static inline void setCBit(callsign *call, bool cbit)
{
	union _callsign *c = (union _callsign*)call;
	if (cbit)
		c->octets[6] |= C_BIT;
	else
		c->octets[6] &= ~C_BIT;
}

/**
 * @brief Get the number of digipeaters of an addressField.
 * @param af AddressField to investigate.
 * @return Number of digipeaters in addressField.
 */
static inline int getNRepeaters(struct addressField *af)
{
	if (getXBit(af->source))
		return 0;
	else if (getXBit(af->repeaters[0]))
		return 1;
	else
		return 2;
}

/**
 * @brief Get number of octets required for the address field in a AX25
 *        frame.
 * @param af AddressField to investigate.
 * @return Number of octets.
 */
static inline size_t getFrameAddressLength(struct addressField *af)
{
	return 14 + getNRepeaters(af) * 7;
}

/**
 * @brief Put address field into a frame.
 * @param af Address Field to put.
 * @param pframe Pointer to memory to put to.
 * @return Number of bytes to put.
 */
extern size_t putFrameAddress(struct addressField *af, uint8_t *pframe);

#endif /* AX25V2_2_CALLSIGN_H_ */
