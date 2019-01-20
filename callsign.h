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
#ifndef CALLSIGN_H_
#define CALLSIGN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

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
 * @brief Get the H-Bit of a callsign.
 * @param call The callsign to investigate.
 * @return H-Bit.
 */
static inline bool getHBit(callsign call)
{
	return (((uint8_t*)(&call))[6] & 0x80);
}

/**
 * @brief Set the H-Bit of a callsign.
 * @param call The callsign to modify.
 * @param h Value of H-Bit to set.
 * @return Modified callsign.
 */
static inline void setHBit(callsign *call, bool h)
{
	uint8_t *po = &(((uint8_t*)(call))[6]);
	if (h)
		*po |= 0x80;
	else
		*po &= ~0x80;
}

/**
 * @brief Get the X-Bit of a callsign.
 * @param call The callsign to investigate.
 * @return X-Bit.
 */
static inline bool getXBit(callsign call)
{
	return (((uint8_t*)(&call))[6] & 0x01);
}

/**
 * @brief Set the X-Bit of a callsign.
 * @param call The callsign to modify.
 * @param x Value of X-Bit to set.
 * @return Modified callsign.
 */
static inline void setXBit(callsign *call, bool x)
{
	uint8_t *po = &(((uint8_t*)(call))[6]);
	if (x)
		*po |= 0x01;
	else
		*po &= ~0x01;
}

/**
 * @brief Get the C-Bit of a callsign.
 * @param call The callsign to investigate.
 * @return C-Bit.
 */
static inline bool getCBit(callsign call)
{
	return (((uint8_t*)(&call))[6] & 0x80);
}

/**
 * @brief Set the C-Bit of a callsign.
 * @param call The callsign to modify.
 * @param c Value of C-Bit to set.
 * @return Modified callsign.
 */
static inline void setCBit(callsign *call, bool c)
{
	uint8_t *po = &(((uint8_t*)(call))[6]);
	if (c)
		*po |= 0x80;
	else
		*po &= ~0x80;
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

#ifdef __cplusplus
}
#endif

#endif /* CALLSIGN_H_ */
