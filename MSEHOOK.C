/****************************************************************************
    msehook.h - msehook definitions

    Copyright (c) 1997, 2001 by Mark Kimes
    Copyright (c) 2001 by Steven Levine and Associates, Inc.

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

Revisions	19 Aug 01 MK - Baseline
		25 Sep 01 SHL - bypass xor.h

*****************************************************************************/

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_DOSERRORS

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "msehook.h"
//#include "xor.h"
#include "version.h"
#include "dialog.h"

#pragma data_seg(SHAREDATA)

#define NUMRECENT 25

/***********************************************************************/
/*  Global variables.                                                  */
/***********************************************************************/
char          viewer[CCHMAXPATH],editor[CCHMAXPATH];
char          recentdirs[NUMRECENT][CCHMAXPATH];

#pragma data_seg(SHAREDATA2)

char          recentfiles[NUMRECENT][CCHMAXPATH];

#pragma data_seg(SHAREDATA3)

char          mydir[CCHMAXPATH],*appname = "MSE",amclosing;
HWND          hwndConfig;
HAB           habDLL;
HWND          hwndDesktop,hwndWPS,hwndBottom,ClipHwnd,ObjectHwnd,ObjectHwnd2,
              ObjectHwnd3,ObjectHwnd4;
HMODULE       hModPrime,hMod,hMod2,hMod3;
DATETIME      dtr;
PFN           pfnInput,pfnSend,pfnDestroy;
PFNWP         PFNWPTitle,PFNWPStatic,PFNWPMenu,PFNWPFrame;
BOOL          fSlidingFocus,fNoZorderChange,fWrapMouse,fDisabled,fSuspend,
              showclock,fNextIsCommand,fDesktops,fJustBumped,
              fShiftBump = TRUE,fChords = TRUE,fScreenSave,
              fUsingCorners,fEnhanceFileDlg,fEnhanceTitlebars,
              first,fConfirmFileDel,fConfirmDirDel,fRememberDirs,
              fRememberFiles,fAutoDropCombos,fAggressive,fEnableTitlebarMenus,
              fExtraMenus,showdate,ampm,fClockInTitlebars;
USHORT        cmds[3][8],nextcommand,crnrcmds[4];
long          xScreen,yScreen,yTitleBar,ySizeBorder,lBumpDesks;
ULONG         ulVersionMajor = VERMAJOR,ulVersionMinor = VERMINOR,ulLastMouse,
              ulPointerHidden,ulPrevMouse,ulDelay,ulNumFileDlg,ulDir,ulFile;
PID           dtPid,myPid,ClockFore = RGB_NEONGREEN,ClockBack = RGB_BLACK;
POINTL        lastpos;
QMSG          pickqMsg,tMsg;
HBITMAP       hbmTitles;
UCHAR         redStart,greenStart,blueStart,
              redEnd,greenEnd,blueEnd,
              redText,greenText,blueText,
              sSteps,tBorder,vGrade,cGrade,fTexture;
RECTL         ClipRectl;

#pragma alloc_text(ONCE,InitDLL,StartHook,StopHook,InitVars)
#pragma alloc_text(HOOK,InputProc,TopFrame,FindFrame,KillMsg)
#pragma alloc_text(MYFILEDLG,MyFileDlgProc,MySubProc,MakeFileName)
#pragma alloc_text(MYFILEDLG,MkdirProc,AddRecentDir,AddRecentFile)
#pragma alloc_text(EXTERNAL,SetPresParams,IsFile,IsRoot,is_alpha)
#pragma alloc_text(EXTERNAL,SaveRestorePos,LoadRestorePos,PostMsg)
#pragma alloc_text(EXTERNAL,to_upper,stri_cmp,SaveDir)
#pragma alloc_text(TITLEBAR,SendProc,PaintGradientWindow,TitleBarProc,ProgName)
#pragma alloc_text(TITLEBAR,DrawClock,ClockText)
#pragma alloc_text(FRAME,MenuProc,FrameProc)
#pragma alloc_text(DESTROY,DestroyProc)


/* forward declarations */
void EXPENTRY SendProc (HAB hab,PSMHSTRUCT psmh,BOOL fInterTask);
BOOL EXPENTRY InputProc (HAB hab, PQMSG pqMsg, ULONG fs);


unsigned long _System _DLL_InitTerm (unsigned long hModule,
                                     unsigned long ulFlag) {

  switch(ulFlag) {
    case 0:
      hModPrime = hModule;  // used to get resources
      break;
    case 1:
      break;
    default:
      return 0;
  }
  return 1;
}


APIRET SaveDir (char *curdir) {

  APIRET  ret;
  ULONG   curdirlen,curdrive,drivemap;

  *curdir = 0;
  ret = DosQCurDisk(&curdrive,&drivemap);
  curdirlen = CCHMAXPATH - 4;   /* NOTE!!!!!!!!! */
  ret += DosQCurDir(curdrive,&curdir[3],&curdirlen);
  *curdir = (CHAR)('@' + (INT)curdrive);
  curdir[1] = ':';
  curdir[2] = '\\';
  return ret;
}


void SetPresParams (HWND hwnd,long fore,long back,long border,char *font) {

  if(font)
    WinSetPresParam(hwnd,
                    PP_FONTNAMESIZE,
                    strlen(font) + 1L,
                    (PVOID)font);
  if(back != -1)
    WinSetPresParam(hwnd,
                    PP_BACKGROUNDCOLOR,
                    sizeof(long),
                    (PVOID)&back);
  if(fore != -1)
    WinSetPresParam(hwnd,
                    PP_FOREGROUNDCOLOR,
                    sizeof(long),
                    (PVOID)&fore);
  if(border != -1)
    WinSetPresParam(hwnd,
                    PP_BORDERCOLOR,
                    sizeof(long),
                    (PVOID)&border);
}


BOOL is_alpha (int key) {

  if((key >= 'a' &&
      key <= 'z') ||
     (key >= 'A' &&
      key <= 'Z'))
    return TRUE;
  return FALSE;
}


int to_upper (register int key) {

  if(key >= 'a' &&
     key <= 'z')
    return ((key) + 'A' - 'a');
  return key;
}


int stri_cmp (register char *a,register char *b) {

  while(*a && *b) {
    if(to_upper(*a) != to_upper(*b))
      return ((to_upper(*a) > to_upper(*b)) ?
               1 :
               (to_upper(*b) > to_upper(*a)) ?
                -1 :
                0);
    a++;
    b++;
  }
  if(*a ||
     *b)
    return 1;
  return 0;
}


BOOL IsRoot (char *filename) {

  return (filename &&
          is_alpha(*filename) &&
          filename[1] == ':' &&
          filename[2] == '\\' &&
          !filename[3]);
}


int IsFile (char *filename) {

  /* returns:  -1 (error), 0 (is a directory), or 1 (is a file) */

  FILEFINDBUF3 fb3;
  HDIR         hdir = HDIR_CREATE;
  ULONG        nm = 1;
  APIRET       rc;

  if(filename) {
    DosError(FERR_DISABLEHARDERR);
    rc = DosFindFirst(filename,
                      &hdir,
                      FILE_NORMAL    | FILE_HIDDEN   | FILE_SYSTEM |
                      FILE_DIRECTORY | FILE_ARCHIVED | FILE_READONLY,
                      &fb3,
                      sizeof(fb3),
                      &nm,
                      FIL_STANDARD);
    if(!rc) {
      DosFindClose(hdir);
      return ((fb3.attrFile & FILE_DIRECTORY) == 0);
    }
    else {  /* special-case root drives -- FAT "feature" */
      if(IsRoot(filename))
        return 0;
    }
  }
  return -1;  /* error; doesn't exist or null filename */
}


void SaveRestorePos (SWP *swp,HWND hwnd) {

  WinSetWindowUShort(hwnd,
                     QWS_XRESTORE,
                     (USHORT)swp->x);
  WinSetWindowUShort(hwnd,
                     QWS_YRESTORE,
                     (USHORT)swp->y);
  WinSetWindowUShort(hwnd,
                     QWS_CXRESTORE,
                     (USHORT)swp->cx);
  WinSetWindowUShort(hwnd,
                     QWS_CYRESTORE,
                     (USHORT)swp->cy);
}


void LoadRestorePos (SWP *swp,HWND hwnd) {

  swp->x = WinQueryWindowUShort(hwnd,
                                QWS_XRESTORE);
  swp->y = WinQueryWindowUShort(hwnd,
                                QWS_YRESTORE);
  swp->cx = WinQueryWindowUShort(hwnd,
                                 QWS_CXRESTORE);
  swp->cy = WinQueryWindowUShort(hwnd,
                                 QWS_CYRESTORE);
}


HWND FindFrame (HWND hwnd) {

  static UCHAR ucClassname[9];

  while(hwnd) {
    *ucClassname = 0;
    WinQueryClassName(hwnd,
                      sizeof(ucClassname),
                      ucClassname);
    if(!strcmp(ucClassname,"#1") ||
       !strcmp(ucClassname,"wpFolder") ||
       hwnd == hwndDesktop ||
       hwnd == hwndWPS)
      break;
    hwnd = WinQueryWindow(hwnd,QW_PARENT);
  }
  return hwnd;
}


HWND TopFrame (HWND hwnd) {

  HWND hwndParent = WinQueryWindow(hwnd,QW_PARENT);

  while(hwndParent != hwndDesktop &&
        hwndParent != hwndWPS) {
    hwnd = hwndParent;
    hwndParent = WinQueryWindow(hwnd,QW_PARENT);
  }
  return hwnd;
}


BOOL KillMsg (PQMSG pqMsg) {

  pqMsg->hwnd = NULLHANDLE;
  pqMsg->msg =  WM_NULL;
  pqMsg->mp1 =  MPVOID;
  pqMsg->mp2 =  MPVOID;
  return TRUE;
}


BOOL PostMsg (HWND h, ULONG msg, MPARAM mp1, MPARAM mp2) {

  BOOL rc = WinPostMsg(h,msg,mp1,mp2);

  if(!rc) {

    LONG cntr = 0;
    PIB *ppib;
    TIB *ptib;

    if(!DosGetInfoBlocks(&ptib,&ppib)) {

      PID pid;
      TID tid;

      if(WinQueryWindowProcess(h,
                               &pid,
                               &tid)) {
        if(pid != ppib->pib_ulpid ||
           tid != ptib->tib_ptib2->tib2_ultid) {
          for(;;) {
            DosSleep(33);
            rc = WinPostMsg(h,
                            msg,
                            mp1,
                            mp2);
            if(!rc) {

              ERRORID eid;

              if(!WinIsWindow((HAB)0,h))
                break;
              eid = WinGetLastError((HAB)0);
              if(ERRORIDERROR(eid) == PMERR_INVALID_HWND)
                break;
            }
            else
              break;
            cntr++;
            if(cntr > 19)
              break;
          }
        }
      }
    }
  }
  return rc;
}


/***********************************************************************/
/*  InitDLL: This function sets up the DLL and sets all variables      */
/***********************************************************************/
int EXPENTRY InitDLL (HAB hab,HINI prof,ULONG vmajor,ULONG vminor) {

  static APIRET rcl,rcq;

  habDLL = hab;

  rcq = 0;
  rcl = DosLoadModule(NULL,
                      0,
                      "MSEHOOK",
                      &hMod);
  if(!rcl) {
    rcq = DosQueryProcAddr(hMod,
                           1,
                           "InputProc",
                           &pfnInput);
    if(!rcq) {
      rcl = DosLoadModule(NULL,
                          0,
                          "MSEHOOK",
                          &hMod2);
      if(!rcl) {
        rcq = DosQueryProcAddr(hMod2,
                               2,
                               "SendProc",
                               &pfnSend);
        if(!rcq) {
          rcl = DosLoadModule(NULL,
                              0,
                              "MSEHOOK",
                              &hMod3);
          if(!rcl)
            rcq = DosQueryProcAddr(hMod3,
                                   88,
                                   "DestroyProc",
                                   &pfnDestroy);
        }
      }
    }
    if(!rcl &&
       !rcq)
      return 1;
  }
  {
    static char s[80];

    sprintf(s,
            "LoadRC: %lu, QueryRC: %lu",
            rcl,
            rcq);
    WinMessageBox(HWND_DESKTOP,
                  HWND_DESKTOP,
                  s,
                  "MSE InitDLL error",
                  0,
                  MB_ENTER | MB_MOVEABLE | MB_ICONEXCLAMATION);
  }
  return 0;
}


/***********************************************************************/
/* InitVars:  This function initializes some variables so we don't     */
/*            have to keep checking things OTF                         */
/***********************************************************************/
void InitVars (void) {

  static char  Class[9];
  static HENUM henum;
  static HWND  hwndChild;
  static TID   tid;

  xScreen     = WinQuerySysValue(HWND_DESKTOP,
                                 SV_CXSCREEN);
  yScreen     = WinQuerySysValue(HWND_DESKTOP,
                                 SV_CYSCREEN);
  yTitleBar   = WinQuerySysValue(HWND_DESKTOP,
                                 SV_CYTITLEBAR);
  ySizeBorder = WinQuerySysValue(HWND_DESKTOP,
                                 SV_CYSIZEBORDER);
  /* Without the WPS installed we can only get the desktop window handle */
  hwndBottom = WinQueryWindow(HWND_DESKTOP,
                              QW_BOTTOM);
  hwndDesktop = WinQueryDesktopWindow(habDLL,
                                      NULLHANDLE);
  hwndWPS = (HWND)0;
  henum = WinBeginEnumWindows(hwndBottom);
  while((hwndChild = WinGetNextWindow(henum)) != NULLHANDLE) {
    /* Now get the class name of that window handle */
    WinQueryClassName(hwndChild,
                      sizeof(Class),
                      (PCH)Class);
    if(!strcmp(Class,"#37")) {        /* desktop class */
      hwndWPS = hwndChild;
      break;
    }
  }
  WinEndEnumWindows(henum);
  /* find the PID of the desktop process */
  WinQueryWindowProcess((hwndWPS) ?
                         hwndWPS :
                         hwndDesktop,
                        &dtPid,
                        &tid);
  /* set defaults for titlebar colors */
  if(!first) {

    ULONG tcolor;

    tcolor = WinQuerySysColor(HWND_DESKTOP,
                              SYSCLR_ACTIVETITLE,
                              0);
    redStart   = ((tcolor & 0x00ff0000) >> 16);
    greenStart = ((tcolor & 0x0000ff00) >> 8);
    blueStart  = (tcolor & 0x000000ff);
    tcolor = WinQuerySysColor(HWND_DESKTOP,
                              SYSCLR_ACTIVETITLETEXT,
                              0);
    redText    = ((tcolor & 0x00ff0000) >> 16);
    greenText  = ((tcolor & 0x0000ff00) >> 8);
    blueText   = (tcolor & 0x000000ff);
    sSteps     = 64;
    first      = TRUE;
  }
  if(!*mydir) {

    ULONG curdirlen,curdrive,drivemap;

    *mydir = 0;
    DosQCurDisk(&curdrive,
                &drivemap);
    curdirlen = CCHMAXPATH - 4;
    DosQCurDir(curdrive,
               &mydir[3],
               &curdirlen);
    *mydir = (CHAR)('@' + (INT)curdrive);
    mydir[1] = ':';
    mydir[2] = '\\';
    if(mydir[strlen(mydir) - 1] != '\\')
      strcat(mydir,"\\");
  }
}


/***********************************************************************/
/*  StartHook: This function starts the hook filtering.                */
/***********************************************************************/
BOOL EXPENTRY StartHook (void) {

  static BOOL  rc;
  static ULONG check;
  static char  s[80];

  rc = WinSetHook(habDLL,
                  (HMQ)0,
                  HK_INPUT,
                  pfnInput,
                  hMod);
  if(!rc) {
    check = WinGetLastError(habDLL);
    sprintf(s,
            "ErrorID: %08lx",
            check);
    WinMessageBox(HWND_DESKTOP,
                  HWND_DESKTOP,
                  s,
                  "MSE StartHook error -- Input",
                  0,
                  MB_ENTER | MB_MOVEABLE | MB_ICONEXCLAMATION);
    return rc;
  }
  rc = WinSetHook(habDLL,
                  (HMQ)0,
                  HK_SENDMSG,
                  pfnSend,
                  hMod2);
  if(!rc) {
    check = WinGetLastError(habDLL);
    WinReleaseHook(habDLL,
                   (HMQ)0,
                   HK_INPUT,
                   pfnInput,
                   hMod);
    sprintf(s,
            "ErrorID: %08lx",
            check);
    WinMessageBox(HWND_DESKTOP,
                  HWND_DESKTOP,
                  s,
                  "MSE StartHook error -- Send",
                  0,
                  MB_ENTER | MB_MOVEABLE | MB_ICONEXCLAMATION);
  }
  rc = WinSetHook(habDLL,
                  (HMQ)0,
                  HK_DESTROYWINDOW,
                  pfnDestroy,
                  hMod3);
  if(!rc) {
    check = WinGetLastError(habDLL);
    WinReleaseHook(habDLL,
                   (HMQ)0,
                   HK_SENDMSG,
                   pfnSend,
                   hMod2);
    WinReleaseHook(habDLL,
                   (HMQ)0,
                   HK_INPUT,
                   pfnInput,
                   hMod);
    sprintf(s,
            "ErrorID: %08lx",
            check);
    WinMessageBox(HWND_DESKTOP,
                  HWND_DESKTOP,
                  s,
                  "MSE StartHook error -- Destroy",
                  0,
                  MB_ENTER | MB_MOVEABLE | MB_ICONEXCLAMATION);
  }
  return rc;
}


/***********************************************************************/
/*  StopHook: This function stops the hook filtering.                  */
/***********************************************************************/
BOOL EXPENTRY StopHook (void) {

  static BOOL   rc,src,wrc;
  static APIRET drc;
  static ULONG  check1 = 0,check2 = 0,check3 = 0;
  static char s[80];

  wrc = WinReleaseHook(habDLL,
                       (HMQ)0,
                       HK_DESTROYWINDOW,
                       pfnDestroy,
                       hMod3);
  if(!wrc)
    check3 = WinGetLastError(habDLL);
  src = WinReleaseHook(habDLL,
                       (HMQ)0,
                       HK_SENDMSG,
                       pfnSend,
                       hMod2);
  if(!src)
    check2 = WinGetLastError(habDLL);
  rc = WinReleaseHook(habDLL,
                      (HMQ)0,
                      HK_INPUT,
                      pfnInput,
                      hMod);
  if(!rc)
    check1 = WinGetLastError(habDLL);
  drc = DosFreeModule(hMod3);
  if(drc) {
    DosSleep(250);
    drc = DosFreeModule(hMod3);
  }
  drc = DosFreeModule(hMod2);
  if(drc) {
    DosSleep(250);
    drc = DosFreeModule(hMod2);
  }
  if((drc = DosFreeModule(hMod)) != 0 ||
     !rc ||
     !src ||
     !wrc) {
    sprintf(s,
            "ErrorID1: %08lx  ErrorID2: %08lx  ErrorID3: %08lx  DosRC: %lu",
            check1,
            check2,
            check3,
            drc);
    WinMessageBox(HWND_DESKTOP,
                  HWND_DESKTOP,
                  s,
                  "MSE StopHook error",
                  0,
                  MB_ENTER | MB_MOVEABLE | MB_ICONEXCLAMATION);
    return FALSE;
  }
  return TRUE;
}


/***********************************************************************/
/*  InputProc: This is the input filter routine (for posted msgs).     */
/*  While the hook is active, all messages come here                   */
/*  before being dispatched.                                           */
/***********************************************************************/
BOOL EXPENTRY InputProc (HAB hab, PQMSG pqMsg, ULONG fs) {

  if(fs != PM_NOREMOVE) {
    switch(pqMsg->msg) {
      case WM_BUTTON1CLICK:
        if(!fDisabled &&
           !fSuspend &&
           (WinGetKeyState(HWND_DESKTOP,VK_SCRLLOCK) & 0x0001) == 0 &&
           fAutoDropCombos) {
          if(WinQueryWindowUShort(pqMsg->hwnd,QWS_ID) == CBID_EDIT) {

            static SHORT isit;

            isit = (SHORT)WinSendMsg(WinQueryWindow(pqMsg->hwnd,QW_PARENT),
                                                    CBM_ISLISTSHOWING,
                                                    MPVOID,
                                                    MPVOID);
            isit = (isit) ? 0 : 1;
            WinSendMsg(WinQueryWindow(pqMsg->hwnd,QW_PARENT),
                       CBM_SHOWLIST,
                       MPFROMSHORT(isit),
                       MPVOID);
          }
        }
        break;

      case WM_CHAR:
        if(!fDisabled &&
           !fSuspend &&
           (WinGetKeyState(HWND_DESKTOP,VK_SCRLLOCK) & 0x0001) == 0) {
          if((SHORT1FROMMP(pqMsg->mp1) & KC_KEYUP) == 0) {
            if((SHORT1FROMMP(pqMsg->mp1) & KC_VIRTUALKEY) != 0 &&
               SHORT2FROMMP(pqMsg->mp2) == VK_PRINTSCRN &&
               fScreenSave)
              PostMsg(ObjectHwnd3,
                      UM_SAVESCREEN,
                      MPVOID,
                      MPVOID);
          }
        }
        break;

      case WM_MOUSEMOVE:
        if(ulPointerHidden) {
          ulPointerHidden--;
          if(!ulPointerHidden)
            WinShowPointer(HWND_DESKTOP,TRUE);
        }
        if(lastpos.x != pqMsg->ptl.x ||
           lastpos.y != pqMsg->ptl.y) {
          ulLastMouse = pqMsg->time;
          lastpos.x = pqMsg->ptl.x;
          lastpos.y = pqMsg->ptl.y;
        }

        if(fDisabled || fSuspend ||
           WinQueryCapture(HWND_DESKTOP) != NULLHANDLE ||
           (WinGetKeyState(HWND_DESKTOP,VK_SCRLLOCK) & 0x0001) != 0 ||
           (ulDelay &&
            (ulPrevMouse + ulDelay > ulLastMouse || ulLastMouse < ulPrevMouse)))
          break;

        ulPrevMouse = ulLastMouse;

        if(fUsingCorners) {

          static int corner;

          corner = (pqMsg->ptl.x < 3 && pqMsg->ptl.y > yScreen - 4) ? 1 :
                   (pqMsg->ptl.x > xScreen - 4 && pqMsg->ptl.y > yScreen - 4) ? 2 :
                   (pqMsg->ptl.x < 3 && pqMsg->ptl.y < 3) ? 3 :
                   (pqMsg->ptl.x > xScreen - 4 && pqMsg->ptl.y < 3) ? 4 : 0;

          if(corner) {
            if(crnrcmds[corner - 1] > CR_NONE &&
               crnrcmds[corner - 1] <= CR_MAXCMDS) {
              switch(crnrcmds[corner - 1]) {
                case CR_CALC:
                  PostMsg(hwndConfig,
                          UM_CALC,
                          MPFROM2SHORT(pqMsg->ptl.x,pqMsg->ptl.y),
                          MPVOID);
                  break;
                case CR_SHOWME:
                  PostMsg(hwndConfig,
                          UM_SHOWME,
                          MPFROM2SHORT(pqMsg->ptl.x,pqMsg->ptl.y),
                          MPVOID);
                  break;
                case CR_TASKLIST:
                  PostMsg(hwndConfig,
                          UM_SWITCHLIST,
                          MPVOID,
                          MPVOID);
                  break;
                case CR_OPEN:
                  PostMsg(hwndConfig,
                          UM_OPEN,
                          MPVOID,
                          MPVOID);
                  break;
                case CR_MENU1:
                case CR_MENU2:
                case CR_MENU3:
                case CR_MENU4:
                case CR_MENU5:
                case CR_MENU6:
                case CR_MENU7:
                  PostMsg(hwndConfig,
                          UM_STARTMENU,
                          MPFROM2SHORT(C_MENU1 +
                                       (crnrcmds[corner - 1] - CR_MENU1),
                                       0),
                          MPVOID);
                  break;
                case CR_VIRTUAL:
                  PostMsg(hwndConfig,
                          UM_VIRTUAL,
                          MPVOID,
                          MPVOID);
                  break;
                case CR_CLIPMGR:
                  PostMsg(ObjectHwnd3,
                          UM_CLIPMGR,
                          MPVOID,
                          MPVOID);
                  break;
              }
              return KillMsg(pqMsg);
            }
          }
        }

        if(((fDesktops && lBumpDesks) || fWrapMouse) &&
           (pqMsg->ptl.x <= 0 ||
            pqMsg->ptl.x >= xScreen - 1 ||
            pqMsg->ptl.y <= 0 ||
            pqMsg->ptl.y >= yScreen - 1)) {
          if(fDesktops && lBumpDesks) {

            SHORT xd = 0, yd = 0;

            if(!fShiftBump ||
              (((SHORT2FROMMP(pqMsg->mp2) & KC_SHIFT) != 0) &&
               (SHORT2FROMMP(pqMsg->mp2) & KC_CTRL) == 0 &&
               (SHORT2FROMMP(pqMsg->mp2) & KC_ALT) == 0)) {
              if(!fJustBumped) {
                if(pqMsg->ptl.x <= 0)
                  xd = -1;
                else if(pqMsg->ptl.x >= xScreen - 1)
                  xd = 1;
                if(pqMsg->ptl.y <= 0)
                  yd = 1;
                else if(pqMsg->ptl.y >= yScreen - 1)
                  yd = -1;
                if(xd ||
                   yd) {
                  PostMsg(ObjectHwnd3,
                          UM_BUMPDESKS,
                          MPFROM2SHORT(xd,yd),
                          MPVOID);
                  fJustBumped = TRUE;
                }
                break;
              }
            }
          }
          if(fWrapMouse &&
             !fJustBumped) {
            if(pqMsg->ptl.x <= 0)
              pqMsg->ptl.x = xScreen - 2;
            else if(pqMsg->ptl.x >= xScreen - 1)
              pqMsg->ptl.x = 1;
            if(pqMsg->ptl.y <= 0)
              pqMsg->ptl.y = yScreen - 2;
            else if(pqMsg->ptl.y >= yScreen - 1)
              pqMsg->ptl.y = 1;
            WinSetPointerPos(HWND_DESKTOP,
                             pqMsg->ptl.x,
                             pqMsg->ptl.y);
            break;
          }
        }
        else
          fJustBumped = FALSE;

        /*                                                                                      *\
         * If enabled, here we catch all mouse movements, to set the window under the mouse     *
         * pointer as the active one, if it isn't currently active or the window list or        *
         * optionally the Desktop window.                                                       *
        \*                                                                                      */
        /* If enabled, use sliding focus to activate window
         * under the mouse pointer (with some exceptions).
         * Caution! Menus have a class WC_MENU, but their
         * parent is not the frame window WC_FRAME but the
         * Desktop itself.
         */
        while(fSlidingFocus) {

          static UCHAR ucClassname[9],ucWindowText[64];
          static HWND  hwndActive,hwndTitlebar,hwndApplication,hwndTop;
          static HENUM henum;

          /*
           * Query the currently active window, where HWND_DESKTOP
           * is the parent window. It will be a WC_FRAME class
           * window
           */
          hwndActive = WinQueryActiveWindow(HWND_DESKTOP);
          if(fNoZorderChange) {
            henum = WinBeginEnumWindows(HWND_DESKTOP);
            hwndTop = WinGetNextWindow(henum);
            WinEndEnumWindows(henum);
          }
          else
            hwndTop = (HWND)0;
          WinQueryWindowText(hwndActive,sizeof(ucWindowText),
                             ucWindowText);
          /*
           * Don't switch away from the WC_FRAME class tasklist
           */
          if(!strcmp(ucWindowText,"Window List"))
            break;
          /*
           * Get message target window
           */
          hwndApplication = pqMsg->hwnd;
          /*
           * If the window under the mouse pointer is one of the
           * Desktops, don't do any changes
           */
          if((hwndApplication == hwndDesktop) ||
             (hwndApplication == hwndWPS))
            break;
          /*
           * Get parent window of current window
           */
          hwndApplication = TopFrame(hwndApplication);

          if(hwndApplication == WinQueryActiveWindow(HWND_DESKTOP))
            break;

          /*
           * Query the class of the frame window of the
           * designated target of WM_MOUSEMOVE
           */
          WinQueryClassName(hwndApplication,
                            sizeof(ucClassname),
                            ucClassname);
          /*
           * Don't switch to menu windows
           */
          if(!strcmp(ucClassname,"#4"))
            return(FALSE);
          /*
           * Don't switch to a combobox's listbox windows
           */
          if(!strcmp(ucClassname,"#7"))
            return(FALSE);
          /*
           * Query the frame window name of the designated
           * target of WM_MOUSEMOVE
           */
          WinQueryWindowText(hwndApplication, sizeof(ucWindowText),
                             ucWindowText);
          /*
           * Don't switch to seamless Win-OS2 menus
           */
          if(strstr(ucWindowText,"Seamless"))
            return(FALSE);
          /*
           * Sort with expected descending probability, to avoid
           * unnecessary cpu load
           */
          for(;;) {
            /*
             * Don't switch if previous windows equals current one
             */
            if(hwndActive == hwndApplication) {
              if(hwndTop == hwndApplication || !fNoZorderChange)
                break;
            }
            /*
             * Only switch to WC_FRAME class windows and WPS folders
             */
            if(strcmp(ucClassname,"#1") &&
               strcmp(ucClassname,"wpFolder"))
              break;
            if(fNoZorderChange) {
              /*
               * Change focus, but preserve Z-order
               * Don't send WM_ACTIVATE to window with new focus
               */
              if(WinIsWindow(WinQueryAnchorBlock(pqMsg->hwnd),
                             WinWindowFromID(hwndApplication,FID_CLIENT)))
                WinFocusChange(HWND_DESKTOP,
                               WinWindowFromID(hwndApplication,
                                               FID_CLIENT),
                               FC_NOSETACTIVE);
              else
                WinFocusChange(HWND_DESKTOP,
                               hwndApplication,
                               FC_NOSETACTIVE);
              /*
               * Activate new window
               */
              if((hwndTitlebar = WinWindowFromID(hwndApplication,
                                                 FID_TITLEBAR)) != (HWND)0)
               PostMsg(hwndApplication,
                       WM_ACTIVATE,
                       MPFROMSHORT(TRUE),
                       MPFROMHWND(hwndTitlebar));
            }
            else {
              /*
               * Switch to the new frame window. It will generate
               *  all messages of deactivating old and activating
               *  new frame window
               */
              if(WinIsWindow(WinQueryAnchorBlock(pqMsg->hwnd),
                             WinWindowFromID(hwndApplication,FID_CLIENT)))
                WinSetFocus(HWND_DESKTOP,WinWindowFromID(hwndApplication,
                            FID_CLIENT));
              else
                WinSetFocus(HWND_DESKTOP,hwndApplication);
            }
            /*
             * We changed the focus, don't pass this message to
             * the next hook in the chain
             */
            return(TRUE);
          }
          break;
        }
        break;

      case WM_BUTTON1DOWN:
      case WM_BUTTON1UP:
      case WM_BUTTON2DOWN:
      case WM_BUTTON2UP:
      case WM_BUTTON3DOWN:
      case WM_BUTTON3UP:
        if(fDisabled ||
           fSuspend ||
           WinQueryCapture(HWND_DESKTOP) != NULLHANDLE ||
           (WinGetKeyState(HWND_DESKTOP,VK_SCRLLOCK) & 0x0001) != 0)
          break;
        {
          static BOOL ctrl,alt,shift,up,isa;
          static int  cmd,button;
          static HWND hwndFrame;

          if(!fNextIsCommand) {
            ctrl =  ((SHORT2FROMMP(pqMsg->mp2) & KC_CTRL) != 0);
            alt =   ((SHORT2FROMMP(pqMsg->mp2) & KC_ALT) != 0);
            shift = ((SHORT2FROMMP(pqMsg->mp2) & KC_SHIFT) != 0);
            up = FALSE;
            cmd = (shift) + (ctrl * 2) + (alt * 4);
            button = (pqMsg->msg == WM_BUTTON2DOWN ||
                      pqMsg->msg == WM_BUTTON2UP) ? 1 :
                       (pqMsg->msg == WM_BUTTON3DOWN ||
                        pqMsg->msg == WM_BUTTON3UP) ? 2 : 0;
            nextcommand = cmds[button][cmd];
          }
          else
            fNextIsCommand = FALSE;
          if((pqMsg->msg == WM_BUTTON1UP ||
              pqMsg->msg == WM_BUTTON2UP ||
              pqMsg->msg == WM_BUTTON3UP))
            up = TRUE;

          if(nextcommand == C_NONE &&
             fChords) {
            if(!up) {

              BOOL b1,b2,b3;

              b1 = ((WinGetKeyState(HWND_DESKTOP,VK_BUTTON1) & 0x8000) != 0);
              b2 = ((WinGetKeyState(HWND_DESKTOP,VK_BUTTON2) & 0x8000) != 0);
              b3 = ((WinGetKeyState(HWND_DESKTOP,VK_BUTTON3) & 0x8000) != 0);
              if(fDesktops && (b1 && b3)) {
                nextcommand = C_VIRTUAL;
                goto CommandusInterruptus;
              }
              else if((alt || ctrl || shift) && b2 && b3) {
                nextcommand = C_CLIPMGR;
                goto CommandusInterruptus;
              }
              else if(b2 && b3) {
                nextcommand = C_SHOWME;
                goto CommandusInterruptus;
              }
            }
          }
          else {
CommandusInterruptus:
            switch(nextcommand) {
              case C_NONE:
                break;

              case C_CMDLINE:
                if(!up) {

                  static char s[CCHMAXPATH + 1];

                  *s = 0;
                  SaveDir(s);
                  PostMsg(hwndConfig,
                          UM_CMDLINE,
                          MPVOID,
                          ((*s) ? MPFROMP(s) : MPVOID));
                }
                return KillMsg(pqMsg);

              case C_CALC:
                if(!up)
                  PostMsg(hwndConfig,
                          UM_CALC,
                          MPFROM2SHORT(pqMsg->ptl.x,pqMsg->ptl.y),
                          MPVOID);
                return KillMsg(pqMsg);

              case C_HANDICAP:
                if(!up) {
                  pickqMsg = *pqMsg;
                  PostMsg(hwndConfig,
                          UM_HANDICAP,
                          MPFROM2SHORT(pqMsg->ptl.x,pqMsg->ptl.y),
                          MPVOID);
                }
                return KillMsg(pqMsg);

              case C_VERTSCROLL:
                if(!up) {
                  static USHORT id;

                  id = WinQueryWindowUShort(pqMsg->hwnd,
                                            QWS_ID);
                  switch(id) {
                    case FID_CLIENT:
                    case FID_SYSMENU:
                    case FID_TITLEBAR:
                    case FID_MENU:
                      hwndFrame = FindFrame(pqMsg->hwnd);
                      hwndFrame = WinWindowFromID(hwndFrame,
                                                  FID_VERTSCROLL);
                      break;
                    default:
                      hwndFrame = pqMsg->hwnd;
                      break;
                  }
                  PostMsg(ObjectHwnd,
                          UM_HOOK2,
                          MPFROM2SHORT(nextcommand,0),
                          MPFROMLONG((LONG)hwndFrame));
                }
                return KillMsg(pqMsg);

              case C_CLOSEA:
              case C_CLOSE:
                if(nextcommand == C_CLOSE)
                  hwndFrame = WinQueryWindow(pqMsg->hwnd,QW_PARENT);
                else
                  hwndFrame = TopFrame(pqMsg->hwnd);
                if(hwndFrame == hwndDesktop || hwndFrame == hwndWPS ||
                   hwndFrame == hwndBottom)
                  break;
                if(nextcommand == C_CLOSEA ||
                   pqMsg->hwnd == WinWindowFromID(hwndFrame,
                                                  FID_TITLEBAR) ||
                   pqMsg->hwnd == WinWindowFromID(hwndFrame,
                                                  FID_SYSMENU)) {
                  if(!up)
                    PostMsg(ObjectHwnd,
                            UM_HOOK2,
                            MPFROM2SHORT(nextcommand,0),
                            MPFROMLONG((LONG)hwndFrame));
                  return KillMsg(pqMsg);
                }
                break;

              case C_BACKA:
              case C_BACK:
                if(nextcommand == C_BACK)
                  hwndFrame = WinQueryWindow(pqMsg->hwnd,QW_PARENT);
                else
                  hwndFrame = TopFrame(pqMsg->hwnd);
                if(nextcommand == C_BACKA ||
                   pqMsg->hwnd == WinWindowFromID(hwndFrame,
                                                  FID_TITLEBAR)||
                   pqMsg->hwnd == WinWindowFromID(hwndFrame,
                                                  FID_SYSMENU)) {
                  if(!up)
                    PostMsg(ObjectHwnd,
                            UM_HOOK2,
                            MPFROM2SHORT(nextcommand,0),
                            MPFROMLONG((LONG)hwndFrame));
                  return KillMsg(pqMsg);
                }
                break;

              case C_WINDOWLIST:
                if(!up)
                  PostMsg(hwndConfig,
                          WM_SYSCOMMAND,
                          MPFROM2SHORT(SC_TASKMANAGER,0),
                          MPVOID);
                return KillMsg(pqMsg);

              case C_TASKLIST:
                if(!up)
                  PostMsg(hwndConfig,
                          UM_SWITCHLIST,
                          MPVOID,
                          MPVOID);
                return KillMsg(pqMsg);

              case C_CHORD:
              case C_B1DBLCLK:
                if(!up) {

                  QMSG *sMsg;

                  tMsg = *pqMsg;

                  sMsg = WinSendMsg(ObjectHwnd,
                                    UM_ALLOC,
                                    MPFROMP(&tMsg),
                                    MPVOID);

                  if(sMsg)
                    PostMsg(ObjectHwnd,
                            UM_HOOK,
                            MPFROM2SHORT(nextcommand,0),
                            MPFROMP(sMsg));
                }
                return KillMsg(pqMsg);

              case C_WINCLIPA:
              case C_WINCLIP:
                if(!up) {
                  if(WinOpenClipbrd(WinQueryAnchorBlock(pqMsg->hwnd))) {

                    char         *hold = NULL,*clip = NULL,*p;
                    static long   len,flen;

                    len = WinQueryWindowTextLength(pqMsg->hwnd);
                    flen = len;
                    if(len) {
                      if(nextcommand == C_WINCLIPA) {
                        clip = (char *)WinQueryClipbrdData(WinQueryAnchorBlock(pqMsg->hwnd),
                                                           CF_TEXT);
                        if(clip) {
                          flen += strlen(clip);
                          flen += 2;
                        }
                      }
                      if(!DosAllocSharedMem((PPVOID)&hold,
                                            (PSZ)NULL,
                                            flen + 2,
                                            PAG_COMMIT | OBJ_GIVEABLE |
                                            PAG_READ   | PAG_WRITE) &&
                         hold) {
                        p = hold;
                        if(clip) {
                          strcpy(hold,clip);
                          p += strlen(hold);
                          if(*(p - 1) != '\n') {  /* append cr/lf */
                            if(*(p - 1) != '\r') {
                              *p = '\r';
                              p++;
                            }
                            *p = '\n';
                            p++;
                          }
                        }
                        if(WinQueryWindowText(pqMsg->hwnd,
                                              len + 1,
                                              p)) {
                          if(!WinSetClipbrdData(WinQueryAnchorBlock(pqMsg->hwnd),
                                                (ULONG)hold,
                                                CF_TEXT,
                                                CFI_POINTER))
                            DosFreeMem(hold);
                        }
                      }
                    }
                    WinCloseClipbrd(WinQueryAnchorBlock(pqMsg->hwnd));
                  }
                }
                return KillMsg(pqMsg);

              case C_CLIPWIN:
                if(!up) {

                  static char ucClassname[9];

                  WinQueryClassName(pqMsg->hwnd,
                                    sizeof(ucClassname),
                                    ucClassname);
                  if(!strcmp(ucClassname,"#6"))         /* entryfield */
                    PostMsg(pqMsg->hwnd,
                            EM_PASTE,
                            MPVOID,
                            MPVOID);
                  else if(!strcmp(ucClassname,"#10"))   /* mle */
                    PostMsg(pqMsg->hwnd,
                            MLM_PASTE,
                            MPVOID,
                            MPVOID);
                  else {
                    if(WinOpenClipbrd(WinQueryAnchorBlock(pqMsg->hwnd))) {

                      char *p = (char *)WinQueryClipbrdData(WinQueryAnchorBlock(pqMsg->hwnd),
                                                            CF_TEXT);

                      if(p &&
                         *p)
                        WinSetWindowText(pqMsg->hwnd,p);
                      WinCloseClipbrd(WinQueryAnchorBlock(pqMsg->hwnd));
                    }
                  }
                }
                return KillMsg(pqMsg);

              case C_ROLLUPA:
              case C_ROLLUP:
                if(nextcommand == C_ROLLUP)
                  hwndFrame = WinQueryWindow(pqMsg->hwnd,QW_PARENT);
                else
                  hwndFrame = TopFrame(pqMsg->hwnd);
                if(hwndFrame == hwndDesktop ||
                   hwndFrame == hwndWPS ||
                   hwndFrame == hwndBottom)
                  break;
                if(nextcommand == C_ROLLUPA ||
                   pqMsg->hwnd == WinWindowFromID(hwndFrame,FID_TITLEBAR) ||
                   pqMsg->hwnd == WinWindowFromID(hwndFrame,
                                                  FID_SYSMENU)) {
                  if(!up)
                    PostMsg(ObjectHwnd,
                            UM_HOOK2,
                            MPFROM2SHORT(nextcommand,0),
                            MPFROMLONG((LONG)hwndFrame));
                  return KillMsg(pqMsg);
                }
                break;

              case C_SHOWME:
                if(!up)
                  PostMsg(hwndConfig,
                          UM_SHOWME,
                          MPFROM2SHORT(pqMsg->ptl.x,pqMsg->ptl.y),
                          MPVOID);
                return KillMsg(pqMsg);

              case C_SHOWA:
              case C_SHOW:
                if(nextcommand == C_SHOW)
                  hwndFrame = WinQueryWindow(pqMsg->hwnd,QW_PARENT);
                else
                  hwndFrame = TopFrame(pqMsg->hwnd);
                if(nextcommand == C_SHOWA ||
                   pqMsg->hwnd == WinWindowFromID(hwndFrame,FID_TITLEBAR) ||
                   pqMsg->hwnd == WinWindowFromID(hwndFrame,
                                                  FID_SYSMENU)) {
                  if(!up)
                    PostMsg(ObjectHwnd,
                            UM_HOOK2,
                            MPFROM2SHORT(nextcommand,0),
                            MPFROMLONG((LONG)hwndFrame));
                  return KillMsg(pqMsg);
                }
                break;

              case C_MENU1:
              case C_MENU2:
              case C_MENU3:
              case C_MENU4:
              case C_MENU5:
              case C_MENU6:
              case C_MENU7:
                if(!up)
                  PostMsg(hwndConfig,
                          UM_STARTMENU,
                          MPFROM2SHORT(nextcommand,0),
                          MPVOID);
                return KillMsg(pqMsg);

              case C_OPEN:
                if(!up)
                  PostMsg(hwndConfig,
                          UM_OPEN,
                          MPVOID,
                          MPVOID);
                return KillMsg(pqMsg);

              case C_CLIPMGR:
                if(!up)
                  PostMsg(ObjectHwnd3,
                          UM_CLIPMGR,
                          MPVOID,
                          MPVOID);
                return KillMsg(pqMsg);

              case C_VIRTUAL:
                if(!up)
                  PostMsg(hwndConfig,
                          UM_VIRTUAL,
                          MPVOID,
                          MPVOID);
                return KillMsg(pqMsg);

              case C_MINALL:
                if(!up)
                  PostMsg(ObjectHwnd3,
                          UM_MINALL,
                          MPVOID,
                          MPVOID);
                return KillMsg(pqMsg);

              case C_STARTFM2:
                if(!up)
                  PostMsg(hwndConfig,
                          UM_STARTWIN,
                          MPFROM2SHORT(nextcommand,0),
                          MPVOID);
                return KillMsg(pqMsg);

              case C_NEXT:
                if(!up) {
                  hwndFrame = TopFrame(WinQueryActiveWindow(HWND_DESKTOP));
                  PostMsg(ObjectHwnd,
                          UM_HOOK2,
                          MPFROM2SHORT(nextcommand,0),
                          MPFROMLONG((LONG)hwndFrame));
                }
                return KillMsg(pqMsg);

              case C_MOVEA:
              case C_SIZEA:
              case C_MAXIMIZEA:
              case C_MINIMIZEA:
              case C_MOVE:
              case C_SIZE:
              case C_MAXIMIZE:
              case C_MINIMIZE:
                isa = TRUE;
                switch(nextcommand) {
                  case C_MOVE:
                  case C_SIZE:
                  case C_MAXIMIZE:
                  case C_MINIMIZE:
                    isa = FALSE;
                    hwndFrame = WinQueryWindow(pqMsg->hwnd,QW_PARENT);
                    break;
                  default:
                    hwndFrame = TopFrame(pqMsg->hwnd);
                    break;
                }
                if(hwndFrame == hwndDesktop ||
                   hwndFrame == hwndWPS ||
                   hwndFrame == hwndBottom)
                  break;
                if(isa ||
                   pqMsg->hwnd == WinWindowFromID(hwndFrame,FID_TITLEBAR) ||
                   pqMsg->hwnd == WinWindowFromID(hwndFrame,
                                                  FID_SYSMENU)) {
                  if(!up) {

                    static SHORT sc;

                    switch(nextcommand) {
                      case C_SIZEA:
                      case C_SIZE:
                        sc = SC_SIZE;
                        break;
                      case C_MAXIMIZEA:
                      case C_MAXIMIZE:
                        sc = SC_MAXIMIZE;
                        break;
                      case C_MINIMIZEA:
                      case C_MINIMIZE:
                        sc = SC_MINIMIZE;
                        break;
                      default:
                        sc = SC_MOVE;
                        break;
                    }
                    PostMsg(ObjectHwnd,
                            UM_HOOK2,
                            MPFROM2SHORT(nextcommand,sc),
                            MPFROMLONG((LONG)hwndFrame));
                  }
                  return KillMsg(pqMsg);
                }
                break;
            }
          }
        }
        break;
    }
  }
  /* Pass the message on to the next hook in line. */
  return FALSE;
}


BOOL MakeFileName (HWND hwnd,char *s,int full) {

  FILEDLG *f;
  SHORT    sSelect;
  char    *p;

  f = WinQueryWindowPtr(hwnd,0);
  if(f) {
    strcpy(s,
           f->szFullFile);
    if(!full) {
      /* remove trailing backslash if req'd */
      sSelect = strlen(s);
      if(sSelect > 3 &&
         s[sSelect - 1] == '\\')
        s[sSelect - 1] = 0;
      if(IsFile(s) == 1) {
        sSelect = strlen(s);
        while(sSelect) {
          if(sSelect == 3 &&
             s[sSelect - 1] == '\\') {
            s[sSelect] = 0;
            break;
          }
          if(s[sSelect] == '\\') {
            s[sSelect] = 0;
            break;
          }
          sSelect--;
        }
      }
      return TRUE;
    }
    else if(full == 2) {
      /* add selected directory name */
      sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
                                         DID_DIRECTORY_LB,
                                         LM_QUERYSELECTION,
                                         MPFROMSHORT(LIT_CURSOR),
                                         MPVOID);
      if(sSelect >= 0) {
        if((SHORT)WinSendDlgItemMsg(hwnd,
                                    DID_DIRECTORY_LB,
                                    LM_QUERYITEMTEXT,
                                    MPFROM2SHORT(sSelect,
                                                 CCHMAXPATH - strlen(s)),
                                    MPFROMP(s + strlen(s))) > 0)
        return TRUE;
      }
      else if(sSelect == LIT_NONE) {
        if((SHORT)WinSendDlgItemMsg(hwnd,
                                    DID_DIRECTORY_LB,
                                    LM_QUERYITEMTEXT,
                                    MPFROM2SHORT(0,
                                                 CCHMAXPATH - strlen(s)),
                                    MPFROMP(s + strlen(s))) > 0)
          return TRUE;
      }
    }
    else if(full == 3) {  /* add name from entry field */
      sSelect = strlen(s);
      if(WinQueryDlgItemText(hwnd,
                             DID_FILENAME_ED,
                             CCHMAXPATH - strlen(s),
                             s + sSelect)) {
        p = s + sSelect;
        if(is_alpha(*p) &&
           p[1] == ':' &&
           p[2] == '\\') {
          strcpy(s,p);
        }
        else if(strchr(p,'\\'))
          strcpy(s + 3,p);
        return TRUE;
      }
    }
    else {
      /* add selected file name */
      sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
                                         DID_FILES_LB,
                                         LM_QUERYSELECTION,
                                         MPFROMSHORT(LIT_CURSOR),
                                         MPVOID);
      if(sSelect >= 0) {
        if((SHORT)WinSendDlgItemMsg(hwnd,
                                    DID_FILES_LB,
                                    LM_QUERYITEMTEXT,
                                    MPFROM2SHORT(sSelect,
                                                 CCHMAXPATH - strlen(s)),
                                    MPFROMP(s + strlen(s))) > 0)
        return TRUE;
      }
      else if(sSelect == LIT_NONE) {
        if((SHORT)WinSendDlgItemMsg(hwnd,
                                    DID_FILES_LB,
                                    LM_QUERYITEMTEXT,
                                    MPFROM2SHORT(0,
                                                 CCHMAXPATH - strlen(s)),
                                    MPFROMP(s + strlen(s))) > 0)
        return TRUE;
      }
    }
  }
  return FALSE;
}


MRESULT EXPENTRY MkdirProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_INITDLG:
      if(mp2) {
        WinSetWindowPtr(hwnd,
                        0,
                        mp2);
        WinSendDlgItemMsg(hwnd,
                          CRD_PATH,
                          EM_SETTEXTLIMIT,
                          MPFROM2SHORT(CCHMAXPATH,0),
                          MPVOID);
        WinSetDlgItemText(hwnd,
                          CRD_PATH,
                          (char *)mp2);
        *((char *)mp2) = 0;
        break;
      }
      WinDismissDlg(hwnd,0);
      break;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case DID_CANCEL:
          WinDismissDlg(hwnd,0);
          break;
        case DID_OK:
          {
            char   s[CCHMAXPATH],*p;
            long   was;
            APIRET last = 0;

            *s = 0;
            WinQueryDlgItemText(hwnd,
                                CRD_PATH,
                                CCHMAXPATH,
                                s);
            if((strchr(s,'?') || strchr(s,'*')) || IsFile(s) == 1) {
              DosBeep(150,100);
              WinSetFocus(HWND_DESKTOP,
                          WinWindowFromID(hwnd,CRD_PATH));
              break;
            }
            p = WinQueryWindowPtr(hwnd,0);
            if(p) {
              strcpy(p,s);
              p = s;
              while(*p) {
                if(*p == '/')
                  *p = '\\';
                p++;
              }
              p = s;
              do {
                p = strchr(p,'\\');
                if(p && *p) {
                  *p = 0;
                  was = 1;
                }
                else
                  was = 0;
                last = DosCreateDir(s,NULL);
                if(last == ERROR_ACCESS_DENIED &&
                   !IsFile(s))
                  last = 0;
                if(was) {
                  *p = '\\';
                  p++;
                }
              } while(p && *p);
              WinDismissDlg(hwnd,
                            (last) ? 0 : 1);
            }
          }
          break;
      }
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


BOOL AddRecentDir (HWND hwnd,char *dir) {

  long x;

  if(!dir ||
     !*dir)
    return FALSE;
  for(x = 0;x < NUMRECENT;x++) {
    if(!stri_cmp(dir,recentdirs[x])) {
      if(x != ulDir) {  /* move to top of list */

        static char s[CCHMAXPATH];

        strcpy(s,recentdirs[ulDir]);
        strcpy(recentdirs[ulDir],dir);
        strcpy(recentdirs[x],s);
      }
      return FALSE;
    }
  }
  if((SHORT)WinSendMsg(hwnd,
                       LM_SEARCHSTRING,
                       MPFROM2SHORT(0,0),
                       MPFROMP(dir)) >= 0)
    return FALSE;
  if(*recentdirs[ulDir])
    ulDir++;
  if(ulDir >= NUMRECENT)
    ulDir = 0;
  strcpy(recentdirs[ulDir],dir);
  return TRUE;
}


BOOL AddRecentFile (HWND hwnd,char *file) {

  long x;

  if(!file ||
     !*file)
    return FALSE;
  for(x = 0;x < NUMRECENT;x++) {
    if(!stri_cmp(file,recentfiles[x])) {
      if(x != ulFile) {  /* move to top of list */

        static char s[CCHMAXPATH];

        strcpy(s,recentfiles[ulFile]);
        strcpy(recentfiles[ulFile],file);
        strcpy(recentfiles[x],s);
      }
      return FALSE;
    }
  }
  if((SHORT)WinSendMsg(hwnd,
                       LM_SEARCHSTRING,
                       MPFROM2SHORT(0,0),
                       MPFROMP(file)) >= 0)
    return FALSE;
  if(*recentfiles[ulFile])
    ulFile++;
  if(ulFile >= NUMRECENT)
    ulFile = 0;
  strcpy(recentfiles[ulFile],file);
  return TRUE;

}


MRESULT EXPENTRY MySubProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  PFNWP oldproc = (PFNWP)WinQueryWindowPtr(hwnd,0);

  switch(msg) {
    case UM_REMIND: /* reacquire control of file dialog */
      {
        HWND hwndF = WinQueryWindow(WinQueryWindow(hwnd,QW_PARENT),QW_PARENT);

        if((PFNWP)WinQueryWindowPtr(hwndF,QWP_PFNWP) != MyFileDlgProc)
          WinSubclassWindow(hwndF,
                            MyFileDlgProc);
      }
      return 0;

    case WM_MOUSEMOVE:
      {
        USHORT id = WinQueryWindowUShort(hwnd,QWS_ID);
        USHORT helpid;
        HWND   hwndHelp;

        if(id == CBID_EDIT)
          hwndHelp = WinWindowFromID(WinQueryWindow(WinQueryWindow(hwnd,
                                                                   QW_PARENT),
                                                    QW_PARENT),
                                     IDF_HELP);
        else
          hwndHelp = WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),IDF_HELP);
        if(!hwndHelp)
          break;
        helpid = WinQueryWindowUShort(hwndHelp,0);
        switch(id) {
          case IDF_HELP:
            {
              FILEDLG *f;

              f = WinQueryWindowPtr(WinQueryWindow(hwnd,QW_PARENT),0);
              if(fAggressive ||
                 !f ||
                 !f->pfnDlgProc) {
                if(helpid != id)
                  WinSetWindowText(hwndHelp,
                                   "Helpful little me.  Get a context menu.");
                helpid = id;
              }
              else        // MR/2 Ice bug workaround...
                return 0;
            }
            break;
          case IDF_FILEDATA:
            if(helpid != id)
              WinSetWindowText(hwndHelp,
                               "Data of last selected file appears here.");
            helpid = id;
            break;
          case DID_FILENAME_TXT:
            if(helpid != id)
              WinSetWindowText(hwndHelp,
                               "Click to view current file.");
            helpid = id;
            break;
          case DID_FILES_TXT:
            if(helpid != id)
              WinSetWindowText(hwndHelp,
                               "Click to open selected file.");
            helpid = id;
            break;
          case DID_DIRECTORY_TXT:
            if(helpid != id)
              WinSetWindowText(hwndHelp,
                               "Click to open current directory, B2-click to create dir.");
            helpid = id;
            break;
          case DID_FILENAME_ED:
            if(helpid != id)
              WinSetWindowText(hwndHelp,
                               "Type in a filename, mask or directory, then press [Enter].");
            helpid = id;
            break;
          case DID_FILES_LB:
            if(helpid != id)
              WinSetWindowText(hwndHelp,
                               "Select a file or get a context menu.");
            helpid = id;
            break;
          case DID_DIRECTORY_LB:
            if(helpid != id)
              WinSetWindowText(hwndHelp,
                               "Select a directory or get a context menu.");
            helpid = id;
            break;
          case CBID_EDIT:
            id = WinQueryWindowUShort(WinQueryWindow(hwnd,QW_PARENT),QWS_ID);
            switch(id) {
              case IDF_FILES:
                if(helpid != id)
                  WinSetWindowText(hwndHelp,
                                   "Select a file or get a context menu.");
                helpid = id;
                break;
              case IDF_DIRS:
                if(helpid != id)
                  WinSetWindowText(hwndHelp,
                                   "Select a directory or get a context menu.");
                helpid = id;
                break;
              case DID_FILTER_CB:
                if(helpid != id)
                  WinSetWindowText(hwndHelp,"Select a file type.");
                helpid = id;
                break;
              case DID_DRIVE_CB:
                if(helpid != id)
                  WinSetWindowText(hwndHelp,
                                   "Select a drive.");
                helpid = id;
                break;
            }
            break;
        }
        WinSetWindowUShort(hwndHelp,
                           0,
                           helpid);
      }
      break;
//      return MRFROMSHORT(TRUE);

    case WM_CONTEXTMENU:
      {
        USHORT id = WinQueryWindowUShort(hwnd,QWS_ID);

        if(id == CBID_EDIT)
          id = WinQueryWindowUShort(WinQueryWindow(hwnd,QW_PARENT),QWS_ID);
        switch(id) {
          case DID_DIRECTORY_TXT:
            {
              static char filename[CCHMAXPATH * 2];
              char       *p;
              HWND        hwndF = WinQueryWindow(hwnd,QW_PARENT);
              BOOL        added = FALSE;
              ULONG       ret;

              *filename = 0;
              if(MakeFileName(hwndF,
                              filename,
                              0)) {
                if(filename[strlen(filename) - 1] != '\\')
                  strcat(filename,"\\");
                ret = WinDlgBox(HWND_DESKTOP,
                                WinQueryWindow(hwnd,QW_PARENT),
                                MkdirProc,
                                hModPrime,
                                CRD_FRAME,
                                filename);
                if(ret > 0 &&
                   ret != DID_ERROR &&
                   *filename) {
                  WinQueryDlgItemText(hwndF,
                                      DID_FILENAME_ED,
                                      CCHMAXPATH,
                                      filename);
                  p = strrchr(filename,'\\');
                  if(p)
                    memmove(filename,
                            p,
                            strlen(p) + 1);
                  if(!strchr(filename,'?') &&
                     !strchr(filename,'*')) {
                    strcat(filename,"*");
                    added = TRUE;
                  }
                  WinSetDlgItemText(hwndF,
                                    DID_FILENAME_ED,
                                    filename);
                  WinSendMsg(hwndF,
                             WM_COMMAND,
                             MPFROM2SHORT(DID_OK,0),
                             MPVOID);
                  if(*filename &&
                     added) {
                    filename[strlen(filename) - 1] = 0;
                    WinSetDlgItemText(hwndF,
                                      DID_FILENAME_ED,
                                      filename);
                  }
                }
              }
            }
            break;

          case IDF_DIRS:
          case IDF_FILES:
            PostMsg(WinQueryWindow(WinQueryWindow(hwnd,QW_PARENT),QW_PARENT),
                    UM_CONTEXTMENU,
                    mp1,
                    MPFROM2SHORT(id,0));
            return 0;

          case IDF_FILEDATA:
          case IDF_HELP:
          case DID_FILES_LB:
          case DID_DIRECTORY_LB:
            PostMsg(WinQueryWindow(hwnd,QW_PARENT),
                    UM_CONTEXTMENU,
                    mp1,
                    MPFROM2SHORT(id,0));
            return 0;
        }
      }
      break;
  }

  return (WinQueryWindowUShort(hwnd,QWS_ID) == IDF_HELP ||
          WinQueryWindowUShort(hwnd,QWS_ID) == IDF_FILEDATA) ?
          PFNWPStatic(hwnd,msg,mp1,mp2) :
          oldproc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY MyFileDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_INITDLG:
      {
        SWP   swp,swpM,swpO;
        ULONG lasty,efh;
        char  buttext[80];
        PFNWP oldproc;

        ulNumFileDlg++;
        WinQueryWindowPos(hwnd,&swp);
        WinGetMaxPosition(hwnd,&swpM);
        if(swpM.y < 0)
          swpM.y = 0;
        if(swpM.cy > yScreen)
          swpM.cy = yScreen;
        swp.y = swpM.y;
        swp.cy = swpM.cy;

        WinSetWindowPos(hwnd,
                        HWND_TOP,
                        swp.x,
                        swp.y,
                        swp.cx,
                        swp.cy,
                        SWP_SIZE | SWP_MOVE);

        WinQueryWindowPos(WinWindowFromID(hwnd,DID_OK_PB),&swpO);
        WinQueryWindowPos(WinWindowFromID(hwnd,DID_FILENAME_ED),&swpM);
        efh = swpM.cy;
        lasty = swp.cy - (WinQuerySysValue(HWND_DESKTOP,SV_CYTITLEBAR) +
                          WinQuerySysValue(HWND_DESKTOP,SV_CYDLGFRAME) +
                          4);   /* starting position */

        /* create help text field */
        WinQueryWindowPos(WinWindowFromID(hwnd,DID_FILENAME_TXT),&swpM);
        WinCreateWindow(hwnd,
                        WC_STATIC,
                        (PSZ)NULL,
                        WS_VISIBLE | SS_TEXT | DT_CENTER | DT_VCENTER,
                        4 + WinQuerySysValue(HWND_DESKTOP,SV_CXDLGFRAME),
                        lasty - swpM.cy,
                        swp.cx - (8 + (WinQuerySysValue(HWND_DESKTOP,
                                                        SV_CXDLGFRAME) * 2)),
                        swpM.cy,
                        hwnd,
                        HWND_TOP,
                        IDF_HELP,
                        NULL,
                        NULL);
        lasty -= swpM.cy + 4;
        WinSubclassWindow(WinWindowFromID(hwnd,IDF_HELP),
                          MySubProc);
        /* create file data text field */
        WinCreateWindow(hwnd,
                        WC_STATIC,
                        (PSZ)NULL,
                        WS_VISIBLE | SS_TEXT | DT_CENTER | DT_VCENTER,
                        4 + WinQuerySysValue(HWND_DESKTOP,SV_CXDLGFRAME),
                        lasty - swpM.cy,
                        swp.cx - (8 + (WinQuerySysValue(HWND_DESKTOP,
                                                        SV_CXDLGFRAME) * 2)),
                        swpM.cy,
                        hwnd,
                        HWND_TOP,
                        IDF_FILEDATA,
                        NULL,
                        NULL);
        lasty -= swpM.cy + 4;
        WinSubclassWindow(WinWindowFromID(hwnd,IDF_FILEDATA),
                          MySubProc);

        /* create "favorite" dropdowns */
        WinQueryWindowPos(WinWindowFromID(hwnd,DID_FILES_LB),&swpM);
        WinCreateWindow(hwnd,
                        WC_COMBOBOX,
                        (PSZ)NULL,
                        WS_VISIBLE | CBS_DROPDOWN | LS_HORZSCROLL,
                        swpM.x,
                        swpO.y + swpO.cy + 4,
                        swpM.cx,
                        lasty - (swpO.y + swpO.cy + 4),
                        hwnd,
                        HWND_TOP,
                        IDF_FILES,
                        NULL,
                        NULL);

        WinQueryWindowPos(WinWindowFromID(hwnd,DID_DIRECTORY_LB),&swpM);
        WinCreateWindow(hwnd,
                        WC_COMBOBOX,
                        (PSZ)NULL,
                        WS_VISIBLE | CBS_DROPDOWN | LS_HORZSCROLL,
                        swpM.x,
                        swpO.y + swpO.cy + 4,
                        swpM.cx,
                        lasty - (swpO.y + swpO.cy + 4),
                        hwnd,
                        HWND_TOP,
                        IDF_DIRS,
                        NULL,
                        NULL);
        lasty -= efh + 2;

        *buttext = '~';
        buttext[1] = 0;

        /* reposition/resize default controls */
        WinQueryWindowPos(WinWindowFromID(hwnd,DID_FILENAME_TXT),&swpM);
        WinQueryWindowText(WinWindowFromID(hwnd,DID_FILENAME_TXT),
                           79,
                           buttext + 1);
        WinDestroyWindow(WinWindowFromID(hwnd,DID_FILENAME_TXT));
        WinCreateWindow(hwnd,
                        WC_BUTTON,
                        buttext,
                        WS_VISIBLE | BS_PUSHBUTTON | BS_NOPOINTERFOCUS |
                        BS_NOBORDER,
                        swpM.x,
                        lasty - swpM.cy,
                        swpM.cx,
                        swpM.cy,
                        hwnd,
                        HWND_TOP,
                        DID_FILENAME_TXT,
                        MPVOID,
                        MPVOID);
        lasty -= swpM.cy + 2;
        WinQueryWindowPos(WinWindowFromID(hwnd,DID_FILENAME_ED),&swpM);
        WinSetWindowPos(WinWindowFromID(hwnd,DID_FILENAME_ED),
                        HWND_TOP,
                        swpM.x,
                        lasty - swpM.cy,
                        swpM.cx,
                        swpM.cy,
                        SWP_MOVE);
        lasty -= swpM.cy + 2;
        WinQueryWindowPos(WinWindowFromID(hwnd,DID_FILTER_TXT),&swpM);
        WinSetWindowPos(WinWindowFromID(hwnd,DID_FILTER_TXT),
                        HWND_TOP,
                        swpM.x,
                        lasty - swpM.cy,
                        swpM.cx,
                        swpM.cy,
                        SWP_MOVE);
        lasty -= swpM.cy + 2;
        WinQueryWindowPos(WinWindowFromID(hwnd,DID_FILTER_CB),&swpM);
        WinSetWindowPos(WinWindowFromID(hwnd,DID_FILTER_CB),
                        HWND_TOP,
                        swpM.x,
                        swpO.y + swpO.cy + 4,
                        swpM.cx,
                        lasty - (swpO.y + swpO.cy + 4),
                        SWP_MOVE | SWP_SIZE);
        lasty -= efh + 2;
        WinQueryWindowPos(WinWindowFromID(hwnd,DID_DRIVE_TXT),&swpM);
        WinSetWindowPos(WinWindowFromID(hwnd,DID_DRIVE_TXT),
                        HWND_TOP,
                        swpM.x,
                        lasty - swpM.cy,
                        swpM.cx,
                        swpM.cy,
                        SWP_MOVE);
        WinQueryWindowPos(WinWindowFromID(hwnd,DID_FILES_TXT),&swpM);
        WinQueryWindowText(WinWindowFromID(hwnd,DID_FILES_TXT),
                           79,
                           buttext + 1);
        WinDestroyWindow(WinWindowFromID(hwnd,DID_FILES_TXT));
        WinCreateWindow(hwnd,
                        WC_BUTTON,
                        buttext,
                        WS_VISIBLE | BS_PUSHBUTTON | BS_NOPOINTERFOCUS |
                        BS_NOBORDER,
                        swpM.x,
                        lasty - swpM.cy,
                        swpM.cx,
                        swpM.cy,
                        hwnd,
                        HWND_TOP,
                        DID_FILES_TXT,
                        MPVOID,
                        MPVOID);
        lasty -= swpM.cy + 2;
        WinQueryWindowPos(WinWindowFromID(hwnd,DID_DRIVE_CB),&swpM);
        WinSetWindowPos(WinWindowFromID(hwnd,DID_DRIVE_CB),
                        HWND_TOP,
                        swpM.x,
                        swpO.y + swpO.cy + 4,
                        swpM.cx,
                        lasty - (swpO.y + swpO.cy + 4),
                        SWP_MOVE | SWP_SIZE);
        WinQueryWindowPos(WinWindowFromID(hwnd,DID_FILES_LB),&swpM);
        WinSetWindowPos(WinWindowFromID(hwnd,DID_FILES_LB),
                        HWND_TOP,
                        swpM.x,
                        swpO.y + swpO.cy + 4,
                        swpM.cx,
                        lasty - (swpO.y + swpO.cy + 4),
                        SWP_MOVE | SWP_SIZE);
        lasty -= efh + 2;
        WinQueryWindowPos(WinWindowFromID(hwnd,DID_DIRECTORY_TXT),&swpM);
        WinQueryWindowText(WinWindowFromID(hwnd,DID_DIRECTORY_TXT),
                           79,
                           buttext + 1);
        WinDestroyWindow(WinWindowFromID(hwnd,DID_DIRECTORY_TXT));
        WinCreateWindow(hwnd,
                        WC_BUTTON,
                        buttext,
                        WS_VISIBLE | BS_PUSHBUTTON | BS_NOPOINTERFOCUS |
                        BS_NOBORDER,
                        swpM.x,
                        lasty - swpM.cy,
                        swpM.cx,
                        swpM.cy,
                        hwnd,
                        HWND_TOP,
                        DID_DIRECTORY_TXT,
                        MPVOID,
                        MPVOID);
        lasty -= swpM.cy + 2;
        WinQueryWindowPos(WinWindowFromID(hwnd,DID_DIRECTORY_LB),&swpM);
        WinSetWindowPos(WinWindowFromID(hwnd,DID_DIRECTORY_LB),
                        HWND_TOP,
                        swpM.x,
                        swpO.y + swpO.cy + 4,
                        swpM.cx,
                        lasty - (swpO.y + swpO.cy + 4),
                        SWP_MOVE | SWP_SIZE);
        oldproc = WinSubclassWindow(WinWindowFromID(WinWindowFromID(hwnd,
                                                                    IDF_FILES),
                                                    CBID_EDIT),
                                    MySubProc);
        if(oldproc)
          WinSetWindowPtr(WinWindowFromID(WinWindowFromID(hwnd,IDF_FILES),
                                          CBID_EDIT),
                          0,
                          (PVOID)oldproc);
        oldproc = WinSubclassWindow(WinWindowFromID(WinWindowFromID(hwnd,
                                                                    IDF_DIRS),
                                                    CBID_EDIT),
                                    MySubProc);
        if(oldproc)
          WinSetWindowPtr(WinWindowFromID(WinWindowFromID(hwnd,IDF_DIRS),
                                          CBID_EDIT),
                          0,
                          (PVOID)oldproc);
        PostMsg(WinWindowFromID(WinWindowFromID(hwnd,IDF_DIRS),CBID_EDIT),
                UM_REMIND,
                MPVOID,
                MPVOID);
      }
      SetPresParams(WinWindowFromID(hwnd,IDF_HELP),
                    RGB_WHITE,
                    RGB_DARKBLUE,
                    -1,
                    "8.Helv");
      SetPresParams(WinWindowFromID(hwnd,IDF_FILEDATA),
                    RGB_WHITE,
                    RGB_CHARCOAL,
                    -1,
                    "8.Helv");
      WinSendMsg(WinWindowFromID(WinWindowFromID(hwnd,IDF_FILES),
                                 CBID_EDIT),
                 EM_SETTEXTLIMIT,
                 MPFROM2SHORT(CCHMAXPATH,0),
                 MPVOID);
      WinSendMsg(WinWindowFromID(WinWindowFromID(hwnd,IDF_DIRS),
                                 CBID_EDIT),
                 EM_SETTEXTLIMIT,
                 MPFROM2SHORT(CCHMAXPATH,0),
                 MPVOID);
      PostMsg(hwnd,
              UM_SETUP,
              MPFROMLONG(1),
              MPFROMLONG(-1));
      PostMsg(hwnd,
              UM_REMIND,
              MPFROMLONG(1),
              MPFROMLONG(-1));
      break;

    case UM_REMIND:
      if((long)mp1 != 1 ||
         (long)mp2 != -1)
        break;
      {
        PFNWP    oldproc;
        FILEDLG *f;

        f = WinQueryWindowPtr(hwnd,0);
        if(fAggressive ||
           !f ||
           !f->pfnDlgProc) {
          /* subclass windows */
          WinSubclassWindow(WinWindowFromID(hwnd,IDF_HELP),
                            MySubProc);
          oldproc = WinSubclassWindow(WinWindowFromID(hwnd,DID_DIRECTORY_TXT),
                                      MySubProc);
          if(oldproc)
            WinSetWindowPtr(WinWindowFromID(hwnd,DID_DIRECTORY_TXT),0,
                            (PVOID)oldproc);
          oldproc = WinSubclassWindow(WinWindowFromID(hwnd,DID_FILES_TXT),
                                      MySubProc);
          if(oldproc)
            WinSetWindowPtr(WinWindowFromID(hwnd,DID_FILES_TXT),0,
                            (PVOID)oldproc);
          oldproc = WinSubclassWindow(WinWindowFromID(hwnd,DID_FILENAME_ED),
                                      MySubProc);
          if(oldproc)
            WinSetWindowPtr(WinWindowFromID(hwnd,DID_FILENAME_ED),0,
                            (PVOID)oldproc);
          oldproc = WinSubclassWindow(WinWindowFromID(hwnd,DID_FILENAME_TXT),
                                      MySubProc);
          if(oldproc)
            WinSetWindowPtr(WinWindowFromID(hwnd,DID_FILENAME_TXT),0,
                            (PVOID)oldproc);
          oldproc = WinSubclassWindow(WinWindowFromID(hwnd,DID_FILES_LB),
                                      MySubProc);
          if(oldproc)
            WinSetWindowPtr(WinWindowFromID(hwnd,DID_FILES_LB),0,
                            (PVOID)oldproc);
          oldproc = WinSubclassWindow(WinWindowFromID(hwnd,DID_DIRECTORY_LB),
                                      MySubProc);
          if(oldproc)
            WinSetWindowPtr(WinWindowFromID(hwnd,DID_DIRECTORY_LB),0,
                            (PVOID)oldproc);
          oldproc = WinSubclassWindow(WinWindowFromID(WinWindowFromID(hwnd,
                                                                      DID_DRIVE_CB),
                                                      CBID_EDIT),
                                      MySubProc);
          if(oldproc)
            WinSetWindowPtr(WinWindowFromID(WinWindowFromID(hwnd,
                                                            DID_DRIVE_CB),
                                            CBID_EDIT),
                            0,
                            (PVOID)oldproc);
          oldproc = WinSubclassWindow(WinWindowFromID(WinWindowFromID(hwnd,
                                                                      DID_FILTER_CB),
                                                      CBID_EDIT),
                                      MySubProc);
          if(oldproc)
            WinSetWindowPtr(WinWindowFromID(WinWindowFromID(hwnd,
                                                            DID_FILTER_CB),
                                            CBID_EDIT),
                            0,
                            (PVOID)oldproc);
        }
      }
      return 0;

    case WM_MOUSEMOVE:
      if(WinQueryWindowUShort(WinWindowFromID(hwnd,IDF_HELP),0)) {
        WinSetDlgItemText(hwnd,
                          IDF_HELP,
                          "");
        WinSetWindowUShort(WinWindowFromID(hwnd,IDF_HELP),0,0);
      }
      break;

    case WM_MENUEND:
      {
        FILEDLG *f;

        f = WinQueryWindowPtr(hwnd,0);
        if(!f ||
           !f->pfnDlgProc)
          WinDestroyWindow((HWND)mp2);
        WinSetFocus(HWND_DESKTOP,hwnd);
      }
      break;

    case WM_APPTERMINATENOTIFY:
      PostMsg(hwnd,
              UM_SETUP,
              MPFROMLONG(1),
              MPFROMLONG(-1));
      break;

    case UM_SETUP:
      if((long)mp1 != 1 ||
         (long)mp2 != -1)
        break;
      { /* fill "favorite" pulldowns */
        char  *p,s[CCHMAXPATH + 4];
        HFILE  hf;
        ULONG  action,lastposn;
        long   pos;

        WinSendDlgItemMsg(hwnd,
                          IDF_FILES,
                          LM_DELETEALL,
                          MPVOID,
                          MPVOID);
        WinSendDlgItemMsg(hwnd,
                          IDF_DIRS,
                          LM_DELETEALL,
                          MPVOID,
                          MPVOID);
        WinSetDlgItemText(hwnd,
                          IDF_FILES,
                          "User-defined files");
        WinSetDlgItemText(hwnd,
                          IDF_DIRS,
                          "User-defined directories");
        sprintf(s,
                "%sFILES.LST",
                mydir);
        if(!DosOpen(s,
                    &hf,
                    &action,
                    0,
                    FILE_NORMAL,
                    OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                    OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_RANDOMSEQUENTIAL |
                    OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYWRITE |
                    OPEN_ACCESS_READONLY,
                    NULL)) {
          while(!DosRead(hf,
                         s,
                         CCHMAXPATH + 2,
                         &action) &&
                action) {
            DosSetFilePtr(hf,
                          0,
                          FILE_CURRENT,
                          &lastposn);
            s[action] = 0;
            p = s;
            while(*p &&
                  *p == ' ')
              p++;
            if(p > s)
              memmove(s,
                      p,
                      strlen(p) + 1);
            p = s;
            while(*p &&
                  *p != '\r' &&
                  *p != '\n')
              p++;
            pos = strlen(s) - (p - s);
            if(*p == '\r' ||
               *p == '\n') {
              pos--;
              if(*(p + 1) == '\n')
                pos--;
            }
            *p = 0;
            if(p > s)
              p--;
            while(*s &&
                  (*p == ' ' ||
                   *p == '\x1a')) {
              *p = 0;
              p--;
            }
            if(pos)
              DosSetFilePtr(hf,
                            lastposn - pos,
                            FILE_BEGIN,
                            &action);
            if(*s &&
               IsFile(s) == 1)
              WinSendDlgItemMsg(hwnd,
                                IDF_FILES,
                                LM_INSERTITEM,
                                MPFROM2SHORT(LIT_END,0),
                                MPFROMP(s));
          }
          DosClose(hf);
        }
        if(*recentfiles[ulFile]) {
          for(pos = ulFile;pos < NUMRECENT;pos++) {
            if(*recentfiles[pos])
              WinSendDlgItemMsg(hwnd,
                                IDF_FILES,
                                LM_INSERTITEM,
                                MPFROM2SHORT(LIT_END,0),
                                MPFROMP(recentfiles[pos]));
          }
          for(pos = 0;pos < ulFile;pos++) {
            if(*recentfiles[pos])
              WinSendDlgItemMsg(hwnd,
                                IDF_FILES,
                                LM_INSERTITEM,
                                MPFROM2SHORT(LIT_END,0),
                                MPFROMP(recentfiles[pos]));
          }
        }
        sprintf(s,
                "%sDIRS.LST",
                mydir);
        if(!DosOpen(s,
                    &hf,
                    &action,
                    0,
                    FILE_NORMAL,
                    OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                    OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_RANDOMSEQUENTIAL |
                    OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYWRITE |
                    OPEN_ACCESS_READONLY,
                    NULL)) {
          while(!DosRead(hf,
                         s,
                         CCHMAXPATH + 2,
                         &action) &&
                action) {
            DosSetFilePtr(hf,
                          0,
                          FILE_CURRENT,
                          &lastposn);
            s[action] = 0;
            p = s;
            while(*p && *p == ' ')
              p++;
            if(p > s)
              memmove(s,
                      p,
                      strlen(p) + 1);
            p = s;
            while(*p &&
                  *p != '\r' &&
                  *p != '\n')
              p++;
            pos = strlen(s) - (p - s);
            if(*p == '\r' ||
               *p == '\n') {
              pos--;
              if(*(p + 1) == '\n')
                pos--;
            }
            *p = 0;
            if(p > s)
              p--;
            while(*s &&
                  (*p == ' ' ||
                   *p == '\x1a')) {
              *p = 0;
              p--;
            }
            if(pos)
              DosSetFilePtr(hf,
                            lastposn - pos,
                            FILE_BEGIN,
                            &action);
            if(*s /* && IsFile(s) == 0 */)
              WinSendDlgItemMsg(hwnd,
                                IDF_DIRS,
                                LM_INSERTITEM,
                                MPFROM2SHORT(LIT_END,0),
                                MPFROMP(s));
          }
          DosClose(hf);
        }
        if(*recentdirs[ulDir]) {
          for(pos = ulDir;pos < NUMRECENT;pos++) {
            if(*recentdirs[pos])
              WinSendDlgItemMsg(hwnd,
                                IDF_DIRS,
                                LM_INSERTITEM,
                                MPFROM2SHORT(LIT_END,0),
                                MPFROMP(recentdirs[pos]));
          }
          for(pos = 0;pos < ulDir;pos++) {
            if(*recentdirs[pos])
              WinSendDlgItemMsg(hwnd,
                                IDF_DIRS,
                                LM_INSERTITEM,
                                MPFROM2SHORT(LIT_END,0),
                                MPFROMP(recentdirs[pos]));
          }
        }
      }
      return 0;

    case UM_OPEN:
      {
        char        s[CCHMAXPATH];
        PROGDETAILS pgd;

        switch(SHORT1FROMMP(mp2)) {
          case IDF_FILES:
          case DID_FILES_LB:
            sprintf(s,
                    "%sFILES.LST",
                    mydir);
            break;

          case IDF_DIRS:
          case DID_DIRECTORY_LB:
            sprintf(s,
                    "%sDIRS.LST",
                    mydir);
            break;

          default:
            return 0;
        }
        memset(&pgd,0,sizeof(pgd));
        pgd.Length = sizeof(pgd);
        pgd.swpInitial.fl = SWP_ACTIVATE | SWP_ZORDER;
        pgd.swpInitial.hwndInsertBehind = HWND_TOP;
        pgd.progt.fbVisible = SHE_VISIBLE;
        pgd.progt.progc = PROG_DEFAULT;
        pgd.pszExecutable = (*editor) ? editor : "E.EXE";
        WinStartApp(hwnd,
                    &pgd,
                    s,
                    NULL,
                    0);
      }
      return 0;

    case UM_CONTEXTMENU:
      {
        HWND hwndMenu;

        hwndMenu = WinLoadMenu(HWND_DESKTOP,
                               hModPrime,
                               SHORT1FROMMP(mp2));
        if(hwndMenu) {

          POINTL ptl;

          WinQueryPointerPos(HWND_DESKTOP,&ptl);
          WinPopupMenu(HWND_DESKTOP,
                       hwnd,
                       hwndMenu,
                       ptl.x - 4,
                       ptl.y - 4,
                       0,
                       PU_HCONSTRAIN | PU_VCONSTRAIN |
                       PU_KEYBOARD   | PU_MOUSEBUTTON1);
        }
      }
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case DID_FILES_LB:
          switch(SHORT2FROMMP(mp1)) {
            case LN_SELECT:
              WinSetDlgItemText(hwnd,
                                IDF_FILEDATA,
                                "");
              {
                static char s[CCHMAXPATH];

                if(MakeFileName(hwnd,
                                s,
                                1)) {

                  static FILEFINDBUF4 fb4;
                  HDIR                hdir = HDIR_CREATE;
                  ULONG               nm   = 1;

                  if(!DosFindFirst(s,
                                   &hdir,
                                   FILE_NORMAL | FILE_ARCHIVED |
                                   FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY,
                                   &fb4,
                                   sizeof(fb4),
                                   &nm,
                                   FIL_QUERYEASIZE)) {
                    DosFindClose(hdir);
                    sprintf(s,
                            "%lu byte%s   %lu bytes EAs   "
                            "%02u:%02u:%02u %04u/%02u/%02u   "
                            "Attr: [%c%c%c%c]",
                            fb4.cbFile,
                            &"s"[fb4.cbFile == 1],
                            ((fb4.cbList > 4) ?
                              fb4.cbList / 2 :
                              0),
                            fb4.ftimeLastWrite.hours,
                            fb4.ftimeLastWrite.minutes,
                            fb4.ftimeLastWrite.twosecs * 2,
                            fb4.fdateLastWrite.year + 1980,
                            fb4.fdateLastWrite.month,
                            fb4.fdateLastWrite.day,
                            ((fb4.attrFile & FILE_ARCHIVED) != 0) ? 'A' : '-',
                            ((fb4.attrFile & FILE_READONLY) != 0) ? 'R' : '-',
                            ((fb4.attrFile & FILE_HIDDEN)   != 0) ? 'H' : '-',
                            ((fb4.attrFile & FILE_SYSTEM)   != 0) ? 'S' : '-');
                    WinSetDlgItemText(hwnd,
                                      IDF_FILEDATA,
                                      s);
                  }
                }
              }
              break;
          }
          break;

        case IDF_FILES:
        case IDF_DIRS:
          {
            char s[CCHMAXPATH],filename[CCHMAXPATH];

            switch(SHORT2FROMMP(mp1)) {
              case CBN_ENTER:
                WinQueryDlgItemText(hwnd,
                                    SHORT1FROMMP(mp1),
                                    CCHMAXPATH,
                                    s);
                if(*s) {
                  switch(SHORT1FROMMP(mp1)) {
                    case IDF_FILES:
                      WinSetDlgItemText(hwnd,
                                        DID_FILENAME_ED,
                                        s);
                      PostMsg(hwnd,
                              WM_COMMAND,
                              MPFROM2SHORT(DID_OK,0),
                              MPVOID);
                      WinSetDlgItemText(hwnd,
                                        SHORT1FROMMP(mp1),
                                        "User-defined files");
                      break;

                    case IDF_DIRS:
ReDir:
                      if(!IsFile(s)) {
                        if(s[strlen(s) - 1] != '\\')
                          strcat(s,"\\");
                        WinQueryDlgItemText(hwnd,
                                            DID_FILENAME_ED,
                                            CCHMAXPATHCOMP,
                                            filename);
                        strncat(s,
                                filename,
                                CCHMAXPATH - strlen(s));
                        s[CCHMAXPATH - 1] = 0;
                        if(!strchr(s,'?') &&
                           !strchr(s,'*'))
                          strcat(s,"*");
                        WinSetDlgItemText(hwnd,
                                          DID_FILENAME_ED,
                                          s);
                        WinSendMsg(hwnd,
                                   WM_COMMAND,
                                   MPFROM2SHORT(DID_OK,0),
                                   MPVOID);
                        if(*filename)
                          WinSetDlgItemText(hwnd,
                                            DID_FILENAME_ED,
                                            filename);
                      }
                      else {

                        ULONG ret;

                        ret = WinDlgBox(HWND_DESKTOP,
                                        WinQueryWindow(hwnd,QW_PARENT),
                                        MkdirProc,
                                        hModPrime,
                                        CRD_FRAME,
                                        s);
                        if(ret &&
                           ret != DID_ERROR &&
                           *s)
                          goto ReDir;
                      }
                      WinSetDlgItemText(hwnd,
                                        SHORT1FROMMP(mp1),
                                        "User-defined directories");
                      break;
                  }
                }
                break;
            }
          }
          return 0;
      }
      break;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case DID_DRIVE_CB:
        case DID_FILTER_CB:
          {
            static char s[CCHMAXPATH * 2],filename[CCHMAXPATH];
            char       *fname,test;
            BOOL        full;
            USHORT      id;
            HFILE       hf;
            ULONG       action,lastposn;
            long        x;

            switch(SHORT1FROMMP(mp1)) {
              case DID_DRIVE_CB:
                id = IDF_DIRS;
                full = 0;
                fname = "DIRS.LST";
                break;
              case DID_FILTER_CB:
                id = IDF_FILES;
                full = 3;
                fname = "FILES.LST";
                break;
            }
            if(MakeFileName(hwnd,
                            s,
                            full)) {
              for(x = 0;x < NUMRECENT;x++) {
                if(!stri_cmp(s,
                             ((id == IDF_DIRS) ?
                              recentdirs[x] :
                              recentfiles[x])))
                  break;
              }
              if(x < NUMRECENT ||
                 (SHORT)WinSendDlgItemMsg(hwnd,
                                          id,
                                          LM_SEARCHSTRING,
                                          MPFROM2SHORT(0,0),
                                          MPFROMP(s)) < 0) {
                if(x >= NUMRECENT)
                  WinSendDlgItemMsg(hwnd,
                                    id,
                                    LM_INSERTITEM,
                                    MPFROM2SHORT(LIT_END,0),
                                    MPFROMP(s));
                sprintf(filename,
                        "%s%s",
                        mydir,
                        fname);
                if(!DosOpen(filename,
                            &hf,
                            &action,
                            0,
                            FILE_NORMAL,
                            OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                            OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_SEQUENTIAL |
                            OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYWRITE |
                            OPEN_ACCESS_READWRITE,
                            NULL)) {
                  if(!DosSetFilePtr(hf,
                                    -1,
                                    FILE_END,
                                    &lastposn)) {
                    action = 1;
                    if(!DosRead(hf,
                                &test,
                                action,
                                &action)) {
                      if(test == 26)
                        DosSetFileSize(hf,
                                       lastposn);
                      if(!DosSetFilePtr(hf,
                                        -1,
                                        FILE_END,
                                        &lastposn)) {
                        action = 1;
                        if(!DosRead(hf,
                                    &test,
                                    action,
                                    &action)) {
                          if(test != '\n') {
                            action = 2;
                            DosWrite(hf,
                                     "\r\n",
                                     action,
                                     &action);
                          }
                        }
                      }
                    }
                  }
                  if(!DosSetFilePtr(hf,
                                    0,
                                    FILE_END,
                                    &lastposn)) {
                    action = strlen(s);
                    if(!DosWrite(hf,
                                 s,
                                 action,
                                 &action)) {
                      action = 2;
                      DosWrite(hf,
                               "\r\n",
                               action,
                               &action);
                    }
                  }
                  DosClose(hf);
                }
              }
            }
          }
          return 0;

        case DID_FILES_LB:
        case DID_DIRECTORY_LB:
          PostMsg(hwnd,
                  UM_OPEN,
                  MPVOID,
                  MPFROM2SHORT(SHORT1FROMMP(mp1),0));
          return 0;

        case DID_DRIVE_TXT:
        case DID_FILTER_TXT:
        case DID_FILE_DIALOG:
        case DID_FILENAME_TXT:
        case DID_DIRECTORY_TXT:
        case DID_FILES_TXT:
        case DID_FILENAME_ED:
          {
            static char s[CCHMAXPATH * 2];
            BOOL        full;
            APIRET      rc;

            *s = 0;
            switch(SHORT1FROMMP(mp1)) {
              case DID_FILENAME_ED:
              case DID_FILE_DIALOG:
              case DID_FILES_TXT:
                full = 1;
                break;
              case DID_DIRECTORY_TXT:
              case DID_DRIVE_TXT:
              case DID_FILTER_TXT:
                full = 2;
                break;
              case DID_FILENAME_TXT:
                full = 3;
                break;
              default:
                return 0;
            }
            if(SHORT1FROMMP(mp1) == DID_DIRECTORY_TXT &&
               SHORT1FROMMP(mp2) == CMDSRC_PUSHBUTTON)
              full = 0;
            if(MakeFileName(hwnd,
                            s,
                            full)) {
              switch(SHORT1FROMMP(mp1)) {
                case DID_DRIVE_TXT:
                case DID_FILE_DIALOG:
                case DID_FILENAME_TXT:
                  {
                    static PROGDETAILS pgd;
                    static char        args[CCHMAXPATH * 3];
                    char              *env,*p;

                    env = (SHORT1FROMMP(mp1) == DID_DRIVE_TXT) ?
                           "VDIR.CMD" :
                           (*viewer) ?
                            viewer :
                            "AV2.CMD";
                    p = strrchr(env,'.');
                    if(p &&
                       (!stri_cmp(p,".EXE") ||
                        !stri_cmp(p,".COM"))) {
                      env = viewer;
                      strcpy(args,s);
                    }
                    else {
                      sprintf(args,
                              "/C %s%s%s %s%s%s",
                              (strchr(env,' ') != NULL) ?
                               "\"" :
                               "",
                              env,
                              (strchr(env,' ') != NULL) ?
                               "\"" :
                               "",
                              (strchr(s,' ') != NULL) ?
                               "\"" :
                               "",
                              s,
                              (strchr(s,' ') != NULL) ?
                               "\"" :
                               "");
                      if(DosScanEnv("COMSPEC",&env)) {
                        if(DosScanEnv("OS2_SHELL",&env)) {
                          env = "CMD.EXE";
                        }
                      }
                    }
                    memset(&pgd,
                           0,
                           sizeof(pgd));
                    pgd.Length = sizeof(pgd);
                    pgd.swpInitial.fl = SWP_ACTIVATE | SWP_ZORDER;
                    if(SHORT1FROMMP(mp1) == DID_DRIVE_TXT ||
                       !*viewer)
                      pgd.swpInitial.fl |= SWP_MINIMIZE;
                    pgd.swpInitial.hwndInsertBehind = HWND_TOP;
                    pgd.progt.fbVisible = SHE_VISIBLE;
                    pgd.progt.progc = PROG_DEFAULT;
                    pgd.pszExecutable = env;
                    WinStartApp((HWND)0,
                                &pgd,
                                args,
                                NULL,
                                0);
                  }
                  break;
                case DID_DIRECTORY_TXT:
                case DID_FILES_TXT:
                  {
                    HOBJECT hWPSObject;

                    hWPSObject = WinQueryObject(s);
                    if(hWPSObject != NULLHANDLE) {
                      WinSetFocus(HWND_DESKTOP,
                                  hwndDesktop);
                      WinSetObjectData(hWPSObject,
                                       "OPEN=DEFAULT");
                    }
                  }
                  break;
                case DID_FILENAME_ED:
                  if(fConfirmFileDel) {
                    if(WinMessageBox(HWND_DESKTOP,
                                     hwnd,
                                     s,
                                     "Delete this file?",
                                     0,
                                     MB_YESNOCANCEL | MB_MOVEABLE |
                                     MB_ICONASTERISK) != MBID_YES)
                      return 0;
                  }
                  rc = DosDelete(s);
                  if(!rc) {

                    SHORT sSelect;

                    sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
                                                       DID_FILES_LB,
                                                       LM_QUERYSELECTION,
                                                       MPFROMSHORT(LIT_CURSOR),
                                                       MPVOID);
                    if(sSelect >= 0)
                      WinSendDlgItemMsg(hwnd,
                                        DID_FILES_LB,
                                        LM_DELETEITEM,
                                        MPFROMSHORT(sSelect),
                                        MPVOID);
                  }
                  else {
                    DosBeep(50,100);
                    WinSetDlgItemText(hwnd,
                                      IDF_HELP,
                                      "Error removing file.");
                  }
                  break;
                case DID_FILTER_TXT:
                  if(fConfirmDirDel) {
                    if(WinMessageBox(HWND_DESKTOP,
                                     hwnd,
                                     s,
                                     "Remove this directory?",
                                     0,
                                     MB_YESNOCANCEL | MB_MOVEABLE |
                                     MB_ICONASTERISK) != MBID_YES)
                      return 0;
                  }
                  rc = DosDeleteDir(s);
                  if(!rc) {

                    SHORT sSelect;

                    sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
                                                       DID_DIRECTORY_LB,
                                                       LM_QUERYSELECTION,
                                                       MPFROMSHORT(LIT_CURSOR),
                                                       MPVOID);
                    if(sSelect >= 0)
                      WinSendDlgItemMsg(hwnd,
                                        DID_DIRECTORY_LB,
                                        LM_DELETEITEM,
                                        MPFROMSHORT(sSelect),
                                        MPVOID);
                  }
                  else {
                    DosBeep(50,100);
                    WinSetDlgItemText(hwnd,
                                      IDF_HELP,
                                      (rc == 16) ?
                                       "That directory isn't empty or is the "
                                       "current directory of this or some "
                                       "other session" :
                                       "Error removing directory.");
                  }
                  break;
                default:
                  DosBeep(50,100);
                  break;
              }
            }
            else {
              DosBeep(250,50);
              WinSetDlgItemText(hwnd,
                                IDF_HELP,
                                "Select an item first.");
              WinSetWindowUShort(WinWindowFromID(hwnd,IDF_HELP),
                                 0,
                                 DID_FILES_TXT);
            }
          }
          return 0;
      }
      break;

    case WM_DESTROY:
      if(fRememberDirs) {

        FILEDLG    *f;
        static char s[CCHMAXPATH * 2];

        f = WinQueryWindowPtr(hwnd,0);
        if(f &&
           f->lReturn == DID_OK_PB) {
          if(MakeFileName(hwnd,
                          s,
                          0))
            AddRecentDir(WinWindowFromID(hwnd,IDF_DIRS),
                         s);
        }
      }
      if(fRememberFiles) {

        FILEDLG *f;

        f = WinQueryWindowPtr(hwnd,0);
        if(f &&
           f->lReturn == DID_OK_PB)
          AddRecentFile(WinWindowFromID(hwnd,IDF_FILES),
                        f->szFullFile);
      }
      ulNumFileDlg--;
      WinSubclassWindow(hwnd,
                        WinDefFileDlgProc);
      break;

    default:
      break;
  }
  {
    FILEDLG *f;

    f = WinQueryWindowPtr(hwnd,0);
    if(f &&
       f->pfnDlgProc)
      return f->pfnDlgProc(hwnd,msg,mp1,mp2);
  }
  return WinDefFileDlgProc(hwnd,msg,mp1,mp2);
}


char *ProgName (void) {

  static char ret[CCHMAXPATH];
  char       *p;
  TIB        *pTib;
  PIB        *pPib;

  p = NULL;
  if(!DosGetInfoBlocks(&pTib, &pPib)) {
    if(!DosQueryModuleName(pPib->pib_hmte,
                           sizeof(ret),
                           ret)) {
      p = strrchr(ret,'\\');
      if(p)
        p++;
      else
        p = ret;
    }
  }
  return p;
}


void ClockText (char *s) {

  static DATETIME dt;

  if(s) {
    *s = 0;
    if(!DosGetDateTime(&dt)) {
      sprintf(s,
              " %0*u:%02u%s ",
              ((ampm) ? 0 : 2),
              (dt.hours % ((ampm) ? 12 : 24)) +
               ((ampm && !(dt.hours % 12)) ? 12 : 0),
              dt.minutes,
              ((ampm) ? (dt.hours > 11) ? "pm" : "am" : ""));
      if(showdate)
        sprintf(s + strlen(s),
                "%02u/%02u ",
                dt.month,
                dt.day);
    }
  }
}


void DrawClock (HWND hwnd,HPS hps,char *s) {

  POINTL aptl[TXTBOX_COUNT];
  RECTL  rcl;
  POINTL ptl;

  if(s &&
     *s) {
    GpiQueryTextBox(hps,
                    strlen(s),
                    s,
                    TXTBOX_COUNT,
                    aptl);
    WinQueryWindowRect(hwnd,
                       &rcl);
    ptl.x = rcl.xRight - aptl[TXTBOX_TOPRIGHT].x;
    ptl.y = ((rcl.yTop / 2) -
              ((aptl[TXTBOX_TOPRIGHT].y +
               aptl[TXTBOX_BOTTOMLEFT].y) / 2));
    ptl.y++;
    if(ptl.x < 0)
      ptl.x = 0;
    if(ptl.y < 0)
      ptl.y = 0;
    GpiSetBackColor(hps,
                    ClockBack);
    GpiSetColor(hps,
                ClockFore);
    GpiSetMix(hps,
              FM_OVERPAINT);
    GpiSetBackMix(hps,
                  BM_OVERPAINT);
    GpiMove(hps,
            &ptl);
    GpiCharStringAt(hps,
                    &ptl,
                    strlen(s),
                    s);
  }
}


void PaintGradientWindow (HWND hwnd,HPS hps,RECTL *cliprcl,BOOL hilite,
                          UCHAR redStart,UCHAR greenStart,UCHAR blueStart,
                          UCHAR redEnd,UCHAR greenEnd,UCHAR blueEnd,
                          UCHAR redText,UCHAR greenText,UCHAR blueText,
                          UCHAR sSteps,UCHAR vGrade,UCHAR cGrade,
                          UCHAR tBorder,UCHAR fTexture,HBITMAP hbm) {

  register USHORT j;
  RECTL           rcl;
  POINTL          ptl;
  ULONG           tcolor;

  GpiCreateLogColorTable(hps,
                         0,
                         LCOLF_RGB,
                         0,
                         0,
                         0);
  GpiSetBackMix(hps,
                BM_LEAVEALONE);
  GpiSetMix(hps,
            FM_OVERPAINT);
  WinQueryWindowRect(hwnd,
                     &rcl);
  if(hilite) {
    if(hbm)
      ClipRectl = *cliprcl;
    if(!hbm ||
       !ObjectHwnd ||
       (!WinSendMsg(ObjectHwnd,
                    UM_TILEBITMAP,
                    MPFROMLONG((ULONG)hwnd),
                    MPFROMLONG((ULONG)hbm)) &&
        !WinSendMsg(ObjectHwnd,
                    UM_TILEBITMAP,
                    MPFROMLONG((ULONG)hwnd),
                    MPFROMLONG((ULONG)hbm)))) {

      SHORT redDiff,greenDiff,blueDiff,boost,limit;

      redDiff = redEnd - redStart;
      greenDiff = greenEnd - greenStart;
      blueDiff = blueEnd - blueStart;
      boost = (1 + (cGrade != 0));
      limit = sSteps / boost;
      for(j = 0;j < limit;j++) {

        USHORT  rj = (USHORT)redStart   + (redDiff   * j / ((sSteps / boost) - 1));
        USHORT  gj = (USHORT)greenStart + (greenDiff * j / ((sSteps / boost) - 1));
        USHORT  bj = (USHORT)blueStart  + (blueDiff  * j / ((sSteps / boost) - 1));

        GpiSetColor(hps,
                    (ULONG)(rj << 16) + (gj << 8) + bj);
        if(!cGrade) {
          if(!vGrade) {
            ptl.y = 0;
            ptl.x = j * rcl.xRight / sSteps;
            GpiMove(hps,
                    &ptl);
            ptl.y = rcl.yTop;
            ptl.x += rcl.xRight / sSteps;
          }
          else {
            ptl.x = 0;
            ptl.y = j * rcl.yTop / sSteps;
            GpiMove(hps,
                    &ptl);
            ptl.x = rcl.xRight;
            ptl.y += rcl.yTop / sSteps;
          }
          GpiBox(hps,
                 DRO_FILL,
                 &ptl,
                 0L,
                 0L);
        }
        else {
          if(!vGrade) {
            ptl.y = 0;
            ptl.x = (rcl.xRight / 2) - (j * rcl.xRight / sSteps);
            GpiMove(hps,
                    &ptl);
            ptl.y = rcl.yTop;
            ptl.x -= rcl.xRight / sSteps;
            GpiBox(hps,
                   DRO_FILL,
                   &ptl,
                   0L,
                   0L);
            ptl.y = 0;
            ptl.x = (rcl.xRight / 2) + (j * rcl.xRight / sSteps);
            GpiMove(hps,
                    &ptl);
            ptl.y = rcl.yTop;
            ptl.x += rcl.xRight / sSteps;
            GpiBox(hps,
                   DRO_FILL,
                   &ptl,
                   0L,
                   0L);
          }
          else {
            ptl.x = 0;
            ptl.y = (rcl.yTop / 2) - (j * rcl.yTop / sSteps);
            GpiMove(hps,
                    &ptl);
            ptl.x = rcl.xRight;
            ptl.y -= rcl.yTop / sSteps;
            GpiBox(hps,
                   DRO_FILL,
                   &ptl,
                   0L,
                   0L);
            ptl.x = 0;
            ptl.y = (rcl.yTop / 2) + (j * rcl.yTop / sSteps);
            GpiMove(hps,
                    &ptl);
            ptl.x = rcl.xRight;
            ptl.y += rcl.yTop / sSteps;
            GpiBox(hps,
                   DRO_FILL,
                   &ptl,
                   0L,
                   0L);
          }
        }
      }
    }
  }
  else {
    if(!WinQueryPresParam(hwnd,
                          PP_INACTIVECOLOR,
                          0,
                          NULL,
                          sizeof(LONG),
                          (PVOID)&tcolor,
                          QPF_PURERGBCOLOR))
      tcolor = WinQuerySysColor(HWND_DESKTOP,
                                SYSCLR_INACTIVETITLE,
                                0);
    GpiSetColor(hps,
                tcolor);
    ptl.x = 0;
    ptl.y = 0;
    GpiMove(hps,
            &ptl);
    ptl.x = rcl.xRight;
    ptl.y = rcl.yTop;
    GpiBox(hps,
           DRO_FILL,
           &ptl,
           0,
           0);
  }
  if(tBorder) {
    GpiSetColor(hps,
                (tBorder == 1) ?
                 RGB_DARKGRAY :
                 RGB_WHITE);
    ptl.x = 0;
    ptl.y = 0;
    GpiMove(hps,
            &ptl);
    ptl.y = rcl.yTop;
    GpiLine(hps,
            &ptl);
    ptl.x = rcl.xRight;
    GpiLine(hps,
            &ptl);
    GpiSetColor(hps,
                (tBorder == 1) ?
                 RGB_WHITE :
                 RGB_DARKGRAY);
    ptl.y = 0;
    GpiLine(hps,
            &ptl);
    ptl.x = 0;
    GpiLine(hps,
            &ptl);
  }
  {
    POINTL aptl[TXTBOX_COUNT];
    char   str[CCHMAXPATH];

    memset(aptl,
           0,
           sizeof(aptl));
    *str = 0;
    WinQueryWindowText(hwnd,
                       CCHMAXPATH,
                       str);
    if(*str) {
      GpiQueryTextBox(hps,
                      strlen(str),
                      str,
                      TXTBOX_COUNT,
                      aptl);
      ptl.x = (aptl[TXTBOX_TOPRIGHT].x / strlen(str)) * 2;
      ptl.y = (((rcl.yTop - rcl.yBottom) -
               aptl[TXTBOX_TOPRIGHT].y) / 2) -
                aptl[TXTBOX_BOTTOMLEFT].y;
      if(!WinQueryPresParam(hwnd,
                            ((hilite) ?
                             PP_ACTIVETEXTFGNDCOLOR :
                             PP_INACTIVETEXTFGNDCOLOR),
                            0,
                            NULL,
                            sizeof(LONG),
                            (PVOID)&tcolor,
                            QPF_PURERGBCOLOR))
        tcolor = WinQuerySysColor(HWND_DESKTOP,
                                  ((hilite) ?
                                  SYSCLR_ACTIVETITLETEXT :
                                  SYSCLR_INACTIVETITLETEXT),
                                  0);
      GpiSetColor(hps,
                  tcolor);
      GpiCharStringAt(hps,
                      &ptl,
                      strlen(str),
                      str);
    }
    if(fTexture) {

      long numbars,x,offset,
           startx = ptl.x + aptl[TXTBOX_TOPRIGHT].x + 8;

      if(startx < (rcl.xRight - rcl.xLeft) - 4) {
        numbars = ((rcl.yTop - rcl.yBottom) - 4) / 4;
        offset = (((rcl.yTop - rcl.yBottom) - (numbars * 4)) + 2) / 2;
        if(numbars) {
          ptl.x = startx;
          for(x = 0;x < numbars;x++) {
            ptl.y = (x * 4) + offset;
            GpiSetColor(hps,
                        (hilite) ?
                         RGB_DARKGRAY :
                         RGB_GRAY);
            GpiMove(hps,
                    &ptl);
            ptl.x = (rcl.xRight - rcl.xLeft) - 4;
            GpiLine(hps,
                    &ptl);
            if(!hilite)
              GpiSetColor(hps,
                          RGB_BLACK);
            ptl.y++;
            GpiLine(hps,
                    &ptl);
            if(hilite)
              GpiSetColor(hps,
                          RGB_WHITE);
            ptl.x = startx;
            GpiLine(hps,
                    &ptl);
            if(hilite) {
              ptl.y--;
              GpiLine(hps,
                      &ptl);
            }
          }
        }
      }
    }
    if(fClockInTitlebars &&
       hilite &&
       WinQueryActiveWindow(HWND_DESKTOP) == WinQueryWindow(hwnd,QW_PARENT)) {

      static char s[80];

      ClockText(s);
      if(*s)
        DrawClock(hwnd,
                  hps,
                  s);
    }
  }
}


MRESULT EXPENTRY TitleBarProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  static HWND hwndMenu = (HWND)0;
  static BOOL noadjust = FALSE;

  if(fEnhanceTitlebars) {
    switch(msg) {
      case UM_TIMER:
        if(WinSendMsg(hwnd,
                      TBM_QUERYHILITE,
                      MPVOID,
                      MPVOID) &&
           WinQueryActiveWindow(HWND_DESKTOP) ==
             WinQueryWindow(hwnd,QW_PARENT)) {

          HPS hps;

          hps = WinGetClipPS(hwnd,
                             (HWND)0,
                             0);
          if(hps) {

            static char s[80];

            GpiCreateLogColorTable(hps,
                                   0,
                                   LCOLF_RGB,
                                   0,
                                   0,
                                   0);
            ClockText(s);
            if(*s)
              DrawClock(hwnd,
                        hps,
                        s);
            WinReleasePS(hps);
          }
        }
        return 0;

      case WM_BUTTON2MOTIONSTART:
      case WM_BUTTON2DOWN:
      case WM_BUTTON2UP:
        if(fEnableTitlebarMenus) {
          if(msg == WM_BUTTON2DOWN) {
            if(WinQueryActiveWindow(HWND_DESKTOP) != WinQueryWindow(hwnd,
                                                                    QW_PARENT))
              WinSetActiveWindow(HWND_DESKTOP,
                                 WinQueryWindow(hwnd,QW_PARENT));
          }
          else if(msg == WM_BUTTON2MOTIONSTART)
            WinSendMsg(WinQueryWindow(hwnd,QW_PARENT),
                       WM_TRACKFRAME,
                       MPFROMSHORT(TF_MOVE),
                       MPVOID);
          return 0;
        }
        break;

      case WM_CONTEXTMENU:
        if(fEnableTitlebarMenus) {
          hwndMenu = WinLoadMenu(HWND_DESKTOP,
                                 hModPrime,
                                 (!fExtraMenus) ?
                                  FID_TITLEBAR :
                                  FID_TITLEBAREXTRA);
          if(hwndMenu) {

            POINTL ptl;
            HWND   hwndSysMenu;

            hwndSysMenu = WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                                          FID_SYSMENU);
            if(hwndSysMenu) {

              SHORT x,attr,cattrs[] = {SC_RESTORE,
                                       SC_MOVE,
                                       SC_SIZE,
                                       SC_MINIMIZE,
                                       SC_MAXIMIZE,
                                       SC_HIDE,
                                       SC_CLOSE,
                                       SC_TASKMANAGER,
                                       0};

              for(x = 0;cattrs[x];x++) {
                attr = (SHORT)WinSendMsg(hwndSysMenu,
                                         MM_QUERYITEMATTR,
                                         MPFROM2SHORT(cattrs[x],TRUE),
                                         MPFROMSHORT(MIA_DISABLED));
                if(attr)
                  WinSendMsg(hwndMenu,
                             MM_SETITEMATTR,
                             MPFROM2SHORT(cattrs[x],TRUE),
                             MPFROM2SHORT(MIA_DISABLED,MIA_DISABLED));
              }
            }
            ptl.x = SHORT1FROMMP(mp1);
            ptl.y = SHORT2FROMMP(mp1);
            WinMapWindowPoints(hwnd,
                               HWND_DESKTOP,
                               &ptl,
                               1);
            if(!WinPopupMenu(HWND_DESKTOP,
                             hwnd,
                             hwndMenu,
                             ptl.x - 4,
                             ptl.y - 4,
                             0,
                             PU_HCONSTRAIN | PU_VCONSTRAIN |
                             PU_KEYBOARD   | PU_MOUSEBUTTON1))
              WinDestroyWindow(hwndMenu);
          }
          return MRFROMSHORT(TRUE);
        }
        break;

      case WM_COMMAND:
        if(fEnableTitlebarMenus &&
           fExtraMenus) {
          switch(SHORT1FROMMP(mp1)) {
            case C_BACK:
              PostMsg(ObjectHwnd,
                      UM_HOOK2,
                      MPFROM2SHORT(C_BACK,0),
                      MPFROMLONG(WinQueryWindow(hwnd,QW_PARENT)));
              break;

            case C_SHOW:
              PostMsg(ObjectHwnd,
                      UM_HOOK2,
                      MPFROM2SHORT(C_SHOW,0),
                      MPFROMLONG(WinQueryWindow(hwnd,QW_PARENT)));
              break;

            case C_CMDLINE:
              {
                static char s[CCHMAXPATH + 1];
                ULONG  mp12;

                mp12 = ((WinGetKeyState(HWND_DESKTOP,VK_CTRL) & 0x8000) != 0);
                *s = 0;
                SaveDir(s);
                PostMsg(hwndConfig,
                        UM_CMDLINE,
                        MPFROMLONG(mp12),
                        ((*s) ? MPFROMP(s) : MPVOID));
              }
              break;

            case C_CALC:
              {
                POINTL ptl;

                WinQueryPointerPos(HWND_DESKTOP,
                                   &ptl);
                PostMsg(hwndConfig,
                        UM_CALC,
                        MPFROM2SHORT(ptl.x,ptl.y),
                        MPVOID);
              }
              break;
            case C_ROLLUP:
              PostMsg(ObjectHwnd,
                      UM_HOOK2,
                      MPFROM2SHORT(C_ROLLUP,0),
                      MPFROMLONG(WinQueryWindow(hwnd,QW_PARENT)));
              break;
            case C_TASKLIST:
              PostMsg(hwndConfig,
                      UM_SWITCHLIST,
                      MPVOID,
                      MPVOID);
              break;
            case C_VIRTUAL:
              PostMsg(hwndConfig,
                      UM_VIRTUAL,
                      MPVOID,
                      MPVOID);
              break;
            case C_CLIPMGR:
              PostMsg(ObjectHwnd3,
                      UM_CLIPMGR,
                      MPVOID,
                      MPVOID);
              break;
            case C_SHOWME:
              {
                POINTL ptl;

                WinQueryPointerPos(HWND_DESKTOP,
                                   &ptl);
                PostMsg(hwndConfig,
                        UM_SHOWME,
                        MPFROM2SHORT(ptl.x,ptl.y),
                        MPVOID);
              }
              break;
          }
          return 0;
        }
        break;

      case WM_SYSCOMMAND:
        if(fEnableTitlebarMenus)
          WinPostMsg(WinQueryWindow(hwnd,QW_PARENT),
                     msg,
                     mp1,
                     mp2);
        return 0;

      case WM_MENUEND:
        if((HWND)mp2 == hwndMenu) {

          HWND hwndFocus;

          WinDestroyWindow(hwndMenu);
          hwndMenu = (HWND)0;
          if(!noadjust) {
            hwndFocus = WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),FID_CLIENT);
            if(!hwndFocus)
              hwndFocus = WinQueryWindow(hwnd,QW_PARENT);
            WinSetFocus(HWND_DESKTOP,
                        hwndFocus);
          }
          noadjust = FALSE;
        }
        break;

      case WM_PRESPARAMCHANGED:
        if((ULONG)mp1 == TITLEBAR_PARM) {

          TBARPARMS tbp;

          if(!WinQueryPresParam(hwnd,
                                TITLEBAR_PARM,
                                0,
                                NULL,
                                sizeof(tbp),
                                (PVOID)&tbp,
                                QPF_NOINHERIT))
            WinSubclassWindow(hwnd,
                              (PFNWP)PFNWPTitle);
          else {
            if(hwndConfig &&
               (tbp.size != sizeof(TBARPARMS) ||
                tbp.hwnd != WinQueryWindow(hwnd,QW_PARENT))) {

              char   *progname = ProgName();
              MRESULT mr;

              mr = WinSendMsg(hwndConfig,
                              UM_CHECKTTLS,
                              MPFROMP(progname),
                              MPFROMLONG(hwnd));
              if((ULONG)mr < 2) {
                WinSubclassWindow(hwnd,
                                  (PFNWP)PFNWPTitle);
                WinRemovePresParam(hwnd,
                                   TITLEBAR_PARM);
              }
            }
            WinInvalidateRect(hwnd,
                              NULL,
                              FALSE);
          }
        }
        break;

      case TBM_SETHILITE:
        {
          MRESULT mr;

          mr = PFNWPTitle(hwnd,msg,mp1,mp2);
          WinInvalidateRect(hwnd,
                            NULL,
                            FALSE);
          return mr;
        }

      case WM_PAINT:
        {
          TBARPARMS    tbp;
          HPS          hps;
          RECTL        cliprcl;
          BOOL         okay = FALSE;

          if(WinQueryPresParam(hwnd,
                               TITLEBAR_PARM,
                               0,
                               NULL,
                               sizeof(tbp),
                               (PVOID)&tbp,
                               QPF_NOINHERIT)) {
            if(tbp.size == sizeof(TBARPARMS) &&
               tbp.hwnd == WinQueryWindow(hwnd,QW_PARENT)) {
              okay = TRUE;
              if(tbp.exclude)
                return PFNWPTitle(hwnd,msg,mp1,mp2);
            }
            else {

              char *progname = ProgName();

              if((ULONG)WinSendMsg(hwndConfig,
                                   UM_CHECKTTLS,
                                   MPFROMP(progname),
                                   MPFROMLONG(hwnd)) < 2) {
                WinSubclassWindow(hwnd,
                                  (PFNWP)PFNWPTitle);
                WinRemovePresParam(hwnd,
                                   TITLEBAR_PARM);
                return PFNWPTitle(hwnd,msg,mp1,mp2);
              }
            }
          }
          if(!okay) {
            memset(&tbp,
                   0,
                   sizeof(tbp));
            tbp.size       = sizeof(TBARPARMS);
            tbp.hwnd       = WinQueryWindow(hwnd,QW_PARENT);
            tbp.redStart   = redStart;
            tbp.greenStart = greenStart;
            tbp.blueStart  = blueStart;
            tbp.redEnd     = redEnd;
            tbp.greenEnd   = greenEnd;
            tbp.blueEnd    = blueEnd;
            tbp.redText    = redText;
            tbp.greenText  = greenText;
            tbp.blueText   = blueText;
            tbp.sSteps     = sSteps;
            tbp.vGrade     = vGrade;
            tbp.cGrade     = cGrade;
            tbp.tBorder    = tBorder;
            tbp.fTexture   = fTexture;
            tbp.hbm        = hbmTitles;
          }
          hps = WinBeginPaint(hwnd,
                              (HPS)0,
                              &cliprcl);
          if(hps) {
            PaintGradientWindow(hwnd,
                                hps,
                                &cliprcl,
                                (BOOL)WinSendMsg(hwnd,
                                                 TBM_QUERYHILITE,
                                                 MPVOID,
                                                 MPVOID),
                                tbp.redStart,
                                tbp.greenStart,
                                tbp.blueStart,
                                tbp.redEnd,
                                tbp.greenEnd,
                                tbp.blueEnd,
                                tbp.redText,
                                tbp.greenText,
                                tbp.blueText,
                                tbp.sSteps,
                                tbp.vGrade,
                                tbp.cGrade,
                                tbp.tBorder,
                                tbp.fTexture,
                                tbp.hbm);
            WinEndPaint(hps);
          }
        }
        return 0;

      case WM_DESTROY:
        {
          TBARPARMS tbp;

          if(WinQueryPresParam(hwnd,
                               TITLEBAR_PARM,
                               0,
                               NULL,
                               sizeof(tbp),
                               (PVOID)&tbp,
                               QPF_NOINHERIT) &&
             tbp.size == sizeof(TBARPARMS)) {
            if(tbp.hbm)
              WinSendMsg(ObjectHwnd,
                         UM_BITMAP,
                         MPFROMLONG(tbp.hbm),
                         MPVOID);
            WinRemovePresParam(hwnd,
                               TITLEBAR_PARM);
          }
        }
        break;
    }
  }
  else {
    WinSubclassWindow(hwnd,
                      (PFNWP)PFNWPTitle);
    {
      TBARPARMS tbp;

      if(WinQueryPresParam(hwnd,
                           TITLEBAR_PARM,
                           0,
                           NULL,
                           sizeof(tbp),
                           (PVOID)&tbp,
                           QPF_NOINHERIT) &&
         tbp.size == sizeof(TBARPARMS)) {
        if(tbp.hbm)
          WinSendMsg(ObjectHwnd,
                     UM_BITMAP,
                     MPFROMLONG(tbp.hbm),
                     MPVOID);
        WinRemovePresParam(hwnd,
                           TITLEBAR_PARM);
      }
    }
  }

  return PFNWPTitle(hwnd,msg,mp1,mp2);
}


#ifdef TESTING

MRESULT EXPENTRY FrameProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_SYSCOMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case SC_SIZE - 1:
          DosBeep(50,100);
          break;
      }
      break;
  }
  if(amclosing)
    WinSubclassWindow(hwnd,
                      PFNWPFrame);
  return PFNWPFrame(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY MenuProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case UM_SETUP:
      if(WinQueryWindowUShort(hwnd,QWS_ID) != FID_MINMAX)
        WinSubclassWindow(hwnd,PFNWPMenu);
      else {

        static MENUITEM mi;

        memset(&mi,
               0,
               sizeof(mi));
        mi.iPosition = 0;
        mi.hwndSubMenu = (HWND)0;
        mi.hItem = 0L;
        mi.afStyle = MIS_TEXT | MIS_SYSCOMMAND;
        mi.afAttribute = 0;
        mi.id = SC_SIZE - 1;
        WinSendMsg(hwnd,
                   MM_INSERTITEM,
                   MPFROMP(&mi),
                   MPFROMP("^"));
        WinSendMsg(WinQueryWindow(hwnd,QW_PARENT),
                   WM_UPDATEFRAME,
                   MPFROMLONG(FCF_SIZEBORDER),
                   MPVOID);
      }
      return 0;
  }
  if(amclosing) {
    WinSendMsg(hwnd,
               MM_DELETEITEM,
               MPFROMSHORT(SC_SIZE - 1),
               MPVOID);
    WinSubclassWindow(hwnd,
                      PFNWPMenu);
  }
  return PFNWPMenu(hwnd,msg,mp1,mp2);
}

#endif


void EXPENTRY SendProc (HAB hab,PSMHSTRUCT psmh,BOOL fInterTask) {

  switch(psmh->msg) {
    case TBM_QUERYHILITE:
    case TBM_SETHILITE:
    case WM_PAINT:
    case WM_CREATE:
      if(!amclosing &&
         psmh->hwnd != hwndDesktop) {

        static char ucClassname[9];

        WinQueryClassName(psmh->hwnd,
                          sizeof(ucClassname),
                          ucClassname);
        if(fEnhanceTitlebars &&
           WinQueryWindowUShort(psmh->hwnd,QWS_ID) == FID_TITLEBAR &&
           WinQueryWindowPtr(psmh->hwnd,QWP_PFNWP) == (PVOID)PFNWPTitle &&
           !strcmp(ucClassname,"#9")) {

          char *progname = ProgName();

          if(hwndConfig) {
            if(!WinSendMsg(hwndConfig,
                           UM_CHECKTTLS,
                           MPFROMP(progname),
                           MPFROMLONG(psmh->hwnd)))
              break;
          }
          WinSetWindowBits(psmh->hwnd,
                           QWL_STYLE,
                           0,
                           WS_PARENTCLIP);
          WinSubclassWindow(psmh->hwnd,
                            TitleBarProc);
          WinInvalidateRect(psmh->hwnd,
                            NULL,
                            FALSE);
        }
        else {
          if(psmh->msg == WM_CREATE &&
             WinQueryWindowPtr(psmh->hwnd,QWP_PFNWP) == (PVOID)PFNWPFrame)
            PostMsg(ObjectHwnd,
                    UM_ADDFRAME,
                    MPFROMLONG(psmh->hwnd),
                    MPVOID);
        }

#ifdef TESTING
        else {
          if(WinQueryWindowPtr(psmh->hwnd,QWP_PFNWP) == (PVOID)PFNWPMenu &&
             !strcmp(ucClassname,"#4")) {
            WinSubclassWindow(psmh->hwnd,
                              MenuProc);
            PostMsg(psmh->hwnd,
                    UM_SETUP,
                    MPVOID,
                    MPVOID);
          }
          else if(WinQueryWindowPtr(psmh->hwnd,QWP_PFNWP) == (PVOID)PFNWPFrame &&
                  (!strcmp(ucClassname,"#1") ||
                   !strcmp(ucClassname,"wpFolder")))
            WinSubclassWindow(psmh->hwnd,
                              FrameProc);
        }
#endif
      }
      break;

    case WM_INITDLG:
      if(!fDisabled &&
         !fSuspend &&
         (WinGetKeyState(HWND_DESKTOP,
                         VK_SCRLLOCK) & 0x0001) == 0) {
        if(fEnhanceFileDlg &&
           WinQueryWindowPtr(psmh->hwnd,QWP_PFNWP) ==
            (PVOID)WinDefFileDlgProc &&
           WinQueryWindowUShort(psmh->hwnd,QWS_ID) ==
            DID_FILE_DIALOG) {

          HENUM  henum;
          HWND   hwndC;
          BOOL   doit = TRUE;
          USHORT id;

          henum = WinBeginEnumWindows(psmh->hwnd);
          while((hwndC = WinGetNextWindow(henum)) != (HWND)0) {
            id = WinQueryWindowUShort(hwndC,QWS_ID);
            if((id < DID_FILE_DIALOG ||
                id > DID_APPLY_PB + 2) &&
               id != DID_OK &&
               id != DID_CANCEL &&
               id != 32770 &&
               id != 32771) {
              doit = FALSE;
              break;
            }
          }
          WinEndEnumWindows(henum);
          if(doit) {
            /* check program name against fexcludes list */
            char *progname = ProgName();

            if(progname &&
               hwndConfig &&
               WinSendMsg(hwndConfig,
                          UM_CHECKEXCLUDE,
                          MPFROMP(progname),
                          MPVOID))
              doit = FALSE;
            if(doit)
              WinSubclassWindow(psmh->hwnd,
                                MyFileDlgProc);
          }
        }
        PostMsg(ObjectHwnd,
                UM_POSMOUSE,
                MPFROMLONG(psmh->hwnd),
                MPVOID);
      }
      break;
  }
}


BOOL EXPENTRY DestroyProc (HAB hab,HWND hwnd,ULONG dummy) {

  PostMsg(ObjectHwnd,
          UM_REMOVEFRAME,
          MPFROMLONG(hwnd),
          MPVOID);
  return TRUE;
}

