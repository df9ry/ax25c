/*
 *  Project: ax25c - File: configuration.h
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
 * @brief Configuration of the AX25C system.
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <mapc/mapc.h>

struct instance {
	const char *name;      /**< Instance name.    */
	struct mapc_node node; /**< Map node.         */
};

struct plugin {
	const char *name;      /**< Plugin name.      */
	const char *file;      /**< Plugin filename.  */
	void *module_handle;   /**< SO handle.        */
	struct mapc_node node; /**< Map node.         */
	struct mapc instances; /**< Map of instances. */
};

struct configuration {
	const char *name;    /**< Configuration name. */
	struct mapc plugins; /**< Map of plugins.     */
};

#ifdef __cplusplus
}
#endif

#endif /* CONFIGURATION_H_ */
