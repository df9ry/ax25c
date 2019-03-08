# Copyright 2017 Tania Hagn

# This file is part of ax25c.
# 
#     Daisy is free software: you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.
# 
#     Daisy is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
# 
#     You should have received a copy of the GNU General Public License
#     along with Daisy.  If not, see <http://www.gnu.org/licenses/>.

ifeq (,$(filter _%,$(notdir $(CURDIR))))
include ../target.mk
else
#----- End Boilerplate

VPATH = $(SRCDIR)

CFLAGS   =  $(WINFLAGS) \
			-shared -Wall -g -ggdb -fpic -fmessage-length=0 -pthread \
			-I$(LOCAL)/include/
LDFLAGS  =  $(WINFLAGS) \
			-shared -Wall -g -ggdb -fpic -fmessage-length=0 -pthread
			
TARGET   =  axtnos.so
OBJS     =  module.o lapb.o ax25.o ax25_subr.o mbuf.o timer.o misc.o \
			unix.o kernel.o
LIBS     =  $(WINLIBS) \
			-L$(SRCDIR)/../_$(_CONF) -lax25c_runtime \
			-lpthread

all: $(TARGET)
	cp $(TARGET) ../../_$(_CONF)
	
clean:
	rm -rf $(SRCDIR)/$(OBJDIR)/* $(SRCDIR)/$(DOCDIR)/*

install:

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
	
%.o: %.c $(SRCDIR)
	$(CC) $(CFLAGS) -c $<	

#----- Begin Boilerplate
endif
