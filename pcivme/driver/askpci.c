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
// askpci.c - a hardware independent tool to get 
//            information about searched pci-hardware
//
// $Log: askpci.c,v $
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

/*--- INCLUDES -------------------------------------------------------------*/
#include <common.h>  /* must be the first include */

#include <linux/pci.h>
#include <asm/types.h>
#include <askpci.h>

/*--- DEFINES ---------------------------------------------------------------*/


/*--- FUNCTIONS -------------------------------------------------------------*/
void DeletePCIConfig(DRIVER_OBJ *drv)
{
    PCIConfig *ch;

    while (!list_empty(&drv->pciList))     // cycle through the list of pci devices and remove them
    {
        ch = (PCIConfig *)drv->pciList.prev; // empty in reverse order
        list_del(&ch->list);
        kfree(ch);
    }
}

int GetPCIConfig(DRIVER_OBJ *drv, u16 device_id, u16 vendor_id, u16 subsys_id, u16 subven_id)
{
    int result     = 0;
    PCIConfig *dev = NULL;
    int i          = 0;

    // search pci devices
    PRINTK(KERN_DEBUG "%s : GetPCIConfig(0x%04x, 0x%04x, 0x%04x, 0x%04x)\n", DEVICE_NAME, device_id, vendor_id, subsys_id, subven_id);

    if (pci_present())
    {
        struct pci_dev *pciDev;

        struct pci_dev *from = NULL;
        do
        {
            pciDev = pci_find_device((unsigned int)vendor_id, (unsigned int)device_id, from);

            if (pciDev != NULL)
            {
                u16 wSubSysID;
                u16 wSubVenID;

                // a PCI device with PCAN_PCI_VENDOR_ID and PCAN_PCI_DEVICE_ID was found
                from = pciDev;

                // get the PCI Subsystem-ID
                result = pci_read_config_word(pciDev, PCI_SUBSYSTEM_ID, &wSubSysID);
                if (result)
                {
                    result = -ENXIO;
                    goto fail;
                }

                // get the PCI Subvendor-ID
                result = pci_read_config_word(pciDev, PCI_SUBSYSTEM_VENDOR_ID, &wSubVenID);
                if (result)
                {
                    result = -ENXIO;
                    goto fail;
                }

                // get next if the subsys and subvendor ids do not match
                if ((wSubVenID != subven_id) || (wSubSysID != subsys_id))
                    continue;

                // create space for PCIConfig descriptor
                if ((dev = (PCIConfig *)kmalloc(sizeof(PCIConfig), GFP_KERNEL)) == NULL)
                {
                    result = -ENOMEM;
                    goto fail;
                }

                // put data into pci device
                dev->pciDev = pciDev;  

                list_add_tail(&dev->list, &drv->pciList);  // add this device to the list of unchecked devices
                dev->index++;
                i++;
            }
        } while (pciDev != NULL);

        result = 0;
    }
    else
    {
        printk(KERN_ERR "%s: No pcibios present!\n", DEVICE_NAME);  
        result = -ENXIO;
    }

    fail:
    if (result)
        DeletePCIConfig(drv);

    PRINTK(KERN_DEBUG "%s : %d devices found (%d).\n", DEVICE_NAME, i, result);

    return result;  
}


/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

