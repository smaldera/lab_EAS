#ifndef __VIC_H__
#define __VIC_H__

//****************************************************************************
// Copyright (C) 2001,2002  ARW Elktronik Germany
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
//****************************************************************************

//****************************************************************************
//
// vic.h - all definitions about the VIC68A chip
//
// $Log: vic.h,v $
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

//****************************************************************************
// INCLUDES
#include <linux/types.h>

//****************************************************************************
// DEFINES
#ifndef __KERNEL__
    #define u8  __u8
    #define u16 __u16
    #define u32 __u32
#endif

#define VICR1   (u16)0x07      /* VMEbus Interrupt Control Register #..  */
#define VICR2   (u16)0x0b
#define VICR3   (u16)0x0f   
#define VICR4   (u16)0x13
#define VICR5   (u16)0x17
#define VICR6   (u16)0x1b
#define VICR7   (u16)0x1f

#define LICR1   (u16)0x27      /* Local interrupt control register ..     */
#define LICR2   (u16)0x2b
#define LICR3   (u16)0x2f  
#define LICR4   (u16)0x33
#define LICR5   (u16)0x37
#define LICR6   (u16)0x3b
#define LICR7   (u16)0x3f
#define LIVBR   (u16)0x57     /* Local interrupt vector base register     */

#define ICGSICR (u16)0x43     /* ICGS interrupt control register          */
#define ICGSVBR (u16)0x4f     /* ICGS vector base register                */

#define ICMSICR (u16)0x47     /* ICMS interrupt control register          */
#define ICMSVBR (u16)0x53     /* ICMS vector base register                */

#define EGICR   (u16)0x4b     /* Error group interrupt control register   */
#define EGIVBR  (u16)0x5b     /* Error group interrupt vector base rg     */

#define ICSR    (u16)0x5f     /* Interprozessor communication switch rg   */
#define ICR0    (u16)0x63
#define ICR1    (u16)0x67
#define ICR2    (u16)0x6b
#define ICR3    (u16)0x6f
#define ICR4    (u16)0x73
#define ICR5    (u16)0x77
#define ICR6    (u16)0x7b 
#define ICR7    (u16)0x7f

#define VIICR   (u16)0x03     /* VMEbus Interrupter Interrupt Control   */
#define VIRSR   (u16)0x83     /* VMEbus interrupt request status reg    */
#define VIVR1   (u16)0x87     /* VMEbus interrupt vector register ..    */
#define VIVR2   (u16)0x8b
#define VIVR3   (u16)0x8f
#define VIVR4   (u16)0x93
#define VIVR5   (u16)0x97
#define VIVR6   (u16)0x9b
#define VIVR7   (u16)0x9f

#define TTR     (u16)0xa3     /* transfer timeout register               */
#define LTR     (u16)0xa7     /* local timing register                   */
#define ICR     (u16)0xaf     /* interface configuration register        */

#define ARCR    (u16)0xb3     /* arbiter/requester configuration register*/
#define AMSR    (u16)0xb7     /* address modifier source register        */
#define BESR    (u16)0xbb     /* bus error source register               */

#define DSICR   (u16)0x23     /* DMA status interrupt control register   */
#define DSR     (u16)0xbf     /* DMA status register                     */

#define SSCR00  (u16)0xc3     /* slave select 0 control register 0       */
#define SSCR01  (u16)0xc7     /* slave select 0 control register 1       */
#define SSCR10  (u16)0xcb     /* slave select 1 control register 0       */
#define SSCR11  (u16)0xcf     /* slave select 1 control register 1       */

#define RCR     (u16)0xd3     /* release control register                */

#define BTDR    (u16)0xab     /* block transfer definition register      */
#define BTCR    (u16)0xd7     /* block transfer control register         */
#define BTLR0   (u16)0xdb     /* block transfer length register 0        */
#define BTLR1   (u16)0xdf     /* block transfer length register 1        */

#define SRR     (u16)0xe3     /* system reset register                   */


#endif // __VIC_H__


