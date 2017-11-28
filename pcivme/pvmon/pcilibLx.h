#ifndef __PCILIBLX_H__
#define __PCILIBLX_H__

//-------------------------------------------------------------------------------------------
// pcilib.h - defaults and interface functions of pcilib.c for LINUX
//
// (c) 1999-2002 ARW Elektronik
//
// this source code is published under GPL (Open Source). You can use, redistrubute and 
// modify it unless this header   is not modified or deleted. No warranty is given that 
// this software will work like expected.
// This product is not authorized for use as critical component in life support systems
// wihout the express written approval of ARW Elektronik Germany.
//
// Please announce changes and hints to ARW Elektronik
// 
// $Log: pcilibLx.h,v $
// Revision 1.3  2002/10/20 11:49:33  klaus
// first parts working
//
// Revision 1.2  2002/10/19 09:44:37  klaus
// first success compiling project
//
// Revision 1.1.1.1  2002/10/18 22:14:29  klaus
//

//-----------------------------------------------------------------------------
// DEFINES
//
#define BUFFERLENGTH 128

//-----------------------------------------------------------------------------
// PROTOTYPES
//
int   Init_Interface(char *szDevicePath, char AdrMode, int *nIfcHandle); /* Inits to DefAModifier */
void  DeInit_Interface(int nIfcHandle);         /* de-initializes Interface */
void  Reset_VME(int nIfcHandle);                               /* generates SYSRESET on VMEbus */

int   GetError(int nIfcHandle);                                /* checks the ERROR flag */
void  ClearError(int nIfcHandle);                              /* clears the ERROR flag */

char  ReadByte(int nIfcHandle, unsigned long,unsigned short);        /* Get byte from any address */
void  WriteByte(int nIfcHandle, unsigned long,char,unsigned short);  /* write byte to any address */
short ReadWord(int nIfcHandle, unsigned long,unsigned short);        /* get word from any address */
void  WriteWord(int nIfcHandle, unsigned long, short,unsigned short);/* write word to any address */
long  ReadLong(int nIfcHandle, unsigned long,unsigned short);        /* read longword from any address */
void  WriteLong(int nIfcHandle, unsigned long,long,unsigned short);  /* write longword to any address */

char  ReadVectorByte(int nIfcHandle);                          /* reads a vector byte from VME_LEVEL */
short ReadVectorWord(int nIfcHandle);                          /* reads a vector word from VME_LEVEL */
long  ReadVectorLong(int nIfcHandle);                          /* reads a vector longword from ..    */

char  TAS(int nIfcHandle, unsigned long,unsigned short);             /* 68K TAS (semafore) emulation */
void  SetSfail(int nIfcHandle);                                /* set SYSFAIL */
void  ClrSfail(int nIfcHandle);                                /* clear SYSFAIL (own) */
unsigned short PollSfail(int nIfcHandle);                      /* get SYSFAIL status */

void GetInterfaceInfo(int nIfcHandle, char type);                    /* request some information from driver */

/* set and get register contents of ..  */
unsigned long _SetRegister(int nIfcHandle, unsigned long Address, unsigned long Value);
unsigned long _GetRegister(int nIfcHandle, unsigned long Address);

int setWordMode(int nMode);

/*-------------------------- End of Prototypes -------------------------------*/

#endif // __PCILIBLX_H__
