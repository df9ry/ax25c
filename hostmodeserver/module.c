/*
 *  Project: ax25c - File: module.c
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

#include "../config/configuration.h"
#include "../runtime/primbuffer.h"
#include "../runtime/runtime.h"
#include "../runtime/dlsap.h"

#include "_internal.h"

#include <unistd.h>
#include <signal.h>
#include <assert.h>

#undef DEBUG_POLLS

struct plugin_handle plugin;

static struct  setting_descriptor plugin_settings_descriptor[] = {
		{ NULL }
};

static struct setting_descriptor instance_settings_descriptor[] = {
		{ "comport",  CSTR_T, offsetof(struct instance_handle, comport),  "COM1"   },
		{ "baudrate", UINT_T, offsetof(struct instance_handle, baudrate), "9600"   },
		{ "channels", UINT_T, offsetof(struct instance_handle, channels), "4"      },
		{ NULL }
};

static int read_n(HANDLE h, char *pb, int cb)
{
	int l = 0, n;
	while (l < cb) {
		n = readFromSerialPort(h, &pb[l], cb - l);
		if (n < 0)
			return n;
		l += n;
	} /* end while */
	return l;
}

static int write_n(HANDLE h, char *pb, int cb)
{
	int l = 0, n;
	while (l < cb) {
		n = writeToSerialPort(h, &pb[l], cb - l);
		if (n < 0)
			return n;
		l += n;
	} /* end while */
	return l;
}

static inline int write_cstr(struct instance_handle *instance,
		int channel, int code, const char *s)
{
	assert(instance);
	assert(s);
	int l = strlen(s);
	assert(l < S_SIO_BUF - 3);
	instance->sio_buf[0] = channel;
	instance->sio_buf[1] = code;
	memcpy(&instance->sio_buf[2], s, l+1);
#ifdef DEBUG_POLLS
	ax25c_log(DEBUG_LEVEL_DEBUG,
			MODULE_NAME ":worker:response [%i,%i] \"%s\"",
			channel, code, s);
#endif
	return write_n(instance->serial, instance->sio_buf, l+3);
}

static inline int write_ok(struct instance_handle *instance,
		int channel)
{
	assert(instance);
	instance->sio_buf[0] = channel;
	instance->sio_buf[1] = 0;
#ifdef DEBUG_POLLS
	ax25c_log(DEBUG_LEVEL_DEBUG,
			MODULE_NAME ":worker:response [%i,OK]",
			channel);
#endif
	return write_n(instance->serial, instance->sio_buf, 2);
}

static void *worker(void *id)
{
	struct instance_handle *instance = id;
	int n, channel, cmd, len;

	assert(instance);
	//configuration.loglevel = DEBUG_LEVEL_DEBUG;
	while (instance->alive) {
		n = read_n(instance->serial, instance->sio_buf, 3);
		if (!instance->alive)
			return NULL;
		if (n < 0) {
			ax25c_log(DEBUG_LEVEL_ERROR,
					MODULE_NAME ":worker:read_n(cmd) error %i:%s",
					-n, strerror(-n));
			continue;
		}
		assert (n == 3);
		channel = instance->sio_buf[0];
		cmd = instance->sio_buf[1];
		len = instance->sio_buf[2] + 1;
		assert(len <= 256);
		/*
		ax25c_log(DEBUG_LEVEL_DEBUG,
				MODULE_NAME ":worker: channel %i [%s] %i bytes",
				channel, cmd ? "cmd" : "data", len);
		*/
		n = read_n(instance->serial, instance->sio_buf, len);
		if (!instance->alive)
			return NULL;
		if (n < 0) {
			ax25c_log(DEBUG_LEVEL_ERROR,
					MODULE_NAME ":worker:read_n(data) error %i:%s",
					-n, strerror(-n));
			continue;
		}
		assert(n == len);
		instance->sio_buf[len] = '\0';

		if ((channel < 0) || (channel > instance->channels)) {
			n = write_cstr(instance, channel, 2, "INVALID CHANNEL NUMBER");
			continue;
		}

		if (cmd) {
			/* Handle command */
#ifdef DEBUG_POLLS
			ax25c_log(DEBUG_LEVEL_DEBUG,
					MODULE_NAME ":worker:command [%i]: \"%s\"",
					channel, instance->sio_buf);
#endif
			char c = instance->sio_buf[0];
			if (c >= 0x20) {
				switch (c) {
				case 'A':
					ax25c_log(DEBUG_LEVEL_INFO,
							MODULE_NAME ": AUTO LINEFEED [%i] %s",
							channel, instance->sio_buf);
					n = write_ok(instance, channel);
					break;
				case 'B':
					ax25c_log(DEBUG_LEVEL_INFO,
							MODULE_NAME ": TERMINAL BAUDRATE [%i] %s",
							channel, instance->sio_buf);
					n = write_ok(instance, channel);
					break;
				case 'C':
					if (len == 1) {
						n = write_cstr(instance, channel, 1, "CHANNEL NOT CONNECTED");
					} else {
						ax25c_log(DEBUG_LEVEL_INFO,
								MODULE_NAME ": CONNECT REQUEST [%i] %s",
								channel, instance->sio_buf);
						n = write_ok(instance, channel);
					}
					break;
				case 'D':
					ax25c_log(DEBUG_LEVEL_INFO,
							MODULE_NAME ": DISCONNECT [%i] %s",
							channel, instance->sio_buf);
					n = write_ok(instance, channel);
					break;
				case 'E':
					ax25c_log(DEBUG_LEVEL_INFO,
							MODULE_NAME ": ECHO INPUT [%i] %s",
							channel, instance->sio_buf);
					n = write_ok(instance, channel);
					break;
				case 'F':
					ax25c_log(DEBUG_LEVEL_INFO,
							MODULE_NAME ": FRAME ACKNOWLEDGE [%i] %s",
							channel, instance->sio_buf);
					n = write_ok(instance, channel);
					break;
				case 'G':
					n = write_ok(instance, channel);
					break;
				case 'H':
					ax25c_log(DEBUG_LEVEL_INFO,
							MODULE_NAME ": HDLC BAUDRATE [%i] %s",
							channel, instance->sio_buf);
					n = write_ok(instance, channel);
					break;
				case 'I':
					n = write_ok(instance, channel);
					break;
				case 'J':
					if (strcmp(instance->sio_buf, "JHOST1") == 0)
						ax25c_log(DEBUG_LEVEL_INFO,
								MODULE_NAME ": HOSTMODE ENTER");
					else
						ax25c_log(DEBUG_LEVEL_INFO,
								MODULE_NAME ": HOSTMODE EXIT");
					n = write_ok(instance, channel);
					break;
				case 'K':
					ax25c_log(DEBUG_LEVEL_INFO,
							MODULE_NAME ": CALIBRATE [%i] %s",
							channel, instance->sio_buf);
					n = write_ok(instance, channel);
					break;
				case 'L':
					ax25c_log(DEBUG_LEVEL_INFO,
							MODULE_NAME ": L POLL [%i] %s",
							channel, instance->sio_buf);
					if (channel == 0)
						n = write_cstr(instance, channel, 1, "0 0");
					else
						n = write_cstr(instance, channel, 1, "0 0 0 0 0 1");
					break;
				case 'M':
					ax25c_log(DEBUG_LEVEL_INFO,
							MODULE_NAME ": MONITOR CONFIG [%i] %s",
							channel, instance->sio_buf);
					n = write_ok(instance, channel);
					break;
				case 'O':
					ax25c_log(DEBUG_LEVEL_INFO,
							MODULE_NAME ": OUTSTANDING I FRAMES [%i] %s",
							channel, instance->sio_buf);
					n = write_ok(instance, channel);
					break;
				case 'P':
					ax25c_log(DEBUG_LEVEL_INFO,
							MODULE_NAME ": PERM COMMAND [%i] %s",
							channel, instance->sio_buf);
					n = write_ok(instance, channel);
					break;
				case 'Q':
					ax25c_log(DEBUG_LEVEL_INFO,
							MODULE_NAME ": RESTART FIRMWARE [%i] %s",
							channel, instance->sio_buf);
					n = write_ok(instance, channel);
					break;
				case 'R':
					ax25c_log(DEBUG_LEVEL_INFO,
							MODULE_NAME ": REPEATER ENABLE [%i] %s",
							channel, instance->sio_buf);
					n = write_ok(instance, channel);
					break;
				case 'S':
					ax25c_log(DEBUG_LEVEL_INFO,
							MODULE_NAME ": SELECT CHANNEL [%i] %s",
							channel, instance->sio_buf);
					n = write_ok(instance, channel);
					break;
				case 'T':
					ax25c_log(DEBUG_LEVEL_INFO,
							MODULE_NAME ": TRANSMITTER DELAY [%i] %s",
							channel, instance->sio_buf);
					n = write_ok(instance, channel);
					break;
				case 'U':
					ax25c_log(DEBUG_LEVEL_INFO,
							MODULE_NAME ": UNATTENDED MODE [%i] %s",
							channel, instance->sio_buf);
					n = write_ok(instance, channel);
					break;
				case 'V':
					n = write_cstr(instance, channel, 1, "2");
					break;
				case 'W':
					ax25c_log(DEBUG_LEVEL_INFO,
							MODULE_NAME ": TRANSMITTER WAIT [%i] %s",
							channel, instance->sio_buf);
					n = write_ok(instance, channel);
					break;
				case 'X':
					ax25c_log(DEBUG_LEVEL_INFO,
							MODULE_NAME ": PTT ENABLE [%i] %s",
							channel, instance->sio_buf);
					n = write_ok(instance, channel);
					break;
				case 'Y':
					ax25c_log(DEBUG_LEVEL_INFO,
							MODULE_NAME ": MAXIMUM CONNECTIONS [%i] %s",
							channel, instance->sio_buf);
					n = write_ok(instance, channel);
					break;
				case 'Z':
					ax25c_log(DEBUG_LEVEL_INFO,
							MODULE_NAME ": FLOW CONTROL [%i] %s",
							channel, instance->sio_buf);
					n = write_ok(instance, channel);
					break;
				case '@':
					ax25c_log(DEBUG_LEVEL_INFO,
							MODULE_NAME ": BUFFERS [%i] %s",
							channel, instance->sio_buf);
					n = write_ok(instance, channel);
					break;
				case '#':
					n = write_cstr(instance, channel, 1, "CHANNEL NOT CONNECTD");
					break;
				default:
					ax25c_log(DEBUG_LEVEL_INFO,
							MODULE_NAME ": UNIDENTIFIED COMMAND [%i] %s",
							channel, instance->sio_buf);
					n = write_ok(instance, channel);
					break;
				} /* end switch */
			} else {
				n = write_cstr(instance, channel, 2, "INVALID COMMAND");
			}
		} else {
			/* Handle data */
#ifdef DEBUG_POLLS
			ax25c_log(DEBUG_LEVEL_DEBUG,
					MODULE_NAME ":worker:data [%i]: \"%s\"",
					channel, instance->sio_buf);
#endif
			n = write_ok(instance, channel);
		}
		if (n < 0) {
			ax25c_log(DEBUG_LEVEL_ERROR,
					MODULE_NAME ":worker:write_str(resp) error %i:%s",
					-n, strerror(-n));
			continue;
		}
	} /* end while */
	return NULL;
}

static void *get_plugin(const char *name,
		configurator_func configurator, void *context, struct exception *ex)
{
	assert(name);
	assert(configurator);
	memset(&plugin, 0x00, sizeof(struct plugin_handle));
	plugin.name = name;
	if (!configurator(&plugin, plugin_settings_descriptor, context, ex)) {
		return NULL;
	}
	return &plugin;
}

static bool start_plugin(struct plugin_handle *plugin, struct exception *ex) {
	assert(plugin);
	DBG_DEBUG("hostmodeserver start", plugin->name);
	return true;
}

static bool stop_plugin(struct plugin_handle *plugin, struct exception *ex) {
	assert(plugin);
	DBG_DEBUG("hostmodeserver stop", plugin->name);
	return true;
}

static void *get_instance(const char *name,
		configurator_func configurator, void *context, struct exception *ex)
{
	struct instance_handle *instance;
	assert(name);
	assert(configurator);
	DBG_DEBUG("hostmodeserver instance create", name);
	instance = (struct instance_handle*)malloc(sizeof(struct instance_handle));
	assert(instance);
	memset(instance, 0x00, sizeof(struct instance_handle));
	instance->name = name;
	if (!configurator(instance, instance_settings_descriptor, context, ex)) {
		free(instance);
		return NULL;
	}
	instance->name = name;
	return instance;
}

static bool start_instance(struct instance_handle *instance, exception_t *ex)
{
	pthread_attr_t thread_args;
	int erc;

	assert(instance);
	DBG_DEBUG("hostmodeserver instance start", instance->name);

	DBG_INFO("Open serial port", instance->comport);
	instance->serial = openSerialPort(instance->comport, instance->baudrate,
			8, ONESTOPBIT, NOPARITY, ex);
	if (!instance->serial)
		return false;

	instance->alive = true;
	pthread_attr_init(&thread_args);
	pthread_attr_setdetachstate(&thread_args, PTHREAD_CREATE_JOINABLE);
	erc = pthread_create(&instance->thread, &thread_args, worker, instance);
	if (erc != 0) {
		exception_fill(ex, erc, MODULE_NAME, "start_instance",
				"Error creating thread", instance->name);
		instance->alive = false;
	}
	pthread_attr_destroy(&thread_args);
	return instance->alive;
}

static bool stop_instance(struct instance_handle *instance, exception_t *ex) {
	assert(instance);
	DBG_DEBUG("hostmodeserver instance stop", instance->name);

	if (instance->alive) {
		instance->alive = false;
		pthread_kill(instance->thread, SIGINT);
	}

	DBG_INFO("Close serial port", instance->comport);
	closeSerialPort(instance->serial);
	return true;
}

struct plugin_descriptor plugin_descriptor = {
		get_plugin,	  (start_func)start_plugin,   (stop_func)stop_plugin,
		get_instance, (start_func)start_instance, (stop_func)stop_instance
};
