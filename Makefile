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
			-I$(LOCAL)/include/

LIBS     =  -L$(SRCDIR)/runtime/_$(_CONF)/ -lax25c_runtime \
			-L$(LOCAL)/$(SODIR) -l uki \
			-lpthread
TARGET   =  ax25c
OBJS     =  ax25c.o

.PHONY: all
all: runtime serial config terminal mm_simple ax25v2_2 axudp \
			hostmodeserver axtnos $(TARGET)
	@echo "** Build ax25c OK ***"

$(TARGET): runtime $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

.PHONY: runtime
runtime:
	$(MAKE) -C $(SRCDIR)/runtime all
	
.PHONY: serial
serial:
	$(MAKE) -C $(SRCDIR)/serial all

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

.PHONY: axudp
axudp:
	$(MAKE) -C $(SRCDIR)/axudp all

.PHONY: hostmodeserver
hostmodeserver:
	$(MAKE) -C $(SRCDIR)/hostmodeserver all

.PHONY: axtnos
axtnos:
	$(MAKE) -C $(SRCDIR)/axtnos all

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
	rm -rf $(SRCDIR)/$(OBJDIR)/* $(SRCDIR)/$(DOCDIR)/*
	@$(MAKE) -C $(SRCDIR)/runtime clean
	@$(MAKE) -C $(SRCDIR)/serial clean
	@$(MAKE) -C $(SRCDIR)/config clean
	@$(MAKE) -C $(SRCDIR)/terminal clean
	@$(MAKE) -C $(SRCDIR)/mm_simple clean
	@$(MAKE) -C $(SRCDIR)/ax25v2_2 clean
	@$(MAKE) -C $(SRCDIR)/axudp clean
	@$(MAKE) -C $(SRCDIR)/hostmodeserver clean
	@$(MAKE) -C $(SRCDIR)/axtnos clean

install: all

run: all
	@echo "Executing $(TARGET)"
	@cp $(LOCAL)/$(SODIR)/libstringc.$(SOEXT) .
	@cp $(LOCAL)/$(SODIR)/libmapc.$(SOEXT) .
	@cp $(LOCAL)/$(SODIR)/libuki.$(SOEXT) .
	@LD_LIBRARY_PATH=./	./$(TARGET) \
		"--loglevel:DEBUG" \
		"--pid:$(SRCDIR)/_$(_CONF)/$(TARGET).pid" \
		"$(SRCDIR)/$(TARGET).xml"
	@echo "OK"
	
test: all
	@echo "Executing $(TARGET)"
	@cp $(LOCAL)/$(SODIR)/libstringc.$(SOEXT) .
	@cp $(LOCAL)/$(SODIR)/libmapc.$(SOEXT) .
	@cp $(LOCAL)/$(SODIR)/libuki.$(SOEXT) .
	@LD_LIBRARY_PATH=./	./$(TARGET) \
		"--loglevel:NONE" \
		"--pid:$(SRCDIR)/_$(_CONF)/$(TARGET).pid" \
		"--esc:\\" \
		"$(SRCDIR)/$(TARGET).xml"
	@echo "OK"
	
testclient:
	@echo "Executing Client in _Client"
	@cp $(LOCAL)/$(SODIR)/libstringc.$(SOEXT) .
	@cp $(LOCAL)/$(SODIR)/libmapc.$(SOEXT) .
	@cp $(LOCAL)/$(SODIR)/libuki.$(SOEXT) .
	@cp -rp ../_Debug ../_Client
	@LD_LIBRARY_PATH=../_Client ../_Client/$(TARGET) \
		"--loglevel:DEBUG" "../testclient.xml"
	@echo "OK"
	
testserver:
	@echo "Executing Server in _Server"
	@cp $(LOCAL)/$(SODIR)/libstringc.$(SOEXT) .
	@cp $(LOCAL)/$(SODIR)/libmapc.$(SOEXT) .
	@cp $(LOCAL)/$(SODIR)/libuki.$(SOEXT) .
	@cp -rp ../_Debug ../_Server
	@LD_LIBRARY_PATH=../_Server ../_Server/$(TARGET) \
		"--loglevel:DEBUG" "../testserver.xml"
	@echo "OK"
	
stop:
	@echo "Stopping $(TARGET)"
	@bash -c "kill -s SIGINT `cat $(SRCDIR)/_$(_CONF)/$(TARGET).pid`"

endif
