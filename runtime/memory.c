/*
 *  Project: ax25c - File: ax25c_runtime.c
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

#include "memory.h"

#include <stdlib.h>
#include <assert.h>

static struct mm_interface *mm = NULL;

void registerMemoryManager(struct mm_interface *_mm)
{
	assert(_mm);
	assert(!mm);
	mm = _mm;
}

void *mem_alloc(uint32_t cb, struct exception *ex)
{
	if (mm)
		return mm->mem_alloc(cb, ex);
	else
		return NULL;
}

uint32_t mem_size(void *mem)
{
	if (mm)
		return mm->mem_size(mem);
	else
		return 0;
}

void mem_lock(void *mem)
{
	if (mm)
		mm->mem_lock(mem);
}

void mem_free(void *mem) {
	if (mm)
		mm->mem_free(mem);
}

