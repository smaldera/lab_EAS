//****************************************************************************
// Copyright (C) 2000-2002  ARW Elektronik Germany
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
// simpleTest.c -- a simple test program for the PCIVME PCI to VME Interface
//
// $Log: simpleTest.c,v $
// Revision 1.3  2002/10/17 19:05:03  klaus
// VME access is working through test to lib to driver
//
// Revision 1.2  2002/10/12 22:12:19  klaus
// simple change
//
// Revision 1.1.1.1  2002/10/09 19:36:29  klaus
// initial import
//
//
//****************************************************************************

/*--- INCLUDES -----------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>  // rand()
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <errno.h>
#include <ctype.h>

#include <../driver/pcivme.h>
#include <../lib/pcivme_ni.h>

/*--- DEFINES ------------------------------------------------------------------------------------*/
#define DEVICE_NAME   "/dev/vmemm_1"
#define DEFAULT_WIDTH BYTE_ACCESS
#define DEFAULT_MODIFIER 0x39 
#define BUFFER_SIZE   0x10000            // local buffer for temporary use

/*--- TYPEDEFS -----------------------------------------------------------------------------------*/

/*--- GLOBALS ------------------------------------------------------------------------------------*/
char *cszPrgName;

/*--- FUNCTIONS ----------------------------------------------------------------------------------*/
void hlpMsg(void)
{
    printf("simpleTest - a program to do a RAM test with help of the PCIVME interface of ARW Elektronik Germany.\n");
    printf("Copyright: see the GPL of the free software foundation. K.Hitschler, %s.\n", __DATE__);
    printf("usage: simpleTest [-d=DeviceName] -s=StartAddress [-l=Length] [-m=AddressModifier] [-w=AccessWidth] [-?]\n");
    printf("                   -d - choose a device to use. (Default: %s)\n", DEVICE_NAME);
    printf("                   -s=StartAddress - address to start the test, mandatory.\n");
    printf("                   -w=AccessWidth  - data element width, 1,2 or 4. (Default: %d)\n", DEFAULT_WIDTH);
    printf("                   -l=Length       - area length to test in bytes. (Default: equal AccessWidth)\n");
    printf("                   -m=AddressModifier - VME address modifier for accesses. (Default: 0x%02x)\n", DEFAULT_MODIFIER);
    printf("                   -? - this help.\n"); 
}

/*--- TEST RAM LOOP ------------------------------------------------------------------------------*/
__u32 SimpleRamTest(int handle, __u32 start, __u32 length, __u8 accessWidth)
{
    int  error = 0;
    __u32 r, w;
    __u32 dwErrorCount = 0;

    while (length >= accessWidth) 
    {
        w = rand();

        error = VMEwrite(handle, start, accessWidth, 1, &w);
        if (error)
        {
            dwErrorCount++;
            printf("%s : Can't write @ adr: 0x%08x (%s)\n", cszPrgName, start, strerror(error));
        }
        else
        {
            error = VMEread(handle, start, accessWidth, 1, &r);
            if (error)
            {
                dwErrorCount++;
                printf("%s : Can't read @ adr: 0x%08x (%s)\n", cszPrgName, start, strerror(error));
            }
            else
            {
                error = ENOANO;

                switch (accessWidth)
                {
                    case BYTE_ACCESS:
                        if ((w & 0xff) != (r & 0xff))
                        {
                            dwErrorCount++;
                            printf("%s : Compare failed @ adr:0x%08x w:0x%02x r:0x%02x\n",cszPrgName, start, w & 0xff, r & 0xff);
                        }
                        break;
                    case WORD_ACCESS:
                        if ((w & 0xffff) != (r & 0xffff))
                        {
                            dwErrorCount++;
                            printf("%s : Compare failed @ adr:0x%08x w:0x%04x r:0x%04x\n",cszPrgName, start, w & 0xffff, r & 0xffff);
                        }
                        break;
                    case LONG_ACCESS:
                        if (w != r)
                        {
                            dwErrorCount++;
                            printf("%s : Compare failed @ adr:0x%08x w:0x%08x r:0x%08x\n",cszPrgName, start, w, r);
                        }
                        break;
                }
            }
        }
        length -= accessWidth;
        start  += accessWidth;
    }

    return dwErrorCount;
}

int main(int argc, char **argv)
{
    char *fname = DEVICE_NAME;
    char *ptr;
    char ch;
    int  i;
    int  error = 0;
    int  handle;
    __u8 bAddressModifier = DEFAULT_MODIFIER;
    __u8 bAccessWidth     = DEFAULT_WIDTH;
    __u32 dwStartAddress  = -1;
    __u32 dwLength        = -1;

    //-----------------------------------------------------------------------------------
    // scan command line
    cszPrgName = argv[0];
    for (i = 1; i < argc; i++)
    {
        ptr = argv[i];

        if (*ptr == '-')
            ptr++;
        ch = *ptr;

        ptr++;
        if (*ptr == '=')
            ptr++;

        switch (tolower(ch))
        {
            case 'h':
            case '?': hlpMsg(); exit(0); 
            case 'd': fname = ptr; break;
            case 's': dwStartAddress = strtoul(ptr, NULL, 16); break;
            case 'w': bAccessWidth = (__u8)atoi(ptr); break;
            case 'l': dwLength = strtoul(ptr, NULL, 16); break;
            case 'm': bAddressModifier = (__u8)strtoul(ptr, NULL, 16); break;
            default:  printf("%s : Unknown command \"%c\"!\n", cszPrgName, ch); exit(0);    
        }
    }

    //-----------------------------------------------------------------------------------
    // test for correct parameters
    if (!fname)
    {
        printf("%s : Must have devicename!\n", cszPrgName);
        exit(EINVAL);
    }

    if (dwStartAddress == -1)
    {
        printf("%s : Must have a start address!\n", cszPrgName);
        exit(EINVAL);
    }

    if ((bAccessWidth > 4) || (bAccessWidth == 3))
    {
        printf("%s : Illegal AccessWidth (%d)!\n", cszPrgName, bAccessWidth);
        exit(EINVAL);
    }

    if (bAddressModifier > 0x3F)
    {
        printf("%s : Illegal VME AddressModifer (0x%02x)!\n", cszPrgName, bAddressModifier);
        exit(EINVAL);
    }

    if (dwLength == -1)
        dwLength = bAccessWidth;

    printf("%s: Testing %s from 0x%08x to 0x%08x with Modifier 0x%02x.\n", 
           cszPrgName, 
           (bAccessWidth == BYTE_ACCESS) ? "bytes" : ((bAccessWidth == WORD_ACCESS) ? "words" : "longs"),
           dwStartAddress, dwStartAddress + dwLength, bAddressModifier);

    //-----------------------------------------------------------------------------------
    // open the path to device
    error = VMEopen(fname, bAddressModifier, &handle);
    if (error)
    {
        printf("%s : Can't open path to %s! (%s)\n", cszPrgName, fname, strerror(error));
        exit(error);
    }

    //-----------------------------------------------------------------------------------
    // loop until error
    error = SimpleRamTest(handle, dwStartAddress, dwLength, bAccessWidth);
    if (error)
        printf("%s: %d test errors!\n", cszPrgName, error);
    else
        printf("%s: No test errors.\n", cszPrgName);

    //-----------------------------------------------------------------------------------
    // close the path to device
    error = VMEclose(handle);
    if (!error)
        printf("%s: Close OK.\n", cszPrgName);
    else
        printf("%s: Close with error, %s!\n", cszPrgName, strerror(error));

    return error;
}

