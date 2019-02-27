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
CXXFLAGS = -shared -Wall -g -ggdb -fpic -fmessage-length=0 -pthread \
			-I$(LOCAL)/include/

TARGET   = ax25c_config.so
OBJS     = DOMTreeErrorReporter.o ax25c_config.o
LIBS     = -L$(SRCDIR)/../_$(_CONF) -lax25c_runtime \
			-lxerces-c \
			-L$(LOCAL)/$(SODIR) -lmapc -lstringc \
			-lpthread

all: $(TARGET)
	cp $(TARGET) $(SRCDIR)/../_$(_CONF)
	
clean:
	rm -rf $(SRCDIR)/$(OBJDIR)/* $(SRCDIR)/$(DOCDIR)/*

install:

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
	
%.o: %.cpp $(SRCDIR)
	$(CXX) $(CXXFLAGS) -c $<	

endif
