/*
 *  Project: ringbuffer - File: reingbuffer_test.c
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "ringbuffer.h"

static char test[100] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklom"
						"1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklom";
static char buf[100];

int main(int argc, char *argv[]) {
	struct ringbuffer rb;

	printf("Testing rb_init\n");
	assert(rb_init(&rb, 250) == 0);
	printf("OK\n");

	printf("Testing rb_getsize\n");
	assert(rb_get_size(&rb) == 250);
	printf("OK\n");

	printf("Testing rb_write_block\n");
	assert(rb_get_used(&rb) == 0);
	assert(rb_get_free(&rb) == 250);
	assert(rb_write_block_ch(&rb, test, 50) == 50);
	assert(rb_get_used(&rb) == 50);
	assert(rb_get_free(&rb) == 200);
	assert(rb_write_block_ch(&rb, test, 50) == 50);
	assert(rb_get_used(&rb) == 100);
	assert(rb_get_free(&rb) == 150);
	assert(rb_write_block_ch(&rb, test, 50) == 50);
	assert(rb_get_used(&rb) == 150);
	assert(rb_get_free(&rb) == 100);
	assert(rb_write_block_ch(&rb, test, 50) == 50);
	assert(rb_get_used(&rb) == 200);
	assert(rb_get_free(&rb) == 50);
	assert(rb_write_block_ch(&rb, test, 50) == 50);
	assert(rb_get_used(&rb) == 250);
	assert(rb_get_free(&rb) == 0);
	printf("OK\n");

	printf("Testing rb_read_block\n");
	assert(rb_read_block_ch(&rb, buf, 10) == 10);
	assert(memcmp(test, buf, 10) == 0);
	assert(rb_get_used(&rb) == 240);
	assert(rb_get_free(&rb) == 10);
	assert(rb_read_block_ch(&rb, buf, 40) == 40);
	assert(memcmp(&test[10], buf, 40) == 0);
	assert(rb_get_used(&rb) == 200);
	assert(rb_get_free(&rb) == 50);
	assert(rb_read_block_ch(&rb, buf, 60) == 60);
	assert(memcmp(test, buf, 60) == 0);
	assert(rb_get_used(&rb) == 140);
	assert(rb_get_free(&rb) == 110);
	assert(rb_read_block_ch(&rb, buf, 70) == 70);
	assert(memcmp(&test[10], buf, 70) == 0);
	assert(rb_get_used(&rb) == 70);
	assert(rb_get_free(&rb) == 180);
	assert(rb_write_block_ch(&rb, test, 100) == 100);
	assert(rb_get_used(&rb) == 170);
	assert(rb_get_free(&rb) == 80);
	assert(rb_read_block_ch(&rb, buf, 50) == 50);
	assert(memcmp(&test[30], buf, 50) == 0);
	assert(rb_get_used(&rb) == 120);
	assert(rb_get_free(&rb) == 130);
	assert(rb_read_block_ch(&rb, buf, 50) == 50);
	assert(memcmp(&test[30], buf, 50) == 0);
	assert(rb_get_used(&rb) == 70);
	assert(rb_get_free(&rb) == 180);
	assert(rb_read_block_ch(&rb, buf, 100) == 70);
	assert(memcmp(&test[30], buf, 70) == 0);
	assert(rb_write_block_ch(&rb, test, 90) == 90);
	printf("OK\n");

	printf("Testing rb_clear\n");
	rb_clear(&rb);
	assert(rb_get_used(&rb) == 0);
	assert(rb_get_free(&rb) == 250);
	printf("OK\n");

	printf("Testing rb_write_nonblock\n");
	assert(rb_get_used(&rb) == 0);
	assert(rb_get_free(&rb) == 250);
	assert(rb_write_nonblock_ch(&rb, test, 50) == 50);
	assert(rb_get_used(&rb) == 50);
	assert(rb_get_free(&rb) == 200);
	assert(rb_write_nonblock_ch(&rb, test, 50) == 50);
	assert(rb_get_used(&rb) == 100);
	assert(rb_get_free(&rb) == 150);
	assert(rb_write_nonblock_ch(&rb, test, 50) == 50);
	assert(rb_get_used(&rb) == 150);
	assert(rb_get_free(&rb) == 100);
	assert(rb_write_nonblock_ch(&rb, test, 50) == 50);
	assert(rb_get_used(&rb) == 200);
	assert(rb_get_free(&rb) == 50);
	assert(rb_write_nonblock_ch(&rb, test, 100) == -ENOMEM);
	assert(rb_get_used(&rb) == 200);
	assert(rb_get_free(&rb) == 50);
	assert(rb_write_nonblock_ch(&rb, test, 50) == 50);
	assert(rb_get_used(&rb) == 250);
	assert(rb_get_free(&rb) == 0);
	printf("OK\n");

	printf("Testing rb_read_nonblock\n");
	assert(rb_read_nonblock_ch(&rb, buf, 10) == 10);
	assert(memcmp(test, buf, 10) == 0);
	assert(rb_get_used(&rb) == 240);
	assert(rb_get_free(&rb) == 10);
	assert(rb_read_nonblock_ch(&rb, buf, 40) == 40);
	assert(memcmp(&test[10], buf, 40) == 0);
	assert(rb_get_used(&rb) == 200);
	assert(rb_get_free(&rb) == 50);
	assert(rb_read_nonblock_ch(&rb, buf, 60) == 60);
	assert(memcmp(test, buf, 60) == 0);
	assert(rb_get_used(&rb) == 140);
	assert(rb_get_free(&rb) == 110);
	assert(rb_read_nonblock_ch(&rb, buf, 70) == 70);
	assert(memcmp(&test[10], buf, 70) == 0);
	assert(rb_get_used(&rb) == 70);
	assert(rb_get_free(&rb) == 180);
	assert(rb_read_nonblock_ch(&rb, buf, 100) == -EAGAIN);
	assert(rb_get_used(&rb) == 70);
	assert(rb_get_free(&rb) == 180);
	assert(rb_write_nonblock_ch(&rb, test, 100) == 100);
	assert(rb_get_used(&rb) == 170);
	assert(rb_get_free(&rb) == 80);
	assert(rb_read_nonblock_ch(&rb, buf, 50) == 50);
	assert(memcmp(&test[30], buf, 50) == 0);
	assert(rb_get_used(&rb) == 120);
	assert(rb_get_free(&rb) == 130);
	assert(rb_read_nonblock_ch(&rb, buf, 50) == 50);
	assert(memcmp(&test[30], buf, 50) == 0);
	assert(rb_get_used(&rb) == 70);
	assert(rb_get_free(&rb) == 180);
	printf("OK\n");

	printf("Testing rb_loos and ..lost\n");
	assert(rb_get_lost(&rb) == 0);
	assert(rb_loose(&rb, 25) == 25);
	assert(rb_get_lost(&rb) == 25);
	assert(rb_loose(&rb, 25) == 50);
	assert(rb_get_lost(&rb) == 50);
	assert(rb_clear_lost(&rb) == 50);
	assert(rb_get_lost(&rb) == 0);
	printf("OK\n");

	printf("Testing rb_destroy\n");
	assert(rb_destroy(&rb) == 0);
	printf("OK\n");

	return EXIT_SUCCESS;
}
