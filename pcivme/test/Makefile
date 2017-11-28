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
# Makefile - makefile for ARW Elektronik PCIVME interfaces test program
#
# $Log: Makefile,v $
# Revision 1.4  2002/10/20 18:08:21  klaus
# don't know what has been changed?
#
# Revision 1.3  2002/10/17 19:05:03  klaus
# VME access is working through test to lib to driver
#
# Revision 1.2  2002/10/12 22:12:19  klaus
# simple change
#
# Revision 1.1.1.1  2002/10/09 19:36:29  klaus
# initial import
#
#
#****************************************************************************
 
FILES = simpleTest

INCLUDEDIR = /usr/include
CFLAGS = -O2 -fomit-frame-pointer -Wall -I$(INCLUDEDIR) -g -I. -lpcivme

all: $(FILES)

clean:
	rm -f $(FILES) *~ core 
	

