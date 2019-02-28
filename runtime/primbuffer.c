/*
 *  Project: ax25c - File: primbuffer.c
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

#include "primbuffer.h"
#include "runtime.h"
#include "primitive.h"
#include "_internal.h"

#include <uki/list.h>
#include <uki/kernel.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>

static inline void _cond_wait(primbuffer_t *pb)
{
	pthread_mutex_lock(&pb->cond_lock);
	pthread_cond_wait(&pb->cond, &pb->cond_lock);
	pthread_mutex_unlock(&pb->cond_lock);
}

static inline void _cond_signal(primbuffer_t *pb)
{
	pthread_mutex_lock(&pb->cond_lock);
	pthread_cond_signal(&pb->cond);
	pthread_mutex_unlock(&pb->cond_lock);
}

void primbuffer_init(primbuffer_t *pb)
{
	int erc;

	assert(pb);
	pb->size = 0;
	erc = pthread_spin_init(&pb->spinlock, PTHREAD_PROCESS_PRIVATE);
	assert(erc == 0);
	erc = pthread_mutex_init(&pb->cond_lock, NULL);
	assert(erc == 0);
	erc = pthread_cond_init(&pb->cond, NULL);
	assert(erc == 0);
	INIT_LIST_HEAD(&pb->expedited_list);
	INIT_LIST_HEAD(&pb->routine_list);
}

void primbuffer_destroy(primbuffer_t *pb)
{
	if (!pb)
		return;
	while(!list_empty(&pb->expedited_list))
		list_del_init(&(list_first_entry(
				&pb->expedited_list, struct primitive, node)->node));
	while(!list_empty(&pb->routine_list))
		list_del_init(&(list_first_entry(
				&pb->routine_list, struct primitive, node)->node));

	pthread_cond_destroy(&pb->cond);
	pthread_mutex_destroy(&pb->cond_lock);
	pthread_spin_destroy(&pb->spinlock);
}

void primbuffer_stats(primbuffer_t *pb, struct primbuffer_stats *stats)
{
	int erc;

	assert(pb);
	assert(stats);
	erc = pthread_spin_lock(&pb->spinlock); /*--v*/
	assert(erc == 0);
	stats->size = pb->size;
	erc = pthread_spin_unlock(&pb->spinlock); /*^*/
	assert(erc == 0);
}

void primbuffer_write_nonblock(primbuffer_t *pb, struct primitive *prim,
		bool expedited)
{
	int erc;

	assert(pb);
	assert(prim);
	mem_chck(prim);
	use_prim(prim);
	erc = pthread_spin_lock(&pb->spinlock); /*-------------------------------v*/
	assert(erc == 0);
	list_add_tail(&prim->node,
			expedited ? &pb->expedited_list : &pb->routine_list);
	_cond_signal(pb);
	erc = pthread_spin_unlock(&pb->spinlock); /*-----------------------------^*/
	assert(erc == 0);
}

struct primitive *primbuffer_read_nonblock(primbuffer_t *pb,
		bool *expedited)
{
	primitive_t *prim;
	int erc;

	erc = pthread_spin_lock(&pb->spinlock); /*---------------------------v*/
	assert(erc == 0);
	prim = list_first_entry_or_null(&pb->expedited_list,
			struct primitive, node);
	if (prim) {
		list_del_init(&prim->node);
		if (expedited)
			*expedited = true;
		goto exit;
	}
	prim = list_first_entry_or_null(&pb->routine_list,
			struct primitive, node);
	if (prim) {
		list_del_init(&prim->node);
		if (expedited)
			*expedited = false;
		goto exit;
	}
exit:
	erc = pthread_spin_unlock(&pb->spinlock); /*-------------------------^*/
	assert(erc == 0);
	return prim;
}

struct primitive *primbuffer_read_block(primbuffer_t *pb, bool *expedited)
{
	primitive_t *prim = NULL;

	assert(pb);
	while (!prim) {
		prim = primbuffer_read_nonblock(pb, expedited);
		if (!prim)
			_cond_wait(pb);
	} /* end while */
	return prim;
}
