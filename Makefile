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
CFLAGS   =  -Wall -Werror -g -ggdb -fmessage-length=0 -pthread \
			-I/usr/local/include/

LIBS     =  -L$(SRCDIR)/runtime/_$(_CONF)/ -lax25c_runtime \
			-L/usr/local/lib/ -lringbuffer \
			-lpthread
TARGET   =  ax25c
OBJS     =  ax25c.o

.PHONY: all
all: runtime config terminal mm_simple ax25v2_2 $(TARGET)
	@echo "** Build ax25c OK ***"

.PHONY: sub
sub:
	@( cd $(SRCDIR)/user_kernel_interface/ && make all )
	@( cd $(SRCDIR)/stringc/               && make all )
	@( cd $(SRCDIR)/mapc/                  && make all )
	@( cd $(SRCDIR)/ringbuffer/            && make all )

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

.PHONY: ax25v2_2
ax25v2_2:
	$(MAKE) -C $(SRCDIR)/ax25v2_2 all

%.o: %.c %.h Makefile
	$(CC) $(CFLAGS) -c $<	

.PHONY: doc	
doc:
	doxygen $(SRCDIR)/doxygen.conf
	( cd $(SRCDIR)/$(DOCDIR)/latex && make )
	cp $(SRCDIR)/$(DOCDIR)/latex/refman.pdf \
		$(SRCDIR)/$(DOCDIR)/$(TARGET).pdf

.PHONY: clean
clean:
	rm -rf $(SRCDIR)/$(OBJDIR) $(SRCDIR)/$(DOCDIR)
	@$(MAKE) -C $(SRCDIR)/runtime clean
	@$(MAKE) -C $(SRCDIR)/config clean
	@$(MAKE) -C $(SRCDIR)/terminal clean
	@$(MAKE) -C $(SRCDIR)/mm_simple clean
	@$(MAKE) -C $(SRCDIR)/ax25v2_2 clean

.PHONY: cleansub
cleansub:
	( cd $(SRCDIR)/user_kernel_interface/ && make clean )
	( cd $(SRCDIR)/stringc/               && make clean )
	( cd $(SRCDIR)/mapc/                  && make clean )
	( cd $(SRCDIR)/ringbuffer/            && make clean )

install: all

.PHONY: installsub
installsub:
	@( cd $(SRCDIR)/user_kernel_interface/ && make install )
	@( cd $(SRCDIR)/stringc/               && make install )
	@( cd $(SRCDIR)/mapc/                  && make install )
	@( cd $(SRCDIR)/ringbuffer/            && make install )

run: all
	@echo "Executing $(TARGET)"
	@cp /usr/local/lib/libstringc.$(SOEXT) .
	@cp /usr/local/lib/libmapc.$(SOEXT) .
	@cp /usr/local/lib/libringbuffer.$(SOEXT) .
	@LD_LIBRARY_PATH=./	./$(TARGET) "--loglevel:DEBUG" "--pid:$(TARGET).pid" \
		"../$(TARGET).xml"
	@echo "OK"
	
test: all
	@echo "Executing $(TARGET)"
	@cp /usr/local/lib/libstringc.$(SOEXT) .
	@cp /usr/local/lib/libmapc.$(SOEXT) .
	@cp /usr/local/lib/libringbuffer.$(SOEXT) .
	@LD_LIBRARY_PATH=./	./$(TARGET) "--loglevel:NONE" "--pid:$(TARGET).pid" \
		"../$(TARGET).xml"
	@echo "OK"
	
stop:
	@echo "Stopping $(TARGET)"
	@bash -c "kill -s SIGINT `cat $(SRCDIR)/_$(_CONF)/$(TARGET).pid`"

endif
