#   Project ringbuffer
#   Copyright (C) 2019 tania@df9ry.de
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU Affero General Public License as
#   published by the Free Software Foundation, either version 3 of the
#   License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Affero General Public License for more details.
#
#   You should have received a copy of the GNU Affero General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
	
ifeq (,$(filter _%,$(notdir $(CURDIR))))
include target.mk
else

VPATH    = $(SRCDIR)
CFLAGS   =  -Wall -Werror -g -ggdb -fpic -fmessage-length=0 -pthread

OBJS     =  ringbuffer.o
LIBS     =  -lpthread
_TARGET  =  ringbuffer
TARGET   =  lib$(_TARGET).$(SOEXT)

all: $(TARGET)
	@echo "*** Build ringbuffer OK ***"

$(TARGET): $(OBJS)
	$(CC) -shared $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
	
%.o: %.c $(SRCDIR)
	$(CC) -shared $(CFLAGS) -c $<
	
doc:
	doxygen $(SRCDIR)/doxygen.conf
	( cd $(SRCDIR)/$(DOCDIR)/latex && make )
	cp $(SRCDIR)/$(DOCDIR)/latex/refman.pdf \
		$(SRCDIR)/$(DOCDIR)/$(_TARGET).pdf

test: $(TARGET) $(_TARGET)_test.c
	$(CC) $(CFLAGS) -o $(_TARGET)_test \
	-L$(SRCDIR)/_Debug -l$(_TARGET) \
	$(SRCDIR)/$(_TARGET)_test.c
	sh -c "LD_LIBRARY_PATH=./ ./$(_TARGET)_test"

install: all
ifneq ($(OS),GNU/Linux)
	cp $(TARGET) /usr/local/lib/
else
	cp $(TARGET) /usr/local/lib/$(TARGET).0.1.0
	chmod 0755   /usr/local/lib/$(TARGET).0.1.0	
	( cd /usr/local/lib && ln -sf $(TARGET).0.1.0 $(TARGET).0.1 )
	( cd /usr/local/lib && ln -sf $(TARGET).0.1.0 $(TARGET).0   )
	( cd /usr/local/lib && ln -sf $(TARGET).0.1.0 $(TARGET)     )
endif
	cp -rf ../$(_TARGET).h /usr/local/include/

endif
