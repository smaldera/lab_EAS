#****************************************************************************
# Copyright (C) 2001-2002  ARW Elektronik Germany
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# Maintainer(s): Klaus Hitschler (klaus.hitschler@gmx.de)
#****************************************************************************

#****************************************************************************
#
# Makefile - makefile for ARW Elektronik CAMAC interfaces shared library
#
# $Log: Makefile,v $
# Revision 1.2  2002/10/19 08:50:05  klaus
# commit after repair
#
#
#****************************************************************************

TARGET = libpcivme.so.1.0.1
FILES  = pcivme_ni
LIB    = /usr/lib

INCLUDEDIR = /usr/include
CFLAGS = -shared -Wall -Wl,-soname,libpcivme.so.1 -g -lc -I$(INCLUDEDIR) -I.

all: $(FILES)
	mv $(FILES) $(TARGET)

clean:
	rm -f $(FILES) *~ core

# only root may install
install:
	cp $(TARGET) $(LIB)/$(TARGET)
	ln -sf $(LIB)/$(TARGET) $(LIB)/libpcivme.so.1
	ln -sf $(LIB)/libpcivme.so.1 $(LIB)/libpcivme.so
	/sbin/ldconfig
    
	


