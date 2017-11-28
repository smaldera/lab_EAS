//-------------------------------------------------------------------------------------------
// pcivme_ni.c - shared library for ARW pcivme interface (libpcivme.so)
//
// Copyright (C) 2002 ARW Elektronik Germany
//
// this source code is published under GPL (Open Source). You can use, redistrubute and 
// modify it unless this header  is  not modified or deleted. No warranty is given that 
// this software will work like expected.
// This product is not authorized for use as critical component in life support systems
// wihout the express written approval of ARW Elektronik Germany.
//
// Please announce changes and hints to ARW Elektronik
// 
// $Log: pcivme_ni.c,v $
// Revision 1.7  2002/10/20 18:07:18  klaus
// changed error handling
//
// Revision 1.6  2002/10/18 21:56:28  klaus
// completed functional features, untested
//
// Revision 1.5  2002/10/18 21:56:28  klaus
// completed functional features, untested
//
// Revision 1.4  2002/10/17 21:16:03  klaus
// filled function bodies
//
// Revision 1.3  2002/10/17 21:16:03  klaus
// filled function bodies
//
// Revision 1.2  2002/10/17 19:05:03  klaus
// VME access is working through test to lib to driver
//
// Revision 1.1  2002/10/12 22:04:30  klaus
// first work done
//

//-------------------------------------------------------------------------------------------
// INCLUDES
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <../driver/pcivme.h>
#include <../driver/vic.h>
#include <pcivme_ni.h>

//-------------------------------------------------------------------------------------------
// DEFINES
//
#define LOCAL_STRING_LEN 40

//-------------------------------------------------------------------------------------------
// TYPEDEFS
//

// storage for path specific data
typedef struct
{
    int nFileNo;              // file number to f
    __u8 cAddressModifier;    // associated VME address modifier
    __u8 cAccessWidth;        // current access width
    int  nLastError;          // != 0 if a previous error occurred
} VMEMM_DEVICE;

//-------------------------------------------------------------------------------------------
// FUNCTIONS
//

// construct a device file name
static char *szDeviceName(const char *cszBaseName, int nVMEMM)
{
  static char path[LOCAL_STRING_LEN];
  int i = LOCAL_STRING_LEN - 1;

  path[0] = 0;

  memset(path, 0, LOCAL_STRING_LEN);

  if (strlen(cszBaseName) >= (LOCAL_STRING_LEN - 3))
      return "";

  if (nVMEMM > 15)
      return "";

  strncpy(path, cszBaseName, LOCAL_STRING_LEN - 3);

  while ((i--) && (path[i] != '_'));  // search for '_'
 
  if (i)
  {
      i++; // go after '_'
      if (nVMEMM >= 10)
      {
          path[i] = '1';
          nVMEMM -= 10;
          i++;
      }
      path[i] = '0' + nVMEMM;
      i++;
      path[i] = 0; // trailing 0
  }
  else
      return "";

  return path;
}

static int initHardware(VMEMM_DEVICE *dev)
{
    PCIVME_INIT_COMMAND init;

    init.sVie[0].bDestination = STOP;
    init.sVie[0].bAccessType  =
    init.sVie[0].dwValue      = 
    init.sVie[0].wOffset      = 0;

    if (ioctl(dev->nFileNo, PCIVME_INIT_HARDWARE, &init) < 0)
    {
        dev->nLastError = errno;
        return errno;
    }

    return 0;
}

static int deInitHardware(VMEMM_DEVICE *dev)
{
    PCIVME_INIT_COMMAND deinit;

    deinit.sVie[0].bDestination = STOP;
    deinit.sVie[0].bAccessType  =
    deinit.sVie[0].dwValue      = 
    deinit.sVie[0].wOffset      = 0;

    if (ioctl(dev->nFileNo, PCIVME_DEINIT_HARDWARE, &deinit) < 0)
    {
        dev->nLastError = errno;
        return errno;
    }

    return 0;
}

int VMEopen(const char *cszDeviceName, unsigned char ubAddressModifier, int *pnHandle)
{
    VMEMM_DEVICE *dev = (VMEMM_DEVICE *)NULL;
    int error;

    *pnHandle = 0;

    dev = (VMEMM_DEVICE *)malloc(sizeof(*dev));
    if (!dev)
        return errno;

    dev->nFileNo = open(cszDeviceName, O_RDWR);

    if (dev->nFileNo == -1)
    {
        error = errno;
        free(dev);
        return error;
    }

    dev->cAddressModifier = ubAddressModifier;
    *pnHandle             = (int)dev;

    error = initHardware(dev);
    if (error)
        return error;

    dev->nLastError = 0;

    return setAccessProperties(*pnHandle, dev->cAddressModifier, BYTE_ACCESS); // set access properties to default
}

int VMEinit(const char *cszDeviceName, unsigned short nVMEMM, unsigned char ubAddressModifier, int *pnHandle)
{
    char *szLocalDeviceName = szDeviceName(cszDeviceName, nVMEMM);

    return VMEopen(szLocalDeviceName, ubAddressModifier, pnHandle);
}

int setAccessProperties(int nHandle, unsigned char bModifier, unsigned char bAccessType)
{
    VMEMM_DEVICE *dev = (VMEMM_DEVICE *)nHandle;
    PCIVME_ACCESS_COMMAND access_command;

    access_command.bAccessType = 
    access_command.bIncrement  = bAccessType;  // increment and accessType are the same
    access_command.bModifier   = bModifier;

    if (ioctl(dev->nFileNo, PCIVME_SET_ACCESS_PARA, &access_command) < 0)
    {
        dev->nLastError = errno;
        return errno;
    }

    dev->cAccessWidth     = bAccessType;
    dev->cAddressModifier = bModifier;

    return 0;
}

int VMEread(int nHandle, unsigned long ulAddress, unsigned char ubAccessWidth, unsigned long ulElementCount, void *pvBuffer)
{
    VMEMM_DEVICE *dev = (VMEMM_DEVICE *)nHandle;
    size_t count      = (size_t)(ulElementCount * ubAccessWidth);
    ssize_t result;
    int error;

    if (dev->cAccessWidth != ubAccessWidth)
    {
        if ((error = setAccessProperties(nHandle, dev->cAddressModifier, ubAccessWidth)))
            return error;
    }

    if (lseek(dev->nFileNo, ulAddress, SEEK_SET) < 0)
        return errno;

    result = read(dev->nFileNo, pvBuffer, count);

    if (result != count)
    {
        if (result < 0)
        {
            dev->nLastError = errno;
            return errno;
        }
        else
            return EFAULT;
    }

    return 0;
}

int VMEwrite(int nHandle, unsigned long ulAddress, unsigned char ubAccessWidth, unsigned long ulElementCount, void *pvBuffer)
{
    VMEMM_DEVICE *dev = (VMEMM_DEVICE *)nHandle;
    size_t count      = (size_t)(ulElementCount * ubAccessWidth);
    ssize_t result;
    int error;

    if (dev->cAccessWidth != ubAccessWidth)
    {
        if ((error = setAccessProperties(nHandle, dev->cAddressModifier, ubAccessWidth)))
            return error;
    }

    if (lseek(dev->nFileNo, ulAddress, SEEK_SET) < 0)
        return errno;

    result = write(dev->nFileNo, pvBuffer, count);

    if (result != count)
    {
        if (result < 0)
        {
            dev->nLastError = errno;
            return errno;
        }
        else
            return EFAULT;
    }

    return 0;
}

int VMEaccessVIC(int nHandle, unsigned char ubAccessMode, unsigned short uwAddress, unsigned char *ubContent)
{
    VMEMM_DEVICE *dev = (VMEMM_DEVICE *)nHandle;
    PCIVME_VIC68A_ACTION vic68a_action;

    vic68a_action.bAccessMode      = ubAccessMode;
    vic68a_action.bContent         = *ubContent;  
    vic68a_action.wRegisterAddress = uwAddress;

    if (ioctl(dev->nFileNo, PCIVME_ACCESS_VIC68A, &vic68a_action) < 0)
    {
        dev->nLastError = errno;
        return errno;
    }

    *ubContent = vic68a_action.bContent;

    return 0;
}

int VMEreset(int nHandle)
{
    VMEMM_DEVICE *dev = (VMEMM_DEVICE *)nHandle;
    PCIVME_RESET_COMMAND reset_command;
    int i = 10;

    reset_command.bCommand = GLOBAL_RESET_CMD;
    reset_command.bResult  = 0xff;  

    if (ioctl(dev->nFileNo, PCIVME_RESET, &reset_command) < 0)
    {
        dev->nLastError = errno;
        return errno;
    }

    do
    {
        usleep(100);
        reset_command.bCommand = POLL_RESET_CMD;
        reset_command.bResult  = 0xff;  
    
        if (ioctl(dev->nFileNo, PCIVME_RESET, &reset_command) < 0)
        {
            dev->nLastError = errno;
            return errno;
        }
    } while ((reset_command.bResult) && (i--));

    if (!i)
        return ETIME;

    dev->nLastError = 0;

    return 0;
}

int VMETAS(int nHandle, unsigned long ulAddress, unsigned char *ubResult)
{
    VMEMM_DEVICE *dev = (VMEMM_DEVICE *)nHandle;
    PCIVME_TAS_STRUCT tas;

    tas.bContent  = *ubResult;
    tas.bModifier = dev->cAddressModifier;  
    tas.dwAddress = ulAddress;

    if (ioctl(dev->nFileNo, PCIVME_TAS, &tas) < 0)
    {
        dev->nLastError = errno;
        return errno;
    }

    *ubResult = tas.bContent;

    return 0;
}

int VMEinterrupt(int nHandle, unsigned char *ubVector)
{
    VMEMM_DEVICE *dev = (VMEMM_DEVICE *)nHandle;
	PCIVME_VECTOR_LEVEL ubLocalVector; 

    if (ioctl(dev->nFileNo, PCIVME_READ_VECTOR_POLL, &ubLocalVector) < 0)
    {
        dev->nLastError = errno;
        return errno;
    }

    *ubVector = (__u8)ubLocalVector.dwStatusID;

    return 0;
}

int VMEsysfailGet(int nHandle, BOOLEAN *bResult)
{
    VMEMM_DEVICE *dev = (VMEMM_DEVICE *)nHandle;
	PCIVME_VIC68A_ACTION sAction;    // structure to access vic chip

	sAction.wRegisterAddress = EGICR;
	sAction.bAccessMode      = VIC68A_READ;
	sAction.bContent         = 0;

    if (ioctl(dev->nFileNo, PCIVME_ACCESS_VIC68A, &sAction) < 0)
    {
        dev->nLastError = errno;
        return errno;
    }

	*bResult = (sAction.bContent & 0x08) ? FALSE : TRUE;

    return 0;
}

int VMEsysfailSet(int nHandle, BOOLEAN bForce)
{
    VMEMM_DEVICE *dev = (VMEMM_DEVICE *)nHandle;
	PCIVME_VIC68A_ACTION sAction;    // structure to access vic chip

	sAction.wRegisterAddress = ICR7;
	sAction.bAccessMode      = (bForce == TRUE) ? VIC68A_AND : VIC68A_OR;
	sAction.bContent         = (bForce == TRUE) ? 0x3F		 : 0x80;

    if (ioctl(dev->nFileNo, PCIVME_ACCESS_VIC68A, &sAction) < 0)
    {
        dev->nLastError = errno;
        return errno;
    }

    return 0;
}

int VMEerror(int nHandle)
{
    __u8 ubVector;

    VMEinterrupt(nHandle, &ubVector);

    if (ubVector == 7)
        return EFAULT;  // it's a bus error
    else
        return 0;
}

int VMEclose(int nHandle)
{
    int error = 0;
    VMEMM_DEVICE *dev = (VMEMM_DEVICE *)nHandle;

    if (dev != (VMEMM_DEVICE *)NULL)
    {
        deInitHardware(dev);

        if (dev->nFileNo != -1)
            close(dev->nFileNo);
        else
            error = -EINVAL;

        free(dev);
    }

    return error;
}

int VMEcontrolInterrupt(int nHandle, BOOLEAN *bEnable)
{
    VMEMM_DEVICE *dev = (VMEMM_DEVICE *)nHandle;
	PCIVME_IRQ_CONTROL  control; 

    control.bEnable = *bEnable;

    if (ioctl(dev->nFileNo, PCIVME_CONTROL_INTERRUPTS, &control) < 0)
    {
        dev->nLastError = errno;
        return errno;
    }

    // status of interrupt enable before set
    *bEnable = control.bEnable;

    return 0;
}

int GetLastError(int nHandle)
{
    VMEMM_DEVICE *dev  = (VMEMM_DEVICE *)nHandle;
    int nLocalError;

    nLocalError = dev->nLastError;
    dev->nLastError = 0;

    return nLocalError;
}


