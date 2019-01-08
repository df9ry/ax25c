/*
 *  Project: ax25c - File: ax25c_config.h
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
 * @brief Configuration by reading the XML file.
 */

#ifndef CONFIG_AX25C_CONFIG_H_
#define CONFIG_AX25C_CONFIG_H_

#include "../configuration.h"
#include "../exception.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configure the AX25C program.
 * @param argc Number of command line arguments.
 * @param argv Array of command line arguments.
 * @param conf Pointer to the configuration data structure.
 * @param ex Exception structure (optional).
 * @return true when configuration could be read. If false, ex contains
 *         useful information.
 */
typedef bool (*config_func_t)(
		int argc, char *argv[],
		struct configuration *conf,
		struct exception *ex);

/**
 * @brief Configure the AX25C program.
 * @param argc Number of command line arguments.
 * @param argv Array of command line arguments.
 * @param conf Pointer to the configuration data structure.
 * @param ex Exception structure (optional).
 * @return true when configuration could be read. If false, ex contains
 *         useful information.
 */
extern bool configure(
		int argc, char *argv[],
		struct configuration *conf,
		struct exception *ex);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_AX25C_CONFIG_H_ */
