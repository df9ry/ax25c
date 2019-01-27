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

VPATH    = $(SRCDIR)
CFLAGS   = -shared -Wall -g -ggdb -fpic -fmessage-length=0 -pthread \
			-I/usr/local/include/
			
TARGET   = libax25c_runtime.$(SOEXT)
OBJS     = ax25c_runtime.o memory.o log.o tick.o dlsap.o dl_prim.o
LIBS     = -L/usr/local/lib -lmapc -lringbuffer -ldl -lpthread

all: $(TARGET)
	cp $(TARGET) $(SRCDIR)/../_$(_CONF)
	
clean:
	rm -rf $(SRCDIR)/$(OBJDIR) $(SRCDIR)/$(DOCDIR)

install:

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
	
%.o: %.c $(SRCDIR)
	$(CC) $(CFLAGS) -c $<	

endif

