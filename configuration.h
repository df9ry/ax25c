/*
 *  Project: ax25c - File: configuration.h
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
 * @file
 * @brief Configuration of the AX25C system.
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "exception.h"

#include <mapc/mapc.h>

#include <stdint.h>

/**
 * @brief Copy string into a static buffer.
 * If the string is longer than 36 characters, it will be truncated and
 * elipsed.
 * @param s String to copy.
 * @return Copy of string in a static buffer.
 */
extern const char *strbuf(const char *s);

/**
 * @brief Description of a <Instance>.
 */
struct instance {
	const char *name;          /**< Instance name.        */
	void       *handle;        /**< Instance handle.      */
	struct mapc_node node;     /**< Map node.             */
};

struct plugin_descriptor;

/**
 * @brief Description of a <Plugin>.
 */
struct plugin {
	const char *name;          /**< Plugin name.          */
	void       *handle;        /**< Plugin handle.        */
	const char *file;          /**< Plugin filename.      */
	void *module_handle;       /**< SO handle.            */
	struct plugin_descriptor *plugin_descriptor; /**< Plugin descriptor. */
	struct mapc_node node;     /**< Map node.             */
	struct mapc instances;     /**< Map of instances.     */
};

enum debug_level_t {
	DEBUG_LEVEL_NONE    = 0,
	DEBUG_LEVEL_ERROR   = 1,
	DEBUG_LEVEL_WARNING = 2,
	DEBUG_LEVEL_INFO    = 3,
	DEBUG_LEVEL_DEBUG   = 4
};

/**
 * @brief Description of a <Configuration>.
 */
struct configuration {
	const char         *name;         /**< Configuration name.   */
	void               *handle;       /**< Configuration handle. */
	struct mapc         plugins;      /**< Map of plugins.       */
	unsigned int        tick;         /**< Timer tick in ms.     */
	enum debug_level_t  loglevel;     /**< Log Level.            */
};

/**
 * @brief Supported types for <Setting>.
 */
enum setting_type_t {
	INT_T   = 0,   /**< Setting is an 'int'          */
	UINT_T  = 1,   /**< Setting is an 'unsigned int' */
	SIZE_T  = 2,   /**< Setting is a 'size_t'        */
	CSTR_T  = 3,   /**< Setting is a 'char*'         */
	DEBUG_T = 4    /**< Setting is a 'debug_level_t' */
};

/**
 * @brief Description of a <Setting>.
 */
struct setting_descriptor {
	const char    *name;       /**< Setting name.         */
	enum setting_type_t type;  /**< Type of the setting.  */
	size_t         offset;     /**< Offset in descriptor. */
	const char    *value;      /**< Default value.        */
};

/**
 * @brief Load <Settings> into an object.
 * @param handle Object handle.
 * @param descriptor <Setting> descriptor array (NULL terminated).
 * @param context User data from get_handle_func.
 * @param ex Exception object (optional).
 * @return True on successful completion.
 */
typedef bool (*configurator_func)(void *handle,
		struct setting_descriptor *descriptor, void *context,
		struct exception *ex);

/**
 * @brief Get a handle from the module.
 * @param name Name for the new handle.
 * @param configurator Configurator to use for the new handle.
 * @param context User data for configurator_func.
 * @param ex Exception object (optional).
 * @return New handle when successful, NULL otherwise.
 */
typedef void *(*get_handle_func)(const char *name,
		configurator_func configurator, void *context, struct exception *ex);

/**
 * @brief Start up object denoted by handle.
 * @param Handle of the object to start.
 * @param ex Exception object (optional).
 * @return True on successful completion.
 */
typedef bool (*start_func)(void *handle, struct exception *ex);

/**
 * @brief Stop object denoted by handle.
 * @param Handle of the object to stop.
 * @param ex Exception object (optional).
 * @return True on successful completion.
 */
typedef bool (*stop_func)(void *handle, struct exception *ex);

/**
 * @brief Descriptor exported by a loadable module.
 */
struct plugin_descriptor {
	get_handle_func get_plugin_handle;   /**< Pointer to function to get the plugin handle.  */
	start_func start_plugin;             /**< Pointer to function to start up the plugin.    */
	stop_func stop_plugin;               /**< Pointer to function to stop the plugin.        */
	get_handle_func get_instance_handle; /**< Pointer to function to get an instance handle. */
	start_func start_instance;           /**< Pointer to function to start up the instance.  */
	stop_func stop_instance;             /**< Pointer to function to stop the instance.      */
};

#ifdef __cplusplus
}
#endif

#endif /* CONFIGURATION_H_ */
