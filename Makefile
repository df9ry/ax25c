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

CFLAGS   =  -Wall -g -fmessage-length=0 -pthread

LDFLAGS  =  -Wall -g -fmessage-length=0 -pthread
			
OBJS     =  \
	ax25.o
			
LIBS     =  -lpthread

TARGET   =  ax25

$(TARGET):  $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
	
%.o: %.c $(SRCDIR)
	$(CC) $(CFLAGS) -c $<	
	
all: $(TARGET)
	echo "Build OK"

doc:
	doxygen $(SRCDIR)/doxygen.conf
	( cd $(SRCDIR)/_Documentation/latex && make )
	cp $(SRCDIR)/_Documentation/latex/refman.pdf \
		$(SRCDIR)/_Documentation/ax25c.pdf

test: $(TARGET)
	./$(TARGET)
	
install: $(TARGET) doc
	sudo cp $(TARGET) /usr/local/bin
	sudo chown root:staff /usr/local/bin/$(TARGET)
	sudo cp -rf $(SRCDIR)/uki /usr/local/include
	sudo chown -R root:staff /usr/local/include/uki
	sudo mkdir -p /usr/local/doc
	sudo cp $(SRCDIR)/_Documentation/ax25c.pdf /usr/local/doc
	
#----- Begin Boilerplate
endif
