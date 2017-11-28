#ifndef __PLX9050_H__
#define __PLX9050_H__

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
// plx9050.h - Include header for the PCIbus target 
//             interface chip PLX9050 from PLX Technology (www.plxtech.com)
//
// $Log: plx9050.h,v $
// Revision 1.5  2002/10/18 21:56:28  klaus
// completed functional features, untested
//
// Revision 1.4  2002/10/18 21:56:28  klaus
// completed functional features, untested
//
// Revision 1.3  2002/10/10 18:57:46  klaus
// source beautyfied
//
// derived from original code from Dirk Muehlenberg and ? Mathes   AR 18.02.2000
//
//****************************************************************************

#include <asm/io.h>

/*
 * defining the offsets from PCI CFG Register Area
 * (PCI registers, only accessible during a configuration 0 cycle)
 */
/* 15:0 VendorId | 31:16 DeviceId */
#define PLX9050_PCIIDR   0x0

#define PLX9050_PCICR    0x4
#define PLX9050_PCISR    0x6
#define PLX9050_PCIREV   0x8
#define PLX9050_PCICCR   0xB
#define PLX9050_PCICLSR  0xC
#define PLX9050_PCILTR   0xD
#define PLX9050_PCIHTR   0xE
#define PLX9050_PCIBISTR 0xF
/*
** PCI Base Address Register
*/
#define PLX9050_PCIBAR0  0x10
#define PLX9050_PCIBAR1  0x14
#define PLX9050_PCIBAR2  0x18
#define PLX9050_PCIBAR3  0x1C
#define PLX9050_PCIBAR4  0x20
#define PLX9050_PCIBAR5  0x24

#define PLX9050_PCICIS   0x28

/* 15:0 Subsystem VendorId */ 
#define PLX9050_PCISVID  0x2C

#define PLX9050_PCIERBAR 0x30

/* interrupt line routing */
#define PLX9050_PCIILR   0x3C
/* interrupt pin register */
#define PLX9050_PCIIPR   0x3D

#define PLX9050_PCIMGR   0x3E
#define PLX9050_PCIMLR   0x3F


/*
 * defining the offsets from Local Base Address
 * (local configuration registers, accessible by way of i/o- or
 * memory cycle)
 */
#define PLX9050_LAS0RR   0x0
#define PLX9050_LAS1RR   0x4
#define PLX9050_LAS2RR   0x8
#define PLX9050_LAS3RR   0xC
#define PLX9050_EROMRR   0x10

#define PLX9050_LAS0BA   0x14
#define PLX9050_LAS1BA   0x18
#define PLX9050_LAS2BA   0x1C
#define PLX9050_LAS3BA   0x20
#define PLX9050_EROMBA   0x24

/*
 * Local Address Space I Bus Region Descriptor Register
 * Bit
 * 0 : Burst enable
 * 1 : Ready Input Enable
 * 2 : Bterm Input Enable
 * 4:3 : Prefetch Count - 00 no, 01 4 lwords, 10 8 lwords, 11 16 lwords
 * 5 : Prefetch Count Enable
 */
#define PLX9050_LAS0BRD  0x28
#define PLX9050_LAS1BRD  0x2C
#define PLX9050_LAS2BRD  0x30
#define PLX9050_LAS3BRD  0x34
#define PLX9050_EROMBRD  0x38

#define PLX9050_CS0BASE  0x3C
#define PLX9050_CS1BASE  0x40
#define PLX9050_CS2BASE  0x44
#define PLX9050_CS4BASE  0x48

#define PLX9050_INTCSR   0x4C

#define PLX9050_CNTRL    0x50

#ifndef L_SETBIT
    #define L_SETBIT(addr, b) writel(readl(addr) | (1<<(b)), addr);
    #define W_SETBIT(addr, b) writew(readw(addr) | (1<<(b)), addr);
    #define B_SETBIT(addr, b) writeb(readb(addr) | (1<<(b)), addr);

    #define L_CLRBIT(addr, b) writel(readl(addr) & ~(1<<(b)), addr);
    #define W_CLRBIT(addr, b) writew(readw(addr) & ~(1<<(b)), addr);
    #define B_CLRBIT(addr, b) writeb(readb(addr) & ~(1<<(b)), addr);
#endif

#define PLX9050_ENABLE_BURST(base, i)  L_SETBIT(base+PLX9050_LAS0BRD+i*4, 0)
#define PLX9050_DISABLE_BURST(base, i) L_CLRBIT(base+PLX9050_LAS0BRD+i*4, 0)

#define PLX9050_SET_PREFETCH0(base, i)  writel(readl(base+PLX9050_LAS0BRD+i*4) & ~0x180       , base+PLX9050_LAS0BRD+i*4);
#define PLX9050_SET_PREFETCH4(base, i)  writel(readl(base+PLX9050_LAS0BRD+i*4) & ~0x180 | 0x80, base+PLX9050_LAS0BRD+i*4);
#define PLX9050_SET_PREFETCH8(base, i)  writel(readl(base+PLX9050_LAS0BRD+i*4) & ~0x180 | 0x100, base+PLX9050_LAS0BRD+i*4);
#define PLX9050_SET_PREFETCH16(base, i) writel(readl(base+PLX9050_LAS0BRD+i*4)          | 0x180, base+PLX9050_LAS0BRD+i*4);

#endif /* __PLX9050_H__ */
