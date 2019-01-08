/*
 *  Project: ax25c - File: ax25c_runtime.h
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
 * ax25c_runtime.h
 *
 *  Created on: 08.01.2019
 *      Author: tania
 */

#ifndef RUNTIME_AX25C_RUNTIME_H_
#define RUNTIME_AX25C_RUNTIME_H_

/** @file */

#include <stdbool.h>

/**
 * @brief Data structure to be filled out in case of an exception.
 */
struct exception {
	int   erc;      /**< Error code                               */
	char *module;   /**< Name of te module causing the exception. */
	char *message;  /**< Error message text.                      */
};

/**
 * @brief Load a shared object file.
 * @param name Name of the shared object file without extension.
 * @param ifc  Name of the interface to request from the module.
 * @param excp Pointer to exception buffer. May be NULL.
 * @return True if the request could be succesful handled. If false
 *   and excp was not NULL, excp was filled with useful information.
 */
extern bool load(char *name, char *ifc, struct exception *excp);

#endif /* RUNTIME_AX25C_RUNTIME_H_ */
