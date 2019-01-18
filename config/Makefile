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

ifeq ($(OS),Cygwin)
	XERCESLIB := -L/usr/bin/ -lxerces-c-3-1
else
	XERCESLIB := -lxerces-c
endif

VPATH    = $(SRCDIR)
CXXFLAGS = -shared -Wall -g -ggdb -fpic -fmessage-length=0 -pthread

TARGET   = ax25c_config.$(SOEXT)
OBJS     = DOMTreeErrorReporter.o ax25c_config.o
LIBS     = -L$(SRCDIR)/../_$(_CONF) -lax25c_runtime \
			$(XERCESLIB) \
			-L/usr/local/lib -lmapc \
			-lpthread

all: $(TARGET)
	cp $(TARGET) $(SRCDIR)/../_$(_CONF)
	
clean:
	rm -rf $(SRCDIR)/$(OBJDIR) $(SRCDIR)/$(DOCDIR)

install:

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
	
%.o: %.cpp $(SRCDIR)
	$(CXX) $(CXXFLAGS) -c $<	

endif
