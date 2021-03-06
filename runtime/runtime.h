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

#ifndef RUNTIME_RUNTIME_H_
#define RUNTIME_RUNTIME_H_

/**
 * @file
 * @brief Common runtime for all loadable modules.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "../config/configuration.h"

#include "../runtime/exception.h"

#include <uki/list.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

struct primitive;

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

extern void _dump(enum debug_level_t loglevel, const uint8_t *p, uint32_t c);

/**
 * @brief Dump data block.
 * @param loglevel Loglevel for the dump.
 * @param p Pointer to data to dump.
 * @param c Number of bytes to dump.
 */
static inline void dump (enum debug_level_t loglevel,
		const uint8_t *p, uint32_t c)
{
	if (configuration.loglevel >= DEBUG_LEVEL_ERROR)
		_dump(loglevel, p, c);
}

/**
 * @brief DEBUG method.
 * @param msg Message to print.
 * @param par Parameter to print.
 */
static inline void DBG_DEBUG(const char *msg, const char *par)
{
	if (configuration.loglevel >= DEBUG_LEVEL_DEBUG)
		ax25c_log(DEBUG_LEVEL_DEBUG, "%s:%s", msg, par);
}

/**
 * @brief ERROR method.
 * @param msg Message to print.
 * @param par Parameter to print.
 */
static inline void DBG_ERROR(const char *msg, const char *par)
{
	if (configuration.loglevel >= DEBUG_LEVEL_ERROR)
		ax25c_log(DEBUG_LEVEL_ERROR, "%s:%s", msg, par);
}

/**
 * @brief WARNING method.
 * @param msg Message to print.
 * @param par Parameter to print.
 */
static inline void DBG_WARNING(const char *msg, const char *par)
{
	if (configuration.loglevel >= DEBUG_LEVEL_WARNING)
		ax25c_log(DEBUG_LEVEL_WARNING, "%s:%s", msg, par);
}

/**
 * @brief INFO method.
 * @param msg Message to print.
 * @param par Parameter to print.
 */
static inline void DBG_INFO(const char *msg, const char *par)
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
 * @brief Check the integrity of a memory block. Asserts, when memory block is
 *        overwritten.
 * @param mem Pointer to the memory to check.
 */
extern void mem_chck(void *mem);

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

/**
 * @brief Get monitor info for a frame.
 * @param prim The primitive to get monitor info from.
 * @param pb Pointer to buffer for monitor output.
 * @param cb Size of the monitor buffer.
 * @param ex Pointer to exception struct or NULL.
 * @return Number of bytes written to pb or negative error code.
 */
extern int monitor(struct primitive *prim, char *pb, size_t cb,
		struct exception *ex);

/**
 * @brief Type for monitor listeners.
 * @param prim Primitive to monitor.
 * @param service Name of the service The primitive provides.
 * @param tx True for TX primitive, false for RX.
 * @param data User data passed from the register function.
 */
typedef void (monitor_listener_function)(struct primitive *prim,
		const char *service, bool tx, void *data);

/**
 * @brief Register a monitor listener.
 * @param listener Handler function to register. Note: Must never sleep!
 * @param data User data to be passed to each callback.
 * @return Handle to the registration.
 */
extern void *register_monitor_listener(monitor_listener_function *listener,
		void *data);

/**
 * @brief Unregister a monitor listener.
 * @param handle Registration handle.
 * @return True if the handle was registered.
 */
extern bool unregister_monitor_listener(void *handle);

/**
 * @brief Pupply a primitive for monitoring.
 * @param prim The primitive to supply.
 * @param service Static name of the service the primitive supplies.
 * @param tx Set true when transmitting the prim, false when receiving.
 */
extern void monitor_put(struct primitive *prim, const char *service, bool tx);

/**
 * @brief Extended isprint, that treats ASCII codes from 128 to 254 as
 *        printable too.
 * @param ch The caharacter to test.
 * @return True, when this character is printable.
 */
static inline bool extended_isprint(char ch)
{
	uint8_t _ch = (uint8_t)ch;
	return isprint(ch) || ((_ch >= 128) && (_ch < 255));
}

/**
 * @brief Escape character.
 */
extern char escapeChar;

/**
 * @brief Write leads to colorize output.
 */
extern bool writeLeads;

#ifdef __cplusplus
}
#endif

#endif /* RUNTIME_RUNTIME_H_ */
