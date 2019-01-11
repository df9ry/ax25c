#   Project ax25c
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
#----- End Boilerplate

VPATH = $(SRCDIR)

CFLAGS   =  -Wall -g -ggdb -fmessage-length=0 -pthread

LDFLAGS  =  -Wall -g -ggdb -fmessage-length=0 -pthread
			
LIBS     =  -L$(SRCDIR)/runtime/_$(_CONF) -lax25c_runtime \
			-lpthread

TARGET   =  runtime ax25c

.PHONY: all $(TARGET)

runtime:
	$(MAKE) -C $(SRCDIR)/runtime all

ax25c: ax25c.o
	$(CC) $(LDFLAGS) -o ax25c ax25c.o $(LIBS)
	
%.o: %.c $(SRCDIR)
	$(CC) $(CFLAGS) -c $<	
	
all: $(TARGET)
	echo "Build OK"

doc:
	doxygen $(SRCDIR)/doxygen.conf
	( cd $(SRCDIR)/_Documentation/latex && make )
	cp $(SRCDIR)/_Documentation/latex/refman.pdf \
		$(SRCDIR)/_Documentation/ax25c.pdf
		
clean:
	$(MAKE) -C $(SRCDIR)/runtime clean
	
install: ax25c doc
	sudo cp ax25c /usr/local/bin
	sudo chown root:staff /usr/local/bin/ax25c
	sudo mkdir -p /usr/local/doc
	sudo cp $(SRCDIR)/_Documentation/ax25c.pdf /usr/local/doc
	$(MAKE) -C $(SRCDIR)/runtime install
	
run: $(TARGET)
	@LD_LIBRARY_PATH=$(SRCDIR)/runtime/_$(_CONF) \
		$(SRCDIR)/_$(_CONF)/ax25c
	
	
#----- Begin Boilerplate
endif
