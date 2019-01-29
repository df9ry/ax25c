/*
 *  Project: ax25c - File: memory.h
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

#ifndef RUNTIME_MEMORY_H_
#define RUNTIME_MEMORY_H_

#include <stdint.h>

struct exception;

struct mm_interface {
	void *(*mem_alloc)(uint32_t cb, struct exception *ex);
	uint32_t (*mem_size)(void *mem);
	void (*mem_lock)(void *mem);
	void (*mem_free)(void *mem);
	void (*mem_chck)(void *mem);
};

/**
 * @brief register the memory manager with the runtime.
 * @param mm Handle of the memory
 */
extern void registerMemoryManager(struct mm_interface *mm);

#endif /* RUNTIME_MEMORY_H_ */
