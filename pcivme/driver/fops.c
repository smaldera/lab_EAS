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
// fops.c -- the file operations module for the PCIVME PCI to VME Interface
//
// $Log: fops.c,v $
// Revision 1.8  2002/10/20 18:06:51  klaus
// changed error handling
//
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

/*--- INCLUDES -----------------------------------------------------------------------------------*/
#include <common.h>  /* must be the first include */

#include <linux/kernel.h> /* printk() */
#include <linux/module.h> /* only here ?cause of MAJOR ... */
#include <linux/pci.h>
#include <linux/list.h>
#include <asm/errno.h>
#include <asm/types.h>
#include <asm/uaccess.h>

#include <fops.h>
#include <plx9050.h>
#include <pcivme.h>      /* the common ioctl commands and structures between driver and application */
#include <main.h>
#include <askpci.h>
#include <pciif.h>
#include <vic.h>
#include <vme.h>

/*--- DEFINES ------------------------------------------------------------------------------------*/
static PCIVME_INIT_ELEMENT init_element[] =
{{LCR,  WORD_ACCESS, PLX9050_INTCSR, DISABLE_PCIADA_IRQS}, // disable interrupts
    {LCR,  WORD_ACCESS, PLX9050_CNTRL,  RELEASE_VMEMM},       // enable interface

    {VIC,  BYTE_ACCESS, VIICR, 0xf8+1},      // VIICR

    {VIC,  BYTE_ACCESS, VICR1, 0x78+1},      // VICR1
    {VIC,  BYTE_ACCESS, VICR2, 0x78+2},
    {VIC,  BYTE_ACCESS, VICR3, 0x78+3},
    {VIC,  BYTE_ACCESS, VICR4, 0x78+4},
    {VIC,  BYTE_ACCESS, VICR5, 0x78+5},
    {VIC,  BYTE_ACCESS, VICR6, 0x78+6},
    {VIC,  BYTE_ACCESS, VICR7, 0x78+7},      // VICR7

    {VIC,  BYTE_ACCESS, DSICR, 0xf8+0},      // DSICR

    {VIC,  BYTE_ACCESS, LICR1, 0xf8+1},      // LICR1
    {VIC,  BYTE_ACCESS, LICR2, 0xf8+2},
    {VIC,  BYTE_ACCESS, LICR3, 0xf8+3},
    {VIC,  BYTE_ACCESS, LICR4, 0xf8+4},
    {VIC,  BYTE_ACCESS, LICR5, 0xf8+5},
    {VIC,  BYTE_ACCESS, LICR6, 0x38+6}, 
    {VIC,  BYTE_ACCESS, LICR7, 0x38+7},      // LICR7

    {VIC,  BYTE_ACCESS, ICGSICR, 0xf8+2},    // ICGS
    {VIC,  BYTE_ACCESS, ICMSICR, 0xf8+3},    // ICMS

    {VIC,  BYTE_ACCESS, EGICR, 0xf8+6},      // EGICR

    {VIC,  BYTE_ACCESS, ICGSVBR, 0x08},      // ICGS-IVBR (!)
    {VIC,  BYTE_ACCESS, ICMSVBR, 0x0c},      // ICMS-IVBR (!)

    {VIC,  BYTE_ACCESS, LIVBR, 0x00},        // LIVBR (!)

    {VIC,  BYTE_ACCESS, EGIVBR, 0x10},       // EGIVBR (!)

    {VIC,  BYTE_ACCESS, ICSR, 0x00},         // ICSR

    {VIC,  BYTE_ACCESS, ICR0, 0x00},         // ICR0
    {VIC,  BYTE_ACCESS, ICR1, 0x00},
    {VIC,  BYTE_ACCESS, ICR2, 0x00},
    {VIC,  BYTE_ACCESS, ICR3, 0x00},
    {VIC,  BYTE_ACCESS, ICR4, 0x00},         // ICR4

    {VIC,  BYTE_ACCESS, VIRSR, 0xfe},        // VIRSR

    {VIC,  BYTE_ACCESS, VIVR1, 0x0f},        // VIVR1
    {VIC,  BYTE_ACCESS, VIVR2, 0x0f},
    {VIC,  BYTE_ACCESS, VIVR3, 0x0f},
    {VIC,  BYTE_ACCESS, VIVR4, 0x0f},
    {VIC,  BYTE_ACCESS, VIVR5, 0x0f},
    {VIC,  BYTE_ACCESS, VIVR6, 0x0f},
    {VIC,  BYTE_ACCESS, VIVR7, 0x0f},        // VIVR7

    {VIC,  BYTE_ACCESS, TTR, 0x3c},          // TTR

    {VIC,  BYTE_ACCESS, ARCR, 0x40},         // ARCR
    {VIC,  BYTE_ACCESS, AMSR, 0x29},         // AMSR
    {VIC,  BYTE_ACCESS, RCR, 0x00},          // RCR

    {IFR,  LONG_ACCESS, (u16)ADRHL, 0xF0F0F0F0},  // ADR-H, ADR-L
    {IFR,  WORD_ACCESS, (u16)CSR  , 0x0000},      // Contr-Reg

    {VIC,  BYTE_ACCESS, ICR7, 0x80},         // ICR7

    {LCR,  WORD_ACCESS, PLX9050_INTCSR, DISABLE_PCIADA_IRQS},  // disable interrupts

    {STOP, WORD_ACCESS, 0,     0}};

static PCIVME_INIT_ELEMENT deinit_element_pre[] =
{{VIC,  BYTE_ACCESS, ICR7, 0x00},         // ICR7 - sysfail
    {LCR,  WORD_ACCESS, PLX9050_INTCSR, DISABLE_PCIADA_IRQS},  // disable interrupts
    {STOP, WORD_ACCESS, 0,    0}};

static PCIVME_INIT_ELEMENT deinit_element_post[] =
{{LCR,  WORD_ACCESS, PLX9050_CNTRL, INHIBIT_VMEMM},     // disable interface
    {STOP, WORD_ACCESS, 0,    0}};


/*--- EXTERNALS ----------------------------------------------------------------------------------*/

/*--- TYPEDEFS -----------------------------------------------------------------------------------*/

/*--- FUNCTIONS ----------------------------------------------------------------------------------*/
static inline void switch_VMEMM_on(DEVICE_OBJ *pd)
{
    writew(RELEASE_VMEMM, pd->pLCR + PLX9050_CNTRL); /* enable access */
}

static inline void switch_VMEMM_off(DEVICE_OBJ *pd)
{
    writew(INHIBIT_VMEMM, pd->pLCR + PLX9050_CNTRL); /* enable access */
}

static inline void setPageAddress(DEVICE_OBJ *pd, u32 newPageAddress)
{
    PRINTK(KERN_DEBUG "%s : setPageAddress(0x%08x)\n", DEVICE_NAME, newPageAddress);

    writel(newPageAddress, pd->pAdrReg);
    pd->dwCurrentPageAddress = newPageAddress;
}

static inline void setModifier(DEVICE_OBJ *pd, u8 newModifier)
{
    PRINTK(KERN_DEBUG "%s : setModifier(0x%02x)\n", DEVICE_NAME, newModifier);

    writeb(newModifier, pd->pAdrMod);
    pd->bCurrentModifier = newModifier;
}

/* read and write functions -----------------------------------------------------------------------*/
static void readByte(DEVICE_OBJ *pd, void **pvBuffer, u32 dwLocalAddressInPage)
{
    u8 tmp;

    tmp = readb(pd->pVME + dwLocalAddressInPage);
    __put_user(tmp, ((u8 *)*pvBuffer)++);
}

static void writeByte(DEVICE_OBJ *pd, u32 dwLocalAddressInPage, void **pvBuffer)
{
    u8 tmp;

    __get_user(tmp, ((u8 *)*pvBuffer)++);
    writeb(tmp, pd->pVME + dwLocalAddressInPage);
}

static void readWord(DEVICE_OBJ *pd, void **pvBuffer, u32 dwLocalAddressInPage)
{
    u16 tmp;

    tmp = readw(pd->pVME + dwLocalAddressInPage);
    __put_user(tmp, ((u16 *)*pvBuffer)++);
}

static void writeWord(DEVICE_OBJ *pd, u32 dwLocalAddressInPage, void **pvBuffer)
{
    u16 tmp;

    __get_user(tmp, ((u16 *)*pvBuffer)++);
    writew(tmp, pd->pVME + dwLocalAddressInPage);
}

static void readLong(DEVICE_OBJ *pd, void **pvBuffer, u32 dwLocalAddressInPage)
{
    u32 tmp;

    tmp = readl(pd->pVME + dwLocalAddressInPage);
    __put_user(tmp, ((u32 *)*pvBuffer)++);
}

static void writeLong(DEVICE_OBJ *pd, u32 dwLocalAddressInPage, void **pvBuffer)
{
    u32 tmp;

    __get_user(tmp, ((u32 *)*pvBuffer)++);
    writel(tmp, pd->pVME + dwLocalAddressInPage);
}

/* test alignment functions -----------------------------------------------------------------------*/
static int MisalignmentForByteAccess(loff_t offset)
{
    return 0;
}

static int MisalignmentForWordAccess(loff_t offset)
{
    return(offset & 1);
}

static int MisalignmentForLongAccess(loff_t offset)
{
    return(offset & 3);
}

// helper functions --------------------------------------------------------------------------------
int check_command(const PCIVME_INIT_ELEMENT *psInitElement)
{
    u16 range;
    u16 access_size;

    // PRINTK(KERN_DEBUG "%s : check_command()\n", DEVICE_NAME);

    switch (psInitElement->bDestination)
    {
        case LCR: 
            range = 0x54;     
            break;
        case IFR: 
            range = 0x0c;     
            break;
        case VIC: 
            range = 0xe4;
            if ((psInitElement->wOffset & 3) != 3)
                return -EINVAL;
            break;
        default:  
            return -EINVAL;        
            break;
    }

    // check alignment and allowed address range
    switch (psInitElement->bAccessType)
    {
        case LONG_ACCESS: 
            if (psInitElement->wOffset & 3)
                return -EINVAL;
            access_size = sizeof(u32);
            break;
        case WORD_ACCESS: 
            if (psInitElement->wOffset & 1)
                return -EINVAL;
            access_size = sizeof(u16);
            break;
        case BYTE_ACCESS: 
            access_size = sizeof(u8); 
            break;
        default         : 
            return -EINVAL;        
            break;
    }

    if ((psInitElement->wOffset + access_size) > range)
        return -EINVAL;       // ignore it

    return 0;
}

static int CmdMachine(DEVICE_OBJ *pd, const PCIVME_INIT_ELEMENT *psInitElement)
{
    u32 adr;
    int err;

    PRINTK(KERN_DEBUG "%s : CmdMachine()\n", DEVICE_NAME);

    // loop through the init (or deinit) list
    while (psInitElement->bDestination != STOP)
    {
        err = check_command(psInitElement);
        if (!err)
        {
            switch (psInitElement->bDestination)
            {
                case LCR: 
                    adr = pd->pLCR; 
                    break;
                case VIC: 
                    adr = pd->pCtl + VICBASE; 
                    break;
                case IFR:
                    adr = pd->pCtl + CSR; 
                    break;  
                default: 
                    return -EINVAL; 
            }

            switch (psInitElement->bAccessType)
            {
                case LONG_ACCESS:
                    writel(psInitElement->dwValue, adr + psInitElement->wOffset);
                    break;
                case WORD_ACCESS:
                    writew((u16)psInitElement->dwValue, adr + psInitElement->wOffset);
                    break;
                case BYTE_ACCESS:
                    writeb((u8)psInitElement->dwValue, adr + psInitElement->wOffset);
                    break;
                default:
                    return -EINVAL;
            }
        }
        else
            return err;

        psInitElement++;
    }

    return 0;
}

// all ioctls --------------------------------------------------------------------------------------
static int init_hardware(PATH_OBJ *pp, DEVICE_OBJ *pd, PCIVME_INIT_COMMAND *init)
{ 
    int err;
    PCIVME_INIT_ELEMENT *element = init->sVie;

    PRINTK(KERN_DEBUG "%s : init_hardware()\n", DEVICE_NAME);

    err = CmdMachine(pd, element);
    if (err)
    {
        PRINTK(KERN_DEBUG "%s : init failed with err = %d!\n", DEVICE_NAME, err); 
        return err;
    }

    // sync storage with hardware
    pd->bCurrentModifier     = readb(pd->pAdrMod) & 0x3f;
    pd->dwCurrentPageAddress = readl(pd->pAdrReg) & HI_ADDRESS_MASK;

    return 0;
}

static int deinit_hardware(PATH_OBJ *pp, DEVICE_OBJ *pd, PCIVME_INIT_COMMAND *deinit)
{
    int err;
    PCIVME_INIT_ELEMENT *element = deinit->sVie;

    PRINTK(KERN_DEBUG "%s : deinit_hardware()\n", DEVICE_NAME);

    err = CmdMachine(pd, deinit_element_pre);  
    if (err)
        goto fail;

    err = CmdMachine(pd, element);
    if (err)
        goto fail;

    err = CmdMachine(pd, deinit_element_post);
    if (err)
        goto fail;

    return 0;

    fail:
    return err;
}

static int access_command(PATH_OBJ *pp, DEVICE_OBJ *pd, PCIVME_ACCESS_COMMAND *cmd)
{
    PRINTK(KERN_DEBUG "%s : access_command()\n", DEVICE_NAME);

    pp->bModifier    = cmd->bModifier; 
    pp->bAccessType  = cmd->bAccessType;
    pp->bIncrement   = cmd->bIncrement; 

    switch (pp->bAccessType)
    {
        case BYTE_ACCESS:
            pp->read  = readByte;
            pp->write = writeByte;
            pp->AlignmentCheck = MisalignmentForByteAccess;
            break;
        case WORD_ACCESS:
            pp->read  = readWord;
            pp->write = writeWord;
            pp->AlignmentCheck = MisalignmentForWordAccess;
            break;
        case LONG_ACCESS:
            pp->read  = readLong;
            pp->write = writeLong;
            pp->AlignmentCheck = MisalignmentForLongAccess;
            break;
        default:
            return -EINVAL;
    }

    return 0;
}

static int get_static_status(PATH_OBJ *pp, DEVICE_OBJ *pd, PCIVME_STATIC_STATUS *static_status)
{
    PRINTK(KERN_DEBUG "%s : get_static_status()\n", DEVICE_NAME);

    static_status->bConnected        = pd->bConnected;
    static_status->cModuleNumber     = pd->cModuleNumber;
    static_status->cFPGAVersion      = pd->cFPGAVersion;
    static_status->cSystemController = pd->cSystemController;
    static_status->cWordMode         = pd->cWordMode;

    return 0;
}

static int get_dynamic_status(PATH_OBJ *pp, DEVICE_OBJ *pd, PCIVME_DYNAMIC_STATUS *dynamic_status)
{
    u16 cntrl  = readw(pd->pPCIADACntrl); 
    u16 intCSR = readw(pd->pPCIADAIntCSR);

    PRINTK(KERN_DEBUG "%s : get_dynamic_status()\n", DEVICE_NAME);

    dynamic_status->bConnected = (cntrl  & 0x0800) ? 1 : 0;
    dynamic_status->bPCIADAIrq = (intCSR & 0x0020) ? 1 : 0;
    dynamic_status->bVMEMMIrq  = (intCSR & 0x0004) ? 1 : 0;

    return 0;
}

static int read_vector_polling(PATH_OBJ *pp, DEVICE_OBJ *pd, PCIVME_VECTOR_LEVEL *vector)
{
    u16 cntrl  = readw(pd->pPCIADACntrl);
    u16 intCSR = readw(pd->pPCIADAIntCSR);

    PRINTK(KERN_DEBUG "%s : read_vector()\n", DEVICE_NAME);

    vector->dwStatusID = 0;  
    vector->bLevel     = 0;
    vector->bPCIADAIrq = 0; 

    if (intCSR & 0x20) // check for PCIADA interrupt
    {
        vector->bPCIADAIrq = 1;
        vector->dwStatusID = 1; // force for PCIADA irqs

        writew(cntrl & ~0x0100, pd->pPCIADACntrl);   // clear pending PCIADA irq
        writew(cntrl,           pd->pPCIADACntrl);
    }
    else
    {
        if ((cntrl & 0x0980) == 0x0980) // check if VMEMM is connected and ready
        {
            vector->bLevel = (u8)readw(pd->pCtl + VICRES);
            if (vector->bLevel & 1)
            {
                if (vector->bLevel != 1)
                    vector->dwStatusID = (u32)readb(pd->pCtl + VECBASE + vector->bLevel);

                vector->bLevel >>= 1; 
            }
        }
    }
    return 0;
}

static int read_vector_blocking(PATH_OBJ *pp, DEVICE_OBJ *pd, PCIVME_VECTOR_LEVEL *vector, struct file *pFile)
{
    int error;

    vector->dwStatusID = 0;  
    vector->bLevel     = 0;
    vector->bPCIADAIrq = 0; 

    // support nonblocking read if requested
    if ((pFile->f_flags & O_NONBLOCK) && (!pd->wIrqStatus))
        return -EAGAIN;

    // sleep until data are available
    if ((error = wait_event_interruptible(pd->event_queue, (pd->wIrqStatus))))
        return error;

    error = read_vector_polling(pp, pd, vector);

    pd->wIrqStatus  = 0;  // clear the status since it is read	

    return error;
}


static int control_interrupts(PATH_OBJ *pp, DEVICE_OBJ *pd, PCIVME_IRQ_CONTROL *irq_control)
{
    u16 intCSR = readw(pd->pPCIADAIntCSR);
    u8  ret    = (intCSR & 0x40) ? 1 : 0;

    PRINTK(KERN_DEBUG "%s : control_interrupts()\n", DEVICE_NAME);

    if (irq_control->bEnable)
        writew(intCSR |  0x40, pd->pPCIADAIntCSR);
    else
        writew(intCSR & ~0x40, pd->pPCIADAIntCSR);

    // return the switch before set
    irq_control->bEnable = ret;

    return 0;
}

static int VME_TAS(PATH_OBJ *pp, DEVICE_OBJ *pd, PCIVME_TAS_STRUCT *tas_cmd)
{
    u32 access_adr = pd->pVME + (tas_cmd->dwAddress & LO_ADDRESS_MASK); // make low part of address
    u8  data;

    // save old contents
    u32 old_address         = readl(pd->pAdrReg);
    u16 old_CSR             = readw(pd->pCSR);
    u16 intCSR              = readw(pd->pPCIADAIntCSR);
    pd->bCurrentModifier    = readb(pd->pAdrMod) & 0x3f;

    PRINTK(KERN_DEBUG "%s : VME_TAS()\n", DEVICE_NAME);

    // set new contents
    writew(DISABLE_PCIADA_IRQS,           pd->pPCIADAIntCSR);
    writeb((u8)tas_cmd->bModifier & 0x3f, pd->pAdrMod);
    writel(tas_cmd->dwAddress,            pd->pAdrReg);
    writew(old_CSR | FLAG_RMC,            pd->pCSR);

    // do the read - modify - write
    data = readb(access_adr);
    writeb(tas_cmd->bContent, access_adr);

    // restore old contents
    writeb(pd->bCurrentModifier, pd->pAdrMod);
    writew(old_CSR,              pd->pCSR);
    writel(old_address,          pd->pAdrReg);
    writew(intCSR,               pd->pPCIADAIntCSR);

    // get back read data
    tas_cmd->bContent = data;

    return 0;
}

static int VMEMM_RESET(PATH_OBJ *pp, DEVICE_OBJ *pd, PCIVME_RESET_COMMAND *reset_cmd)
{
    u16 cntrl  = readw(pd->pPCIADACntrl);
    u16 intCSR = readw(pd->pPCIADAIntCSR);
    int status = 0;

    PRINTK(KERN_DEBUG "%s : VMEMM_RESET()\n", DEVICE_NAME);

    // am I connected and switched on??
    if ((cntrl & 0x0980) == 0x0980)
    {
        // do command
        switch (reset_cmd->bCommand)
        {
            case POLL_RESET_CMD:
                break;
            case VME_RESET_CMD:
                writeb(0, pd->pAdrMod);
                writeb(0xf0, pd->pCtl + VICBASE + SRR);  // make VME reset
                break;
            case LOCAL_RESET_CMD:
                writeb(0, pd->pAdrMod);
                writew(LOCAL_RESET,  pd->pCtl + VICRES);
                break;
            case GLOBAL_RESET_CMD:
                writeb(0, pd->pAdrMod);
                writew(GLOBAL_RESET, pd->pCtl + VICRES);
                break;

            default: status = -EINVAL;
        }

        // inhibit PCIADA generated irqs
        writew(DISABLE_PCIADA_IRQS, pd->pPCIADAIntCSR);

        // always poll reset status - access will sometimes generate PCIADA #2 interrupt
        reset_cmd->bResult = readb(pd->pAdrMod);

        // reset any pending PCIADA interrupt #2
        writew(cntrl & ~0x0100, pd->pPCIADACntrl);
        writew(cntrl          , pd->pPCIADACntrl);

        // restore IRQStatus
        writew(intCSR          , pd->pPCIADAIntCSR);
    }
    else
        status = -EBUSY;

    // sync storage with hardware
    pd->bCurrentModifier = readb(pd->pAdrMod) & 0x3f;

    return status;
}

static int access_VIC68A(PATH_OBJ *pp, DEVICE_OBJ *pd, PCIVME_VIC68A_ACTION *action)
{
    int nStatus = 0;

    PRINTK(KERN_DEBUG "%s : access_VIC68A()\n", DEVICE_NAME);

    if ((action->wRegisterAddress <= SRR) && ((action->wRegisterAddress & 0x03) == 3))
    {
        u32 dwAddress;
        u8  bByte = 0;

        dwAddress = (pd->pCtl + VICBASE + action->wRegisterAddress);

        switch (action->bAccessMode)
        {
            case VIC68A_WRITE_ONLY:
                writeb(action->bContent, dwAddress);
                break;
            case VIC68A_WRITE:  
                writeb(action->bContent, dwAddress);
                action->bContent = readb(dwAddress);
                break;
            case VIC68A_OR:     
                bByte      = readb(dwAddress);
                bByte     |= action->bContent;
                writeb(bByte, dwAddress);
                action->bContent = readb(dwAddress);
                break;
            case VIC68A_AND:    
                bByte      = readb(dwAddress);
                bByte     &= action->bContent;
                writeb(bByte, dwAddress);
                action->bContent = readb(dwAddress);
                break;
            case VIC68A_READ:   
                action->bContent = readb(dwAddress);
                break;
            default:            
                nStatus = -EINVAL;
        }
    }
    else
        nStatus = -EINVAL;

    return nStatus;
}

// the dispatcher ----------------------------------------------------------------------------------
int pcivme_ioctl(struct inode *pInode, struct file *pFile, unsigned int cmd, unsigned long arg)
{
    PATH_OBJ   *pp = (PATH_OBJ *)pFile->private_data;
    DEVICE_OBJ *pd = pp->pDo;
    int err = 1;

    PRINTK(KERN_DEBUG "%s : pcivme_ioctl(0x%08x), size = %d\n", DEVICE_NAME, cmd, _IOC_SIZE(cmd));

    if (_IOC_TYPE(cmd) != PCIVME_MAGIC)
        return -EINVAL;

    // check for accessible user buffer
    if (_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
    if (_IOC_DIR(cmd) & _IOC_WRITE)
        err = !access_ok(VERIFY_READ,  (void *)arg, _IOC_SIZE(cmd));
    if (err)
        return -EFAULT;

    switch (_IOC_NR(cmd))
    {
        case _IOC_NR(PCIVME_READ_VECTOR_BLOCK):
            if (_IOC_SIZE(cmd) < sizeof(PCIVME_VECTOR_LEVEL))
                return -EINVAL;
            return read_vector_blocking(pp, pd, (PCIVME_VECTOR_LEVEL *)arg, pFile);

        case _IOC_NR(PCIVME_READ_VECTOR_POLL):
            if (_IOC_SIZE(cmd) < sizeof(PCIVME_VECTOR_LEVEL))
                return -EINVAL;
            return read_vector_polling(pp, pd, (PCIVME_VECTOR_LEVEL *)arg);

        case _IOC_NR(PCIVME_CONTROL_INTERRUPTS):
            if (_IOC_SIZE(cmd) < sizeof(PCIVME_IRQ_CONTROL))
                return -EINVAL;
            return control_interrupts(pp, pd, (PCIVME_IRQ_CONTROL *)arg);

        case _IOC_NR(PCIVME_TAS):
            if (_IOC_SIZE(cmd) < sizeof(PCIVME_TAS_STRUCT))
                return -EINVAL;
            return VME_TAS(pp, pd, (PCIVME_TAS_STRUCT *)arg);

        case _IOC_NR(PCIVME_ACCESS_VIC68A):
            if (_IOC_SIZE(cmd) < sizeof(PCIVME_VIC68A_ACTION))
                return -EINVAL;
            return access_VIC68A(pp, pd, (PCIVME_VIC68A_ACTION *)arg);

        case _IOC_NR(PCIVME_GET_DYNAMIC_STATUS):
            if (_IOC_SIZE(cmd) < sizeof(PCIVME_DYNAMIC_STATUS))
                return -EINVAL;
            return get_dynamic_status(pp, pd, (PCIVME_DYNAMIC_STATUS *)arg);

        case _IOC_NR(PCIVME_RESET):
            if (_IOC_SIZE(cmd) < sizeof(PCIVME_RESET_COMMAND))
                return -EINVAL;
            return VMEMM_RESET(pp, pd, (PCIVME_RESET_COMMAND *)arg);

        case _IOC_NR(PCIVME_SET_ACCESS_PARA):
            if (_IOC_SIZE(cmd) < sizeof(PCIVME_ACCESS_COMMAND))
                return -EINVAL;
            return access_command(pp, pd, (PCIVME_ACCESS_COMMAND *)arg);

        case _IOC_NR(PCIVME_GET_STATIC_STATUS):
            if (_IOC_SIZE(cmd) < sizeof(PCIVME_STATIC_STATUS))
                return -EINVAL;
            return get_static_status(pp, pd, (PCIVME_STATIC_STATUS *)arg);

        case _IOC_NR(PCIVME_INIT_HARDWARE): 
            if (_IOC_SIZE(cmd) < sizeof(PCIVME_INIT_COMMAND))
                return -EINVAL;
            return init_hardware(pp, pd, (PCIVME_INIT_COMMAND *)arg);

        case _IOC_NR(PCIVME_DEINIT_HARDWARE): 
            if (_IOC_SIZE(cmd) < sizeof(PCIVME_INIT_COMMAND))
                 return -EINVAL;
            return deinit_hardware(pp, pd, (PCIVME_INIT_COMMAND *)arg);

        default: 
            PRINTK(KERN_DEBUG "%s : pcivme_ioctl(0x%08x) is illegal\n", DEVICE_NAME, cmd);
            return -EINVAL;
    } 

    return 0;
}

int pcivme_open(struct inode *pInode, struct file *pFile)
{
    DEVICE_OBJ *pd   = 0;
    DEVICE_OBJ *desc = 0;
    int nMinor       = MINOR(pInode->i_rdev);
    struct list_head *ptr;

    PRINTK(KERN_DEBUG "%s : pcivme_open(), %d, %d, scanning %d devices\n", DEVICE_NAME, MAJOR(pInode->i_rdev), nMinor, drv.count);

    /* search for device */
    for (ptr = drv.devList.next; ptr != &drv.devList; ptr = ptr->next)
    {
        pd = list_entry(ptr, DEVICE_OBJ, list);
        pd->bConnected =  get_module_info(pd);
        if (pd->bConnected)
        {
            if (test_connection(pd))
            {
                printk(KERN_ERR "%s : connection test for module %d failed!\n", DEVICE_NAME, pd->cModuleNumber);
                pd->bConnected = 0;
            }
            else
                if (pd->cModuleNumber == nMinor)
            {
                desc = pd;
                break;
            }
        }
        else
            PRINTK(KERN_DEBUG "%s : module %d not connected!\n", DEVICE_NAME, nMinor);
    }

    if (desc)
    {
        int       err;
        PATH_OBJ  *pp;

        pp = (PATH_OBJ *)kmalloc(sizeof(PATH_OBJ), GFP_ATOMIC);
        if (!pp)
            return -ENOMEM;

        // file PATH_OBJ structure with initialisation data		
        pp->pDo            = pd;
        pp->bAccessType    = pp->bIncrement = BYTE_ACCESS;  
        pp->bModifier      = Short_NoPriv;   
        pp->read           = readByte;
        pp->write          = writeByte;
        pp->AlignmentCheck = MisalignmentForByteAccess;
        pFile->private_data = (void *)pp;

        PRINTK(KERN_DEBUG "%s : found VMEMM module with number %d.\n", DEVICE_NAME, nMinor);

        if (!pd->nOpenCounter)
        {
            err = CmdMachine(pd, init_element);
            if (err)
            {
                printk(KERN_ERR "%s : default init failed with err = %d!\n", DEVICE_NAME, err); 
                kfree_s(pp, sizeof(*pp));    // FREE(pFile->private_data);
                return err;
            }
        }

        pd->nOpenCounter++;
    }
    else
    {
        printk(KERN_ERR "%s : No VMEMM module found.\n", DEVICE_NAME);
        return -ENODEV;
    }       

    MOD_INC_USE_COUNT;
    return 0;
}

int pcivme_release(struct inode *pInode, struct file *pFile)
{
    PATH_OBJ *pp;

    PRINTK(KERN_DEBUG "%s : release()\n", DEVICE_NAME);

    if (pFile->private_data)
    {
        pp = (PATH_OBJ *)pFile->private_data;
        if (pp && pp->pDo )
        {
            DEVICE_OBJ *pd = pp->pDo;

            pd->nOpenCounter--;

            // the last one closes the door
            if (pd->nOpenCounter <= 0)
            {
                CmdMachine(pd, deinit_element_pre);
                CmdMachine(pd, deinit_element_post);

                // Vorsicht ist die Mutter der Porzelankiste!
                pd->nOpenCounter = 0;
            }

            pp->pDo = 0;            
        }

        kfree_s(pp, sizeof(*pp));    // FREE(pFile->private_data);
    }

    MOD_DEC_USE_COUNT;
    return 0;
}

static ssize_t pcivme_read(struct file *pFile, char *pcBuffer, size_t count, loff_t *offp)
{
    PATH_OBJ *pp     = (PATH_OBJ *)pFile->private_data;
    DEVICE_OBJ *pd   = pp->pDo;
    u32 dwLocalCount = count;
    register u32 dwLocalPageAddress;
    u32 dwLocalAddressInPage;

    PRINTK(KERN_DEBUG "%s : pcivme_read(0x%08x, %d)\n", DEVICE_NAME, (u32)*offp, dwLocalCount);

    // inhibit misaligned accesses
    if (pp->AlignmentCheck(*offp))
        return -EFAULT;

    // check for free access to user buffer
    if (!access_ok(VERIFY_WRITE, pcBuffer, count))
        return -EFAULT;

    // do I still have the same modifier?
    if (pp->bModifier != pd->bCurrentModifier)
        setModifier(pd, pp->bModifier);

    while (count >= pp->bAccessType)
    {
        dwLocalPageAddress   = *offp & HI_ADDRESS_MASK;
        dwLocalAddressInPage = *offp & LO_ADDRESS_MASK; 

        // do I still work in the same page?
        if (dwLocalPageAddress != pd->dwCurrentPageAddress)
            setPageAddress(pd, dwLocalPageAddress);

        // standard access method
        pp->read(pd, (void **)&pcBuffer, dwLocalAddressInPage);  

        // decrement count and update pointer to next access address
        count -= pp->bAccessType;
        *offp += pp->bIncrement;
    }

    return dwLocalCount - count;
}

static ssize_t pcivme_write(struct file *pFile, const char *pcBuffer, size_t count, loff_t *offp)
{
    PATH_OBJ *pp     = (PATH_OBJ *)pFile->private_data;
    DEVICE_OBJ *pd   = pp->pDo;
    u32 dwLocalCount = count;
    register u32 dwLocalPageAddress;
    u32 dwLocalAddressInPage;

    PRINTK(KERN_DEBUG "%s : pcivme_write(0x%08x, %d)\n", DEVICE_NAME, (u32)*offp, dwLocalCount);

    // inhibit misaligned accesses
    if (pp->AlignmentCheck(*offp))
        return -EFAULT;

    // check for free access to user buffer
    if (!access_ok(VERIFY_READ, pcBuffer, count))
        return -EFAULT;

    // do I still have the same modifier?
    if (pp->bModifier != pd->bCurrentModifier)
        setModifier(pd, pp->bModifier);

    while (count >= pp->bAccessType)
    {
        dwLocalPageAddress   = *offp & HI_ADDRESS_MASK;
        dwLocalAddressInPage = *offp & LO_ADDRESS_MASK; 

        // do I still work in the same page?
        if (dwLocalPageAddress != pd->dwCurrentPageAddress)
            setPageAddress(pd, dwLocalPageAddress);

        // standard access method
        pp->write(pd, dwLocalAddressInPage, (void **)&pcBuffer); 

        // decrement count and update pointer to next access address
        count -= pp->bAccessType;
        *offp += pp->bIncrement;
    }

    return dwLocalCount - count;
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0)
struct file_operations pcivme_fops =
{
    NULL,             /* lseek */
    pcivme_read,      /* read  */
    pcivme_write,     /* write */
    NULL,             /* readdir */
    NULL,             /* select */
    pcivme_ioctl,     /* ioctl */
    NULL,             /* mmap */
    pcivme_open,      /* open */
    NULL,             /* flush */
    pcivme_release,   /* release */ 
};
#else
struct file_operations pcivme_fops =
{
    read:    pcivme_read,     /* read  */
    write:   pcivme_write,    /* write */
    ioctl:   pcivme_ioctl,    /* ioctl */
    open:    pcivme_open,     /* open */
    release: pcivme_release,  /* release */ 
};
#endif


                                                                                                                             
                                                                                                                             
                                                                                                                             
