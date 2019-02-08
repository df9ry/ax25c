/*
 *  Project: ax25c - File: exception.c
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

#include "../runtime/runtime.h"
#include "../config/configuration.h"

#include "exception.h"

void exception_fill(exception_t *ex, int erc, const char *module,
		const char *function, const char *message, const char *param)
{
	if (!module)
		module = "Unknown";
	if (!function)
		function = "Unknown";
	if (!message)
		message = strerror(erc);
	if (!param)
		param = "";

	ax25c_log(DEBUG_LEVEL_DEBUG,
			"Throw exception %i[%s] in %s:%s with param %s",
			erc, message, module, function, param);
	if (ex) {
		ex->erc = erc;
		STRING_SET_C(ex->module,   module  );
		STRING_SET_C(ex->function, function);
		STRING_SET_C(ex->message,  message );
		STRING_SET_C(ex->param,    param   );
	}
}
