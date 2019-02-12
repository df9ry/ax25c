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
#include <pthread.h>

#include <uki/list.h>

struct exception;
struct primbuffer;
struct primitive;
typedef struct primbuffer primbuffer_t;

struct primbuffer {
	size_t size;
	pthread_spinlock_t spinlock;
	pthread_mutex_t cond_lock;
	pthread_cond_t cond;
	struct list_head expedited_list;
	struct list_head routine_list;
};

struct primbuffer_stats {
	size_t size;
};

/**
 * @brief Initialize a primbuffer.
 * @pb Primbuffer to initialize.
 */
extern void primbuffer_init(primbuffer_t *pb);

/**
 * @brief Clear a primbuffer.
 * @pb Primbuffer to clear.
 */
extern void primbuffer_destroy(primbuffer_t *pb);

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
 */
extern void primbuffer_write_nonblock(primbuffer_t *pb,	struct primitive *prim,
		bool expedited);

/**
 * @brief Read prim from primbuffer, nonblocking.
 * @pb Primbuffer to read from.
 * @expedited. Pointer to bool that is set, when the prim is expedited.
 *             Optional.
 * @return Prim or NULL.
 */
extern struct primitive *primbuffer_read_nonblock(primbuffer_t *pb,
		bool *expedited);

/**
 * @brief Read prim from primbuffer, blocking.
 * @pb Primbuffer to read from.
 * @expedited. Pointer to bool that is set, when the prim is expedited.
 *             Optional.
 * @return Prim.
 */
extern struct primitive *primbuffer_read_block(primbuffer_t *pb, bool *expedited);

#endif /* RUNTIME_PRIMBUFFER_H_ */
