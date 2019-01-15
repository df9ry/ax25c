/*
 *  Project: ax25c - File: io.h
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
 * @file io.h
 * @brief Read and write frames.
 */

#ifndef IO_H_
#define IO_H_

#include "exception.h"

#include <stdbool.h>

extern void *ax25_read(size_t cb, bool f_block, struct exception *ex);

extern int ax25_write(void *pb, size_t cb, struct exception *ex);

#endif /* IO_H_ */
