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
// main.c -- the main driver module for the PCIVME PCI to VME Interface
//           Thanks to A.Rubini's Book and Dirk Muelhlenberg and H.J.Mathes 
//           for their arwvme driver
//
// $Log: main.c,v $
// Revision 1.7  2002/10/18 21:56:28  klaus
// completed functional features, untested
//
// Revision 1.6  2002/10/18 21:56:28  klaus
// completed functional features, untested
//
// Revision 1.5  2002/10/17 19:05:03  klaus
// VME access is working through test to lib to driver
//
//****************************************************************************

#define VERSION_HI 1
#define VERSION_LO 1

/*--- INCLUDES ---------------------------------------------------------------------------*/
#include <common.h>  /* must be the first include */
#include <linux/module.h>

#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/pci.h>
#include <asm/types.h>

#include <askpci.h>
#include <plxbug.h>
#include <plx9050.h>
#include <fops.h>
#include <pcivme.h>
#include <pciif.h>
#include <vic.h>
#include <main.h>

/*--- DEFINES -----------------------------------------------------------------------------*/
MODULE_AUTHOR("klaus.hitschler@gmx.de");
MODULE_DESCRIPTION("Driver for ARW Elektronik PCI VME interface.");
MODULE_SUPPORTED_DEVICE("PCIVME");
MODULE_LICENSE("GPL"); 

#define MAJOR_NO               0                         /* use dynamic assignment */

#define PCIVME_VENDOR_ID  0x10B5
#define PCIVME_DEVICE_ID  0x9050
#define PCIVME_SUBSYS_ID  0x1167
#define PCIVME_SUBVEN_ID  0x9050

/*--- TYPEDEFS ----------------------------------------------------------------------------*/

/*--- GLOBALS -----------------------------------------------------------------------------*/
EXPORT_NO_SYMBOLS; 
DRIVER_OBJ drv;

/*--- LOCALS ------------------------------------------------------------------------------*/

/*--- FUNCTIONS ---------------------------------------------------------------------------*/
static int my_interrupt(u16 intCSR)
{
    int result = NOT_MY_INTERRUPT;

    if (intCSR & 0x0040)  // it is global enabled
    {
        if ((intCSR & 0x0028) == 0x0028) // it is a enabled PCIADA interrupt
            result = PCIADA_INTERRUPT;
        else
            if ((intCSR & 0x0005) == 0x0005) // it is a enabled VMEMM interrupt
            result = VMEMM_INTERRUPT;
    }

    return result;
}

static void pcivme_irqhandler(int irq, void *dev_id, struct pt_regs *regs)
{
    DEVICE_OBJ *pd = (DEVICE_OBJ *)dev_id;

    if (pd)
    {
        // evaluate the reason of the interrupt - if it is mine
        u16 intCSR          = readw(pd->pPCIADAIntCSR);
        int which_interrupt = my_interrupt(intCSR); 

        if (which_interrupt)
        {
            writew(intCSR & ~0x40, pd->pPCIADAIntCSR); /* disable global interrupts */
            pd->wIrqStatus = (u16)which_interrupt;
            pd->dwInterruptCount++;
            wake_up_interruptible(&pd->event_queue);   /* stop blocking if any */
        }
    }
}

static int request_io_memory(PCIConfig *pPch)
{
    if (check_mem_region(pci_resource_start(pPch->pciDev, 0),  LCR_SPACE))
    {
        PRINTK(KERN_DEBUG "%s : LCR 0x%08lx\n", DEVICE_NAME, pci_resource_start(pPch->pciDev, 0)); 
        return -EBUSY;
    }

    if (check_mem_region(pci_resource_start(pPch->pciDev, 2),  CTL_SPACE))
    {
        PRINTK(KERN_DEBUG "%s : CTL 0x%08lx\n", DEVICE_NAME, pci_resource_start(pPch->pciDev, 2)); 
        return -EBUSY;
    }

    if (check_mem_region(pci_resource_start(pPch->pciDev, 2) + CTL_SPACE, VME_SPACE))
    {
        PRINTK(KERN_DEBUG "%s : VME 0x%08lx\n", DEVICE_NAME, pci_resource_start(pPch->pciDev, 2) + CTL_SPACE); 
        return -EBUSY;
    }

    request_mem_region(pci_resource_start(pPch->pciDev, 0),  LCR_SPACE, DEVICE_NAME);
    request_mem_region(pci_resource_start(pPch->pciDev, 2),  CTL_SPACE, DEVICE_NAME);
    request_mem_region(pci_resource_start(pPch->pciDev, 2) + CTL_SPACE, VME_SPACE, DEVICE_NAME);

    return 0;
}

static void release_io_memory(PCIConfig *pPch)
{
    release_mem_region(pci_resource_start(pPch->pciDev, 0),  LCR_SPACE);
    release_mem_region(pci_resource_start(pPch->pciDev, 2),  CTL_SPACE);
    release_mem_region(pci_resource_start(pPch->pciDev, 2) + CTL_SPACE, VME_SPACE);
}

static int translate_addresses(DEVICE_OBJ *pd, PCIConfig *pPch)    /* differs from PCICC32 */
{
    if (pci_resource_start(pPch->pciDev, 0) < LOW_MEMORY)  /* LCR ISA base addresses */
        pd->pLCR = (u32)bus_to_virt(pci_resource_start(pPch->pciDev, 0));
    else
        pd->pLCR = (u32)ioremap(pci_resource_start(pPch->pciDev, 0), LCR_SPACE);

    if (pci_resource_start(pPch->pciDev, 2) < LOW_MEMORY)  /* User ISA base addresses */
    {
        pd->pCtl = (u32)bus_to_virt(pci_resource_start(pPch->pciDev, 2)            );
        pd->pVME = (u32)bus_to_virt(pci_resource_start(pPch->pciDev, 2) + CTL_SPACE);
    }
    else
    {
        pd->pPhysVME = pci_resource_start(pPch->pciDev, 2) + CTL_SPACE;

        pd->pCtl = (u32)ioremap(pci_resource_start(pPch->pciDev, 2)            , CTL_SPACE);
        pd->pVME = (u32)ioremap(pci_resource_start(pPch->pciDev, 2) + CTL_SPACE, VME_SPACE);
    }

    return 0;
}

static void un_translate_addresses(DEVICE_OBJ *pd, PCIConfig *pPch)
{
    if (pci_resource_start(pPch->pciDev, 0) >= LOW_MEMORY)  /* no LCR ISA base addresses */
        iounmap((void *)pd->pLCR);

    if (pci_resource_start(pPch->pciDev, 2) >= LOW_MEMORY)
    {
        pd->pPhysVME = 0;

        iounmap((void *)pd->pCtl);
        iounmap((void *)pd->pVME);
    }
}

static void soft_init(DEVICE_OBJ *pd)
{
    if (pd)
    {
        init_waitqueue_head(&pd->event_queue);

        pd->pLCR = pd->pCtl = pd->pVME = 0;
        pd->pPch = (PCIConfig *)NULL;
        pd->bConnected = 0;
        pd->wInitStep = 0;     
        pd->wIrq = 0xFFFF;          
        pd->dwInterruptCount = 0;
        pd->wIrqStatus = 0; 
        pd->nOpenCounter = 0;

        pd->cModuleNumber = 255;
        pd->cFPGAVersion = 255;
        pd->cSystemController = 0;
        pd->cWordMode         = 0;

        pd->pAdrMod = 0;
        pd->pAdrReg = 0;
        pd->pCSR    = 0;

        pd->pPCIADACntrl  = 0;
        pd->pPCIADAIntCSR = 0;      

        pd->bCurrentModifier     = 0;
        pd->dwCurrentPageAddress = -1;
    }
}

int test_connection(DEVICE_OBJ *pd)
{
    u16 intCSR_store;
    u16 cntrl_store;
    int i;       
    int error   = 0;
    u32 dwADRH  = pd->pCtl + ADRH;
    u32 dwADRL  = pd->pCtl + ADRL;
    u32 dwADRHL = pd->pCtl + ADRHL;
    u32 dwStore;
    u16 wRet;

    cntrl_store  = readw(pd->pPCIADACntrl);         /* read CONTROL register */
    intCSR_store = readw(pd->pPCIADAIntCSR);        /* read interrupt + CSR register */

    writew(0, pd->pPCIADAIntCSR);                   /* disable interrupts */
    writew(cntrl_store | 0x0180, pd->pPCIADACntrl); /* enable access */

    // save adr register
    dwStore = readl(dwADRHL);
    for (i = 1000; i; i--)
    {
        writew(0x5555, dwADRH);
        writew(0xAAAA, dwADRL);
        wRet   = readw(dwADRH);
        if (wRet != 0x5555)
        {
            error = 1;
            break;
        }

        writew(0xAAAA, dwADRH);
        writew(0x5555, dwADRL);
        wRet   = readw(dwADRH);
        if (wRet != 0xAAAA)
        {
            error = 1;
            break;
        }

        writew(0x0000, dwADRH);
        writew(0xFFFF, dwADRL);
        wRet   = readw(dwADRH);
        if (wRet != 0x0000)
        {
            error = 1;
            break;
        }

        writew(0xFFFF, dwADRH);
        writew(0x0000, dwADRL);
        wRet   = readw(dwADRH);
        if (wRet != 0xFFFF)
        {
            error = 1;
            break;
        }
    }

    // restore register
    writel(dwStore, dwADRHL);;

    //clear possible interrupts
    writew(cntrl_store & ~0x0100, pd->pPCIADACntrl); /* clear potential interrupt */

    // restore LCR registers
    writew(cntrl_store,  pd->pPCIADACntrl);
    writew(intCSR_store, pd->pPCIADAIntCSR);

    return error;   
}

int get_module_info(DEVICE_OBJ *pd)
{
    u16 intCSR_store;
    u16 cntrl_store;
    int found = 0;
    u16 data;

    cntrl_store  = readw(pd->pPCIADACntrl);  /* read CONTROL register */
    intCSR_store = readw(pd->pPCIADAIntCSR); /* read interrupt + CSR register */

    PRINTK(KERN_DEBUG "%s : cntrl=0x%04x, intCSR=0x%04x\n", DEVICE_NAME, cntrl_store, intCSR_store); 

    if (cntrl_store & 0x0800) /* a VMEMM is connected */
    {
        u16 bla = cntrl_store | 0x0180;

        writew(0,   pd->pPCIADAIntCSR); /* disable interrupts */
        writew(bla, pd->pPCIADACntrl);  /* enable access */

        // read main status register
        data = readw(pd->pCSR);

        if ((data & 0xF000) != VMEMM_MODULE_TYPE)
        {
            pd->cModuleNumber = pd->cFPGAVersion = 255;
            printk(KERN_ERR "%s : Wrong module type connected @ index %d!\n", DEVICE_NAME, pd->wIndex);
        }
        else
        {
            found = 1;
            pd->cModuleNumber     = (data >> 4) & 0xF;
            pd->cFPGAVersion      = (data >> 8) & 0xF; 
            pd->cSystemController = (data & 0x0008);
            pd->cWordMode         = (data & 0x0004);
        } 

        // clear possible interrupts
        writew(cntrl_store & ~0x0100, pd->pPCIADACntrl); /* clear potential interrupt */

        /* restore all contents */
        writew(cntrl_store,  pd->pPCIADACntrl);
        writew(intCSR_store, pd->pPCIADAIntCSR);
    }

    return found;   
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0)
static int pcivme_read_proc(char *buf, char **start, off_t offset, int len)
#else
static int pcivme_read_proc(char *buf, char **start, off_t offset, int len, int *eof, void *data)
#endif
    {
    int              pos = 0;
    DEVICE_OBJ       *pd;
    PCIConfig        *ch;
    u16                cntrl;
    char             *cause = "none";
    struct list_head *ptr;

    pos += sprintf(buf + pos, "\nPCIVME information. Version %d.%d of %s from Klaus Hitschler.\n", VERSION_HI, VERSION_LO, __DATE__);

    pos += sprintf(buf + pos, " ---------------------\n"); 
    pos += sprintf(buf + pos, " Interfaces found : %d\n", drv.count);
    pos += sprintf(buf + pos, " Major Number     : %d\n", drv.nMajor);

    for (ptr = drv.devList.next; ptr != &drv.devList; ptr = ptr->next)
    {
        pd = list_entry(ptr, DEVICE_OBJ, list);
        ch = pd->pPch;
        cntrl = readw(pd->pLCR + PLX9050_CNTRL);
        pos += sprintf(buf + pos, " --- %d ---------------\n", pd->wIndex + 1); 
        pos += sprintf(buf + pos, " LCR     phys/virt/size : 0x%08lx/0x%08x/%d\n", pci_resource_start(ch->pciDev, 0),             pd->pLCR, LCR_SPACE);
        pos += sprintf(buf + pos, " Control phys/virt/size : 0x%08lx/0x%08x/%d\n", pci_resource_start(ch->pciDev, 2),             pd->pCtl, CTL_SPACE);
        pos += sprintf(buf + pos, " VME     phys/virt/size : 0x%08lx/0x%08x/%d\n", pci_resource_start(ch->pciDev, 2) + CTL_SPACE, pd->pVME, VME_SPACE);
        pos += sprintf(buf + pos, " Irq                    : %d\n", pd->wIrq);

        if (pd->bConnected)
        {
            pos += sprintf(buf + pos, " VMEMM is or was        : (software) connected.\n");
            pos += sprintf(buf + pos, " Module-Number          : %d\n", pd->cModuleNumber);
            pos += sprintf(buf + pos, " FPGA-Version           : %d\n", pd->cFPGAVersion);
            pos += sprintf(buf + pos, " Systemcontroller       : %s\n", (pd->cSystemController) ? "yes" : "no");
            pos += sprintf(buf + pos, " Word Mode              : %s\n", (pd->cWordMode) ? "yes" : "no");
        }
        else
            pos += sprintf(buf + pos, " VMEMM is or was        : not (software) connected.\n");       

        if (!((cntrl & 0x0800) && (!(cntrl & 0x0600))))
            pos += sprintf(buf + pos, " VMEMM is               : powered off or cable disconnected.\n");

        pos += sprintf(buf + pos, " IrqCount               : %d\n", pd->dwInterruptCount);
        if (pd->wIrqStatus & PCIADA_INTERRUPT)
            cause = "Timeout";
        else
            if (pd->wIrqStatus & VMEMM_INTERRUPT)
                cause = "VME";
        pos += sprintf(buf + pos, " Pending IrqStatus      : %s\n", cause);
    }

    pos += sprintf(buf + pos, "\n");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,0)
    *eof = 1;
#endif

    return pos;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0)
struct proc_dir_entry pcimod_proc_entry = 
{
    namelen:    7,                  /* len of name */
    name:       DEVICE_NAME,        /* entry  name */
    mode:       S_IFREG | S_IRUGO,  /* mode */
    nlink:      1,                  /* nlinks */
    get_info:   pcivme_read_proc,  /* function used to read data */
};
#endif

static void deleteMyLists(void)
{
    DEVICE_OBJ      *pd;

    /* delete my lists */
    while (!list_empty(&drv.devList))                 // cycle through the list of pci devices and remove them
    {
        pd = (DEVICE_OBJ *)drv.devList.prev;            // empty in reverse order
        list_del(&pd->list);
        kfree(pd);
    }

    DeletePCIConfig(&drv);
}

int init_module(void)
{
    PCIConfig        *ch;
    DEVICE_OBJ       *pd;
    int              result = 0;
    struct list_head *ptr;

    PRINTK(KERN_DEBUG "%s : init_module\n", DEVICE_NAME);

    /* create list of PCIADAs and work devices */
    INIT_LIST_HEAD(&drv.devList);
    INIT_LIST_HEAD(&drv.pciList);

    drv.count = 0;

    /* search for all PCIADA modules */
    if ((result = GetPCIConfig(&drv, PCIVME_DEVICE_ID, PCIVME_VENDOR_ID, PCIVME_SUBSYS_ID, PCIVME_SUBVEN_ID)))
    {
        deleteMyLists();
        return result;
    }

    /* fix the PLX bug in all PCIADAs */
    for (ptr = drv.pciList.next; ptr != &drv.pciList; ptr = ptr->next)
    {
        ch = list_entry(ptr, PCIConfig, list);
        PLX9050BugFix(ch);
    }

    /* create work_devices and translate the access addresses */
    for (ptr = drv.pciList.next; ptr != &drv.pciList; ptr = ptr->next)
    {
        ch = list_entry(ptr, PCIConfig, list);

        pd = (DEVICE_OBJ *)kmalloc(sizeof(DEVICE_OBJ), GFP_ATOMIC);
        soft_init(pd);
        pd->pPch = ch;
        pd->wIndex = drv.count;

        if (!request_io_memory(ch))
        {
            pd->wInitStep = 1;

            if (translate_addresses(pd, ch))
            {
                printk(KERN_ERR "%s : translation of addresses failed!\n", DEVICE_NAME);
                kfree_s(pd, sizeof(*pd));       // FREE(pd);
            }
            else
            {
                // successful translate_addresses
                pd->wInitStep = 2;

                // create some 'fast access' addresses
                pd->pAdrMod = pd->pCtl + VICBASE + AMSR; 
                pd->pAdrReg = pd->pCtl + ADRHL;
                pd->pCSR    = pd->pCtl + CSR;

                pd->pPCIADACntrl  = pd->pLCR + PLX9050_CNTRL;
                pd->pPCIADAIntCSR = pd->pLCR + PLX9050_INTCSR;

                if (request_irq(pd->pPch->pciDev->irq, pcivme_irqhandler, SA_INTERRUPT | SA_SHIRQ, DEVICE_NAME, pd))
                {
                    printk(KERN_ERR "%s : can't get irq @ %d\n", DEVICE_NAME, pd->pPch->pciDev->irq);
                    kfree_s(pd, sizeof(*pd));       // FREE(pd);
                }
                else
                {
                    // successful request_irq
                    pd->wInitStep = 3;
                    pd->wIrq = pd->pPch->pciDev->irq;

                    list_add_tail(&pd->list, &drv.devList);  /* add this device to list of working devices*/
                    drv.count++;

                    pd->bConnected =  get_module_info(pd);
                    if (pd->bConnected && test_connection(pd))
                    {
                        printk(KERN_ERR "%s : connection test @ driver install failed!\n", DEVICE_NAME);
                        pd->bConnected = 0;
                    }
                }
            }
        }
        else
            printk(KERN_ERR "%s : requested io-memory still claimed!\n", DEVICE_NAME);
    }

    drv.nMajor = MAJOR_NO;
    result = register_chrdev(drv.nMajor, DEVICE_NAME, &pcivme_fops);
    if (result < 0)
    {
        printk(KERN_ERR "%s: Can't install driver (%d)\n", DEVICE_NAME, result);

        /* untranslate translated addresses */
        for (ptr = drv.devList.next; ptr != &drv.devList; ptr = ptr->next)
        {
            pd = list_entry(ptr, DEVICE_OBJ, list);
            ch = pd->pPch;
            un_translate_addresses(pd, ch); 
        }

        /* delete my lists */
        deleteMyLists();

        return result;
    }
    else
    {
        if (drv.nMajor == 0) 
            drv.nMajor = result;

        printk(KERN_DEBUG "%s : major #%d assigned.\n", DEVICE_NAME, drv.nMajor);
    }

    /* register the proc device */ 
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0)  
    proc_register_dynamic(&proc_root, &pcimod_proc_entry);

    return 0;
#else
    return create_proc_read_entry(DEVICE_NAME, 0, NULL, pcivme_read_proc, NULL) ? 0 : -ENODEV; 
#endif
}

void cleanup_module(void)
{
    PCIConfig        *ch;
    DEVICE_OBJ       *pd;
    struct list_head *ptr;

    PRINTK(KERN_DEBUG "%s : cleanup_module.\n", DEVICE_NAME);

    unregister_chrdev(drv.nMajor, DEVICE_NAME);

    /* unregister the proc device */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0)
    proc_unregister(&proc_root, pcimod_proc_entry.low_ino);
#else
    remove_proc_entry(DEVICE_NAME, NULL);
#endif

    /* redo all */
    for (ptr = drv.devList.next; ptr != &drv.devList; ptr = ptr->next)
    {
        pd = list_entry(ptr, DEVICE_OBJ, list); 
        ch = pd->pPch;
        switch (pd->wInitStep)
        {
            case 3:  writew(readw(pd->pLCR + PLX9050_INTCSR) & ~0x40, pd->pLCR + PLX9050_INTCSR);  // disable global interrupts
                     free_irq(pd->wIrq, pd);  
            case 2:  un_translate_addresses(pd, ch);
            case 1:  release_io_memory(ch);
            default: pd->wInitStep = 0; 
        }

        drv.count--;
    }

    deleteMyLists();

    return;
}

