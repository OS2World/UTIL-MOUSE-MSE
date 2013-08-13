/****************************************************************************
    MSE is a WPS enhancement utility
    copyright (c) 2001 by Mark Kimes

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*****************************************************************************/

#define INCL_WIN
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_GPI

#define DEFINE_GLOBALS 1
#define DATAS 1

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "msehook.h"
#include "mse.h"
#include "dialog.h"
#include "version.h"


static void APIENTRY CleanUp (ULONG why) {

  if(prof)
    PrfCloseProfile(prof);
  DosSleep(100);
  if(hookset) {
    StopHook();
    hookset = FALSE;
  }
  DosExitList(EXLST_REMOVE,
              CleanUp);
}


int main (int argc,char *argv[]) {

  HAB    hab;
  HMQ    hmq;
  int    x;
  BOOL   temp;

  MaxCount = 1;
  numtext = 20;
  scrnbitcount = 8;
  lMaxRunahead = 4;
  {
    ULONG value;

    if(!DosQuerySysInfo(QSV_VERSION_MINOR,
                        QSV_VERSION_MINOR,
                        &value,
                        sizeof(value)) &&
       value <= 30)
      oldstyle = TRUE;
  }
  hab = WinInitialize(0);
  if(hab) {
    hmq = WinCreateMsgQueue(hab,728);
    if(hmq) {
      if(argc > 1 &&
         !stricmp(argv[1],"/k")) {  /* kill running instance */
        if(hwndConfig)
          PostMsg(hwndConfig,
                  WM_COMMAND,
                  MPFROM2SHORT(MSE_EXIT2,0),
                  MPVOID);
        goto Abort;
      }
      if(hwndConfig) { /* show existing instance rather than start a new one */

        static HWND hwndShow;
        static SWP  swp;

        hwndShow = hwndConfig;
        if(argc > 1 &&
           !stricmp(argv[1],"/c") &&
           ClipHwnd) /* show clip mgr */
          hwndShow = WinQueryWindow(ClipHwnd,QW_PARENT);
        NormalizeWindow(hwndShow,TRUE);
        WinQueryWindowPos(hwndShow,&swp);
        if(swp.fl & (SWP_MINIMIZE | SWP_HIDE)) {
          WinSendMsg(hwndShow,
                     WM_SYSCOMMAND,
                     MPFROM2SHORT(SC_RESTORE,0),
                     MPVOID);
          WinShowWindow(hwndShow,TRUE);
        }
        else
          WinSetWindowPos(hwndShow,
                          HWND_TOP,
                          swp.x,
                          swp.y,
                          swp.cx,
                          swp.cy,
                          SWP_MOVE | SWP_SHOW | SWP_SIZE | SWP_ACTIVATE |
                          SWP_ZORDER);
        WinSetFocus(HWND_DESKTOP,
                    hwndShow);
        goto Abort;
      }
      temp = fSuspend;
      fSuspend = TRUE;
      for(x = 1;x < argc;x++) {
        switch(*argv[x]) {
          case '/':
          case '-':
            switch(argv[x][1]) {
              case '3':
                oldstyle = (oldstyle) ? FALSE : TRUE;
                break;

              case 'd':   /* startup delay */
              case 'D':
                {
                  long t = atol(&argv[x][2]);

                  if(t > 1 && t < 100)
                    DosSleep(t * 1000);
                }
                break;
              case 'h':   /* hide on startup */
              case 'H':
                minimizeme = TRUE;
                break;
              case 'n':   /* set number of clipboards to save */
              case 'N':
                numtext = atol(&argv[x][2]);
                if(numtext < 5)
                  numtext = 5;
                if(numtext > 99)
                  numtext = 99;
                break;
              case 'u':   /* update */
              case 'U':
                if(hwndCover)
                  PostMsg(hwndCover,
                          WM_COMMAND,
                          MPFROM2SHORT(MSE_RELOAD,0),
                          MPVOID);
                fSuspend = temp;
                goto Abort;
            }
            break;
        }
      }
      InitVars();
      yIcon = WinQuerySysValue(HWND_DESKTOP,
                               SV_CYICON);
      LoadPrf();
      ReadSchemes();
      {
        CLASSINFO clinfo;

        WinQueryClassInfo(hab,
                          WC_STATIC,
                          &clinfo);
        PFNWPStatic = clinfo.pfnWindowProc;
        WinQueryClassInfo(hab,
                          WC_TITLEBAR,
                          &clinfo);
        PFNWPTitle = clinfo.pfnWindowProc;
        WinQueryClassInfo(hab,
                          WC_FRAME,
                          &clinfo);
        PFNWPFrame = clinfo.pfnWindowProc;
        WinQueryClassInfo(hab,
                          WC_MENU,
                          &clinfo);
        PFNWPMenu = clinfo.pfnWindowProc;
      }
      x = InitDLL(hab,
                  prof,
                  VERMAJOR,
                  VERMINOR);
      if(x == 1) {
        StartObjWins();
        WinRegisterClass(hab,
                         clipwinclass,
                         ClipProc,
                         CS_SIZEREDRAW,
                         sizeof(PVOID));
        WinRegisterClass(hab,
                         deskwinclass,
                         OviewProc,
                         CS_SIZEREDRAW,
                         sizeof(PVOID));
        WinRegisterClass(hab,
                         desknoteclass,
                         DeskProc,
                         CS_SIZEREDRAW | CS_SYNCPAINT,
                         sizeof(PVOID));
        WinRegisterClass(hab,
                         monwinclass,
                         SwapProc,
                         CS_SIZEREDRAW | CS_SYNCPAINT,
                         sizeof(PVOID));
        if(StartHook()) {
          hookset = TRUE;
          DosExitList(EXLST_ADD,
                      CleanUp);
          WinDlgBox(HWND_DESKTOP,
                    HWND_DESKTOP,
                    MainProc,
                    0,
                    NTE_FRAME,
                    &hab);
          if(hookset) {
            StopHook();
            hookset = FALSE;
          }
          if(ObjectHwnd) {
            if(!PostMsg(ObjectHwnd,
                        WM_QUIT,
                        MPVOID,
                        MPVOID))
              WinSendMsg(ObjectHwnd,
                         WM_QUIT,
                         MPVOID,
                         MPVOID);
          }
          if(GObjectHwnd) {
            if(!PostMsg(GObjectHwnd,
                        WM_QUIT,
                        MPVOID,
                        MPVOID))
              WinSendMsg(GObjectHwnd,
                         WM_QUIT,
                         MPVOID,
                         MPVOID);
          }
          if(ObjectHwnd2) {
            if(!PostMsg(ObjectHwnd2,
                        WM_QUIT,
                        MPVOID,
                        MPVOID))
              WinSendMsg(ObjectHwnd2,
                         WM_QUIT,
                         MPVOID,
                         MPVOID);
          }
          if(ObjectHwnd3) {
            if(!PostMsg(ObjectHwnd3,
                        WM_QUIT,
                        MPVOID,
                        MPVOID))
              WinSendMsg(ObjectHwnd3,
                         WM_QUIT,
                         MPVOID,
                         MPVOID);
          }
          if(ObjectHwnd4) {
            if(!PostMsg(ObjectHwnd4,
                        WM_QUIT,
                        MPVOID,
                        MPVOID))
              WinSendMsg(ObjectHwnd4,
                         WM_QUIT,
                         MPVOID,
                         MPVOID);
          }
          if(fDesktops &&
             !fNoNormalize)
            NormalizeAll();
          if(ulPointerHidden)
            WinShowPointer(HWND_DESKTOP,
                           TRUE);
          WinShowPointer(HWND_DESKTOP,
                         TRUE);
          DosSleep(1000);
          CleanUp(0);
          {
            int y = 360;

            while(ulNumFileDlg &&
                  --y)
              DosSleep(1000);
          }
          if(ulNumFileDlg) {

            HENUM henum;
            HWND  hwndChild;
            char  Class[4];

            henum = WinBeginEnumWindows(HWND_DESKTOP);
            while((hwndChild = WinGetNextWindow(henum)) != NULLHANDLE) {
              WinQueryClassName(hwndChild,
                                sizeof(Class),
                                (PCH)Class);
              if(!strcmp(Class,"#1") &&
                 WinQueryWindowPtr(hwndChild,QWP_PFNWP) ==
                  (PVOID)MyFileDlgProc)
                WinSendMsg(hwndChild,
                           WM_CLOSE,
                           MPVOID,
                           MPVOID);
            }
            WinEndEnumWindows(henum);
          }
        }
        else
          WinMessageBox(HWND_DESKTOP,
                        HWND_DESKTOP,
                        "MSE couldn't set its hooks -- aborting.",
                        "MSE hook error",
                        0,
                        MB_CANCEL | MB_ICONEXCLAMATION |
                        MB_MOVEABLE);
      }
Abort:
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}

