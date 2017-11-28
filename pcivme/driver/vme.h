#ifndef __VME_H__
#define __VME_H__

//****************************************************************************
// Copyright (C) 2001,2002  ARW Elktronik Germany
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// This product is not authorized for use as critical component in
// life support systems without the express written approval of
// ARW Elektronik Germany.
//
// Please announce changes and hints to ARW Elektronik
//
// Maintainer(s): Klaus Hitschler (klaus.hitschler@gmx.de)
//****************************************************************************

//****************************************************************************
//
// vme.h - some important definitions about VME bus address modifiers
//
// $Log: vme.h,v $
// Revision 1.5  2002/10/18 21:56:28  klaus
// completed functional features, untested
//
// Revision 1.4  2002/10/18 21:56:28  klaus
// completed functional features, untested
//
// Revision 1.3  2002/10/10 18:57:46  klaus
// source beautyfied
//
//****************************************************************************

//****************************************************************************
// INCLUDES
#include <asm/types.h>

//****************************************************************************
// DEFINES
#ifndef __KERNEL__
    #define u8  __u8
    #define u16 __u16
    #define u32 __u32
#endif

typedef u16 ADDRESS_MODIFIER;

#define Std_Sup_Data        (ADDRESS_MODIFIER)0x3d
#define Std_Sup_Prog        (ADDRESS_MODIFIER)0x3e
#define Std_NoPriv_Data     (ADDRESS_MODIFIER)0x39
#define Std_NoPriv_Prog     (ADDRESS_MODIFIER)0x3a

#define Short_Sup           (ADDRESS_MODIFIER)0x2d
#define Short_NoPriv        (ADDRESS_MODIFIER)0x29

#define Ext_Sup_Data        (ADDRESS_MODIFIER)0x0d
#define Ext_Sup_Prog        (ADDRESS_MODIFIER)0x0e
#define Ext_NoPriv_Data     (ADDRESS_MODIFIER)0x09
#define Ext_NoPriv_Prog     (ADDRESS_MODIFIER)0x0a

#endif // __VME_H__


