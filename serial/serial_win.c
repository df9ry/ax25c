/*
 *  Project: ax25c - File: serial_win.c
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

#include <stdio.h>
#include <windows.h>
#include <errno.h>

#include "../runtime/exception.h"

#include "serial.h"

HANDLE openSerialPort(LPCSTR portname, enum Baudrate baudrate, int databits,
		enum Stopbits stopbits, enum Paritycheck parity, struct exception *ex)
{
	DWORD  accessdirection = GENERIC_READ | GENERIC_WRITE;
	HANDLE hSerial = CreateFile(portname, accessdirection, 0, 0, OPEN_EXISTING, 0, 0);
	if (hSerial == INVALID_HANDLE_VALUE) {
		exception_fill(ex, -EINVAL, "SERIAL", "openSerialPort", "CreateFile",
				portname);
		return NULL;
	}

	DCB dcbSerialParams = {0};
	dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
	if (!GetCommState(hSerial, &dcbSerialParams)) {
		exception_fill(ex, -EINVAL, "SERIAL", "openSerialPort", "GetCommState",
				portname);
		return NULL;
	}
	dcbSerialParams.BaudRate = baudrate;
	dcbSerialParams.ByteSize = databits;
	dcbSerialParams.StopBits = stopbits;
	dcbSerialParams.Parity   = parity;
	if(!SetCommState(hSerial, &dcbSerialParams)){
		exception_fill(ex, -EINVAL, "SERIAL", "openSerialPort", "SetCommState",
				portname);
		return NULL;
	}
	COMMTIMEOUTS timeouts={0};
	timeouts.ReadIntervalTimeout         = 50;
	timeouts.ReadTotalTimeoutConstant    = 50;
	timeouts.ReadTotalTimeoutMultiplier  = 10;
	timeouts.WriteTotalTimeoutConstant   = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	if(!SetCommTimeouts(hSerial, &timeouts)){
		exception_fill(ex, -EINVAL, "SERIAL", "openSerialPort", "SetCommTimeouts",
				portname);
		return NULL;
	}
	return hSerial;
}

int readFromSerialPort(HANDLE hSerial, char * buffer, int buffersize)
{
    DWORD dwBytesRead = 0;
    if(!ReadFile(hSerial, buffer, buffersize, &dwBytesRead, NULL)){
        return -EINVAL;
    }
    return (int)dwBytesRead;
}

int writeToSerialPort(HANDLE hSerial, char * data, int length)
{
	DWORD dwBytesRead = 0;
	if(!WriteFile(hSerial, data, length, &dwBytesRead, NULL)){
		return -EINVAL;
	}
	return (int)dwBytesRead;
}

void closeSerialPort(HANDLE hSerial)
{
	CloseHandle(hSerial);
}
