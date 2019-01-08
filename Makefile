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

TARGET   =  ax25c
OBJS     =  ax25c.o
LIBS     =  -L$(SRCDIR)/runtime/_$(_CONF) -lax25c_runtime \
			-lpthread

all: $(OBJS)
	$(MAKE) -C $(SRCDIR)/runtime all
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
	$(MAKE) -C $(SRCDIR)/config all
	$(MAKE) -C $(SRCDIR)/terminal all
	$(MAKE) -C $(SRCDIR)/mm_simple all
	echo "Build OK"

%.o: %.c %.h Makefile
	$(CC) $(CFLAGS) -c $<	
	
doc:
	doxygen $(SRCDIR)/doxygen.conf
	( cd $(SRCDIR)/$(DOCDIR)/latex && make )
	cp $(SRCDIR)/$(DOCDIR)/latex/refman.pdf \
		$(SRCDIR)/$(DOCDIR)/$(TARGET).pdf
		
clean:
	rm -rf $(SRCDIR)/$(OBJDIR) $(SRCDIR)/$(DOCDIR)
	$(MAKE) -C $(SRCDIR)/runtime clean
	$(MAKE) -C $(SRCDIR)/config clean
	$(MAKE) -C $(SRCDIR)/terminal clean
	$(MAKE) -C $(SRCDIR)/mm_simple clean
	
install: ax25c doc
	sudo cp $(TARGET) /usr/local/bin
	sudo chown root:staff /usr/local/bin/$(TARGET)
	sudo mkdir -p /usr/local/doc
	sudo cp $(SRCDIR)/$(DOCDIR)/$(TARGET).pdf /usr/local/doc
	$(MAKE) -C $(SRCDIR)/runtime install
	$(MAKE) -C $(SRCDIR)/config install
	$(MAKE) -C $(SRCDIR)/terminal install
	$(MAKE) -C $(SRCDIR)/mm_simple install
	
run: all
	@echo "Executing $(TARGET)"
	@LD_LIBRARY_PATH=$(SRCDIR)/_$(_CONF)/ \
		$(SRCDIR)/_$(_CONF)/$(TARGET) $(SRCDIR)/$(TARGET).xml
	@echo "OK"

#----- Begin Boilerplate
endif
