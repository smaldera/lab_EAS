#ifndef __PCIVME_NI_H__
#define __PCIVME_NI_H__

//-------------------------------------------------------------------------------------------
// pcivme_ni.h - header for ni-labview dll for ARW pcivme interface
//
// this source code is published under GPL (Open Source). You can use, redistrubute and 
// modify it unless this header  is  not modified or deleted. No warranty is given that 
// this software will work like expected.
// This product is not authorized for use as critical component in life support systems
// wihout the express written approval of ARW Elektronik Germany.
//
// Please announce changes and hints to ARW Elektronik
// 
// $Log: pcivme_ni.h,v $
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
// Revision 1.1  2002/10/12 22:04:44  klaus
// first work done
//
//
// what                                                              who    when
// first steps                                                       AR     17.11.1999
// VMEerror new														 AR     07.01.2000
// made LINUX shared library from windows template                   AR     12.10.2002
//

//-------------------------------------------------------------------------------------------
// INCLUDES
//
#define BOOLEAN int
#if !defined(TRUE) && !defined(FALSE)
    #define FALSE 0
    #define TRUE 1
#endif

//-------------------------------------------------------------------------------------------
// PROTOTYPES
//
#ifdef __cplusplus
extern "C"
{
#endif
int VMEopen(const char *cszDeviceName, unsigned char ubAddressModifier, int *pnHandle);
int VMEinit(const char *cszDeviceName, unsigned short nVMEMM, unsigned char ubAddressModifier, int *pnHandle);
int setAccessProperties(int nHandle, unsigned char bModifier, unsigned char bAccessType);
int VMEread(int nHandle, unsigned long ulAddress, unsigned char ubAccessWidth, unsigned long ulElementCount, void *pvBuffer);
int VMEwrite(int nHandle, unsigned long ulAddress, unsigned char ubAccessWidth, unsigned long ulElementCount, void *pvBuffer);
int VMEaccessVIC(int nHandle, unsigned char ubAccessMode, unsigned short uwAddress, unsigned char *ubContent);
int VMEreset(int nHandle);
int VMETAS(int nHandle, unsigned long ulAddress, unsigned char *ubResult);
int VMEcontrolInterrupt(int nHandle, BOOLEAN *bEnable);
int VMEinterrupt(int nHandle, unsigned char *ubVector);
int VMEsysfailGet(int nHandle, BOOLEAN *bResult);
int VMEsysfailSet(int nHandle, BOOLEAN bForce);
int VMEerror(int nHandle);
int VMEclose(int nHandle);
int GetLastError(int nHandle);
#ifdef __cplusplus
}
#endif

#endif /* __PCIVME_NI_H__ */

