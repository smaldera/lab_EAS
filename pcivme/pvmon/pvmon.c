//-------------------------------------------------------------------------------------------
// pvmon.c - the body of a simple tool to access VME BUS resources
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
// $Log: pvmon.c,v $
// Revision 1.6  2002/11/14 19:57:56  klaus
// improvement, still bugs active
//
// Revision 1.5  2002/10/20 18:07:48  klaus
// mostly working alpha version
//
// Revision 1.4  2002/10/20 11:49:33  klaus
// first parts working
//
// Revision 1.3  2002/10/19 09:47:30  klaus
// first success compiling project
//
// Revision 1.2  2002/10/19 09:44:38  klaus
// first success compiling project
//
// Revision 1.1.1.1  2002/10/18 22:14:29  klaus
//
// first parts written and published from
// Sven Hannover, Sven Tuecke, Klaus Hitschler, Ralf Dux					1991
//

//-------------------------------------------------------------------------------------------
// INCLUDES
//
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <slang.h>

#include <../driver/vme.h>		        /* constants about VME BUS     */
#include <mbuffer.h>                    /* simple message buffering    */
#include <pcilibLx.h>                   /* device access  functions    */

//-------------------------------------------------------------------------------------------
// DEFINES
//
#define VERSION "6.0Lx"
#define True 1
#define False 0

#if !defined(FALSE) || !defined(TRUE)
#define FALSE False
#define TRUE  True
#endif

#define DEVPATH " - No input neccessary!"                 // not used
#define DEFDEVICENAME "/dev/vmemm_1"
#define DEFAULT_ADDRESS   0x00000000                      // default VME window base address
#define DEFAULT_MODIFIER  Std_NoPriv_Data                 // default address modifier
#define DEFAULT_TYPE      sizeof(char)                    // default data BUS access width
#define MAX_TIMEOUT_LOOPS 100000                          // maximum loops for waiting reset finished

//-------------------------------------------------------------------------------------------
// TYPEDEFS
//
typedef char STRG[BUFFERLENGTH];                /* Allgemeiner Stringtyp */

//-------------------------------------------------------------------------------------------
// LOCALS
//
static char  UpCase(char Zchn);             
static char *ParaStr(int Num);
static unsigned long ParaNum(int Num);
static void  SetModifier(void);
static void  PrintItem(unsigned long Addr, char Mode, unsigned char *Asc);
static char  GetZug(char *Zug);
static void  Dump(void);
static char  GetStrg(STRG Stg, int Len);
static void  Examine(void);
static void  Move(void);
static void  Fill(void);
static void  Hilfe(void);
static int   InitAt(char *szDevicePath, int *nInterfaceHandle);
static void  CfgName(STRG Stg);
static void  LoadKonfig(void);
static void  Konfig(void);
static void  ReadIrqVect(void);
static void  JumpToDos(void);
static void  Raus(void);
static void  SearchPort(char *Art,
                        int Anz,
                        unsigned short modf,
                        void(*SFunc)(int nHandle, unsigned long Adr,unsigned short AModifier));
static void  SearchPorts(void);
static unsigned GibNum(char **PSt,char Anz);
static int   _ReadFile(void);
static void  SeekPatt(void);
static void  TestSet(void);
static void  ResetVme(void);
static int   OutHex(FILE *OuF, int Siz, unsigned long Adr, int Typ, char Buf[]);
static int   _WriteFile(void);
static void  ShowModifier(int Mode);
static void  ShowRegister(void);
static int   HauptMenue(STRG Stg);
static void  MyExit(int);
static void  SysFail(void);

//-------------------------------------------------------------------------------------------
// EXTERNALS
//

//-------------------------------------------------------------------------------------------
// GLOBALS
//
static unsigned short AdrMode = Short_NoPriv;  /* Mein initialer Adressmodifier */
static char DefZug = 'B';                /* Default Zugriff */
static char DefVec = 'B';                /* Default Zugriff IrqVecs */
static char **ArgV;                      /* ArgV aus main() */
static STRG InStg;                       /* Allgemeiner Eingabestring */

static char *TsT;
static char Abbruch = 0;                 /* verzweig wg. SIGINT */

static int  nInterfaceHandle;            /* handle of device */
static char *cszDevicePath;              /* path of device */
static int  WordMode;                    /* mode of VME path operation */

static char localBuffer[BUFFERLENGTH] = DEFDEVICENAME;

//-------------------------------------------------------------------------------------------
// FUNCTIONS
//
//-----------------------------------------------------------------------------
// functions to emulate for this platform
static int getch(void)
{  
    return SLang_getkey();
}


static void strlwr(char *str)
{
    int i;
    char *ptr = str;

    for (i = 0; ((i < BUFFERLENGTH) && (*ptr)); i++)
    {
        *ptr = tolower(*ptr);
        ptr++;
    }
}

static int _gets(char *str)
{
    if (fgets(str, BUFFERLENGTH, (FILE *)stdin) == NULL)
        return EINVAL;
    else
    {
        // remove '\n' from string
        int i;
        char *ptr = str;

        for (i = 0; i < BUFFERLENGTH; i++, ptr++)
        {
            if (*ptr == '\n')
            {
                *ptr = 0;
                break;
            }
        }
        return 0;
    }
}

//-----------------------------------------------------------------------------
// get out of here
static void Raus(void)
{
    DeInit_Interface(nInterfaceHandle);
    exit(0);
}

//-----------------------------------------------------------------------------
// return the uppercase char
static char UpCase(char Zchn)                  /* Upcase eines Zeichens */
{
    return((Zchn >= 'a' && Zchn <= 'z') ? Zchn - 0x20 : Zchn);
}

//-----------------------------------------------------------------------------
// get the n-th parameter as string
static char *ParaStr(int Num)          /* Hole n-ten Parameter */
{                                      /* als String aus InStg */
    char *PSt;                           /* Evt. Ergebnis NULL bei (Num>1) */

    PSt=InStg;                           /* Fange bei InStg[0] an */
    if (Num > 1)
    {                                    /* Folgeparameter: suche Anfang */
        if (*PSt!='\0')
        {                                  /* Leerstring ignorieren */
            PSt++;
            switch (*PSt)
            {                                /* Teste evt. Modusparameter */
                case 'L':
                case 'W':
                case 'B':
                case 'X':
                case 'H':PSt++;
            }

            if (*PSt==' ') PSt++;            /* Evt. Delimiter ueberspringen */
        }

        if (*PSt=='\0') PSt=NULL;          /* Kein weiterer Parameter da */
        else
        {
            while (PSt!=NULL && Num>2)
            {
                PSt=strchr(PSt, ' ');          /* Suche nach Delimiter */
                if (PSt!=NULL) PSt++;          /* Delimiter ueberspringen */
                Num--;                         /* Naechster Parameter da */
            } /* while */
        } /* else */
    } /* if */
    return(PSt);
}

//-----------------------------------------------------------------------------
// get the n-th parameter as unsigned long
static unsigned long ParaNum(int Num)  /* Hole n-ten Parameter */
{                                      /* als Zahl aus InStg */
    unsigned long Erg;
    char         *PSt;

    PSt=ParaStr(Num);                    /* Hole Parameterstring */
    Erg=0;                               /* Hole Word aus String */
    if (PSt!=NULL) sscanf(PSt, "%lx", &Erg);
    return(Erg);
}

//-----------------------------------------------------------------------------
// set the address modifier for following accesses
static void SetModifier(void)          /* Neuen Adressmodifier setzen */
{
    int Idx;

    if (ParaStr(1)[1]=='H')
    {                                    /* Wenn Hilfsfunktion gewuenscht */
        if (ParaStr(2)==NULL)
        {                                  /* Noch ein Parameter da? */
            for (Idx=0; Idx<0x40; Idx++)
            {                                /* Nein: Liste ausgeben */
                ShowModifier(Idx);
                if ((Idx == 0x10) || (Idx == 0x20) || (Idx == 0x30))
                {
                    printf("\n go on ?\r");
                    getch();
                }
            }
            printf("\n");
        }
        else ShowModifier((int)ParaNum(2)); /* Nur gewuenschten Mode anzeigen */
    }
    else
    {
        if (ParaStr(2) != NULL)
        {
            if (ParaStr(1)[1] == 'M')
            {
                AdrMode=(int)ParaNum(3) & 0x3f;
            }
            else
            {
                AdrMode=(int)ParaNum(2) & 0x3f;  /* Adressmodifier merken */
            }
        }
        ShowModifier(AdrMode);             /* Status Adressmodifier zeigen */
    } /* else */
}

//-----------------------------------------------------------------------------
// print out an item
static void PrintItem(unsigned long Addr, char Mode, unsigned char *Asc)       
{
    unsigned long xl;
    unsigned int  xi;
    unsigned char xc;

    switch (Mode)
    {
        case 'L': xl=ReadLong(nInterfaceHandle, Addr, AdrMode);
            if (GetError(nInterfaceHandle))
            {
                ClearError(nInterfaceHandle); printf("********   ");
            }
            else
            {
                printf("%08lx   ", xl);
                if (Asc != NULL) *(unsigned long *)Asc=xl;
            }
            break;
        case 'W': xi=ReadWord(nInterfaceHandle, Addr, AdrMode);
            if (GetError(nInterfaceHandle))
            {
                ClearError(nInterfaceHandle); printf("**** ");
            }
            else
            {
                printf("%04hx ", xi);
                if (Asc != NULL) *(unsigned short *)Asc=xi;
            }
            break;
        case 'B': xc=ReadByte(nInterfaceHandle, Addr, AdrMode);
            if (GetError(nInterfaceHandle))
            {
                ClearError(nInterfaceHandle); printf("**");
            }
            else
            {
                printf("%02hx", xc);
                if (Asc != NULL) *Asc=xc;
            }
            break;
    }; /* switch */
}

//-----------------------------------------------------------------------------
// test whether byte word or long access
static char GetZug(char *Zug)          /* Moduszeichen feststellen */
{
    switch (ParaStr(1)[1])
    {                                    /* Moduszchn ist angegeben */
        case 'L':
        case 'W':
        case 'B':*Zug = ParaStr(1)[1];       /* Neues Moduszchn festlegen */
    }
    return(*Zug);
}

//-----------------------------------------------------------------------------
// get or set SYSFAIL
static void SysFail(void)
{
    if (ParaStr(2) != NULL)
    {
        if (ParaNum(2) > 0)
            SetSfail(nInterfaceHandle);
        else
            ClrSfail(nInterfaceHandle);
    }

    if (PollSfail(nInterfaceHandle))
        printf("SYSFAIL deasserted\n");
    else
        printf("SYSFAIL asserted\n");
}


//-----------------------------------------------------------------------------
// dump a range of memory
static void Dump(void)                 /* Ausgabe eines Bereichs */
{
    static unsigned long DefVon=0;       /* Default Addr fuer Dump */

    unsigned long Bis;                   /* Bis wohin ausgeben */
    unsigned int  Len;                   /* Wieviel Bytes/Ausgabe */
    unsigned int  Idx;                   /* Index */
    char Asc[16];        /* ohne static gehts bei dw nicht */

    if (ParaStr(2) != NULL)              /* Von-Adresse angegeben? */
        DefVon=ParaNum(2);
    Len=1;

    switch (GetZug(&DefZug))
    {                                    /* Zugriffsmodus festlegen */
        case 'L':Len+=2;                   /* Auf Long-Adresse biegen */
        case 'W':Len++;                    /* Auf Wort-Adresse biegen */
    }

    DefVon&=-(long)Len;                  /* Adressen geradebiegen */
    if (ParaStr(3) != NULL)
    {                                    /* Bis-Adresse angegeben? */
        Bis=ParaNum(3);
    }
    else 
        Bis=(DefVon+0x7f) | 0x0f;       /* Default fuer Bis errechnen */

    printf("%08lx: ", DefVon);
    for (Idx=0; Idx < (DefVon & 0x0f)/Len*(2*Len+1); Idx++)
        printf(" ");

    memset(Asc, ' ', sizeof(Asc));       /* Initialize String to Spaces */
    while ((True) && (!Abbruch))
    {
        PrintItem(DefVon, DefZug,          /* Gebe eine Speicherstelle aus */
                  &Asc[DefVon & 0x0f]);            /* Merke Zeichen in Asc */
        DefVon+=Len;                       /* Zaehler erhoehen */

        if ((DefVon > Bis) || (!(DefVon & 0x0f)))
        {
            printf("   ");
            for (Idx=0; Idx < sizeof(Asc); Idx++)
            {
                if (Asc[Idx] < ' ') printf("."); /* Ascii-String ausgeben */
                else printf("%c", Asc[Idx]);   /* Ctrl-Zeichen als Punkte */
            }

            printf("\n");
            if (DefVon <= Bis)
            {
                printf("%08lx: ", DefVon);     /* Neue Zeile bei 16er-Grenze */
                memset(Asc, ' ', sizeof(Asc)); /* Init String */
            }
            else return;                     /* Ausstieg */
        }
        else
        {                                  /* Sonst Leerzeichen ausgeben */
            printf(((DefVon & 0x0f) == 0x08) ? "|":" ");
        }
    } /* while */
}

//-----------------------------------------------------------------------------
// read a string with editing functions
static char GetStrg(STRG Stg, int  Len) /* Lese String ein bis Spc */
{
    int  Idx;                            /* Zugriffsindex */
    char Zch;                            /* Eingabezeichen */

    Idx=0;                               /* Vorne anfangen */
    do
    {
        Zch=(char)getch();                 /* Hole ein Zeichen */
        if ((unsigned char)Zch >' ' && Zch!='\t')
        {
            if (Idx<Len)
            {
                printf("%c",Zch);              /* Zeichen ok, Ausgeben */
                Stg[Idx++]=Zch;                /* Zeichen ablegen */
            }
        }
        else
        {
            switch (Zch)
            {
                case '\b':if (Idx)
                    {                    /* Backspace=Delete? */
                        Idx--;             /* Loesche Zeichen aus String */
                        printf("\b \b");   /* und vom Bildschirm */
                    }
                case '\t':
                case '\r':break;               /* Return? Endezeichen 13 */
                default:Zch=0;                 /* Ende mit Endezeichen 0 */
            } /* switch */
        } /* else */
    } while (Zch && Zch!='\r' && Zch!='\n');

    Stg[Idx]='\0';                       /* Stringende eintragen */
    return(Zch);                         /* Returncode = Abschlusstaste */
}

//-----------------------------------------------------------------------------
// examine a memory location
static void Examine(void)              /* Speicherbereich aendern */
{
    unsigned long DefVon;                /* Anfangsadresse */
    unsigned long Inh;                   /* Neuer Inhalt */
    int           Len;                   /* Item-Laenge */
    int           Idx;                   /* Index */
    char          End;                   /* Endmodus */
    STRG          Stg;                   /* Eingabestring */

    if (ParaStr(2)!=NULL)
    {                                    /* Adresse benoetigt */
        Len=1;
        switch (GetZug(&DefZug))
        {         /* Zugriffsmodus festlegen */
            case 'L':Len+=2;                 /* Auf Long-Adresse biegen */
            case 'W':Len++;                  /* Auf Wort-Adresse biegen */
        }
        DefVon=ParaNum(2) & -(long)Len;    /* Adressen geradebiegen */
        if (ParaStr(3)!=NULL)
        {                                  /* Wert angegeben? */
            Inh=ParaNum(3);                  /* Hole auszugebenden Wert */
            switch (DefZug)
            {
                case 'L': WriteLong(nInterfaceHandle, DefVon,Inh,AdrMode);
                    break;
                case 'W': WriteWord(nInterfaceHandle, DefVon,(short)Inh,AdrMode);
                    break;
                case 'B': WriteByte(nInterfaceHandle, DefVon,(char)Inh,AdrMode);
                    break;
            }; /* switch */

            if (GetError(nInterfaceHandle))
            {                                /* Fehlerpruefung: VME-Transfer ok? */
                ClearError(nInterfaceHandle);                  /* Zuruecksetzen Fehlerflag */
                printf("Error\n");             /* Zugriff gescheitert */
            }
        }
        else
        {
            SLang_init_tty(-1, 0, 1);
            SLtt_get_terminfo();
            
            End='\n';                        /* Bei Einstieg drucke Adresse */
            do 
            {
                if (End=='\n' || End=='\177' || !(DefVon % 8))
                {
                    if (End!='\n') printf("\n"); /* Bei Einstieg nicht <CRLF> */
                    printf("%08lx: ", DefVon);   /* Adresse ausgeben */
                }

                PrintItem(DefVon,DefZug,NULL); /* Gebe eine Speicherstelle aus */
                printf(".");
                SLtt_flush_output();

                End=GetStrg(Stg,Len << 1);     /* Hole begrenzte Eingabezeile */

                for (Idx=strlen(Stg); Idx<2+(Len << 1); Idx++)
                    printf(" ");
                if (sscanf(Stg,"%lx",&Inh)>0)
                {                              /* Hexzahl rausholen und ausgeben */
                    switch (DefZug)
                    {
                        case 'L': WriteLong(nInterfaceHandle, DefVon,Inh,AdrMode);
                            break;
                        case 'W': WriteWord(nInterfaceHandle, DefVon,(short)Inh,AdrMode);
                            break;
                        case 'B': WriteByte(nInterfaceHandle, DefVon,(char)Inh,AdrMode);
                            break;
                    }; /* switch */

                    if (GetError(nInterfaceHandle)) 
                        ClearError(nInterfaceHandle);/* Fehlerpruefung: VME-Transfer ok? */
                } /* if sscanf */

                if (End == '\177') DefVon-=Len;/* Naechste Speicherzelle ansteuern */
                else DefVon+=Len;
            } while (End!='\r');
            /* Ende bei <CR> */
            printf("\n");

            SLang_reset_tty();
        } /* else */
    } /* if */
    else printf("\a");                   /* Fehler: zuwenig Parameter */
}

//-----------------------------------------------------------------------------
// fill a range of memory
static void Fill(void)                 /* Fuellt Speicherbereich mit Wert */
{
    char          DefZug;                /* Zugriffsart */
    int           Len;                   /* Item Laenge */
    unsigned long Idx;                   /* Index */
    unsigned long End;                   /* Endadresse */
    unsigned long Patt;                  /* Fuellmuster */
    unsigned char Merk_error = 0;        /* Haelt error flag */

    DefZug=' ';                          /*  Modus muss angeben werden */
    if (GetZug(&DefZug)!=' ' && ParaStr(4)!=NULL)
    {
        Len=1;
        switch (GetZug(&DefZug))
        {                                  /* Zugriffsmodus festlegen */
            case 'L':Len+=2;                 /* Auf Long-Adresse biegen */
            case 'W':Len++;                  /* Auf Wort-Adresse biegen */
        }
        Idx=ParaNum(2) & -(long)Len;       /* Adressen geradebiegen */
        End=ParaNum(3);                    /* Endadresse festlegen */
        Patt=ParaNum(4);                   /* Pattern merken (geht schneller) */

        while ((Idx<=End) && (!Abbruch))
        {
            switch (DefZug)
            {
                case 'L':WriteLong(nInterfaceHandle, Idx, Patt, AdrMode);
                    break;
                case 'W':WriteWord(nInterfaceHandle, Idx, (short)Patt, AdrMode);
                    break;
                case 'B':WriteByte(nInterfaceHandle, Idx, (char)Patt, AdrMode);
                    break;
            } /* switch */

            if (GetError(nInterfaceHandle))
            {
                ClearError(nInterfaceHandle);                  /* Fehler abfangen */
                Merk_error = 1;
            }
            if ((Idx & 0xffl)==0)
            {                                /* Ermoegliche Ctrl-C */
                printf("\r");
            }
            Idx+=Len;
        } /* while */
        if (Merk_error) printf("--> Memory fill failed\a\n");
    }
    else printf("\a");
}

//-----------------------------------------------------------------------------
// moves a range of memory
static void Move(void)                 /* Schiebt Speicherbereich */
{
    char          DefZug;                /* Zugriffsart */
    int           Len;                   /* Item Laenge */
    unsigned long Idx;                   /* Index */
    unsigned long End;                   /* Endadresse */
    unsigned long Dest;                  /* Zieladresse */
    unsigned long Wert;                  /* Kopiewert */
    unsigned char Merk_error = 0;        /* Haelt error flag */


    DefZug=' ';                          /*  Modus muss angeben werden */
    if (GetZug(&DefZug)!=' ' && ParaStr(4)!=NULL)
    {
        Len=1;
        switch (GetZug(&DefZug))
        {                                  /* Zugriffsmodus festlegen */
            case 'L':Len+=2;                 /* Auf Long-Adresse biegen */
            case 'W':Len++;                  /* Auf Wort-Adresse biegen */
        }
        Idx=ParaNum(2) & -(long)Len;       /* Adressen geradebiegen */
        End=ParaNum(3);                    /* Endadresse festlegen */
        Dest=ParaNum(4);                   /* Zieladresse setzen */

        while ((Idx<=End) && (!Abbruch))
        {
            switch (DefZug)
            {
                case 'L': {
                        Wert = ReadLong(nInterfaceHandle, Idx, AdrMode);
                        WriteLong(nInterfaceHandle, Dest, Wert, AdrMode);
                    }
                    break;
                case 'W': {
                        Wert = ReadWord(nInterfaceHandle, Idx, AdrMode);
                        WriteWord(nInterfaceHandle, Dest, (short)Wert, AdrMode);
                    }
                    break;
                case 'B': {
                        Wert = ReadByte(nInterfaceHandle, Idx, AdrMode);
                        WriteByte(nInterfaceHandle, Dest, (char)Wert, AdrMode);
                    }
                    break;
            } /* switch */

            if (GetError(nInterfaceHandle))
            {
                ClearError(nInterfaceHandle);                        /* Fehler abfangen */
                Merk_error = 1;
            }

            if ((Idx & 0xffl)==0)
            {                                      /* Ermoegliche Ctrl-C */
                printf("\r");
            }

            Idx+=Len;
            Dest+=Len;
        } /* while */
        if (Merk_error) printf("--> Memory move failed\a\n");
    }
    else printf("\a");
}

//-----------------------------------------------------------------------------
// print out help to user
static void Hilfe(void)
{
    printf("a[h] [adrmode]\t\t: Change address modifiers, h=help\n");
    printf("c\t\t\t: Configure interface\n");
    printf("d[m] [start] [end]\t: Dump memory area\n");
    printf("e[m] <start> [value]\t: Examine or change memory area\n");
    printf("f<m> <start> <end> <x>\t: Fill memory from <start> til <end> with <x>\n");
    printf("g<m> <st> <en> [l] [x]\t: Generate random memory test. (loop l, seed x)\n");
    printf("h\t\t\t: This help\n");
    printf("i\t\t\t: Interface init\n");
    printf("l[m]\t\t\t: Get VME interrupt status/ID\n");
    printf("m<m> <src> <end> <dest>\t: Move memory area\n");
    printf("o\t\t\t: Jump to OS\n");
    printf("p[adrmode]\t\t: Port search\n");
    printf("q\t\t\t: Quit program\n");
    printf("r[x] <f> <start> [end]\t: Read file <f> to VME, x= x or s (HEX)\n");
    printf("s[m] <start> <end> <p>\t: Search pattern <p>=different Items\n");
    printf("t <start>\t\t: TAS emulation, 'Test and Set' bit 7\n");
    printf("v\t\t\t: Generate VME SYSRESET\n");
    printf("w[x] <f> <start> <end>\t: Write VME into file <f>, h=Intel Hex\n");
    printf("x <start> [val]\t\t: Read/Write to interface register @ start\n");
    printf("y[1/0]\t\t\t: Read/set/clear SYSFAIL\n");
    printf("z[0..2]\t\t\t: Show interface internals\n");
    printf("\n");
    printf("m = mode, e.g. b=byte, w=word, l=long (double) word; h = help, x= hex\n");
    printf("start(address), end(address), src=source, dest=destination, []=option\n");
}

//-----------------------------------------------------------------------------
// initialize the interface to VME
static int InitAt(char *szDevicePath, int *nIfcNum)                      /* Gibt bei Fehler False aus */
{
    int result;

    if (result = Init_Interface(szDevicePath, AdrMode, nIfcNum))         /* Pruefung des Interfaces */
    {
        printf("\n");
        switch (result)
        {
            case ENXIO:          
                printf("Can't find interface driver path!\n");
                printf("Please <q>uit or <c>onfigure interface!\n");
                return FALSE;
            case ENOENT:
                printf("Can't find interface driver!\n");
                printf("Please <q>uit or <c>onfigure interface!\n");
                return FALSE;
            case ENODEV:
                printf("VMEMM #%d not connected or VME crate switched off!\n", nInterfaceHandle);
                printf("Please check connection or switch VME crate on or <c>onfigure.\n");
                printf("Then <q>uit and restart again.\n");
                return FALSE;

            default:
                printf("Unknown error '%d' occured!\n", result);
                printf("Please check the hardware and software setup and restart again.\n");
                return FALSE;

        }
    }

    return(True);                        /* Kein Fehler */
}

//-----------------------------------------------------------------------------
// get the name of the configuration file 
static void CfgName(STRG Stg)          /* Ermittelt Namen Config-File */
{
    Stg[0]='\0';
    if (ArgV[0] != NULL)
    {
        strcpy(Stg,ArgV[0]);
        if (strrchr(Stg,'/')!=NULL)       /* Versuche Dateinamen abzutrennen */
            *(strrchr(Stg,'/')+1)='\0';     /* So daﬂ nur Pfad uebrigbleibt */
        else Stg[0]='\0';                  /* Kein Pfad: String ist leer */
    }
    strcat(Stg,"pvmon.cfg");          /* Mache einen Dateinamen */
}

//-----------------------------------------------------------------------------
// read in contents of configuration file
static void LoadKonfig(void)           /* Wenn Config-Datei da, lese ein */
{
    STRG  Stg;
    FILE *InF;
    char c;
    __u32 dwLocalAdrMode;

    CfgName(Stg);                        /* Hole Dateinamen nach InS */
    if ((InF=fopen(Stg,"rt"))!=NULL)
    {                                    /* Wenn das oeffnen geklappt hat */
        fscanf(InF,"%*[^=]%*1s%s",Stg);
        fscanf(InF,"%*[^=]%*1s%s",cszDevicePath);
        fscanf(InF,"%*[^=]%*1s%x",&dwLocalAdrMode);
        AdrMode = (__u8)dwLocalAdrMode;
        fscanf(InF,"%*[^=]%*1s%c",&c);
        fclose(InF);                       /* Datei wieder schlieﬂen */

        c = tolower(c);
        if (c == 'y')
            WordMode = setWordMode(1);
        else
            WordMode = setWordMode(0);

    } /* if */
}

//-----------------------------------------------------------------------------
// provides configuration functionality to user
static void Konfig(void)               /* Konfiguration einstellen */
{
    STRG  InS;                           /* Eingabestring */
    FILE *OuF;                           /* Ausgabedatei  */
    short change = 0;
    char  c;

    InS[0] = 0;
    printf("Pathname of device (%s):",cszDevicePath);   /* erfrage den Pfad zum Treiber */
    _gets(InS);
    if ((InS[0] != '\n') && (InS[0]))
    {
        strcpy(cszDevicePath, InS); 
        change |= 1;
    }

    InS[0] = 0;
    printf("Default address modifier (%02x):",AdrMode); /* und den default Modifier */
    _gets(InS);
    if ((InS[0] != '\n') && (InS[0]))
    {
        sscanf(InS,"%x",&AdrMode); 
        change |= 4;
    }

    if (WordMode)
        c = 'y';
    else
        c = 'n';
    InS[0] = 0;
    printf("16 bit VME BUS data path (%c) :", c);
    _gets(InS);
    if ((InS[0] != '\n') && (InS[0]))
    {
        sscanf(InS,"%c",&c); 
        change |= 8;
    }
    c = tolower(c);
    if (c == 'y')
        WordMode = setWordMode(1);
    else
        WordMode = setWordMode(0);

    if (change)
    {
        do 
        {
            printf("Save (y/n):");             /* Wiederhole diese Frage bis */
            _gets(InS);                         /* sie ordentlich beantwortet wurde */
            strlwr(InS);                       /* DownCase String */
        } while (InS[0]!='y' && InS[0]!='n');

        if (InS[0]=='y')
        {
            CfgName(InS);                      /* Hole Dateinamen nach InS */
            if ((OuF=fopen(InS,"wt"))!=NULL)
            {
                if (WordMode)
                    c = 'y';
                else
                    c = 'n';

                fprintf(OuF,"Configuration=%s\n",__DATE__);
                fprintf(OuF,"DevicePath=%s\n",cszDevicePath); /* Wenn das oeffnen geklappt hat */
                fprintf(OuF,"AddressModifier=%x\n",AdrMode);
                fprintf(OuF,"WordMode=%c\n", c);
                fclose(OuF);                     /* Datei schliessen */

                if (change & 1)
                    printf("Please restart to put the new driver to work!\n");
            }
            else printf("Can't open %s. ",InS);
        }
    }
}

//-----------------------------------------------------------------------------
// read user initiated interrupt vector from VME BUS
static void ReadIrqVect(void)          /* Interrupt-Vektoren lesen */
{
    STRG OSt;                            /* Ausgabestring */
    short Level = 0;

    switch (GetZug(&DefVec))
    {                                    /* Zugriffsmodus festlegen */
        case 'L':sprintf(OSt, "%08hx", ReadVectorLong(nInterfaceHandle)); break;
        case 'W':sprintf(OSt, "%04hx", ReadVectorWord(nInterfaceHandle)); break;
        case 'B':sprintf(OSt, "%02hx", ReadVectorByte(nInterfaceHandle)); break;
    };

    if (GetError(nInterfaceHandle))
    {                                    /* Im Fehlerfalle 'Error' ausgeben */
        ClearError(nInterfaceHandle);                      /* Fehlerflags zuruecksetzen */
        strcpy(OSt, "Error");
    }
    printf("VME status/ID = %s\n", OSt);
}


//-----------------------------------------------------------------------------
// temporary jump to (D)OS
static void JumpToDos()                       /* (D)OS-Shell aufrufen */
{
    {
        if (system("/bin/sh -c $SHELL") != 0)
            printf("Fail to launch a new shell.\n");
    }
}

//-----------------------------------------------------------------------------
// search responding ports in VME address range
static void SearchPort(char *Art, int Anz, unsigned short modf, 
                       void (*SFunc)(int, unsigned long, unsigned short)) /* Durchsucht Adressraum */
{
    unsigned long Idx;
    unsigned long Fst;                    /* Erster gefundener Port */
    unsigned long Lst;                    /* Letzer gefundener Port */
    unsigned long Ende;                   /* Durchsuch Ende */
    char          Found;                  /* Schon was gefunden? */
    char          Sequ;                   /* Schon eine Portsequenz */
    int           Err;                    /* Fehler dagewesen? */
    int           Tab;                    /* Tabulator-Zaehler */
    unsigned long Step;

    printf("%s-accesses valid with address modifier %02x to address: ", Art,modf);

    if (modf > 0x2F)
    {
        Ende = 0x01000000L;                /* alle Standards */
        Step = 0x100;                      /* Stepweite      */
    }

    if ((modf < 0x30) && (modf > 0x1f))
    {
        Ende = 0x00010000L;                /* Shorts */
        Step = Anz;
    }

    if (modf < 0x20)
    {
        Ende = 0xFFFF0000L;                /* alle Extendets, gemogelt */
        Step = 0x10000;                    /* Step */
    }

    Sequ=False;                          /* Noch keine Sequenz da */
    Found=False;
    Tab=0;
    Idx=0;

    do 
    {                                    /* do while */
        SFunc(nInterfaceHandle, Idx, modf);                   /* Lese versuchsweise Port */
        Err=GetError(nInterfaceHandle);                    /* Fehlerzustand abfragen */
        if (Err) ClearError(nInterfaceHandle);             /* Fehler bestaetigen */
        else
        {
            Lst=Idx;                         /* Merke Port als gueltig */
            if (!Sequ)
            {                                /* Diese Seqenz faengt an? */
                Fst=Idx;                       /* Ja, neue Sequenz, merke */
                Sequ=True;                     /* auch ersten Port */
            }
        }

        Idx+= Step;                        /* Erhoehe Adresse */

        if ((Err || !(Idx < Ende)) && Sequ)
        {                                  /* Ausgeben bei Sequenzende */
            if (!Found)
            {                                /* oder bei Schleifenende */
                if (Idx < Ende) printf("\n");  /* Kein <CRLF> bei Schleifenende */
                Found=True;
            }; 
            /* Weitere Sequenz: Tab ausgeben */
            if (Fst==Lst)
            {                                /* Sequenz mit nur 1 Element */
                printf("%08lx,\t", Fst);
                Tab++;                         /* Merke Tab-Status */
            }
            else
            {
                Tab=0;                         /* Tab-Status wieder zuruecksetzen */
                printf("%08lx-%08lx\n", Fst, Lst); /* Sequenz ausgeben */
            }
            Sequ=False;                      /* Sequenz gilt als abgeschlossen */
        } /* if */
    } while ((Idx < Ende) && (!Abbruch));     /* Bis Idx einmal 'rum ist */

    if (!Found)
        printf("\nnothing found");         /* Wenn keinen Zugriff gefunden */
    printf("\n");                        /* Immer mit <CRLF> abschlieﬂen */
}

//-----------------------------------------------------------------------------
// search responding ports
static void SearchPorts(void)          /* Durchsucht Short-Adressraum */
{                                      /* nach Wort- und Bytes Zugriffen */
    unsigned short modf = AdrMode;   

    if (ParaStr(2)!=NULL)
        modf = (unsigned short)ParaNum(2); /* Anderer Adressmodifier      */

    ShowModifier(modf); printf("\n");
    SearchPort("Byte", 1, modf, (void(*)(int, unsigned long, unsigned short))ReadByte);
    SearchPort("Word", 2, modf, (void(*)(int, unsigned long, unsigned short))ReadWord);
    SearchPort("Long", 4, modf, (void(*)(int, unsigned long, unsigned short))ReadLong);
    printf("\n");
}

//-----------------------------------------------------------------------------
// converts parts of a string to a number
static unsigned int GibNum(char **PSt, char Anz)
{
    unsigned int Val;                    /* Ermittelter Wert */
    unsigned int Num;                    /* Wieviel Zeichen genommen */
    char     Frm[6];                     /* Formatstring */

    Val=0;                               /* Default setzen */
    strcpy(Frm,"%nx%n");                 /* Default Format setzen */
    if (*PSt!=NULL)
    {                                    /* Nur wenn String gueltig */
        Frm[1]=Anz;                        /* Uebertrage Anzahl-Zeichen */
        *PSt=(sscanf(*PSt,Frm,&Val,        /* Hole Nummer aus String */
                     &Num)!=1) ? NULL : (*PSt)+Num;   /* Fehler oder weitersetzen */
    } /* if */
    return(Val);
}

//-----------------------------------------------------------------------------
// read in a file and put the contents to VME 
static int _ReadFile(void)             /* Lese eine Datei in VME-Mem ein */
{
    unsigned long End;                   /* Endadresse */
    unsigned long Idx;                   /* Laufadresse */
    unsigned long Cnt;                   /* Bytezaehler */
    unsigned      Adr;                   /* Adresse Record ab Start */
    int           Len;                   /* Recordlaenge */
    int           Ret;                   /* Returncode */
    int           Hex;                   /* Intel Hex File? */
    int           Typ;                   /* Typ des Records */
    STRG          Nam;                   /* Dateiname */
    STRG          Stg;                   /* Einlese-String */
    char         *PSt;                   /* Scanzeiger */
    FILE         *InF;                   /* Lesedatei */

    Ret=1;                               /* Vorgabe ist Fehler */
    if (ParaStr(3)!=NULL)
    {                                    /* Startadr ist obligat */
        Hex=(ParaStr(1)[1]=='X');          /* Intel-Hex gewuenscht? */
        strcpy(Nam,ParaStr(2));            /* Dateinamen kopieren */
        *strchr(Nam,' ')='\0';             /* Restparameter abschneiden */
        Cnt=0;                             /* Noch nichts gelesen */
        Idx=ParaNum(3);                    /* Lege Startadresse fest */
        End=(ParaStr(4)==NULL)             /* Endadr ist optional */
            ? 0xffffffffl : ParaNum(4);

        if (Idx<=End)
        {                                  /* Falsche Werte abweisen */
            if ((InF=fopen(Nam,(Hex) ? "rt":"rb"))!=NULL)
            {
                if (Hex)
                {                              /* Intel-Hex gewuenscht? */
                    fscanf(InF,"%x",Idx);
                    while (!feof(InF))
                    {                            /* Bis zum Ende lesen */
                        fgets(Stg,sizeof(Stg),InF);
                        if (strlen(Stg)>1)
                        {                          /* Ignoriere leere Zeilen */
                            PSt=strchr(Stg,':');     /* Doppelpunkt ist obligat */
                            if (PSt!=NULL) PSt++;    /* Hinter ':' stellen */
                            Len=GibNum(&PSt,'2');    /* Hole Recordlaenge */
                            Adr=GibNum(&PSt,'4');    /* Hole Adresse */
                            Typ=GibNum(&PSt,'2');
                            if (!Typ)
                            {                        /* Datencode erkannt? */
                                while (PSt!=NULL && Len)
                                {
                                    WriteByte(nInterfaceHandle, Idx+Adr++,(char)GibNum(&PSt,'2'),AdrMode);
                                    Cnt++;               /* 1 Byte mehr gelesen */
                                    Len--;               /* Laenge aufaddieren */
                                } /* while */

                                if (GetError(nInterfaceHandle))
                                {                      /* Fehlerpruefung: VME-Transfer ok? */
                                    ClearError(nInterfaceHandle);        /* Fehlerflag zuruecksetzen */
                                    printf("--> Bus error: Adr=%08lx. ",Idx+Adr);
                                    break;               /* Abbruch mit Fehler */
                                } /* if */
                            }
                            else
                            {
                                if (Typ==1)
                                {                      /* Endcode erkannt? */
                                    Ret=0;               /* Fehlerfrei gelesen */
                                    break;               /* Ende while */
                                }                      /* Ignoriere andere Typen */
                            } /* else */
                            if (PSt==NULL)
                            {
                                printf("Format error\n");
                                break;
                            } /* if */
                        } /* if len */
                    } /* while */
                }
                else
                {                              /* Kein Intel-Hex-Format */
                    do 
                    {
                        if (feof(InF)) Idx=End;    /* Ende der Datei erreicht */
                        else
                        {
                            WriteByte(nInterfaceHandle, Idx,(char)fgetc(InF),AdrMode);
                            if (GetError(nInterfaceHandle))
                            {                        /* Fehlerpruefung: VME-Transfer ok? */
                                ClearError(nInterfaceHandle);          /* Fehlerflag zuruecksetzen */
                                printf("--> Bus error: Adr=%08lx. ",Idx);
                                break;                 /* Abbruch mit Fehler */
                            } /* if GetError */

                            Cnt++;                   /* Ein Byte mehr gelesen */
                        } /* else */
                    } while (Idx++<End);         /* Bis einschliesslich End lesen */
                    fclose(InF);
                    if (Idx==End+1) Ret=0;       /* Genug Byte geschafft? */
                } /* else Hex */
            } /* if fopen */
            else printf("Can't read file %s. ",Nam);
        } /* if */
        printf("%lx Byte(s) read\n",Cnt);
    } /* if */
    else printf("\a");
    return(Ret);
}

//-----------------------------------------------------------------------------
// seek for a pattern in VME BUS
static void SeekPatt(void)             /* Suche nach Datenmustern */
{
#define Max 32                       /* Wieviele Suchbytes max. */
    unsigned long DefVon;                /* Startadresse */
    unsigned long End;                   /* Endadresse */
    int           Idx;                   /* Index */
    int           Idy;                   /* Auch Index */
    int           Len;                   /* Item Laenge */
    int           Ok;                    /* Flag: gefunden oder nicht? */
    int           Merk_error = 0;        /* Fehler Flip-Flop */

    union 
    {                                    /* Suchmuster */
        unsigned char xs[Max];
        unsigned int  xw[Max/2];
        unsigned long xl[Max/4];
    } Patt;

    if (ParaStr(4) != NULL)
    {                                    /* Von, Bis und 1 Item obligat */
        DefVon=ParaNum(2);                 /* Startadresse festlegen */
        End=ParaNum(3);                    /* Endadresse festlegen */
        Len=1;
        switch (GetZug(&DefZug))
        {                                  /* Zugriffsmodus festlegen */
            case 'L':Len+=2;                 /* Auf Long-Adresse biegen */
            case 'W':Len++;                  /* Auf Wort-Adresse biegen */
        }

        DefVon&=-(long)Len;                /* Adressen geradebiegen */
        Idx=0;                             /* Suchmuster sammeln */
        while (Idx<Max/Len && ParaStr(Idx+4)!=NULL)
        {
            switch (DefZug)
            {
                case 'L':Patt.xl[Idx]=ParaNum(Idx+4);                break;
                case 'W':Patt.xw[Idx]=(unsigned)ParaNum(Idx+4);      break;
                case 'B':Patt.xs[Idx]=(unsigned char)ParaNum(Idx+4); break;
            } /* switch */
            Idx++;                           /* Ein Item mehr da */
        } /* while */

        while ((DefVon<=End) && (!Abbruch))
        {                                  /* Suche nun den Bereich ab */
            Ok=True;                         /* Pattern an dieser Adresse? */
            for (Idy=0; Idy<Idx && Ok; Idy++)
            {
                switch (DefZug)
                {
                    case 'L':if (Patt.xl[Idy] != (unsigned long)ReadLong(nInterfaceHandle, DefVon+(Idy<<2),AdrMode))
                            Ok=False;
                        break;
                    case 'W':if (Patt.xw[Idy] != (unsigned short)ReadWord(nInterfaceHandle, DefVon+(Idy<<1),AdrMode))
                            Ok=False;
                        break;
                    case 'B':if (Patt.xs[Idy] != (unsigned char)ReadByte(nInterfaceHandle, DefVon+Idy,AdrMode))
                            Ok=False;
                        break;
                } /* switch */

                if (GetError(nInterfaceHandle))
                {                              /* Busfehler aufgetreten? */
                    ClearError(nInterfaceHandle);                /* Fehlerflags zuruecksetzen */
                    Ok=False;                    /* Gefunden wurde auch nichts */
                    Merk_error = 1;              /* Setze Flip-Flop */
                }
            } /* for */
            if (Ok) printf("%08lx\n",DefVon);/* Was gefunden: Adresse ausgeben */
            DefVon+=Len;

            if ((DefVon & 0xffl)==0)
            {                                /* Ermoegliche Abbruch mit Ctrl-C */
                printf("\r");
            }
        } /* while */
        if (Merk_error) printf("--> Failed to search\n");
    }
    else printf("\a");
}

//-----------------------------------------------------------------------------
// emulate a 68K test and set instruction
static void TestSet()                  /* Fuehre ein Test and Set auf */
{                                      /* Bit #7 eines Byte-Ports aus. */
    char Erg;

    if (ParaStr(2)!=NULL)
    {                                    /* Adresse ist obligat */
        Erg=TAS(nInterfaceHandle, ParaNum(2),AdrMode);       /* Ergebnis merken, damit ein */
        if (GetError(nInterfaceHandle))
        {                                  /* Fehler ausgegeben werden kann */
            ClearError(nInterfaceHandle);
            printf("--> Failed to 'Test and Set'\n"); /* Zugriff gescheitert */
        }
        else printf("Semafore @ 0x%08lx was%s set before.\n",
                    ParaNum(2),(Erg) ? "" : " not");
    }
    else printf("\a");
}

//-----------------------------------------------------------------------------
// raise a VME SYSRESET
static void ResetVme(void)             /* Generiere SysReset auf VME-Bus */
{                                      /* Interrupt bei MailBox beachten */
    printf("Reset to VME raised.\n");
    Reset_VME(nInterfaceHandle);
}

//-----------------------------------------------------------------------------
// print out a line in HEX format
static int OutHex(FILE *OuF, int Siz, unsigned long Adr,int Typ, char Buf[]) 
{
    int Chk;                             /* Pruefsumme */
    int Idx;                             /* Laufindex */

    fprintf(OuF,":%02X%04X%02X",Siz,Adr,Typ);
    Chk=Siz+(Adr & 0xff)+(Adr>>8)+Typ;
    for (Idx=0; Idx<Siz; Idx++)
    {                                    /* Pufferinhalt ausgeben */
        fprintf(OuF,"%02X",(unsigned char)Buf[Idx]);
        Chk+=Buf[Idx];                     /* Pruefsumme mitrechnen */
    }
    fprintf(OuF,"%02X\n",(unsigned char)-Chk); /* Pruefsumme ausgeben */
    if (ferror(OuF))
    {                                    /* Irgend ein Schreibfehler? */
        printf("Failed to write. ");
        return(False);
    }
    else return(True);                   /* Fehlerfrei ausgefuehrt */
}

//-----------------------------------------------------------------------------
// write a file in HEX and get the data from VME
static int _WriteFile()                /* Schreibt eine Datei aus VME */
{
    unsigned long End;                   /* Endadresse */
    unsigned long Idx;                   /* Laufadresse */
    unsigned long Cnt;                   /* Bytezaehler */
    int           Ret;                   /* Returncode */
    int           Adr;                   /* Adresse Record ab Start */
    int           Hex;                   /* Intel Hex File? */
    char          Buf[16];               /* Output-Puffer */
    STRG          Nam;                   /* Dateiname */
    FILE         *OuF;                   /* Lesedatei */

    Ret=1;                               /* Vorgabe ist Fehler */
    if (ParaStr(4)!=NULL)
    {                                    /* Start & Endadr sind obligat */
        Hex=(ParaStr(1)[1]=='X');          /* Intel-Hex gewuenscht? */
        strcpy(Nam,ParaStr(2));            /* Dateinamen kopieren */
        *strchr(Nam,' ')='\0';             /* Restparameter abschneiden */
        Cnt=0;                             /* Noch nichts gelesen */
        Idx=ParaNum(3);                    /* Lege Startadresse fest */
        End=ParaNum(4);                    /* Lege Endadresse fest */
        if (Idx<=End)
        {                                  /* Falsche Werte abweisen */
            if ((OuF=fopen(Nam,(Hex) ? "wt":"wb"))!=NULL)
            {
                if (Hex)
                {                              /* Intel-Hex gewuenscht? */
                    Buf[0]=0 >> 0x8;             /* HighByte Segmentadresse */
                    Buf[1]=0 & 0xff;             /* LowByte Segmentadresse */
                    Adr=0;                       /* Offset grundsaetzlich bei 0 */
                    if (OutHex(OuF,2,Adr,2,Buf))
                    {
                        do 
                        {
                            Buf[(int)Cnt & 0xf]=ReadByte(nInterfaceHandle, Idx,AdrMode);
                            if (GetError(nInterfaceHandle))
                            {                        /* Fehlerpruefung: VME-Transfer ok? */
                                ClearError(nInterfaceHandle);          /* Fehlerflag zuruecksetzen */
                                printf("--> Bus error: Adr=%08lx. ",Idx);
                                break;                 /* Abbruch */
                            } /* if GetError */
                            if (!((int)++Cnt & 0xf))
                            {
                                if (OutHex(OuF,16,Adr,0,Buf)) Adr+=16;
                                else break;            /* Zwischendurch Puffer schreiben */
                            }
                        } while (Idx++<End);       /* Bis einschlieﬂlich End schreiben */

                        if ((Idx==End+1) &&        /* Noch Rest im Puffer? */
                            (!((int)Cnt & 0xf) ||
                             OutHex(OuF,(int)Cnt & 0xf,Adr,0,Buf))
                            && OutHex(OuF,0,0,1,NULL)) Ret=0;
                    } /* if */                   /* Wenn Eof ausgegeben, Returns ok */
                } /* if Hex */
                else
                    do 
                    {                            /* Nicht Intel-Hex */
                        fputc(ReadByte(nInterfaceHandle, Idx,AdrMode),OuF);
                        if (GetError(nInterfaceHandle))
                        {                          /* Fehlerpruefung: VME-Transfer ok? */
                            ClearError(nInterfaceHandle);            /* Fehlerflag zuruecksetzen */
                            printf("--> Bus error: Adr=%08lx. ",Idx);
                            break;                   /* Abbruch */
                        } /* if GetError */
                        if (ferror(OuF))
                        {                          
                            printf("Failed to write. ");
                            break;                   /* Abbruch */
                        } /* if ferror */
                        Cnt++;                     /* Ein Byte mehr geschrieben */
                    } while (Idx++<End);         /* Bis einschlieﬂlich End schreiben */

                if (Idx==End+1) Ret=0;       /* Genug Byte geschafft? */
                fclose(OuF);
            } /* if fopen */
            else
                printf("Can' open file %s. ",Nam);
        } /* if */
        printf("%lx Byte(s) written.\n",Cnt);
    } /* if */
    else
        printf("\a");
    return(Ret);
}


//-----------------------------------------------------------------------------
// show and provide help about the VME address modifiers
static void ShowModifier(int Mode)           /* Klartext fuer Adressmodifier */
{
    printf("Address modifier:\t%02x [", Mode);
    if ((Mode & 0x3b) > 0x38) printf("standard");
    else if ((Mode & 0x3b)==0x29) printf("short");
    else if ((Mode & 0x3b)>=0x09 && (Mode & 0x3b)<=0x0b) printf("extendet");
    else if (Mode>=0x20 || Mode<=0x0f)
    {
        printf("reserved]\n"); return;
    }
    else
    {
        printf("user defined]\n"); return;
    }
    printf(((Mode & 0x0f)>=0x0c) ? " supervisory":" non-privileged");
    switch (Mode & 0x03)
    {
        case 1:printf(" data access"); break;
        case 2:printf(" code access"); break;
        case 3:printf(" block transfer"); break;
    }
    printf("]\n");
}

//-----------------------------------------------------------------------------
// provides some diagnostic information abot interface registers
static void ShowRegister(void)                /* Zeige Inhalt von ? */
{
    char *szInstg;
    char type;

    if ((szInstg = ParaStr(1)) == NULL)
        type = '0';
    else
        type = szInstg[1];

    if (type == 0)
        type = '0';

    GetInterfaceInfo(nInterfaceHandle, type);
}

//-----------------------------------------------------------------------------
// make a random memory test to VME
static void RandomTest(void)
{
    char          DefZug = ' ';          /* Zugriffsart */
    int           Len;                   /* Item Laenge */
    unsigned long Idx;                   /* Index */
    unsigned long End;                   /* Endadresse */
    unsigned long Seed;                  /* initial seed */
    unsigned char Merk_error = 0;        /* Haelt error flag */
    int           Patt;
    unsigned long i;

    unsigned long  lResult;
    unsigned short wResult;
    unsigned char  bResult;
    int            repeats = 0;
    int            Loop = 0;

    if (GetZug(&DefZug)!=' ')
    {
        Len=1;
        switch (GetZug(&DefZug))           /* Zugriffsmodus festlegen */
        {
            case 'L':Len += 2;               /* Auf Long-Adresse biegen */
            case 'W':Len++;                  /* Auf Wort-Adresse biegen */
        }
        Idx=ParaNum(2) & -(long)Len;       /* Adressen geradebiegen */
        End=ParaNum(3);                    /* Endadresse festlegen */
        if (ParaStr(4) == NULL)
            Loop = 1;
        else
            Loop=ParaNum(4);
        if (ParaStr(5) == NULL)
            Seed = 0x1234;
        else
            Seed=ParaNum(5); 

        do
        {
            srand(Seed + repeats);
            i = Idx;
            while ((i <= End) && (!Abbruch))
            {
                Patt = rand();

                switch (DefZug)
                {
                    case 'L':Patt <<= 16;
                        Patt  |= rand();
                        WriteLong(nInterfaceHandle, i, Patt, AdrMode);
                        break;
                    case 'W':WriteWord(nInterfaceHandle, i, (short)Patt, AdrMode);
                        break;
                    case 'B':WriteByte(nInterfaceHandle, i, (char)Patt, AdrMode);
                        break;
                } /* switch */

                if (GetError(nInterfaceHandle))
                {
                    ClearError(nInterfaceHandle);                  /* Fehler abfangen */
                    Merk_error |= 2;
                }

                if ((i & 0xffl)==0)
                {                                /* Ermoegliche Ctrl-C */
                    printf("\r");
                }

                i += Len;
            } /* while */

            // read and compare
            srand(Seed + repeats);
            i = Idx;
            while ((i <= End) && (!Abbruch))
            {
                Patt = rand();

                switch (DefZug)
                {
                    case 'L':lResult = ReadLong(nInterfaceHandle, i, AdrMode);
                        Patt <<= 16;
                        Patt |= rand();
                        if (lResult != (unsigned long)Patt)
                        {
                            printf("Compare Fail 0x%08x w=0x%08lx r=0x%08lx\n", i, Patt, lResult);
                            Merk_error |= 1;
                        }
                        break;
                    case 'W':wResult = ReadWord(nInterfaceHandle, i, AdrMode);
                        if (wResult != (unsigned short)Patt)
                        {
                            printf("Compare Fail 0x%08x w=0x%04x r=0x%04x\n", i, Patt & 0xFFFF, wResult);
                            Merk_error |= 1;
                        }
                        break;
                    case 'B':bResult = ReadByte(nInterfaceHandle, i, AdrMode);
                        if (bResult != (unsigned char)Patt)
                        {
                            printf("Compare Fail 0x%08x w=0x%02x r=0x%02x\n", i, Patt & 0xFF, bResult);
                            Merk_error |= 1;
                        }
                        break;
                } /* switch */

                if (GetError(nInterfaceHandle))
                {
                    ClearError(nInterfaceHandle);                  /* Fehler abfangen */
                    Merk_error |= 2;
                }

                if ((i & 0xffl)==0)
                {                                /* Ermoegliche Ctrl-C */
                    printf("\r");
                }

                i += Len;
            } /* while */

            if (Loop)
                printf("\rRepeats: 0x%x\r", repeats);
            repeats++; 
        } while ((Loop--) && !(Merk_error));

        if (Merk_error)
            printf("--> Compare failed %s\a\n", (Merk_error & 2) ? "with Bus Error" : "");
        else
            printf("--> Compare successfull\n");

    }
    else printf("\a");

}

//-----------------------------------------------------------------------------
// modify interface registers
static void ModifyRegister(void)
{
    __u32 Erg;

    if (ParaStr(2) != NULL)
    {
        if (ParaStr(3) != NULL)
        {
            Erg = _SetRegister(nInterfaceHandle, ParaNum(2),ParaNum(3));
            printf("Interface register @ 0x%08lx set: 0x%02x, get: 0x%02x\n", 
                   ParaNum(2), ParaNum(3), Erg);
        }
        else
        {
            Erg = _GetRegister(nInterfaceHandle, ParaNum(2));
            printf("Interface register @ 0x%08lx get: 0x%02x\n", ParaNum(2), Erg);
        }
    }
    else
        printf("\a");
}

//-----------------------------------------------------------------------------
// the main menu
static int HauptMenue(STRG Stg)        /* Eingabe & Dispatcher */
{
    char *SSt;                           /* Sourcezeiger */
    char *DSt;                           /* Destzeiger */
    char  Del;                           /* Delimiter vorhanden? */
    int   Ret;                           /* Returncode fuer Auto-Mode */
    char  loop;                          /* irgedwie war baengg fuer loop */


    if (Stg == NULL)
        loop = 1;
    else
        loop = 0;

    do 
    {
        if (loop)
        {                                  /* Auto-Modus? */
            if (Abbruch)
            {
                printf("\n");
                Abbruch = 0;
            }

            printf("pv: ");                  /* Nein, Prompt ausgeben und */
            if (*ReadMessageBuffer())
            {
                printf("%s\n", ReadMessageBuffer());
                printf("pv: ");
                InitMessageBuffer();
            }
            _gets(InStg);                     /* Eingabestring holen */
            // GetError(nInterfaceHandle);                      /* because of interrupts */
            SSt=InStg;                       /* Init Sourcezeiger */
        }
        else
            SSt=Stg;                         /* Uebernehme Parameter aus Stg */

        DSt=InStg;                         /* Init Destzeiger */
        Del=True;                          /* Weitere Delimiter raus */
        Ret=0;                             /* Keine Fehler bis jetzt */

        while (*SSt)
        {                                  /* Arbeite String ab */
            if (UpCase(*SSt) >= 'A' &&       /* Filtern gueltiger Zeichen */
                UpCase(*SSt) <= 'Z' ||
                *SSt >= '0'  && *SSt <= '9' ||
                *SSt == ':'  || *SSt == '.' ||
                *SSt == '\\' || *SSt == '?')
            {
                *DSt=UpCase(*SSt);
                Del=False;
                DSt++;
            }
            else
            {
                if (!Del)
                {
                    *DSt=' '; 
                    DSt++; 
                }                              /* Mehrere Delimiter raus */
                Del=True;                      /* und durch ' ' ersetzen */
            }
            SSt++;
        } /* while (*SSt) */
        *DSt=*SSt;                         /* 0 auch uebertragen */

        switch (*ParaStr(1))
        {
            case 'A': SetModifier(); break;
            case 'D': Dump(); break;
            case 'E': Examine(); break;      /* Speicherbereich aendern */
            case 'F': Fill(); break;         /* Speicherbereich fuellen */
            case 'G': RandomTest(); break;   /* random test of memory */
            case 'H':
            case '?': Hilfe(); break;        /* Hilf mir mal */
            case 'I': DeInit_Interface(nInterfaceHandle);
                      InitAt(cszDevicePath, &nInterfaceHandle); 
                      break;                 /* Nochmals initialisieren */
            case 'C': Konfig(); break;       /* Konfiguration */
            case 'L': ReadIrqVect(); break;  /* Interrupt-Vektoren lesen */
            case 'M': Move(); break;         /* Move Funktion */
            case 'O': JumpToDos(); break;    /* DOS Ausgang */
            case 'P': SearchPorts(); break;  /* Ports suchen */
            case 'Q': Raus();return(0);      /* Ende des Debuggers */
            case 'R': Ret=_ReadFile(); break; /* Eine Datei nach VME lesen */
            case 'S': SeekPatt(); break;     /* Suche nach Datenmustern */
            case 'T': TestSet(); break;      /* Fuehre ein TAS aus */
            case 'V': ResetVme(); break;     /* Erzeuge VME-Reset */
            case 'W': Ret=_WriteFile(); break;/* Eine Datei von VME schreiben */
            case 'Y': SysFail(); break;      /* read, set Sysfail */
            case 'X': ModifyRegister(); break; /* modify register of the interface */
            case 'Z': ShowRegister(); break; /* Register ausgeben */
            default : 
                {
                    Ret=2;                /* Fehlercode zurueck fuer Auto */
                    if (!loop)
                    {                     /* Wenn Auto: Hilfsmessage */
                        Hilfe();
                        printf("\nSplit commands with \"/\" ,e.g. \"a39/d1000\"");
                    }
                }
        } /* switch */
    } while (loop);                 /* Hier raus bei Auto-Mode */
    return(Ret);
}

//-------------------------------------------------------------------------------------
// the exit entry
static void MyExit(int bla)            /* Wird im Ctrl-C-Falle aufgerufen */
{
    Abbruch = 1;                       
}

//-------------------------------------------------------------------------------------
// where all starts
int main(int argc, char **argv, char *envp[])
{
    static STRG  Stg;                    /* Zum zusammenlegen Parameter */
    char *PSt;                           /* Arbeitszeiger Multicommands */
    char *SSt;                           /* Quellzeiger Multicommands */
    int   Idx;                           /* Index */
    int   Ret;                           /* Returncode */

    InitMessageBuffer();

    cszDevicePath = &localBuffer[0];

    Ret=1;                               /* Returncode auf Fehler setzen */
    ArgV=argv;                           /* Uebertrage argv fuer LoadKonfig */
    LoadKonfig();                        /* Versuchen Konfigdatei zu lesen */

    if (argc > 1)
    {                                    /* Kommandozeilenparameter da? */
        if (InitAt(cszDevicePath, &nInterfaceHandle))
        {                                  /* Aufsetzen Interface */
            *Stg='\0';                       /* Stg auf nix setzen */
            for (Idx=1; Idx < argc; Idx++)
            {
                strcat(Stg,argv[Idx]);         /* Haenge Parameter hintereinander */
                strcat(Stg," ");               /* Trenne mit Leerzeichen */
            }

            SSt=Stg;                         /* Saubloedes (*Zeug) mit den ARRAYS! */
            do 
            {
                if ((PSt=strchr(SSt,'/'))!=NULL) *PSt='\0';
                Ret=HauptMenue(SSt);           /* Hauptmenue automatisch aufrufen */
                SSt=PSt+1;                     /* SSt auf den Reststring setzen */
            }

            while (PSt!=NULL && !Ret);     /* Bis Fehler oder Fertig */
        }
    }
    else
    {
        printf("Provided under GPL - version %s of pvmon of %s \n\n", VERSION, __DATE__);
        printf("This program is free software;  you can redistribute it and/or modify it\n");
        printf("under the terms of the GPL as published by the FSF (version 2 or later).\n");
        printf("Copyright:  Ralf Dux,  Sven Hannover,  Klaus Hitschler,  Sven Tuecke, AR\n"); 

        InitAt(cszDevicePath, &nInterfaceHandle); /* Aufsetzen Interface */
        signal(SIGINT, MyExit);            /* Eigenen Handler einklinken */
        Ret=HauptMenue(NULL);              /* Hauptmenue manuell aufrufen */
    } /* else */
    DeInit_Interface(nInterfaceHandle);  /* Interface ausschalten */

    return(Ret);                         /* Fehlercode fuer ErrorLevel */
}

//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
