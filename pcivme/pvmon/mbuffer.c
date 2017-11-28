//-------------------------------------------------------------------------------------------
// mbuffer.c - some functions to do a simple message buffering
//
// (c) 1999 ARW Elektronik
//
// this source code is published under GPL (Open Source). You can use, redistrubute and 
// modify it unless this header  is  not modified or deleted. No warranty is given that 
// this software will work like expected.
// This product is not authorized for use as critical component in life support systems
// wihout the express written approval of ARW Elektronik Germany.
//
// Please announce changes and hints to ARW Elektronik
// 
// $Log: mbuffer.c,v $
// Revision 1.3  2002/10/20 11:49:33  klaus
// first parts working
//
// Revision 1.2  2002/10/19 09:47:30  klaus
// first success compiling project
//
// Revision 1.1.1.1  2002/10/18 22:14:29  klaus
//

//-------------------------------------------------------------------------------------------
// DEFINES
//
#define LOCAL_BUFFERLENGTH 250

//-------------------------------------------------------------------------------------------
// INCLUDES
//
#include <stdio.h>
#include <mbuffer.h>

//-------------------------------------------------------------------------------------------
// LOCALS
//
static char MBuffer[LOCAL_BUFFERLENGTH];
 
//------------------------------------------------------------
// add unsolicited interrupt message to buffer
//
void AddIRQtoBuffer(short level, short vector)
{
  sprintf(MBuffer, "Interrupt @ level %d with vector %d signaled.", level, vector);
}

//------------------------------------------------------------
// add unsolicited error message to buffer
//
void AddMsgtoBuffer(int where, unsigned long Error)
{
  sprintf(MBuffer, "Error %d occured (%d)!", Error, where);  
}

//------------------------------------------------------------
// add unsolicited error message as string to buffer
//
void AddMsgAsStringtoBuffer(char *strn)
{
  sprintf(MBuffer,"%s", strn);  
}

//------------------------------------------------------------
// get back a message from the buffer
//
char *ReadMessageBuffer(void)
{
  return MBuffer;
}

//------------------------------------------------------------
// initilaize the buffer
//
void InitMessageBuffer(void)
{
  MBuffer[0] = 0;
}

//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
