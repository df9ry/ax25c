/*
 *  Project: ax25c - File: runtime.h
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

/*
 * runtime.h
 *
 *  Created on: 08.01.2019
 *      Author: tania
 */

#ifndef RUNTIME_H_
#define RUNTIME_H_

#include "configuration.h"

#include <stdio.h>

/**
 * @file
 * @brief Common runtime for all loadable modules.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "exception.h"

#include <stdbool.h>

/**
 * @brief Print an exception to stderr and return the error code.
 * @param ex The exception to print.
 * @return The error code of the exception.
 */
extern int print_ex(struct exception *ex);

/**
 * @brief Load a shared object file into the program.
 * @param name Name of the shared object file.
 * @param handle Pointer to the resulting handle.
 * @param ex Exception structure.
 * @return Execution status.
 */
extern bool load_so(const char *name, void **handle, struct exception *ex);

/**
 * @brief Load a symbol from a loaded shared object file.
 * @param handle Module handle of the shared object to unload.
 * @param name Name of the symbol to load.
 * @param addr Address returned of the symbol, if loaded.
 * @param ex Exception structure.
 * @return Execution status.
 */
extern bool getsym_so(void *handle, const char *name, void ** addr,
						struct exception *ex);

/**
 * @brief Unload a shared object file.
 * @param handle Module handle of the shared object to unload.
 * @param ex Exception structure.
 * @return Execution status.
 */
extern bool unload_so(void *handle, struct exception *ex);

/**
 * @brief Start all plugins and instances.
 * @param ex Exception structure.
 * @return Execution status.
 */
extern bool start(struct exception *ex);

/**
 * @brief This is the global configuration object.
 */
extern struct configuration configuration;

/**
 * @brief DEBUG method.
 * @param msg Message to print.
 * @param par Parameter to print.
 */
static inline void DEBUG(const char *msg, const char *par)
{
	if (configuration.loglevel >= DEBUG_LEVEL_DEBUG)
		fprintf(stderr, "D:%s:%s\n", msg, par);
}

/**
 * @brief ERROR method.
 * @param msg Message to print.
 * @param par Parameter to print.
 */
static inline void ERROR(const char *msg, const char *par)
{
	if (configuration.loglevel >= DEBUG_LEVEL_ERROR)
		fprintf(stderr, "E:%s:%s\n", msg, par);
}

/**
 * @brief WARNING method.
 * @param msg Message to print.
 * @param par Parameter to print.
 */
static inline void WARNING(const char *msg, const char *par)
{
	if (configuration.loglevel >= DEBUG_LEVEL_WARNING)
		fprintf(stderr, "W:%s:%s\n", msg, par);
}

/**
 * @brief INFO method.
 * @param msg Message to print.
 * @param par Parameter to print.
 */
static inline void INFO(const char *msg, const char *par)
{
	if (configuration.loglevel >= DEBUG_LEVEL_INFO)
		fprintf(stderr, "I:%s:%s\n", msg, par);
}

#ifdef __cplusplus
}
#endif

#endif /* RUNTIME_AX25C_RUNTIME_H_ */
