/*-----------------------------
   XTIME.C -- Execute Command at a certain time. 
  -----------------------------*/
#define INCL_DOSSESMGR
#define INCL_DOSFILEMGR
#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#include <os2.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "xtime.h"

MRESULT EXPENTRY ClientWndProc (HWND, ULONG, MPARAM, MPARAM) ;
VOID    SizeTheWindow (HWND) ;
INT     Params (int, char **) ;

CHAR     Process_Executed  = FALSE ;
CHAR     Process_Slayed = FALSE ;

TIMEINFO start_time ;

PULONG idSession, id ;

STARTDATA scntl = {
       50,                        /* Length               */
       FALSE,                     /* Related              */
       FALSE,                     /* FgBg                 */
       FALSE,                     /* TraceOpt             */
       (unsigned char *) "",                        /* PgmTitle             */
       (unsigned char *) "",                        /* PgmName              */
       (unsigned char *) "",                        /* PgmInputs            */
       (unsigned char *) "",                        /* TermQ                */
       0,                         /* Environment          */
       FALSE,                     /* InheritOpt           */
       0,                         /* SessionType          */
       (unsigned char *) "",                        /* IconFile             */
       0,                         /* PgmHandle            */
       0x0000,                    /* PgmControl           */
       0,0,                       /* InitXPos, InitYPos   */
       0,0                        /* InitXSize, InitYSize */
       };


int main (int argc, char *argv[])
     {
     static CHAR  szClientClass[] = "XTIME" ;
     static ULONG flFrameFlags = FCF_TITLEBAR | FCF_SYSMENU  |
                                 FCF_BORDER   | FCF_TASKLIST ;
     HAB          hab ;
     HMQ          hmq ;
     HWND         hwndFrame, hwndClient ;
     QMSG         qmsg ;
     int          exitrc ;
     char         szErrorMsg[32];

     hab = WinInitialize (0) ;
     hmq = WinCreateMsgQueue (hab, 0) ;

     WinRegisterClass (hab, (PCSZ) szClientClass, ClientWndProc, 0L, 0) ;

     hwndFrame = WinCreateStdWindow (HWND_DESKTOP, WS_VISIBLE,
                                     &flFrameFlags, (PCSZ) szClientClass, NULL,
                                     0L, NULLHANDLE, 0, &hwndClient) ;
     SizeTheWindow (hwndFrame) ;

     if (WinStartTimer (hab, hwndClient, ID_TIMER, 1000))
          {
          if ((exitrc = Params(argc, &argv[0])) == NO_ERROR)
              while (WinGetMsg (hab, &qmsg, NULLHANDLE, 0, 0))
                    WinDispatchMsg (hab, &qmsg) ;
          else {
               if (exitrc == PROGRAM_NOT_FOUND){
                  sprintf(szErrorMsg, "Unable to locate Program");
				}
               if (exitrc == SEMANTIC_ERROR){
                  sprintf (szErrorMsg, "Start Session Options invalid or program name not specified");
                  }
				WinMessageBox (HWND_DESKTOP, hwndClient, (PCSZ) "Some Duffus Error", 
                              (PCSZ) szClientClass, 0, MB_OK | MB_ICONEXCLAMATION);
               }
          WinStopTimer (hab, hwndClient, ID_TIMER) ;
          }
     else
          WinMessageBox (HWND_DESKTOP, hwndClient,
                         (PCSZ) "Too many clocks or timers",
                         (PCSZ) szClientClass, 0, MB_OK | MB_ICONEXCLAMATION) ;

     WinDestroyWindow (hwndFrame) ;
     WinDestroyMsgQueue (hmq) ;
     WinTerminate (hab) ;
     return 0 ;
     }

VOID SizeTheWindow (HWND hwndFrame)
     {
     FONTMETRICS fm ;
     HPS         hps ;
     RECTL       rcl ;

     hps = WinGetPS (hwndFrame) ;
     GpiQueryFontMetrics (hps, (LONG) sizeof fm, &fm) ;
     WinReleasePS (hps) ;

     rcl.yBottom = 0 ;
     rcl.yTop    = 11 * fm.lMaxBaselineExt / 4 ;
     rcl.xRight  = WinQuerySysValue (HWND_DESKTOP, SV_CXSCREEN) ;
     rcl.xLeft   = rcl.xRight - 20 * fm.lEmInc ;

     WinCalcFrameRect (hwndFrame, &rcl, FALSE) ;

     WinSetWindowPos (hwndFrame, NULLHANDLE, (LONG) rcl.xLeft, (LONG) rcl.yBottom,
                      (LONG) (rcl.xRight - rcl.xLeft),
                      (LONG) (rcl.yTop - rcl.yBottom), SWP_SIZE | SWP_MOVE) ;
     }


int Params(int argc, char *argv[])
     {
     static CHAR PgmArgs[255]  = "",
                 PgmDrive[6]   = "",
                 PgmDir[255]   = "",
                 PgmName[10]   = "",
                 PgmExt[8]     = "",
                 IconName[255] = "",
                 PgmFName[255] = "",
                 PgmTitle[32]  = "",
                 PgmTime[32]   = "";


     ULONG i,
            hh, mm,
            name_found,
            time_found,
            stype_found,
            title_found,
            exec_cmd,
            retain_cmd,
            SearchCode,
            SearchCnt;

     CHAR SearchName[14],
          SearchPath[255];

     HDIR        hdir = HDIR_SYSTEM;
     FILEFINDBUF FFbufr;

     if (argc <= 1) {
        return SEMANTIC_ERROR; 
        }

     name_found = time_found = stype_found = title_found = exec_cmd = retain_cmd = FALSE;

     /* go through all arguments */
  
     for (i = 1; i < argc; i++) {
 
       /* if argument has a / or a - before it process here */
       if (argv[i][0] == '/' || argv[i][0] == '-') {

          if (0 == strnicmp(_PARM(i), "TIT:", 4))
              strncpy(PgmTitle, &argv[i][5], sizeof(PgmTitle));

          else if (0 == strnicmp(_PARM(i), "TIME:", 5)) {
                  strncpy(PgmTime, &argv[i][6], sizeof(PgmTime));
                  time_found = TRUE;
                  }

          else if (0 == stricmp(_PARM(i), "BG"))
                  scntl.FgBg = TRUE;

          else if (0 == stricmp(_PARM(i), "FS")) {
                  if (stype_found)
                     continue;
                  scntl.SessionType = 1;
                  stype_found = TRUE;
                  }

          else if (0 == stricmp(_PARM(i), "WIN")) {
                  if (stype_found)
                     continue;
                  scntl.SessionType = 2;
                  stype_found = TRUE;
                  }

          else if (0 == stricmp(_PARM(i), "PM")) {
                  if (stype_found)
                     continue;
                  scntl.SessionType = 3;
                  stype_found = TRUE;
                  }
                          
          else if (0 == stricmp(_PARM(i), "MIN"))
                  scntl.PgmControl = (scntl.PgmControl & 0x8008) | 0x0004;

          else if (0 == stricmp(_PARM(i), "MAX"))
                  scntl.PgmControl = (scntl.PgmControl & 0x8008) | 0x0002;

          else if (0 == stricmp(_PARM(i), "NOC"))
            scntl.PgmControl = scntl.PgmControl | 0x0008;

          else if (0 == stricmp(_PARM(i), "INV"))
            scntl.PgmControl = 0x0001;

          else if (0 == stricmp(_PARM(i), "I"))
            scntl.InheritOpt = TRUE;

          else if (0 == stricmp(_PARM(i), "CMD"))
            exec_cmd = TRUE;

          else if (0 == stricmp(_PARM(i), "K"))
            retain_cmd = TRUE;

          else if (0 == stricmp(_PARM(i), "TRACE"))
            scntl.TraceOpt = TRUE;

          else  return SEMANTIC_ERROR;
          
       }
       /* if no / or - before parameter it is the program name */
       else {
           _splitpath(argv[i], PgmDrive, PgmDir, PgmName, PgmExt);
           name_found = TRUE;
           break;
           }
     }
/*end of command line parameters*/

     if (!name_found)
        return SEMANTIC_ERROR;

     if (sscanf(PgmTime, "%lu:%lu", &hh, &mm) == 2)
        if ((hh >= 0) && (mm >= 0) && (hh < 24) && (mm < 60))
           {
           start_time.hours = hh;
           start_time.mins  = mm;
           }
        else return SEMANTIC_ERROR;
     else return SEMANTIC_ERROR;

     /* Search for Program */
     if (PgmDir[0]) {
        strcpy(SearchPath, PgmDrive);
        strcat(SearchPath, PgmDir);
        SearchCode = SEARCH_PATH;
        }

     else {

       strcpy(SearchPath, "PATH");
       SearchCode = SEARCH_ENVIRONMENT + SEARCH_CUR_DIRECTORY;
       }

     strcpy(SearchName, PgmName);

     if (PgmExt[0])
        strcat(SearchName, PgmExt);
     else
        strcat(SearchName, ".*" );

     memset(PgmFName, '\0', sizeof(PgmFName));

     DosSearchPath(SearchCode,
                   (PCSZ) SearchPath,
                   (PCSZ) SearchName,
                   (PBYTE) PgmFName,
                   sizeof(PgmFName));

     if (PgmFName[0]) {
        if (!PgmExt[0]) {

           SearchCnt = 1;
           _splitpath(PgmFName, PgmDrive, PgmDir, PgmName, PgmExt);
           strcpy(PgmExt, ".COM");
           _makepath(PgmFName, PgmDrive, PgmDir, PgmName, PgmExt);
           hdir = HDIR_SYSTEM;

           if (DosFindFirst((PCSZ)PgmFName,
                            &hdir,
                            FILE_NORMAL + FILE_READONLY + FILE_ARCHIVED,
                            &FFbufr,
                            sizeof(FFbufr),
                            &SearchCnt,
                            0L)) {

              strcpy(PgmExt, ".CMD");
              _makepath(PgmFName, PgmDrive, PgmDir, PgmName, PgmExt);
              SearchCnt = 1;
              hdir = HDIR_SYSTEM;

              if (DosFindFirst((PCSZ)PgmFName,
                               &hdir,
                               FILE_NORMAL + FILE_READONLY + FILE_ARCHIVED,
                               &FFbufr,
                               sizeof(FFbufr),
                               &SearchCnt,
                               0L)) {
  
                 strcpy(PgmExt, ".EXE");
                 _makepath(PgmFName, PgmDrive, PgmDir, PgmName, PgmExt);
                 SearchCnt = 1;
                 hdir = HDIR_SYSTEM;

                 if (DosFindFirst((PCSZ)PgmFName,
                                  &hdir,
                                  FILE_NORMAL + FILE_READONLY + FILE_ARCHIVED,
                                  &FFbufr,
                                  sizeof(FFbufr),
                                  &SearchCnt,
                                  0L))
                    return PROGRAM_NOT_FOUND;
                 }
              }
           }
        }
        else if (exec_cmd) {
                strcpy(PgmName, argv[i]);
                strcpy(PgmFName, argv[i]);
                }
             else 
                return PROGRAM_NOT_FOUND;

     if (strcmp(PgmExt, ".CMD") == 0 || exec_cmd) {

        if (scntl.SessionType == 3)
            scntl.SessionType = 2;
        scntl.PgmName = (PSZ) "CMD.EXE";
        strcpy(PgmArgs, (retain_cmd ? "/K " : "/c "));
        strcat(PgmArgs, PgmFName);
        strcat(PgmArgs, " ");
        }
     else
        scntl.PgmName = (PSZ) PgmFName;

     for (i++; i < argc; i++) {

         strcat(PgmArgs, argv[i]);
         strcat(PgmArgs, " ");
         }

     scntl.PgmInputs = (PBYTE) PgmArgs;

     if (PgmTitle[0])
        scntl.PgmTitle = (PSZ) PgmTitle;
     else
        scntl.PgmTitle = scntl.PgmName;


     strcpy(SearchName, PgmName);
     strcat(SearchName, ".ICO");

     memset(IconName, '\0', sizeof(IconName));

     DosSearchPath(SearchCode,
                   (PCSZ) SearchPath,
                   (PCSZ) SearchName,
                   (PBYTE) IconName,
                   sizeof(IconName));
           
     scntl.IconFile = (PSZ) IconName;
     
     return 0;
     }



VOID UpdateTime (HWND hwnd, HPS hps)
     {
     static BOOL        fHaveCtryInfo = FALSE ;
     static CHAR        *szDayName [] = { "Sun", "Mon", "Tue", "Wed",
                                          "Thu", "Fri", "Sat" } ;
     static CHAR        szDateFormat [] = " %s  %d%s%02d%s%02d " ;
     //static COUNTRYCODE ctryc = { 0, 0 } ;
     static COUNTRYINFO ctryi ;
     int                erc ;
     CHAR               szBuffer [20] ;
     DATETIME           dt ;
     RECTL              rcl ;
     //ULONG              usDataLength ;

               /*----------------------------------------
                  Get Country Information, Date and Time
                 ----------------------------------------*/

     if (!fHaveCtryInfo)
          {
          //DosGetCtryInfo (sizeof ctryi, &ctryc, &ctryi, &usDataLength) ;
          fHaveCtryInfo = TRUE ;
          }
     DosGetDateTime (&dt) ;
     if ((dt.hours == (UCHAR)start_time.hours) &&
        (dt.minutes == (UCHAR)start_time.mins))
        {
        if ( !Process_Executed ) {
           Process_Executed = TRUE ;
           GpiErase(hps) ;
           sprintf(szBuffer, "Executing ....") ;
           WinQueryWindowRect (hwnd, &rcl) ;
           rcl.yBottom += 5 * rcl.yTop / 11 ;
           WinDrawText (hps, -1, (PCCH) szBuffer, &rcl, CLR_NEUTRAL, CLR_BACKGROUND,
                        DT_CENTER | DT_VCENTER) ;
/* Execute */
           if ((erc = DosStartSession( &scntl, idSession, id)) != NO_ERROR) {
              sprintf(szBuffer, "Dos Start Session Ended with Error %d", erc);

              /* need to add for szClientClass */
              WinMessageBox (HWND_DESKTOP, hwnd, (PCSZ) szBuffer,
                            (PCSZ) "szClientClass", 0, MB_OK | MB_ICONEXCLAMATION) ;
              }
           }
        }

     dt.year %= 100 ;

               /*------------- 
                  Format Date
                 -------------*/
                                   /*-----------------
                                      mm/dd/yy format
                                     -----------------*/
     if (ctryi.fsDateFmt == 0)

          sprintf (szBuffer, szDateFormat, szDayName [dt.weekday],
                             dt.month, ctryi.szDateSeparator,
                             dt.day,   ctryi.szDateSeparator, dt.year) ;

                                   /*-----------------
                                      dd/mm/yy format
                                     -----------------*/
     else if (ctryi.fsDateFmt == 1)

          sprintf (szBuffer, szDateFormat, szDayName [dt.weekday],
                             dt.day,   ctryi.szDateSeparator,
                             dt.month, ctryi.szDateSeparator, dt.year) ;

                                   /*-----------------
                                      yy/mm/dd format
                                     -----------------*/
     else
          sprintf (szBuffer, szDateFormat, szDayName [dt.weekday],
                             dt.year,  ctryi.szDateSeparator,
                             dt.month, ctryi.szDateSeparator, dt.day) ;

               /*-------------- 
                  Display Date
                 --------------*/

     WinQueryWindowRect (hwnd, &rcl) ;
     rcl.yBottom += 5 * rcl.yTop / 11 ;
/*     WinDrawText (hps, -1, szBuffer, &rcl, CLR_NEUTRAL, CLR_BACKGROUND,
                  DT_CENTER | DT_VCENTER) ; */

               /*------------- 
                  Format Time
                 -------------*/
                                   /*----------------
                                      12-hour format
                                     ----------------*/
     if ((ctryi.fsTimeFmt & 1) == 0)

          sprintf (szBuffer, " %d%s%02d%s%02d %cm ",
                             (dt.hours + 11) % 12 + 1, ctryi.szTimeSeparator,
                             dt.minutes, ctryi.szTimeSeparator,
                             dt.seconds, dt.hours / 12 ? 'p' : 'a') ;

                                   /*----------------
                                      24-hour format
                                     ----------------*/
     else
          sprintf (szBuffer, " %02d%s%02d%s%02d ",
                             dt.hours,   ctryi.szTimeSeparator,
                             dt.minutes, ctryi.szTimeSeparator, dt.seconds) ;

               /*-------------- 
                  Display Time
                 --------------*/

     WinQueryWindowRect (hwnd, &rcl) ;
     rcl.yTop -= 5 * rcl.yTop / 11 ;
     WinDrawText (hps, -1, (PCSZ) szBuffer, &rcl, CLR_NEUTRAL, CLR_BACKGROUND,
                  DT_CENTER | DT_VCENTER) ;
             
               /*-------------------------
                  Display Execute Program
                 -------------------------*/
     if (Process_Executed)
        sprintf(szBuffer, "Program ID %ln Executed.", idSession ) ;
     else
        sprintf (szBuffer, "%s", scntl.PgmName) ;
     WinQueryWindowRect (hwnd, &rcl) ;
     rcl.yBottom += 5 * rcl.yTop /11 ;
     WinDrawText (hps, -1, (PCCH) szBuffer, &rcl, CLR_NEUTRAL, CLR_BACKGROUND,
                  DT_CENTER | DT_VCENTER) ;

     
     }

MRESULT EXPENTRY ClientWndProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
     {
     HPS  hps;

     switch (msg)
          {
          case WM_CREATE:
               return 0 ;

          case WM_TIMER:
               hps = WinGetPS (hwnd) ;
               GpiSetBackMix (hps, BM_OVERPAINT) ;

               UpdateTime (hwnd, hps) ;

               WinReleasePS (hps) ;
               return 0 ;

          case WM_PAINT:
               hps = WinBeginPaint (hwnd, NULLHANDLE, NULL) ;
               GpiErase (hps) ;

               UpdateTime (hwnd, hps) ;

               WinEndPaint (hps) ;
               return 0 ;
          }
     return WinDefWindowProc (hwnd, msg, mp1, mp2) ;
     }
