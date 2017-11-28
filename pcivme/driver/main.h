#ifndef __MAIN_H__
#define __MAIN_H__

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
// main.h -- export parts of main.c
//
// $Log: main.h,v $
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

/*--- INCLUDES ----------------------------------------------------------------------------*/
#include <linux/wait.h>
#include <linux/types.h>
#include <linux/list.h>

/*--- DEFINES -----------------------------------------------------------------------------*/
#define DEVICE_NAME "pcivme"

#define LOW_MEMORY        0x1000000    // 1 Mbyte PC memory border

#define LCR_SPACE         0x0080 // space in bytes of LCR
#define IFR_SPACE         0x2000 // space in bytes of IFR
#define CTL_SPACE         0x1000 // lower part of IFR_SPACE
#define VME_SPACE         (IFR_SPACE - CTL_SPACE) // higher part of IFR_SPACE used for VME window

#define RELEASE_VMEMM       (u16)0x4180 // write this to release access .. 
#define INHIBIT_VMEMM       (u16)0x4080 // write this to inhibit access .. 
#define ENABLE_PCIADA_IRQS  (u16)0x0049 // enable PCIADA IRQs             
#define DISABLE_PCIADA_IRQS (u16)0x0009 // disable PCIADA IRQs            

/*--- TYPEDEFS ----------------------------------------------------------------------------*/
typedef struct
{
    struct list_head devList;             // link anchor for list of devices
    struct list_head pciList;             // link anchor of all unchecked PCIADAs found
    u32    nMajor;                        // asigned major number
    int    count;                         // count of found devices
} DRIVER_OBJ;

typedef struct
{
    struct list_head list;                // chained list of found and not checked devices
    struct pci_dev   *pciDev;             // associated pci descriptors of the system
    u16              index;
} PCIConfig; 

typedef struct
{
    struct list_head list;    /* chain element of list */
    u16 wIndex;               /* running index of all PCIADAs */
    PCIConfig *pPch;          /* associated PCI configuration */
    u32 pLCR;                 /* base of LCR */
    u32 pCtl;                 /* base of control area */
    u32 pVME;                 /* base of VME access area */
    u32 pPhysVME;             /* physical address of VME window */
    u8  bConnected;           /* is it connected ?? */
    u16 wInitStep;            /* trace of initialisation */
    u16 wIrq;                 /* the assigned irq */
    u32 dwInterruptCount;     /* counts the VME and timeout interrupts */
    u16 wIrqStatus;           /* last cause / status of interrupts */
    int nOpenCounter;         /* counts the open path to this device */
    wait_queue_head_t event_queue; /* handle interrupt events */

    u32 pAdrMod;              /* address of address modifier register in VIC */
    u32 pAdrReg;              /* address of VMEMM VME address register */
    u32 pCSR;                 /* address of the VMEMM CSR register */

    u32 pPCIADACntrl;         /* address of the PCIADA control register */
    u32 pPCIADAIntCSR;        /* address of the PCIADA INTCSR register */

    u8  cModuleNumber;        /* module number */
    u8  cFPGAVersion;         /* FPGA Version number */
    u8  cSystemController;    /* set if VMEMM is system controller */
    u8  cWordMode;            /* set if VMEMM is jumpered to word mode */

    u8  bCurrentModifier;     /* the current set address modifier of this device */ 
    u32 dwCurrentPageAddress; /* the current page address association */
} DEVICE_OBJ;

typedef struct
{
    DEVICE_OBJ *pDo;          /* pointer to my PCIADA & connected VMEMM */
    u8  bModifier;            /* the associated address modifier of this device */  
    u8  bAccessType;          /* the next access is byte, word, longword - not for memory mapped access */
    u8  bIncrement;           /* the next increment, normally like accesstype or 0*/
    void (*read)(DEVICE_OBJ*, void**, u32);  /* predifined read function */
    void (*write)(DEVICE_OBJ*, u32, void**); /* same for write */
    int  (*AlignmentCheck)(loff_t offset);   /* function to check access alignment */
} PATH_OBJ;

/*--- PROTOTYPES --------------------------------------------------------------------------*/
int  get_module_info(DEVICE_OBJ *pd);
int  test_connection(DEVICE_OBJ *pd);

/*--- PROTOTYPES --------------------------------------------------------------------------*/
extern DRIVER_OBJ drv;       /* driver globals */

#endif // __MAIN_H__

