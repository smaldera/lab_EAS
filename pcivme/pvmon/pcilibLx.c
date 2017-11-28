//-------------------------------------------------------------------------------------------
// pcilib.c - interface hardware dependend functions to interface to the pvmon
//
// (c) 1999-2002 ARW Elektronik
//
// this source code is published under GPL (Open Source). You can use, redistrubute and 
// modify it unless this header  is  not modified or deleted. No warranty is given that 
// this software will work like expected.
// This product is not authorized for use as critical component in life support systems
// wihout the express written approval of ARW Elektronik Germany.
//
// Please announce changes and hints to ARW Elektronik
// 
// $Log: pcilibLx.c,v $
// Revision 1.4  2002/10/20 18:07:48  klaus
// mostly working alpha version
//
// Revision 1.3  2002/10/20 11:49:33  klaus
// first parts working
//
// Revision 1.2  2002/10/19 09:44:36  klaus
// first success compiling project
//
// Revision 1.1.1.1  2002/10/18 22:14:29  klaus
//

//-------------------------------------------------------------------------------------------
// INCLUDES
//
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include <../driver/pcivme.h>
#include <../driver/vic.h>
#include <../driver/vme.h>
#include <../lib/pcivme_ni.h>
#include <pcilibLx.h>
#include <mbuffer.h>

//-------------------------------------------------------------------------------------------
// DEFINES
//

//-------------------------------------------------------------------------------------------
// TYPEDEFS
//

//-------------------------------------------------------------------------------------------
// GLOBALS
//
static int  storedError    = 0;
static int  nLocalWordMode = 0;   // initial 32 bit mode

//--------------------------------------------------------------------------
// LOCALS
// 

//-------------------------------------------------------------------------------------------
// FUNCTIONS
//

//-------------------------------------------------------------------------------------------
// set word mode
//
int setWordMode(int nMode)
{
    nLocalWordMode = nMode;

    return nLocalWordMode;
}

//-------------------------------------------------------------------------------------------
// set interface special registers - VIC68A
//
unsigned long _SetRegister(int nIfcHandle, unsigned long Address, unsigned long Value)
{
    __u8 ubContent = (__u8)Value;

    VMEaccessVIC(nIfcHandle, VIC68A_WRITE, Address, &ubContent);

    return ubContent;
}


//-------------------------------------------------------------------------------------------
// function to get special interface registers - VIC68A
//
unsigned long _GetRegister(int nIfcHandle, unsigned long Address)
{
    __u8 ubContent;

    VMEaccessVIC(nIfcHandle, VIC68A_READ, Address, &ubContent);

    return ubContent;
}


//-------------------------------------------------------------------------------------------
// init the interface with the current properties
//
int Init_Interface(char *szDevicePath, char AdrMode, int *nIfcHandle)
{
    return  VMEopen(szDevicePath, AdrMode, nIfcHandle);
}


//-------------------------------------------------------------------------------------------
// deinit the current interface
//
void  DeInit_Interface(int nIfcHandle)
{
    VMEclose(nIfcHandle);
}


//-------------------------------------------------------------------------------------------
// check if an error occured
//
int GetError(int nIfcHandle)
{
    int error;

    if ((error = GetLastError(nIfcHandle)))
        storedError = error;
    else
        storedError = VMEerror(nIfcHandle);

    return storedError;
}


//-------------------------------------------------------------------------------------------
// clear a pending error
//
void ClearError(int nIfcHandle)
{
    storedError = 0;
}

//-------------------------------------------------------------------------------------------
// read elements and split a long access into 2 word accesses if word data path
//
char  ReadByte(int nIfcHandle, unsigned long adr, unsigned short modifier)
{
    __u8 c;

    setAccessProperties(nIfcHandle, (__u8)modifier, BYTE_ACCESS);

    storedError = VMEread(nIfcHandle, adr, BYTE_ACCESS, 1, &c);
    
    return c;
}

short ReadWord(int nIfcHandle, unsigned long adr, unsigned short modifier)
{
    __u16 w;

    setAccessProperties(nIfcHandle, (__u8)modifier, WORD_ACCESS);

    storedError = VMEread(nIfcHandle, adr, WORD_ACCESS, 1, &w);
    
    return w;
}

long  ReadLong(int nIfcHandle, unsigned long adr, unsigned short modifier)
{
    __u32 l;
 
    if (nLocalWordMode)
    {
        __u16 partl, parth;

        // lese high anteil von adresse +0
        parth = ReadWord(nIfcHandle, adr, modifier);

        // lese low anteil von adresse +2
        partl = ReadWord(nIfcHandle, adr + sizeof(__u16), modifier);

        l = (parth << 16) | partl;
    }
    else
    {
        setAccessProperties(nIfcHandle, (__u8)modifier, LONG_ACCESS);
    
        storedError = VMEread(nIfcHandle, adr, LONG_ACCESS, 1, &l);
    }
    
    return l;
}

//-------------------------------------------------------------------------------------------
// write a byte/word/long and split a long access into 2 word accesses if word data path
//
void  WriteByte(int nIfcHandle, unsigned long adr, char value, unsigned short modifier)
{
    setAccessProperties(nIfcHandle, (__u8)modifier, BYTE_ACCESS);

    storedError = VMEwrite(nIfcHandle, adr, BYTE_ACCESS, 1, &value);
}

void  WriteWord(int nIfcHandle, unsigned long adr, short value, unsigned short modifier)
{
    setAccessProperties(nIfcHandle, (__u8)modifier, WORD_ACCESS);

    storedError = VMEwrite(nIfcHandle, adr, WORD_ACCESS, 1, &value);
}

void  WriteLong(int nIfcHandle, unsigned long adr, long value, unsigned short modifier)
{
    if (nLocalWordMode)
    {
        __u16 part;

        // high anteil auf adresse +0
        part = (value >> 16) & 0xffff;
        WriteWord(nIfcHandle, adr, part, modifier);

        // low anteil auf adresse +2
        part = value & 0xffff;
        WriteWord(nIfcHandle, adr + sizeof(__u16), part, modifier);
    }
    else
    {
        setAccessProperties(nIfcHandle, (__u8)modifier, LONG_ACCESS);
    
        storedError = VMEwrite(nIfcHandle, adr, LONG_ACCESS, 1, &value);
    }
}

//-------------------------------------------------------------------------------------------
// check for a pending SYSFAIL 
//
unsigned short PollSfail(int nIfcHandle)
{
    BOOLEAN bResult;

    VMEsysfailGet(nIfcHandle, &bResult);

    return (unsigned short)bResult;
}

//-------------------------------------------------------------------------------------------
// clear a interface set SYSFAIL
//
void  ClrSfail(int nIfcHandle)
{
    VMEsysfailSet(nIfcHandle, 0);
}

//-------------------------------------------------------------------------------------------
// set a SYSFAIL
//
void  SetSfail(int nIfcHandle)
{
    VMEsysfailSet(nIfcHandle, 1);
}

//-------------------------------------------------------------------------------------------
// read a interrupt vector as byte/word/long (if supported)
//
char  ReadVectorByte(int nIfcHandle)
{
    __u8 ubVector;

    VMEinterrupt(nIfcHandle, &ubVector);

    return ubVector;
}

short ReadVectorWord(int nIfcHandle)
{
    printf("Word read of a vector not available!\n"); 
    return 0;
}

long  ReadVectorLong(int nIfcHandle)
{
    printf("Longword read of a vector not available!\n");
    return 0;
}

//-------------------------------------------------------------------------------------------
// emulate a 68K TAS (read/modify/write) instruction
//
char  TAS(int nIfcHandle, unsigned long adr, unsigned short modifier)
{
    __u8 ubResult = 0x80;

    VMETAS(nIfcHandle, adr, &ubResult);

    return ubResult;  // check if a read reads a lock
}

//-------------------------------------------------------------------------------------------
// generate a SYSRESET on the vmebus and re-init the interface
//
void  Reset_VME(int nIfcHandle)
{
    VMEreset(nIfcHandle);
}


//-------------------------------------------------------------------------------------------
// print out some interface special info
//
void GetInterfaceInfo(int nIfcHandle, char type)
{
    switch (type)
    {
    }
}

//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
