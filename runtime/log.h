/*
 *  Project: ax25c - File: log.h
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

#ifndef RUNTIME_LOG_H_
#define RUNTIME_LOG_H_

/**
 * @brief Initialize the logging system.
 */
extern void ax25c_log_init(void);

/**
 * @brief Terminate the logging system.
 */
extern void ax25c_log_term(void);

#endif /* RUNTIME_LOG_H_ */
