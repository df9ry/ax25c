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

struct primentry {
	struct list_head list;
	struct primitive *prim;
};

struct primbuffer {
	size_t size, free;
	pthread_spinlock_t spinlock;
	pthread_mutex_t cond_lock;
	pthread_cond_t cond;
	struct list_head free_list;
	struct list_head expedited_list;
	struct list_head normal_list;
	struct primentry pool[0];
};

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

primbuffer_t *primbuffer_new(size_t size, struct exception *ex)
{
	int erc, i;
	struct primentry *pe;

	primbuffer_t *pb = malloc(
			sizeof(struct primbuffer) + size * sizeof(struct primentry));
	if (!pb) {
		exception_fill(ex, ENOMEM, MODULE_NAME,
				"primbuffer_new", "Not enough memory", "");
		return NULL;
	}
	pb->size = size;
	pb->free = size;
	erc = pthread_spin_init(&pb->spinlock, PTHREAD_PROCESS_SHARED);
	assert(erc == 0);
	erc = pthread_mutex_init(&pb->cond_lock, NULL);
	assert(erc == 0);
	erc = pthread_cond_init(&pb->cond, NULL);
	assert(erc == 0);
	INIT_LIST_HEAD(&pb->free_list);
	INIT_LIST_HEAD(&pb->expedited_list);
	INIT_LIST_HEAD(&pb->normal_list);
	for (i = 0, pe = pb->pool; i < size; ++i, ++pe) {
		INIT_LIST_HEAD(&pe->list);
		list_add(&pe->list, &pb->free_list);
		pe->prim = NULL;
	} /* end for */
	return pb;
}

void primbuffer_del(primbuffer_t *pb)
{
	int i;
	struct primentry *pe;

	if (!pb)
		return;
	for (i = 0, pe = pb->pool; i < pb->size; ++i, ++pe) {
		if (pe->prim)
			del_prim(pe->prim);
		pe->prim = NULL;
	} /* end for */
	pthread_cond_destroy(&pb->cond);
	pthread_mutex_destroy(&pb->cond_lock);
	pthread_spin_destroy(&pb->spinlock);
	free(pb);
}

void primbuffer_stats(primbuffer_t *pb, struct primbuffer_stats *stats)
{
	int erc;

	assert(pb);
	assert(stats);
	erc = pthread_spin_lock(&pb->spinlock); /*--v*/
	assert(erc == 0);
	stats->size = pb->size;
	stats->free = pb->free;
	erc = pthread_spin_unlock(&pb->spinlock); /*^*/
	assert(erc == 0);
}

bool primbuffer_write_nonblock(primbuffer_t *pb, struct primitive *prim,
		bool expedited)
{
	int erc;
	bool res;
	struct primentry *pe;

	assert(pb);
	assert(prim);
	mem_chck(prim);
	erc = pthread_spin_lock(&pb->spinlock); /*-------------------------------v*/
	assert(erc == 0);
	pe = list_first_entry_or_null(&pb->free_list, struct primentry, list);
	if (pe) {
		use_prim(prim);
		pe->prim = prim;
		pb->free -= 1;
		if (expedited)
			list_move_tail(&pe->list, &pb->expedited_list);
		else
			list_move_tail(&pe->list, &pb->normal_list);
		res = true;
	} else {
		res = false;
	}
	erc = pthread_spin_unlock(&pb->spinlock); /*-----------------------------^*/
	assert(erc == 0);
	if (res)
		_cond_signal(pb);
	return res;
}

extern struct primitive *primbuffer_read_block(primbuffer_t *pb, bool *expedited)
{
	int erc;
	struct primentry *pe;
	primitive_t *prim = NULL;

	assert(pb);
	while (!prim) {
		erc = pthread_spin_lock(&pb->spinlock); /*---------------------------v*/
		assert(erc == 0);
		pe = list_first_entry_or_null(&pb->expedited_list, struct primentry, list);
		if (!pe)
			pe = list_first_entry_or_null(&pb->normal_list, struct primentry, list);
		if (pe) {
			list_move(&pe->list, &pb->free_list);
			prim = pe->prim;
			pe->prim = NULL;
			pb->free += 1;
		}
		erc = pthread_spin_unlock(&pb->spinlock); /*-------------------------^*/
		assert(erc == 0);
		if (!prim)
			_cond_wait(pb);
	} /* end while */
	mem_chck(prim);
	return prim;
}
