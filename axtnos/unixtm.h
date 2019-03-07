/*
 *  Project: ax25c - File: unixtm.h
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

/** @file unixtm.h */

#ifndef AXTNOS_UNIXTM_H_
#define AXTNOS_UNIXTM_H_

struct date
{
    int da_year;
    int da_day;
    int da_mon;
};

struct time
{
    int ti_min;
    int ti_hour;
    int ti_sec;
    int ti_hund;
};

extern void tnos_getdate (struct date *);
extern void gettime (struct time *);
extern long secclock (void);
extern long msclock (void);
#ifndef _HARDWARE_H
extern long dostounix (struct date *, struct time *);
#endif

#endif /* AXTNOS_UNIXTM_H_ */
