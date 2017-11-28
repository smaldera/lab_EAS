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
// plxbug.c -- plx 9050 bug fix code the PCIVME PCI to VME Interface
//
// $Log: plxbug.c,v $
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

#include <common.h>  /* must be the first include */

#include <asm/types.h>
#include <plxbug.h>
#include <main.h>

/*-------------------------------------------------------------------------
// DEFINES
*/
#define PCR_MEMORY_BUG  0x00       // 1st PCR index of potential bug
#define PCR_IO_BUG      0x01       // 2nd PCR index of potential bug
#define PCR_MEMORY_OK   0x04       // 1st PCR index of no bug 
#define PCR_IO_OK       0x05       // 2nd PCR index of no bug

//-------------------------------------------------------------------------
// EXTERNALS

//-------------------------------------------------------------------------
// function to call for bug fix


// fixes address of LCR through change in address windows - updates PCIConfigHeader
int PLX9050BugFix(PCIConfig *pHeader)
{
    u32 dwDataBug;
    u32 dwDataOK;   
    int result = 0;

    if (pHeader->pciDev->resource[PCR_MEMORY_BUG].start & 0x80)
    {
        result = pci_read_config_dword(pHeader->pciDev, PCI_BASE_ADDRESS_0, &dwDataBug);
        if (result)
            goto fail;

        result = pci_read_config_dword(pHeader->pciDev, PCI_BASE_ADDRESS_4, &dwDataOK);
        if (result)
            goto fail;

        result = pci_write_config_dword(pHeader->pciDev, PCI_BASE_ADDRESS_0, dwDataOK);
        if (result)
            goto fail;

        result = pci_write_config_dword(pHeader->pciDev, PCI_BASE_ADDRESS_4, dwDataBug);
        if (result)
            goto fail;

        // put changes in structures, too
        dwDataBug = pHeader->pciDev->resource[PCR_MEMORY_BUG].start;
        dwDataOK  = pHeader->pciDev->resource[PCR_MEMORY_OK].start;

        pHeader->pciDev->resource[PCR_MEMORY_BUG].start = dwDataOK;
        pHeader->pciDev->resource[PCR_MEMORY_OK].start  = dwDataBug;

        PRINTK(KERN_DEBUG "%s : bugfix memory done.\n", DEVICE_NAME);
    }

    if (pHeader->pciDev->resource[PCR_IO_BUG].start & 0x80)
    {
        result = pci_read_config_dword(pHeader->pciDev, PCI_BASE_ADDRESS_1, &dwDataBug);
        if (result)
            goto fail;

        result = pci_read_config_dword(pHeader->pciDev, PCI_BASE_ADDRESS_5, &dwDataOK);
        if (result)
            goto fail;

        result = pci_write_config_dword(pHeader->pciDev, PCI_BASE_ADDRESS_1, dwDataOK);
        if (result)
            goto fail;

        result = pci_write_config_dword(pHeader->pciDev, PCI_BASE_ADDRESS_5, dwDataBug);
        if (result)
            goto fail;

        // put changes in structures, too
        dwDataBug = pHeader->pciDev->resource[PCR_IO_BUG].start;
        dwDataOK  = pHeader->pciDev->resource[PCR_IO_OK].start;

        pHeader->pciDev->resource[PCR_IO_BUG].start = dwDataOK;
        pHeader->pciDev->resource[PCR_IO_OK].start  = dwDataBug;

        PRINTK(KERN_DEBUG "%s : bugfix io done.\n", DEVICE_NAME);
    }

    fail:
    if (result)
        printk(KERN_ERR "%s : PCI-error %d!\n", DEVICE_NAME, result);

    return result;
}

