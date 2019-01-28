/*
 *  Project: ax25c - File: primbuffer.h
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

#ifndef RUNTIME_PRIMBUFFER_H_
#define RUNTIME_PRIMBUFFER_H_

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

struct exception;
struct primbuffer;
struct primitive;
typedef struct primbuffer primbuffer_t;

struct primbuffer_stats {
	size_t size;
	size_t free;
};

/**
 * @brief Create a new primbuffer.
 * @param size Requested size of the primbuffer.
 * @param ex Exception structure, optional.
 * @return New primbuffer or NULL, in the case of an error.
 */
extern primbuffer_t *primbuffer_new(size_t size, struct exception *ex);

/**
 * @brief Delete a primbuffer.
 * @pb Primbuffer to delete.
 */
extern void primbuffer_del(primbuffer_t *pb);

/**
 * @brief get primbuffer stats.
 * @param pb Primbuffer to investigate.
 * @param stats Pointer to primbuffer stats.
 */
extern void primbuffer_stats(primbuffer_t *pb, struct primbuffer_stats *stats);

/**
 * @brief Write prim to primbuffer nonblocking.
 * @pb Primbuffer to write into.
 * @prim Prim to write.
 * @expedited When true, this is a expedited prim.
 * @return True, when successfully written, false, when no buffer space is
 *         available.
 */
extern bool primbuffer_write_nonblock(primbuffer_t *pb,
		struct primitive *prim, bool expedited);

/**
 * @brief Read prim from primbuffer, blocking.
 * @pb Primbuffer to read from.
 * @expedited. Pointer to bool that is set, when the prim is expedited.
 *             Optional.
 * @return Prim.
 */
extern struct primitive *primbuffer_read_block(primbuffer_t *pb, bool *expedited);

#endif /* RUNTIME_PRIMBUFFER_H_ */
