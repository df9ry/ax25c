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

VPATH    =  $(SRCDIR)
CFLAGS   =  -Wall -Werror -g -ggdb -fmessage-length=0 -pthread

LIBS     =  -L$(SRCDIR)/runtime/_$(_CONF) -lax25c_runtime \
			-lpthread
TARGET   =  ax25c
OBJS     =  ax25c.o

all: runtime config terminal mm_simple $(TARGET)
	$(MAKE) -C $(SRCDIR)/config all
	$(MAKE) -C $(SRCDIR)/terminal all
	$(MAKE) -C $(SRCDIR)/mm_simple all
	echo "Build OK"
	
$(TARGET): runtime $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

.PHONY: runtime
runtime:
	$(MAKE) -C $(SRCDIR)/runtime all

.PHONY: config
config:
	$(MAKE) -C $(SRCDIR)/config all

.PHONY: terminal
terminal:
	$(MAKE) -C $(SRCDIR)/terminal all

.PHONY: mm_simple
mm_simple:
	$(MAKE) -C $(SRCDIR)/mm_simple all

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
	
install: all
	
run: all
	@echo "Executing $(TARGET)"
	cp /usr/local/lib/libstringc.$(SOEXT) .
	cp /usr/local/lib/libmapc.$(SOEXT) .
	LD_LIBRARY_PATH=./	./$(TARGET) "--loglevel:DEBUG" "--pid:$(TARGET).pid" \
		"../$(TARGET).xml"
	@echo "OK"
	
stop:
	@echo "Stopping $(TARGET)"
	@bash -c "kill -s SIGINT `cat $(SRCDIR)/_$(_CONF)/$(TARGET).pid`"

endif
