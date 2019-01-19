/*
 *  Project: ax25c - File: tick.h
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

#ifndef RUNTIME_TICK_H_
#define RUNTIME_TICK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

struct exception;

/**
 * @brief Initialize the tick system.
 */
void ax25c_tick_init(void);

/**
 * @brief Terminate the tick system.
 */
void ax25c_tick_term(void);

/**
 * @brief Heardbeat tick.
 * @param ex Exception structure.
 * @return Execution status.
 */
extern bool tick(struct exception *ex);

#ifdef __cplusplus
}
#endif

#endif /* RUNTIME_TICK_H_ */
