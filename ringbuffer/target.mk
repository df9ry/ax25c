
#   Project ringbuffer
#   Copyright (C) 2019  tania@df9ry.de
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
	
.SUFFIXES:

ifndef _CONF
_CONF := Debug
export _CONF
endif

OS := $(shell uname -o)
export OS
ifneq ($(OS),GNU/Linux)
	SOEXT := dll
else
	SOEXT := so
endif
export SOEXT

OBJDIR := _$(_CONF)
export OBJDIR
DOCDIR := _Documentation
export DOCDIR

MAKETARGET = $(MAKE) --no-print-directory -C $@ -f $(CURDIR)/Makefile \
	SRCDIR=$(CURDIR) $(MAKECMDGOALS)
	
DOCTARGET  = $(MAKE) --no-print-directory -C $@ -f $(CURDIR)/Makefile \
	SRCDIR=$(CURDIR) doc

.PHONY: $(OBJDIR)
$(OBJDIR):
	+@[ -d $@ ] || mkdir -p $@
	+@$(MAKETARGET)

.PHONY: $(DOCDIR)
$(DOCDIR):
	+@[ -d $@ ] || mkdir -p $@
	+@$(DOCTARGET)

Makefile : ;
%.mk :: ;

% :: $(OBJDIR) ;

PHONY: clean
clean:
	rm -rf $(OBJDIR) $(DOCDIR)
