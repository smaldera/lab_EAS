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
# Makefile - makefile for ARW Elektronik PCIVME interfaces pvmon program
#
# $Log: Makefile,v $
# Revision 1.3  2002/11/14 19:57:56  klaus
# improvement, still bugs active
#
# Revision 1.2  2002/10/20 11:49:33  klaus
# first parts working
#
# Revision 1.1.1.1  2002/10/18 22:14:29  klaus
#
#
#
#****************************************************************************
 
SRC     = .
INC     = -I. -I/usr/include
LDLIBS  = -L/lib -L/usr/lib -L/usr/local/lib

DBG     = -g
CFLAGS  = $(DBG) $(INC) $(LDLIBS)

TARGET1 = pvmon
FILES1  = $(SRC)/$(TARGET1).c $(SRC)/mbuffer.c $(SRC)/pcilibLx.c

all:    $(TARGET1)

$(TARGET1): $(FILES1)
	$(CC) $(FILES1) $(CFLAGS) -o $(TARGET1) -lpcivme -lslang

clean:
	rm -f $(SRC)/*~ $(SRC)/*.o $(TARGET1)
	
install:
	cp $(TARGET1) /usr/local/bin

