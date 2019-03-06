/*
 *  Project: ax25c - File: serial.h
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

/** @file serial.h */

#ifndef SERIAL_SERIAL_H_
#define SERIAL_SERIAL_H_

#ifdef __MINGW32__
#include <windows.h>
#else
#endif

struct exception;

enum Baudrate
{
	B50			= 50,
	B110		= 110,
	B150		= 150,
	B300		= 300,
	B1200		= 1200,
	B2400		= 2400,
	B4800		= 4800,
	B9600   	= 9600,
	B19200		= 19200,
	B38400		= 38400,
	B57600		= 57600,
	B115200 	= 115200,
	B230400		= 230400,
	B460800		= 460800,
	B500000 	= 500000,
	B1000000	= 1000000
};

enum Stopbits
{
	one          = ONESTOPBIT,
	onePointFive = ONE5STOPBITS,
	two          = TWOSTOPBITS
};

enum Paritycheck
{
	even = EVENPARITY,
	odd  = ODDPARITY,
	off  = NOPARITY,
	mark = MARKPARITY
};

/**
	@brief Opens a new connection to a serial port
	@param portname		name of the serial port(COM1 - COM9 or \\\\.\\COM1-COM256)
	@param baudrate		the baudrate of this port (for example 9600)
	@param databits     the number of databits
	@param stopbits		the nuber of stoppbits (one, onePointFive or two)
	@param parity		the parity (even, odd, off or mark)
	@param ex           exception struct (optional)
	@return				HANDLE to the serial port or NULL on error
*/
extern HANDLE openSerialPort(LPCSTR portname, enum Baudrate baudrate,
		int databits, enum Stopbits stopbits, enum Paritycheck parity,
		struct exception *ex);

/**
	@brief Read data from the serial port
	@param hSerial		File HANDLE to the serial port
	@param buffer		pointer to the area where the read data will be written
	@param buffersize	maximal size of the buffer area
	@return				amount of data that was read or negative error code
*/
extern int readFromSerialPort(HANDLE hSerial, char *buffer, int buffersize);

/**
	@brief Write data to the serial port
	@param hSerial	File HANDLE to the serial port
	@param buffer	pointer to the area where the read data will be read
	@param length	amount of data to be read
	@return			amount of data that was written or negative errorcode
*/
extern int writeToSerialPort(HANDLE hSerial, char *data, int length);

/**
 	@brief Close serial port
	@param hSerial	File HANDLE to the serial port
*/
extern void closeSerialPort(HANDLE hSerial);

#endif /* SERIAL_SERIAL_H_ */
