#****************************************************************************
# Copyright (C) 2000-2002  ARW Elektronik Germany
#
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
# Makefile - makefile for ARW Elektronik PCI to VME interfaces driver
#
# $Log: Makefile,v $
# Revision 1.1.1.2  2002/10/18 22:14:29  klaus
# *** empty log message ***
#
# Revision 1.1.1.1  2002/10/09 19:36:30  klaus
# initial import
#
#
#****************************************************************************

TARGET   = pcivme.o

FILES    = main.o askpci.o plxbug.o fops.o
VERSION := $(shell uname -r)
KERNSRC  = /lib/modules/$(VERSION)/build
INCLUDE  = $(KERNSRC)/include
DBG      = __NO_DEBUG__

CFLAGS = -O2 -D__KERNEL__ -DMODULE -Wall -I$(INCLUDE) -I. -D$(DBG)

all: $(TARGET)

$(TARGET) : $(FILES)
	$(LD) -r $^ -o $@

clean:
	rm -f *~ $(TARGET) *.o
	
fresh:
	touch *.c
	make all
	
	
