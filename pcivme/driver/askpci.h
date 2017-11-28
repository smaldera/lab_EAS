#ifndef __ASKPCI_H__
#define __ASKPCI_H__

//****************************************************************************
// Copyright (C) 2000-2002  ARW Elektronik Germany
//
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
//
//****************************************************************************

//****************************************************************************
//
// askpci.h - definitions for basic access functions of pci information
//
// $Log: askpci.h,v $
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

/*--- INCLUDES -------------------------------------------------------------------------*/
#include <linux/version.h>
#include <linux/pci.h>
#include <asm/types.h>
#include <linux/list.h>

#include <main.h> 

/*--- TYPEDEFS -------------------------------------------------------------------------*/

/*--- PROTOTYPES -------------------------------------------------------------------------*/
int  GetPCIConfig(DRIVER_OBJ *drv, u16 device_id, u16 vendor_id, u16 subsys_id, u16 subven_id);
void DeletePCIConfig(DRIVER_OBJ *drv);

#endif  /* __ASKPCI_H__ */


