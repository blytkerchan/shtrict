# Vlinder shtrict: very restricted shell
# Copyright (C) 2011   Ronald Landheer-Cieslak
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 3, as 
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.*/
INSTALL_PROGRAM=install
bindir=usr/bin
OBJECTS=main.o

all : shtrict

shtrict : $(OBJECTS)
	gcc -g -Wall -Werror -o $@ $^ -lreadline

%.o : %.c
	gcc -g -Wall -Werror -std=c89 -c -o $@ $<

install :
	mkdir -p $(DESTDIR)/$(bindir) 
	$(INSTALL_PROGRAM) shtrict $(DESTDIR)/$(bindir)/shtrict

debian/manpage.1 : shtrict
	help2man -h -h -n "Very restricted shell" -s 1 -o debian/manpage.1 --no-discard-stderr -v -v -N ./shtrict
