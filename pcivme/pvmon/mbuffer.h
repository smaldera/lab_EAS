#ifndef __MBUFFER_H__
#define __MBUFFER_H__

//-------------------------------------------------------------------------------------------
// mbuffer.h - prototypes for simple message buffering mechanisms
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
// $Log: mbuffer.h,v $
// Revision 1.2  2002/10/19 09:47:30  klaus
// first success compiling project
//
// Revision 1.1.1.1  2002/10/18 22:14:29  klaus
//

void AddIRQtoBuffer(short level, short vector);
void AddMsgtoBuffer(int where, unsigned long Error);
void AddMsgAsStringtoBuffer(char *strn);
char *ReadMessageBuffer(void);
void InitMessageBuffer(void);

#endif /* __MBUFFER_H__ */
