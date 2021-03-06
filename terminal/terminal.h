/*
 *  Project: ax25c - File: terminal.h
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

#ifndef TERMINAL_TERMINAL_H_
#define TERMINAL_TERMINAL_H_

#include <stdbool.h>

struct plugin_handle;
struct exception;

/**
 * @brief Initialize the terminal system.
 * @param h Plugin handle.
 */
extern bool terminal_start(struct plugin_handle *h, struct exception *ex);

/**
 * @brief Terminate the terminal system.
 * @param h Plugin handle.
 */
extern bool terminal_stop(struct plugin_handle *h, struct exception *ex);

#endif /* TERMINAL_TERMINAL_H_ */
