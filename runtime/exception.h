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

#ifndef RUNTIME_EXCEPTION_H_
#define RUNTIME_EXCEPTION_H_

#include <stringc/stringc.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Data structure to be filled out in case of an exception.
 */
struct exception {
	int   erc;         /**< Error code                                */
	string_t module;   /**< Name of the module causing the exception. */
	string_t function; /**< Name of the function that failed.         */
	string_t message;  /**< Error message text.                       */
	string_t param;    /**< Additional information, if available.     */
};

/**
 * @brief Macro to define an exception.
 * @param NAME Variable name.
 */
#define EXCEPTION(NAME) struct exception NAME = { \
	.erc = EXIT_SUCCESS, \
	.module.cb   = 0, .module.pc   = NULL, \
	.function.cb = 0, .function.pc = NULL, \
	.message.cb  = 0, .message.pc  = NULL, \
	.param.cb    = 0, .param.pc    = NULL, }

/**
 * @brief Exception type.
 */
typedef struct exception exception_t;

/**
 * @brief fill in an exception, if specified.
 * @param erc      Error code.
 * @param module   Name of the module causing the exception./
 * @param function Name of the function that failed.
 * @param message  Error message text.
 * @param param    Additional information, if available.
 */
extern void exception_fill(exception_t *ex, int erc, const char *module,
		const char *function, const char *message, const char *param);

/**
 * @brief Reset exception. Must be called before exception memory is freed,
 *        otherwise memory may be leaked.
 * @param ex Exception to reset.
 */
static inline void exception_reset(exception_t *ex)
{
	if (ex) {
		STRING_RESET(ex->module  );
		STRING_RESET(ex->function);
		STRING_RESET(ex->message );
		STRING_RESET(ex->param   );
	}
}

/**
 * @brief Reset exception. Must be called before exception memory is freed,
 *        otherwise memory may be leaked.
 * @param ex Exception to reset.
 */
#define EXCEPTION_RESET(EX) exception_reset(&EX)

#ifdef __cplusplus
}
#endif

#endif /* RUNTIME_EXCEPTION_H_ */
