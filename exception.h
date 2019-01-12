/*
 *  Project: ax25c - File: exception.h
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
 * @file
 * @brief Structure to use in very simple exception like mechanism.
 */

#ifndef EXCEPTION_H_
#define EXCEPTION_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Data structure to be filled out in case of an exception.
 */
struct exception {
	int   erc;            /**< Error code                                */
	const char *module;   /**< Name of the module causing the exception. */
	const char *function; /**< Name of the function that failed.         */
	const char *message;  /**< Error message text.                       */
	const char *param;    /**< Additional information, if available.     */
};

#ifdef __cplusplus
}
#endif

#endif /* EXCEPTION_H_ */
