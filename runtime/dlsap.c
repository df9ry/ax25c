/*
 *  Project: ax25c - File: dlsap.c
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

#include "../dlsap.h"
#include "../exception.h"

#include "_internal.h"
#include "dls.h"

#include <mapc/mapc.h>
#include <uki/kernel.h>

#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

static struct mapc dls_map;

static pthread_spinlock_t spinlock;

void ax25c_dlsap_init(void)
{
	int erc;

	erc = pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE); assert(erc == 0);
	erc = pthread_spin_lock(&spinlock); assert(erc == 0);
	mapc_init(&dls_map, (f_compare)strcmp);
	erc = pthread_spin_unlock(&spinlock); assert(erc == 0);
}

void ax25c_dlsap_term(void)
{
	int erc;

	erc = pthread_spin_lock(&spinlock); assert(erc == 0);
	mapc_clear(&dls_map);
	erc = pthread_spin_unlock(&spinlock); assert(erc == 0);
	erc = pthread_spin_destroy(&spinlock); assert(erc == 0);
}

bool dlsap_register_dls(dls_t *dls, exception_t *ex)
{
	int erc;
	bool res = false;

	if (!dls) {
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME, "dlsap_register_dls",
				"Service Provider pointer is NULL", NULL);
		return false;
	}
	if (!dls->name) {
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME, "dlsap_register_dls",
				"Service Provider name is NULL", NULL);
		return false;
	}

	erc = pthread_spin_lock(&spinlock); assert(erc == 0); /*-----------------v*/
	if (mapc_contains(&dls_map, dls->name)) {
		exception_fill(ex, EEXIST, MODULE_NAME, "dlsap_register_dls",
				"Service Provider is already registered", dls->name);
		goto exit;
	}
	if (!mapc_insert(&dls_map, &dls->node, dls->name)) {
		exception_fill(ex, EEXIST, MODULE_NAME, "dlsap_register_dls",
				"mapc_insert", dls->name);
		goto exit;
	}
	res = true;
exit:
	erc = pthread_spin_unlock(&spinlock); assert(erc == 0); /*---------------^*/
	return res;
}

bool dlsap_unregister_dls(dls_t *dls, exception_t *ex)
{
	int erc;
	bool res = false;
	struct mapc_node *mapc_node;

	if (!dls) {
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME, "dlsap_unregister_dls",
				"Service Provider pointer is NULL", NULL);
		return false;
	}
	if (!dls->name) {
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME, "dlsap_unregister_dls",
				"Service Provider name is NULL", NULL);
		return false;
	}

	erc = pthread_spin_lock(&spinlock); assert(erc == 0); /*-----------------v*/
	mapc_node = mapc_lookup(&dls_map, dls->name);
	if (!mapc_node) {
		exception_fill(ex, ENOENT, MODULE_NAME, "dlsap_unregister_dls",
				"Service Provider not found", dls->name);
		goto exit;
	}
	if (mapc_node != &dls->node) {
		exception_fill(ex, EINVAL, MODULE_NAME, "dlsap_unregister_dls",
				"Service Provider inconsistency", dls->name);
		goto exit;
	}
	mapc_node = mapc_remove(&dls_map, dls->name);
	assert(mapc_node == &dls->node);
	res = true;
exit:
	erc = pthread_spin_unlock(&spinlock); assert(erc == 0); /*---------------^*/
	return res;
}

dls_t *dlsap_lookup_dls(const char *name)
{
	int erc;
	dls_t *res = NULL;
	struct mapc_node *mapc_node;

	if (!name)
		return NULL;
	erc = pthread_spin_lock(&spinlock); assert(erc == 0); /*-----------------v*/
	mapc_node = mapc_lookup(&dls_map, name);
	if (!mapc_node)
		goto exit;
	res = container_of(mapc_node, dls_t, node);
	assert(strcmp(res->name, name) == 0);
exit:
	erc = pthread_spin_unlock(&spinlock); assert(erc == 0); /*---------------^*/
	return res;
}

bool dlsap_set_default_local_addr(dls_t *dls, const char *addr,
		string_t *normal, exception_t *ex)
{
	if (!dls) {
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME,
				"dlsap_set_default_local_addr",
				"Data Link Service is NULL", NULL);
		return false;
	}
	if (!dls->set_default_local_addr) {
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME,
				"dlsap_set_default_local_addr",
				"Service not provided", dls->name);
		return false;
	}
	return dls->set_default_local_addr(dls, addr, normal, ex);
}

bool dlsap_set_default_remote_addr(dls_t *dls, const char *addr,
		string_t *normal, exception_t *ex)
{
	if (!dls) {
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME,
				"dlsap_set_default_remote_addr",
				"Data Link Service is NULL", NULL);
		return NULL;
	}
	if (!dls->set_default_remote_addr) {
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME,
				"dlsap_set_default_remote_addr",
				"Service not provided", dls->name);
		return NULL;
	}
	return dls->set_default_remote_addr(dls, addr, normal, ex);
}

bool dlsap_open(dls_t *dls, dls_t *back, exception_t *ex)
{
	if (!dls) {
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME,
				"dlsap_open",
				"Data Link Service is NULL", NULL);
		return false;
	}
	if (!dls->open) {
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME,
				"dlsap_open",
				"Service not provided", dls->name);
		return false;
	}
	return dls->open(dls, back, ex);
}

void dlsap_close(dls_t *dls)
{
	if (!dls)
		return;
	if (!dls->close)
		return;
	dls->close(dls);
}

bool dlsap_write(dls_t *dls, primitive_t *prim, bool expedited,
		exception_t *ex)
{
	if (!dls) {
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME,
				"dlsap_write",
				"Data Link Service is NULL", NULL);
		return false;
	}
	if (!prim) {
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME,
				"dlsap_write",
				"Primitive is NULL", NULL);
		return false;
	}
	if (!dls->on_write) {
		exception_fill(ex, EXIT_FAILURE, MODULE_NAME,
				"dlsap_write",
				"Service not provided", dls->name);
		return false;
	}
	return dls->on_write(dls, prim, expedited, ex);
}

void get_queue_stats(dls_t *dls, dls_stats_t *stats)
{
	if (!stats)
		return;
	memset(stats, 0x00, sizeof(struct dls_stats));
	if (!dls) {
		return;
	}
	if (!dls->get_queue_stats)
		return;
	dls->get_queue_stats(dls, stats);
}
