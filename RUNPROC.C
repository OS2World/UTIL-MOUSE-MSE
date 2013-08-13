#define INCL_DOS
#define INCL_WIN
#define INCL_GPI

#define DATAN 1

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "msehook.h"
#include "mse.h"
#include "dialog.h"


MRESULT EXPENTRY RunProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  static char lastrun[CCHMAXPATH] = "",lastargs[1024 - CCHMAXPATH],
              lastdir[CCHMAXPATH];

  switch(msg) {
    case WM_INITDLG:
      if(!*lastrun) {

        ULONG size;

        size = sizeof(lastrun);
        PrfQueryProfileData(prof,
                            appname,
                            "LastRun",
                            lastrun,
                            &size);
        *lastdir = *lastargs = 0;
      }
      if(*lastrun &&
         !IsInValidDir(lastrun))
        *lastrun = 0;
      if(*lastdir &&
         IsFile(lastdir))
        *lastdir = 0;
      WinSendDlgItemMsg(hwnd,
                        RUN_NAME,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(CCHMAXPATH,0),
                        MPVOID);
      WinSendDlgItemMsg(hwnd,
                        RUN_DIR,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(CCHMAXPATH,0),
                        MPVOID);
      WinSendDlgItemMsg(hwnd,
                        RUN_ARGS,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(1024 - CCHMAXPATH,0),
                        MPVOID);
      WinSetDlgItemText(hwnd,
                        RUN_NAME,
                        lastrun);
      WinSetDlgItemText(hwnd,
                        RUN_ARGS,
                        lastargs);
      WinSetDlgItemText(hwnd,
                        RUN_DIR,
                        lastdir);
      WinSendDlgItemMsg(hwnd,
                        RUN_NAME,
                        EM_SETSEL,
                        MPFROM2SHORT(0,CCHMAXPATH),
                        MPVOID);
      WinSendDlgItemMsg(hwnd,
                        RUN_DIR,
                        EM_SETSEL,
                        MPFROM2SHORT(0,CCHMAXPATH),
                        MPVOID);
      WinSendDlgItemMsg(hwnd,
                        RUN_ARGS,
                        EM_SETSEL,
                        MPFROM2SHORT(0,1024),
                        MPVOID);
      if(!*lastrun)
        WinEnableWindow(WinWindowFromID(hwnd,DID_OK),
                        FALSE);
      PostMsg(hwnd,
              UM_SETUP,
              MPVOID,
              MPVOID);
      if(hptrApp)
        WinSendMsg(hwnd,
                   WM_SETICON,
                   MPFROMLONG(hptrApp),
                   MPVOID);
      break;

    case UM_SETUP:
      {
        char  name[CCHMAXPATH];
        ULONG apptype;

        *name = 0;
        WinQueryDlgItemText(hwnd,
                            RUN_NAME,
                            CCHMAXPATH,
                            name);
        lstrip(rstrip(name));
        if(*name) {
          if(!DosQueryAppType(name,&apptype) &&
            !(apptype & (FAPPTYP_DLL     | FAPPTYP_PHYSDRV |
                         FAPPTYP_PROTDLL | FAPPTYP_VIRTDRV))) {
            WinEnableWindow(WinWindowFromID(hwnd,RUN_DIR),TRUE);
            WinEnableWindow(WinWindowFromID(hwnd,RUN_DIRHDR),TRUE);
            WinEnableWindow(WinWindowFromID(hwnd,RUN_ARGS),TRUE);
            WinEnableWindow(WinWindowFromID(hwnd,RUN_ARGSHDR),TRUE);
            break;
          }
        }
        WinSetDlgItemText(hwnd,RUN_DIR,"");
        WinSetDlgItemText(hwnd,RUN_ARGS,"");
        WinEnableWindow(WinWindowFromID(hwnd,RUN_DIR),FALSE);
        WinEnableWindow(WinWindowFromID(hwnd,RUN_DIRHDR),FALSE);
        WinEnableWindow(WinWindowFromID(hwnd,RUN_ARGS),FALSE);
        WinEnableWindow(WinWindowFromID(hwnd,RUN_ARGSHDR),FALSE);
      }
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case RUN_NAME:
          switch(SHORT2FROMMP(mp1)) {
            case EN_KILLFOCUS:
              WinSendMsg(hwnd,
                         UM_SETUP,
                         MPVOID,
                         MPVOID);
              break;
            case EN_CHANGE:
              {
                char test[CCHMAXPATH];

                *test = 0;
                WinQueryDlgItemText(hwnd,
                                    RUN_NAME,
                                    CCHMAXPATH,
                                    test);
                lstrip(rstrip(test));
                WinEnableWindow(WinWindowFromID(hwnd,DID_OK),(*test != 0));
              }
              break;
          }
          break;
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case DID_OK:
          {
            char build[1024],name[CCHMAXPATH];

            *name = 0;
            WinQueryDlgItemText(hwnd,
                                RUN_NAME,
                                CCHMAXPATH,
                                name);
            lstrip(rstrip(name));
            if(!*name) {
              WinSetDlgItemText(hwnd,
                                RUN_NAME,
                                "");
              WinEnableWindow(WinWindowFromID(hwnd,DID_OK),FALSE);
              WinSetFocus(HWND_DESKTOP,
                          WinWindowFromID(hwnd,RUN_NAME));
              DosBeep(50,100);
              break;
            }
            strcpy(lastrun,name);
            SavePrf("LastRun",
                    lastrun,
                    strlen(lastrun));
            WinSetDlgItemText(hwnd,
                              RUN_NAME,
                              lastrun);
            *lastargs = 0;
            *lastdir = 0;
            WinQueryDlgItemText(hwnd,RUN_ARGS,1024 - CCHMAXPATH,lastargs);
            WinQueryDlgItemText(hwnd,RUN_DIR,CCHMAXPATH,lastdir);
            sprintf(build,"OPEN=DEFAULT%s%s%s%s",
                    (*lastargs) ? ";PARAMETERS=" : "",
                    lastargs,
                    (*lastdir) ? ";STARTDIR=" : "",
                    lastdir);
            OpenObject(lastrun,
                       build);
          }
          WinDismissDlg(hwnd,1);
          break;

        case RUN_FIND:
          {
            FILEDLG fdlg;
            char    drive[3] = " :",*pdrive = drive,*p;
            ULONG   apptype = 0;
            SWP     swp;
            POINTL  ptl;

            memset(&fdlg,0,sizeof(FILEDLG));
            fdlg.cbSize =       (ULONG)sizeof(FILEDLG);
            fdlg.fl     =       FDS_CENTER | FDS_OPEN_DIALOG;
            fdlg.pszTitle =     "Select something to run/open";
            fdlg.pszOKButton =  "Okay";

            if(isalpha(*lastrun))
              *drive = toupper(*lastrun);
            else {
              if(!DosQuerySysInfo(QSV_BOOT_DRIVE,
                                  QSV_BOOT_DRIVE,
                                  (PVOID)&apptype,
                                  sizeof(apptype)))
                *drive = apptype + '@';
              else
                *drive = 'C';
            }
            fdlg.pszIDrive = pdrive;
            strcpy(fdlg.szFullFile,lastrun);
            p = strrchr(fdlg.szFullFile,'\\');
            if(p) {
              p++;
              *p = 0;
            }
            else
              strcat(fdlg.szFullFile,"\\");
            strcat(fdlg.szFullFile,"*");
            if(*lastrun) {
              p = strrchr(lastrun,'\\');
              if(p) {
                p = strrchr(p,'.');
                if(p) {
                  if(*(p + 1))
                    strcat(fdlg.szFullFile,p);
                }
              }
            }
            if(WinFileDlg(HWND_DESKTOP,
                          hwnd,
                          &fdlg)) {
              if(fdlg.lReturn != DID_CANCEL &&
                 !fdlg.lSRC) {
                WinSetDlgItemText(hwnd,
                                  RUN_NAME,
                                  fdlg.szFullFile);
                strcpy(lastrun,fdlg.szFullFile);
                if(!DosQueryAppType(fdlg.szFullFile,
                                    &apptype) &&
                  !(apptype & (FAPPTYP_DLL     | FAPPTYP_PHYSDRV |
                               FAPPTYP_PROTDLL | FAPPTYP_VIRTDRV))) {
                  p = fdlg.szFullFile;
                  while(*p) {
                    if(*p == '/')
                      *p = '\\';
                    p++;
                  }
                  p = strrchr(fdlg.szFullFile,'\\');
                  if(p)
                    *p = 0;
                  WinSetDlgItemText(hwnd,
                                    RUN_DIR,
                                    fdlg.szFullFile);
                  WinSendDlgItemMsg(hwnd,
                                    RUN_DIR,
                                    EM_SETSEL,
                                    MPFROM2SHORT(0,CCHMAXPATH),
                                    MPVOID);
                  if(p)
                    *p = '\\';
                  WinSetFocus(HWND_DESKTOP,
                              WinWindowFromID(hwnd,RUN_ARGS));
                }
                else
                  WinSetFocus(HWND_DESKTOP,
                              WinWindowFromID(hwnd,RUN_NAME));
                if(fDefButton &&
                   WinQueryWindowPos(WinWindowFromID(hwnd,DID_OK),&swp)) {
                  ptl.x = swp.x + (swp.cx / 2);
                  ptl.y = swp.y + (swp.cy / 2);
                  WinMapWindowPoints(hwnd,HWND_DESKTOP,&ptl,1L);
                  WinSetPointerPos(HWND_DESKTOP,ptl.x,ptl.y);
                }
                p = strrchr(fdlg.szFullFile,'\\');
                if(p) {
                  if(p <= fdlg.szFullFile + 2)
                    p++;
                  WinSendDlgItemMsg(hwnd,
                                    RUN_NAME,
                                    EM_SETSEL,
                                    MPFROM2SHORT(((p) ? p -
                                                  fdlg.szFullFile : 0),
                                                 CCHMAXPATH),
                                    MPVOID);
                }
                WinSendMsg(hwnd,
                           UM_SETUP,
                           MPVOID,
                           MPVOID);
              }
            }
          }
          break;

        case DID_CANCEL:
          WinDismissDlg(hwnd,0);
          break;
      }
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}

