/*
 *  Project: ax25c - File: runtime.h
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

#ifndef RUNTIME_H_
#define RUNTIME_H_

/**
 * @file
 * @brief Common runtime for all loadable modules.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "configuration.h"
#include "exception.h"

#include <uki/list.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize the runtime.
 */
extern void runtime_initialize(void);

/**
 * @brief Terminate the runtime.
 */
extern void runtime_terminate(void);

/**
 * @brief Print an exception to stderr and return the error code.
 * @param ex The exception to print.
 * @return The error code of the exception.
 */
extern int print_ex(struct exception *ex);

/**
 * @brief Load a shared object file into the program.
 * @param name Name of the shared object file.
 * @param handle Pointer to the resulting handle.
 * @param ex Exception structure.
 * @return Execution status.
 */
extern bool load_so(const char *name, void **handle, struct exception *ex);

/**
 * @brief Load a symbol from a loaded shared object file.
 * @param handle Module handle of the shared object to unload.
 * @param name Name of the symbol to load.
 * @param addr Address returned of the symbol, if loaded.
 * @param ex Exception structure.
 * @return Execution status.
 */
extern bool getsym_so(void *handle, const char *name, void ** addr,
						struct exception *ex);

/**
 * @brief Unload a shared object file.
 * @param handle Module handle of the shared object to unload.
 * @param ex Exception structure.
 * @return Execution status.
 */
extern bool unload_so(void *handle, struct exception *ex);

/**
 * @brief Start all plugins and instances.
 * @param ex Exception structure.
 * @return Execution status.
 */
extern bool start(struct exception *ex);

/**
 * @brief Stop the system.
 * @param ex Exception structure.
 * @return Execution status.
 */
extern bool stop(struct exception *ex);

/**
 * @brief This is the global configuration object.
 */
extern struct configuration configuration;

/**
 * @brief Write a message into the log.
 *        This function is non blocking.
 * @param dl Debug level
 * @param fmt Format
 * @param ... Variable list of arguments.
 */
extern void ax25c_log(enum debug_level_t dl, const char *fmt, ...);

/**
 * @brief DEBUG method.
 * @param msg Message to print.
 * @param par Parameter to print.
 */
static inline void DEBUG(const char *msg, const char *par)
{
	if (configuration.loglevel >= DEBUG_LEVEL_DEBUG)
		ax25c_log(DEBUG_LEVEL_DEBUG, "%s:%s", msg, par);
}

/**
 * @brief ERROR method.
 * @param msg Message to print.
 * @param par Parameter to print.
 */
static inline void ERROR(const char *msg, const char *par)
{
	if (configuration.loglevel >= DEBUG_LEVEL_ERROR)
		ax25c_log(DEBUG_LEVEL_ERROR, "%s:%s", msg, par);
}

/**
 * @brief WARNING method.
 * @param msg Message to print.
 * @param par Parameter to print.
 */
static inline void WARNING(const char *msg, const char *par)
{
	if (configuration.loglevel >= DEBUG_LEVEL_WARNING)
		ax25c_log(DEBUG_LEVEL_WARNING, "%s:%s", msg, par);
}

/**
 * @brief INFO method.
 * @param msg Message to print.
 * @param par Parameter to print.
 */
static inline void INFO(const char *msg, const char *par)
{
	if (configuration.loglevel >= DEBUG_LEVEL_INFO)
		ax25c_log(DEBUG_LEVEL_INFO, "%s:%s", msg, par);
}

/**
 * @brief Kill all processes and exit.
 */
extern void die(void);

/**
 * @brief Check if the system ist still alive.
 * @return True, when system is alive.
 */
extern bool isAlive(void);

/**
 * @brief Allocate memory from the memory manager.
 *        The memory is already locked one time. If you don't need the memory any
 * @param cb Number of bytes to alloc.
 * @param ex Exception structure.
 * @return Pointer to the allocated memory or NULL, in case of an error.
 */
extern void *mem_alloc(uint32_t cb, struct exception *ex);

/**
 * @brief Get the size of a memory block.
 * @param Pointer to a memory block.
 * @return Size of the memory block.
 */
extern uint32_t mem_size(void *mem);

/**
 * @brief Increase the lock counter of this memory block by one.
 * @param mem Pointer to the memory block.
 */
extern void mem_lock(void *mem);

/**
 * @brief Decrease the lock counter of this memory block by one.
 *        If the lock counter reaches 0 the memory block is given back to
 *        the operating system or the cache, in the case when a cached memory
 *        manager is in use.
 * @param mem Pointer to the memory to free.
 */
extern void mem_free(void *mem);

/**
 * @brief Structure for registration of a tick listener.
 */
struct tick_listener {
	struct list_head node;            /**< Node in system list.         */
	bool (*onTick)(void *user_data,
			struct exception *ex);    /**< Function to call on tick.    */
	void *user_data;                  /**< User data for function call. */
};

/**
 * @brief Register a tick listener.
 * @param l Tick listener to register.
 */
extern void registerTickListener(struct tick_listener *l);

/**
 * @brief Unegister a tick listener.
 * @param l Tick listener to unregister.
 */
extern void unregisterTickListener(struct tick_listener *l);

#ifdef __cplusplus
}
#endif

#endif /* RUNTIME_AX25C_RUNTIME_H_ */
