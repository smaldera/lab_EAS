#ifndef __PCIIF_H__
#define __PCIIF_H__
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
// pciif.h - all definitions about the VMEMM module
//
// $Log: pciif.h,v $
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
#include <linux/types.h>
#include <vic.h>

//****************************************************************************
// DEFINES

/* type of this VMEMM ---------------------------------------------------*/
#define VMEMM_MODULE_TYPE 0x1000

/*-----------------------------------------------------------------------*/
/* all addresses relative to PCI-Window                                  */

/*------------- addresses of vmemm local devices ------------------------*/
#define CSR     (u32)0x0000   /* control status register                 */
#define VICRES  (u32)0x0004   /* VIC reset register / interrupt status   */
#define ADRHL   (u32)0x0008   /* AdrH and AdrL as long                   */
#define VICBASE (u32)0x0400   /* base of VIC68A                          */
#define VECBASE (u32)0x0800   /* base of vector registers                */
#define VMEBASE (u32)0x1000   /* base of 4k VME-BUS window               */

/*---------- parts of addresses derived from above ----------------------*/
#define IVEC1   (u32)(VECBASE + 3)      /* IACK 1 vector                 */
#define IVEC2   (u32)(VECBASE + 5)      /* IACK 2 vector                 */
#define IVEC3   (u32)(VECBASE + 7)      /* IACK 3 vector                 */
#define IVEC4   (u32)(VECBASE + 9)      /* IACK 4 vector                 */
#define IVEC5   (u32)(VECBASE + b)      /* IACK 5 vector                 */
#define IVEC6   (u32)(VECBASE + d)      /* IACK 6 vector                 */
#define IVEC7   (u32)(VECBASE + f)      /* IACK 7 vector                 */

#define ADRL    (u32)ADRHL               /* u16 access addr. VME-addr    */
#define ADRH    (u32)(ADRHL + 2)

/*--------- address mask ------------------------------------------------*/
#define VME_ADR_MASK (u32)0x00000FFF    /* masks lower part of address   */

/*--------- some masks in CSR -------------------------------------------*/
#define FLAG_RMC     (u16)0x0001        /* set = next cycle is RMC       */
#define FLAG_BLT     (u16)0x0002        /* don't use it. must be 0       */
#define FLAG_WORD    (u16)0x0004         /* it is a u16 wide interface   */
#define FLAG_SYSCTL  (u16)0x0008        /* the system contrl. is enabled */
#define MASK_MODNR   (u16)0x00F0        /* the mask to get the module No */
#define MASK_FPGA    (u16)0x0F00        /* the mask to get the FPGA rev. */
#define MASK_MODTYPE (u16)0xF000        /* the mask to get type of module*/

/*---------- action commands in VICRES -----------------------------------*/
#define GLOBAL_RESET (u16)0x000A        /* write this to reset the intrfc */
#define LOCAL_RESET  (u16)0x0005        /* generate a local reset         */

#endif // __PCIIF_H__


