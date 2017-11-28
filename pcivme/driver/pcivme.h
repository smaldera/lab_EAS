#ifndef __PCIVME_H__
#define __PCIVME_H__

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
// pcivme.h -- the common header for driver and applications for the PCIVME 
//             PCI to VME Interface
//             All commands and constants are similiar to WIN?? drivers, but
//             not equal. Please check if you cross-port or develop.
//
// $Log: pcivme.h,v $
// Revision 1.7  2002/10/20 18:06:51  klaus
// changed error handling
//
// Revision 1.6  2002/10/18 21:56:28  klaus
// completed functional features, untested
//
// Revision 1.5  2002/10/18 21:56:28  klaus
// completed functional features, untested
//
// Revision 1.4  2002/10/17 19:05:03  klaus
// VME access is working through test to lib to driver
//
//****************************************************************************

#include <linux/types.h>
#include <asm/ioctl.h>

#define PCIVME_MAGIC ' '

#define MAX_INIT_ELEMENTS 20  // maximum number of init in PCIVME_(DE)INIT_HARDWARE

#ifndef __KERNEL__
    #define u8  __u8
    #define u16 __u16
    #define u32 __u32
#endif

//----------------------------------------------------------------------------
// shared structures between driver & app to support ioctls ------------------
typedef struct             // one command element to initialize interface or deinitialize
{
    u8     bDestination;     // 0 = lcr, 1 = vme-interface, -1 = stop
    u8     bAccessType;      // 1 = byte access, 2 = word access, 4 = long access
    u16    wOffset;          // offset into interface address range for initialisation
    u32    dwValue;          // value to initialize
} PCIVME_INIT_ELEMENT;

typedef struct
{
    PCIVME_INIT_ELEMENT sVie[MAX_INIT_ELEMENTS];  // at least one zero element must be the last
} PCIVME_INIT_COMMAND;

typedef struct
{
    u8     bModifier;         // set the current modifier
    u8     bAccessType;           // set the current access type (1,2,4), not used
    u8     bIncrement;            // set the current byte increment count, not used 
    u8     bDummy;            // reserved
} PCIVME_ACCESS_COMMAND;

typedef struct
{
    u32    dwAddress;         // tas to address
    u8     bModifier;         // VME address modifier for this window  
    u8     bContent;          // content to store and get back
} PCIVME_TAS_STRUCT;

typedef struct
{
    u16  wRegisterAddress;    // address offset of vic68a register
    u8   bAccessMode;         // read, write, or, and
    u8   bContent;            // content to write, and, or
    u8   bDummy;              // reserved 
} PCIVME_VIC68A_ACTION;

typedef struct
{
    u8  bEnable;              // set to 0 to disable, != 0 to enable 
    u8  bDummy;               // reserved
} PCIVME_IRQ_CONTROL;

typedef struct
{
    u8  bCommand;             // makes and reads different reset commands
    u8  bResult;              // return result, == 0 if command has finished
} PCIVME_RESET_COMMAND;

typedef struct              // static information about status of PCIADA & VMEMM
{
    u8  bConnected;           // is it or it was software connected 
    u8  cModuleNumber;        // module number
    u8  cFPGAVersion;         // FPGA Version number
    u8  cSystemController;    // set if VMEMM is system controller
    u8  cWordMode;            // set if VMEMM is jumpered to word mode
    u8  bDummy;               // reserved
} PCIVME_STATIC_STATUS;

typedef struct              // dynamic information about status of PCIADA & VMEMM
{
    u8  bConnected;           // the current cable connection state
    u8  bPCIADAIrq;           // interrupt pending due to timeout or connection fail
    u8  bVMEMMIrq;            // interrupt pending due to VMEMM event
    u8  bDummy;               // reserved
} PCIVME_DYNAMIC_STATUS;

typedef struct
{
    u32 dwStatusID;           // interrupt-vector (byte, word, long)
    u8  bLevel;               // interrupt-level
    u8  bPCIADAIrq;           // pending PCIADA Irq detected and cleared
} PCIVME_VECTOR_LEVEL;

//----------------------------------------------------------------------------
// commands to support ioctls ------------------------------------------------
#define PCIVME_INIT_HARDWARE       _IOW(PCIVME_MAGIC,   1, PCIVME_INIT_COMMAND)   // initializes the hardware with given parameters
#define PCIVME_DEINIT_HARDWARE     _IOW(PCIVME_MAGIC,   2, PCIVME_INIT_COMMAND)   // uninitializes the hardware

#define PCIVME_SET_ACCESS_PARA     _IOWR(PCIVME_MAGIC,  3, PCIVME_ACCESS_COMMAND) // set the address modifier for this path

#define PCIVME_GET_STATIC_STATUS   _IOR(PCIVME_MAGIC,   4, PCIVME_STATIC_STATUS)  // asks for static status of PCIADA & connected VMEMM
#define PCIVME_GET_DYNAMIC_STATUS  _IOR(PCIVME_MAGIC,   5, PCIVME_DYNAMIC_STATUS) // asks for dynamic status of PCIADA & connected VMEMM

#define PCIVME_READ_VECTOR_POLL    _IOR(PCIVME_MAGIC,   6, PCIVME_VECTOR_LEVEL)   // reads the level and vector of IRQ
#define PCIVME_READ_VECTOR_BLOCK   _IOR(PCIVME_MAGIC,   7, PCIVME_VECTOR_LEVEL)   // reads blocking the level and vector of IRQ

#define PCIVME_CONTROL_INTERRUPTS  _IOWR(PCIVME_MAGIC,  8, PCIVME_IRQ_CONTROL)    // set, clear interrupt enable
#define PCIVME_TAS                 _IOWR(PCIVME_MAGIC,  9, PCIVME_TAS_STRUCT)     // make test and set
#define PCIVME_RESET               _IOWR(PCIVME_MAGIC, 10, PCIVME_RESET_COMMAND)  // make a reset to VME or global

#define PCIVME_ACCESS_VIC68A       _IOWR(PCIVME_MAGIC, 11, PCIVME_VIC68A_ACTION)  // access vic68a register (interface depended)

//----------------------------------------------------------------------------
// input constants of ioctls -------------------------------------------------

// switches for PCIVME_INIT and DEINIT_COMMAND 
#define LCR   (u8)0           // destination is PCIADA (LCR) register
#define IFR   (u8)1           // destination is VMEMM-Interface register
#define VIC   (u8)2           // destination is VIC68A register
#define STOP  (u8)255         // this command stops the init machine

#define BYTE_ACCESS (u8)1     // write byte wise
#define WORD_ACCESS (u8)2     //       word
#define LONG_ACCESS (u8)4     //       long

// switches for PCIVME_ACCESS_VIC68A
#define VIC68A_READ       0   // read only access
#define VIC68A_WRITE      1   // write and read back access
#define VIC68A_OR         2   // read, bitwise 'or' content and read back access
#define VIC68A_AND        3   // read, bitwise 'and' content and read back access
#define VIC68A_WRITE_ONLY 4   // do not read back after write

// switches for PCIVME_VECTOR_CMD
#define READ_CURRENT_LEVEL 0  // try to get the current irq level
#define READ_VECTOR        1  // (if level == 0) read vector @ current LEVEL else @ level

// switches for the PCIVME_RESET
#define VME_RESET_CMD      0  // raise a VME reset only
#define LOCAL_RESET_CMD    1  // raise a local reset only
#define GLOBAL_RESET_CMD   2  // raise a global reset
#define POLL_RESET_CMD     3  // ask if reset is finished

// address masks for the pager - to use for offset and size @ window alignment
#define HI_ADDRESS_MASK   (u32)0xFFFFF000       // masks the high part of a vme address
#define LO_ADDRESS_MASK   (~HI_ADDRESS_MASK)    // masks the low  part of a vme address
#define ONE_PAGE_SIZE     (LO_ADDRESS_MASK + 1) // size of 1 page (hardware related)

// macros to calculate the real base and the real size of demand pages
#define PCIVME_PAGE_BASE(base)       (base & HI_ADDRESS_MASK)  // makes an aligned base for a page
#define PCIVME_PAGE_SIZE(base, size) (((base + size + LO_ADDRESS_MASK) / ONE_PAGE_SIZE) * ONE_PAGE_SIZE)

//----------------------------------------------------------------------------
// results of ioctls ---------------------------------------------------------
#define NOT_MY_INTERRUPT   0
#define PCIADA_INTERRUPT   1
#define VMEMM_INTERRUPT    2

#endif /* __PCIVME_H__ */
