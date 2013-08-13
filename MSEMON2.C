#define INCL_DOS
#define INCL_WIN
#define INCL_GPI

#define DATAM 1

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "msehook.h"
#include "mse.h"
#include "dialog.h"
#include "procstat.h"

extern APIRET16 APIENTRY16 Dos16MemAvail (PULONG pulAvailMem);


static void SetMonitorSize (HWND hwnd,HWND hwndMenu,SWP *swpH,char *s) {

  HPS    hps;
  POINTL aptl[TXTBOX_COUNT];
  SWP    swp;
  ULONG  fl = SWP_SIZE | SWP_SHOW | SWP_MOVE;
  long   cx,cy;

  if(s) {
    WinSetWindowText(hwnd,s);
    if(*s) {
      hps = WinGetPS(hwnd);
      GpiQueryTextBox(hps,
                      strlen(s),
                      s,
                      TXTBOX_COUNT,
                      aptl);
      WinReleasePS(hps);
      cx = aptl[TXTBOX_TOPRIGHT].x + 4;
      cy = aptl[TXTBOX_TOPRIGHT].y + 4;
      WinQueryWindowPos(hwnd,
                        &swp);
      if(swp.cx == cx &&
         swp.cy == cy)
        fl = 0;
      if(swpH->x + cx > xScreen)
        swpH->x = xScreen - cx;
      if(swpH->y + cy > yScreen)
        swpH->y = yScreen - cy;
      if(fSwapFloat &&
         !hwndMenu)
        fl |= SWP_ZORDER;
      if(fl)
        WinSetWindowPos(hwnd,
                        HWND_TOP,
                        swpH->x,
                        swpH->y,
                        cx,
                        cy,
                        fl);
    }
  }
}


static void MonitorLoopThread (void *args) {

  DosSetPriority(PRTYS_THREAD,
                 PRTYC_TIMECRITICAL,
                 PRTYD_MAXIMUM,
                 0);
  while(!amclosing &&
        !closethreads) {

    ULONG msecs,msecs2,cntr,last,this;

    DosQuerySysInfo(QSV_MS_COUNT,
                    QSV_MS_COUNT,
                    &msecs,
                    sizeof(msecs));
    last = IdleCounter;
    DosSleep(MilliSecs);
    this = IdleCounter;
    DosQuerySysInfo(QSV_MS_COUNT,
                    QSV_MS_COUNT,
                    &msecs2,
                    sizeof(msecs2));

    if(msecs != msecs2 &&
       last < this) {
      cntr = (ULONG)(((double)(this - last) * 1000) /
                     ((double)msecs2 - msecs));
      MaxCount = max(MaxCount,cntr);
      if(CPUHwnd &&
         !closethreads)
        PostMsg(CPUHwnd,
                UM_REFRESH,
                MPFROMLONG(cntr),
                MPVOID);
    }
  }
  MonTID = 0;
}


static BOOL StartCPUThreads (void) {

  closethreads = 0;
  IdleTID = _beginthread(MonitorIdleLoopThread,
                         NULL,
                         20480,
                         (PVOID)0);
  MonTID = _beginthread(MonitorLoopThread,
                        NULL,
                        20480,
                        (PVOID)0);
  if(IdleTID == (TID)-1 ||
     MonTID == (TID)-1) {
    closethreads = 1;
    WarbleBeep();
    return FALSE;
  }
  return TRUE;
}


static void FindSwapperDat (char *SwapperDat) {

  char        *filename = "C:\\CONFIG.SYS",*p,*pp;
  char        *input = NULL;
  FILE        *fp;
  FILEFINDBUF3 ffb;
  ULONG        nm = 1L,size = sizeof(SwapperDat);
  HDIR         hdir = HDIR_CREATE;
  APIRET       rc = 1L;

  input = malloc(8192);
  if(input) {
    *input = 0;
    *SwapperDat = 0;
    PrfQueryProfileData(prof,
                        appname,
                        "SwapperDat",
                        (PVOID)SwapperDat,
                        &size);
    if(*SwapperDat) {
      rc = DosFindFirst(SwapperDat,
                        &hdir,
                        FILE_NORMAL | FILE_ARCHIVED |
                        FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY,
                        &ffb,
                        sizeof(ffb),
                        &nm,
                        FIL_STANDARD);
      if(!rc) {
        DosFindClose(hdir);
        fp = fopen(SwapperDat,"r");
        if(fp) {
          fclose(fp);
          *SwapperDat = 0;
          rc = 1L;
        }
      }
      else
        *SwapperDat = 0;
    }
    if(rc) {
      if(DosQuerySysInfo(QSV_BOOT_DRIVE,
                         QSV_BOOT_DRIVE,
                         (PVOID)&nm,
                         (ULONG)sizeof(ULONG)))
        nm = 3L;
      *filename = (char)nm + '@';
      fp = fopen(filename,"r");
      if(fp) {
        while(!feof(fp)) {
          if(!fgets(input,8192,fp))
            break;
          input[8191] = 0;
          lstrip(input);
          if(!strnicmp(input,"SWAPPATH",8)) {
            p = input + 8;
            while(*p == ' ')
              p++;
            if(*p == '=') {
              p++;
              stripcr(p);
              rstrip(p);
              while(*p == ' ')
                p++;
              if(*p == '\"') {
                p++;
                pp = p;
                while(*pp && *pp != '\"')
                  *pp++;
                if(*pp)
                  *pp = 0;
              }
              else {
                pp = strchr(p,' ');
                if(pp)
                  *pp = 0;
              }
              if(*p) {
                strncpy(SwapperDat,p,CCHMAXPATH);
                SwapperDat[CCHMAXPATH - 1] = 0;
                if(SwapperDat[strlen(SwapperDat) - 1] != '\\')
                  strcat(SwapperDat,"\\");
                strcat(SwapperDat,"SWAPPER.DAT");
                hdir = HDIR_CREATE;
                nm = 1L;
                if(!DosFindFirst(SwapperDat,
                                 &hdir,
                                 FILE_NORMAL | FILE_ARCHIVED |
                                 FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY,
                                 &ffb,
                                 sizeof(ffb),
                                 &nm,
                                 FIL_STANDARD)) {
                  DosFindClose(hdir);
                  PrfWriteProfileString(prof,
                                        appname,
                                        "SwapperDat",
                                        SwapperDat);
                }
                else
                  *SwapperDat = 0;
                break;
              }
            }
          }
        }
        fclose(fp);
      }
    }
    if(!*SwapperDat) {
      if(DosQuerySysInfo(QSV_BOOT_DRIVE,
                         QSV_BOOT_DRIVE,
                         (PVOID)&nm,
                         (ULONG)sizeof(ULONG)))
        nm = 3L;
      sprintf(SwapperDat,
              "%c:\\OS2\\SYSTEM\\SWAPPER.DAT",
              (char)nm + '@');
      hdir = HDIR_CREATE;
      nm = 1L;
      if(!DosFindFirst(SwapperDat,
                       &hdir,
                       FILE_NORMAL | FILE_ARCHIVED |
                       FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY,
                       &ffb,
                       sizeof(ffb),
                       &nm,
                       FIL_STANDARD)) {
        DosFindClose(hdir);
        PrfWriteProfileString(prof,
                              appname,
                              "SwapperDat",
                              SwapperDat);
      }
      else
        *SwapperDat = 0;
    }
    free(input);
  }
}


static char MakeNumber (ULONG number,ULONG *high,ULONG *low) {

  char   ret = 'k';
  double nm;

  if(high &&
     low) {
    nm = number;
    if(nm > (1024.0 * 1024.0 * 1024.0)) {
      nm /= (1024.0 * 1024.0 * 1024.0);
      ret = 'g';
    }
    else if(nm > (1024.0 * 1024.0)) {
      nm /= (1024.0 * 1024.0);
      ret = 'm';
    }
    else
      nm /= 1024.0;
    *high = (ULONG)nm;
    *low = (ULONG)((fFractions) ? ((nm - (double)*high) * 100.0) : 0);
  }
  return ret;
}


static APIRET SetDriveText (ULONG ulD,char *s) {

  FSALLOCATE   fsa;
  ULONG        sl,sh;
  APIRET       rc;
  char         ss[8],mb;

  rc = DosQueryFSInfo(ulD,
                      FSIL_ALLOC,
                      &fsa,
                      sizeof(FSALLOCATE));
  if(!rc) {
    mb = MakeNumber(fsa.cUnitAvail * (fsa.cSectorUnit * fsa.cbSector),
                    &sh,
                    &sl);
    *ss = 0;
    if(sl)
      sprintf(ss,
              ".%02lu",
              sl);
    sprintf(s,
            " %c:  %lu%s%cb free (%lu%%) ",
            (char)(ulD + '@'),
            sh,
            ss,
            mb,
            ((fsa.cUnit &&
              fsa.cUnitAvail) ?
              (fsa.cUnitAvail * 100) / fsa.cUnit :
              0));
  }
  else
    sprintf(s,
            " %c:  SYS%04lu",
            (char)(ulD + '@'),
            rc);
  return rc;
}


MRESULT EXPENTRY SwapProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  static HWND hwndMenu = (HWND)0;

  switch(msg) {
    case WM_CREATE:
      {
        MRESULT mr = PFNWPStatic(hwnd,msg,mp1,mp2);

        WinSendMsg(hwnd,
                   UM_SETUP,
                   MPVOID,
                   MPVOID);
        return mr;
      }

    case WM_MOUSEMOVE:

      break;

    case WM_PRESPARAMCHANGED:
      {
        char *rootname = "SwapMon";

        switch(WinQueryWindowUShort(hwnd,QWS_ID)) {
          case CLOCK_FRAME:
            rootname = "Clock";
            break;
          case HARD_FRAME:
            rootname = "Hard";
            break;
          case CPU_FRAME:
            rootname = "CPU";
            break;
          case CLP_FRAME:
            rootname = "ClipMon";
            break;
          case MEM_FRAME:
            rootname = "Mem";
            break;
          case TSK_FRAME:
            rootname = "Task";
            break;
        }
        PresParamChanged(hwnd,
                         rootname,
                         mp1,
                         mp2);
        PostMsg(hwnd,
                UM_TIMER,
                MPVOID,
                MPVOID);
        PostMsg(hwnd,
                UM_REFRESH,
                MPVOID,
                MPFROMSHORT(TRUE));
      }
      break;

    case WM_APPTERMINATENOTIFY:
      if(WinQueryWindowUShort(hwnd,QWS_ID) == CPU_FRAME) {
        if(!StartCPUThreads())
          WinDestroyWindow(hwnd);
      }
      break;

    case WM_BUTTON1MOTIONSTART:
      {
        POINTL ptl;

        ptl.x = SHORT1FROMMP(mp1);
        ptl.y = SHORT2FROMMP(mp1);
        WinMapWindowPoints(hwnd,
                           HWND_DESKTOP,
                           &ptl,
                           1L);
        PostMsg(hwndConfig,
                UM_SHOWME,
                MPFROM2SHORT((USHORT)ptl.x,(USHORT)ptl.y),
                mp2);
      }
      return MRFROMSHORT(TRUE);

    case WM_BUTTON2MOTIONSTART:
      {
        TRACKINFO TrackInfo;
        SWP       Position;

        memset(&TrackInfo,0,sizeof(TrackInfo));
        TrackInfo.cxBorder   = 1 ;
        TrackInfo.cyBorder   = 1 ;
        TrackInfo.cxGrid     = 1 ;
        TrackInfo.cyGrid     = 1 ;
        TrackInfo.cxKeyboard = 8 ;
        TrackInfo.cyKeyboard = 8 ;
        WinQueryWindowPos(hwnd,&Position);
        TrackInfo.rclTrack.xLeft   = Position.x ;
        TrackInfo.rclTrack.xRight  = Position.x + Position.cx ;
        TrackInfo.rclTrack.yBottom = Position.y ;
        TrackInfo.rclTrack.yTop    = Position.y + Position.cy ;
        WinQueryWindowPos(HWND_DESKTOP,&Position);
        TrackInfo.rclBoundary.xLeft   = Position.x ;
        TrackInfo.rclBoundary.xRight  = Position.x + Position.cx ;
        TrackInfo.rclBoundary.yBottom = Position.y ;
        TrackInfo.rclBoundary.yTop    = Position.y + Position.cy ;
        TrackInfo.ptlMinTrackSize.x = 0 ;
        TrackInfo.ptlMinTrackSize.y = 0 ;
        TrackInfo.ptlMaxTrackSize.x = Position.cx ;
        TrackInfo.ptlMaxTrackSize.y = Position.cy ;
        TrackInfo.fs = TF_MOVE | TF_STANDARD | TF_ALLINBOUNDARY ;
        if(WinTrackRect(HWND_DESKTOP,
                        (HPS)0,
                        &TrackInfo)) {
          WinSetWindowPos(hwnd,
                          HWND_TOP,
                          TrackInfo.rclTrack.xLeft,
                          TrackInfo.rclTrack.yBottom,
                          0,
                          0,
                          SWP_MOVE);
          switch(WinQueryWindowUShort(hwnd,QWS_ID)) {
            case SWAP_FRAME:
              WinQueryWindowPos(hwnd,&swpSwap);
              SavePrf("SwapSwp",
                      &swpSwap,
                      sizeof(SWP));
              break;
            case CLOCK_FRAME:
              WinQueryWindowPos(hwnd,&swpClock);
              SavePrf("ClockSwp",
                      &swpClock,
                      sizeof(SWP));
              break;
            case HARD_FRAME:
              WinQueryWindowPos(hwnd,&swpHard);
              SavePrf("HardSwp",
                      &swpHard,
                      sizeof(SWP));
              break;
            case CPU_FRAME:
              WinQueryWindowPos(hwnd,&swpCPU);
              SavePrf("CPUSwp",
                      &swpCPU,
                      sizeof(SWP));
              break;
            case CLP_FRAME:
              WinQueryWindowPos(hwnd,&swpClip);
              SavePrf("ClipSwp",
                      &swpClip,
                      sizeof(SWP));
              break;
            case MEM_FRAME:
              WinQueryWindowPos(hwnd,&swpMem);
              SavePrf("MemSwp",
                      &swpMem,
                      sizeof(SWP));
              break;
            case TSK_FRAME:
              WinQueryWindowPos(hwnd,&swpTask);
              SavePrf("TaskSwp",
                      &swpTask,
                      sizeof(SWP));
              break;
          }
        }
      }
      return MRFROMSHORT(TRUE);

    case WM_BUTTON1DOWN:
      WinSetWindowPos(hwnd,
                      HWND_TOP,
                      0,
                      0,
                      0,
                      0,
                      SWP_ZORDER | SWP_DEACTIVATE);
      return MRFROMSHORT(TRUE);

    case WM_BUTTON1DBLCLK:
    case WM_BUTTON1CLICK:
      if((!fNoMonClick &&
          msg == WM_BUTTON1CLICK) ||
         (fNoMonClick &&
          msg == WM_BUTTON1DBLCLK)) {
        switch(WinQueryWindowUShort(hwnd,QWS_ID)) {
          case HARD_FRAME:
            {
              ULONG ulDriveNum,ulDriveMap,x;

              ulDriveMon = min(ulDriveMon,26);
              if(!DosQCurDisk(&ulDriveNum,
                              &ulDriveMap)) {
                for(x = ulDriveMon + 1;x < 26;x++) {
                  if(ulDriveMap & (1 << x)) {
                    ulDriveMon = x;
                    break;
                  }
                }
                if(x >= 26) {
                  for(x = 3;x < ulDriveMon - 1;x++) {
                    if(ulDriveMap & (1 << x)) {
                      ulDriveMon = x;
                      break;
                    }
                  }
                }
                SavePrf("MonDrive",
                        &ulDriveMon,
                        sizeof(ULONG));
              }
              PostMsg(hwnd,
                      UM_TIMER,
                      MPVOID,
                      MPVOID);
            }
            break;

          case CLP_FRAME:
          case MEM_FRAME:
          case TSK_FRAME:
          case SWAP_FRAME:
          case CLOCK_FRAME:
          case CPU_FRAME:
            {
              USHORT cmd = CPU_PULSE;

              switch(WinQueryWindowUShort(hwnd,QWS_ID)) {
                case MEM_FRAME:
                  cmd = CLOCK_CMDLINE;
                  break;
                case TSK_FRAME:
                  cmd = CPU_KILLPROC;
                  break;
                case CLP_FRAME:
                  cmd = CLOCK_CLIPBOARD;
                  break;
                case SWAP_FRAME:
                  cmd = SWAP_LAUNCHPAD;
                  break;
                case CLOCK_FRAME:
                  cmd = CLOCK_SETTINGS;
                  if((WinGetKeyState(HWND_DESKTOP,VK_SHIFT) & 0x8000) != 0)
                    cmd = CLOCK_CLOCK;
                  break;
              }
              PostMsg(hwnd,
                      WM_COMMAND,
                      MPFROM2SHORT(cmd,0),
                      MPVOID);
            }
            break;
        }
        return MRFROMSHORT(TRUE);
      }
      else
        PostMsg(hwnd,
                UM_TIMER,
                MPVOID,
                MPVOID);
      break;

    case WM_CONTEXTMENU:
      WinInvalidateRect(hwnd,
                        NULL,
                        FALSE);
      WinSendMsg(hwnd,
                 UM_TIMER,
                 MPVOID,
                 MPVOID);
      WinSetWindowPos(hwnd,
                      HWND_TOP,
                      0,
                      0,
                      0,
                      0,
                      SWP_ZORDER | SWP_DEACTIVATE);
      {
        USHORT id;

        id = WinQueryWindowUShort(hwnd,QWS_ID);
        hwndMenu = WinLoadMenu(HWND_DESKTOP,
                               0,
                               id);
        if(hwndMenu) {

          POINTL   ptl;
          SWP      swp;
          ULONG    ulDriveNum,ulDriveMap,x;
          MENUITEM mi;
          char     s[80];

          SetPresParams(hwndMenu,
                        -1,
                        -1,
                        -1,
                        helvtext);
          switch(id) {
            case CLP_FRAME:
              WinSendMsg(hwndMenu,
                         MM_SETITEMATTR,
                         MPFROM2SHORT(CLP_APPEND,FALSE),
                         MPFROM2SHORT(MIA_CHECKED,
                                      ((fClipAppend) ? MIA_CHECKED : 0)));
              break;

            case HARD_FRAME:
              if(!DosQCurDisk(&ulDriveNum,
                              &ulDriveMap)) {
                mi.iPosition = 0;
                mi.hwndSubMenu = (HWND)0;
                mi.hItem = 0L;
                mi.afStyle = MIS_TEXT | MIS_STATIC;
                mi.afAttribute = MIA_FRAMED;
                mi.id = -1;
                sprintf(s,
                        "Current drive is %c:",
                        (char)(ulDriveMon + '@'));
                WinSendMsg(hwndMenu,
                           MM_INSERTITEM,
                           MPFROMP(&mi),
                           MPFROMP(s));
                mi.iPosition = MIT_END;
                for(x = 2;x < 26;x++) {
                  if(ulDriveMap & (1 << x)) {
                    if(x != ulDriveMon - 1) {
                      mi.afStyle = MIS_TEXT;
                      mi.afAttribute = 0;
                      mi.id = HARD_C + (x - 2);
                      if(fShowFreeInMenus)
                        SetDriveText(x + 1,
                                     s);
                      else
                        sprintf(s,
                                "%c:",
                                (char)(x + 'A'));
                      WinSendMsg(hwndMenu,
                                 MM_INSERTITEM,
                                 MPFROMP(&mi),
                                 MPFROMP(s));
                    }
                  }
                }
              }
              break;

            case CLOCK_FRAME:
              WinEnableMenuItem(hwndMenu,
                                CLOCK_VIRTUAL,
                                fDesktops);
              WinEnableMenuItem(hwndMenu,
                                CLOCK_CLIPBOARD,
                                (ClipHwnd != (HWND)0));
              break;

            default:
              break;
          }
          WinQueryWindowPos(hwnd,
                            &swp);
          ptl.x = SHORT1FROMMP(mp1);
          ptl.y = SHORT2FROMMP(mp1);
          WinMapWindowPoints(hwnd,
                             HWND_DESKTOP,
                             &ptl,
                             1L);
          ptl.y = max(ptl.y,swp.y + swp.cy + 4);
          if(!WinPopupMenu(HWND_DESKTOP,
                           hwnd,
                           hwndMenu,
                           ptl.x - 4,
                           ptl.y - 4,
                           0,
                           PU_HCONSTRAIN | PU_VCONSTRAIN |
                           PU_KEYBOARD   | PU_MOUSEBUTTON1)) {
            WinDestroyWindow(hwndMenu);
            hwndMenu = (HWND)0;
          }
        }
      }
      return MRFROMSHORT(TRUE);

    case WM_MENUEND:
      WinSetFocus(HWND_DESKTOP,
                  HWND_DESKTOP);
      WinDestroyWindow((HWND)mp2);
      if(hwndMenu == (HWND)mp2)
        hwndMenu = (HWND)0;
      return 0;

    case UM_SETUP:
      {
        char *rootname = "SwapMon";

        switch(WinQueryWindowUShort(hwnd,QWS_ID)) {
          case SWAP_FRAME:
            SwapHwnd = hwnd;
            swaptick = 0;
            break;
          case CLOCK_FRAME:
            ClockHwnd = hwnd;
            rootname = "Clock";
            break;
          case TSK_FRAME:
            TaskHwnd = hwnd;
            rootname = "Task";
            break;
          case MEM_FRAME:
            MemHwnd = hwnd;
            rootname = "Mem";
            break;
          case HARD_FRAME:
            HardHwnd = hwnd;
            rootname = "Hard";
            break;
          case CPU_FRAME:
            CPUHwnd = hwnd;
            rootname = "CPU";
            PostMsg(hwnd,
                    UM_REFRESH,
                    MPVOID,
                    MPFROMSHORT(TRUE));
            break;
          case CLP_FRAME:
            ClipMonHwnd = hwnd;
            rootname = "ClipMon";
            PostMsg(hwnd,
                    UM_REFRESH,
                    MPVOID,
                    MPFROMSHORT(TRUE));
            break;
        }
        if(!RestorePresParams(hwnd,rootname))
          SetPresParams(hwnd,
                        RGB_WHITE,
                        RGB_BLACK,
                        RGB_BLACK,
                        helvtext);
      }
      PostMsg(hwnd,
              UM_TIMER,
              MPVOID,
              MPVOID);
      return 0;

    case UM_REFRESH:
      switch(WinQueryWindowUShort(hwnd,QWS_ID)) {
        case CLP_FRAME:
          {
            char  s[] = " Clip: [TtBbMmP] + ",*p;
            ULONG fmt[] = {CF_TEXT,CF_DSPTEXT,
                           CF_BITMAP,CF_DSPBITMAP,
                           CF_METAFILE,CF_DSPMETAFILE,
                           CF_PALETTE,0};
            ULONG x,ret;

            if(WinOpenClipbrd(WinQueryAnchorBlock(hwnd))) {
              p = s + 8;
              for(x = 0;fmt[x];x++) {
                ret = WinQueryClipbrdData(WinQueryAnchorBlock(hwnd),
                                          fmt[x]);
                if(!ret)
                  *p = '-';
                p++;
              }
              p += 2;
              if(!fClipAppend)
                *p = 0;
              WinCloseClipbrd(WinQueryAnchorBlock(hwnd));
            }
            else
              strcpy(s + 7,
                     "Can't open. ");
            SetMonitorSize(hwnd,
                           hwndMenu,
                           &swpClip,
                           s);
          }
          break;

        case CPU_FRAME:
          {
            char        s[32];
            ULONG       percent,lastaverage;
            static BOOL lastavgset = 0;

            *s = 0;
            if(mp2) {
              strcpy(s," CPU: -% ");
              if(fAverage)
                strcat(s,"Avg: -%) ");
            }
            else {
              percent = ((MaxCount - (ULONG)mp1) * 100) / MaxCount;
              lastaverage = AveCPU;
              AveCPU = (((AveCPU * NumAveCPU) + percent) /
                        ((ULONG)NumAveCPU + 1));
              NumAveCPU++;
              if(!NumAveCPU)
                NumAveCPU = 65535;
              if(percent != LastPercent ||
                 (AveCPU != lastaverage &&
                  fAverage) ||
                 NumAveCPU == 1 ||
                 lastavgset != fAverage) {
                lastavgset = fAverage;
                LastPercent = percent;
                sprintf(s,
                        " CPU: %lu%% ",
                        percent);
                if(fAverage)
                  sprintf(s + strlen(s),
                          "(Avg: %lu%%) ",
                          AveCPU);
              }
            }
            if(*s)
              SetMonitorSize(hwnd,
                             hwndMenu,
                             &swpCPU,
                             s);
          }
          break;
      }
      return 0;

    case WM_TIMER:
      if(fSwapFloat &&
         !hwndMenu)
        WinSetWindowPos(hwnd,
                        HWND_TOP,
                        0,
                        0,
                        0,
                        0,
                        SWP_ZORDER | SWP_SHOW);
      if(WinQueryWindowUShort(hwnd,QWS_ID) == CLP_FRAME) {
        if(!ClipHwnd)
          WinDestroyWindow(hwnd);
        else
          TakeClipboard();
      }
      return 0;

    case UM_TIMER:
      switch(WinQueryWindowUShort(hwnd,QWS_ID)) {
        case CLP_FRAME:
          ClipMonHwnd = hwnd;
          WinSendMsg(hwnd,
                     WM_TIMER,
                     MPVOID,
                     MPVOID);
          break;

        case CPU_FRAME:
          CPUHwnd = hwnd;
          WinSendMsg(hwnd,
                     WM_TIMER,
                     MPVOID,
                     MPVOID);
          break;

        case HARD_FRAME:
          {
            char s[80];

            HardHwnd = hwnd;
            SetDriveText(ulDriveMon,
                         s);
            SetMonitorSize(hwnd,
                           hwndMenu,
                           &swpHard,
                           s);
          }
          break;

        case SWAP_FRAME:
          {
            FILEFINDBUF3 ffb;
            ULONG        nm = 1,fl = SWP_SIZE | SWP_SHOW | SWP_MOVE,
                         sh,sl,nh;
            HDIR         hdir = HDIR_CREATE;
            FSALLOCATE   fsa;
            char         s[128],ss[8],sss[8],mb,smb;
            static ULONG lastcbFile = 0;
            static BOOL  warned = FALSE;
            static char  SwapperDat[CCHMAXPATH] = "";

            strcpy(s,
                   " Unable to locate SWAPPER.DAT. ");
            SwapHwnd = hwnd;
            if(!*SwapperDat)
              FindSwapperDat(SwapperDat);
            if(*SwapperDat) {
              DosError(FERR_DISABLEHARDERR);
              if(!DosFindFirst(SwapperDat,
                               &hdir,
                               FILE_NORMAL | FILE_HIDDEN |
                               FILE_SYSTEM | FILE_ARCHIVED | FILE_READONLY,
                               &ffb,
                               sizeof(ffb),
                               &nm,
                               FIL_STANDARD)) {
                DosFindClose(hdir);
                if(ffb.cbFile != lastcbFile && lastcbFile)
                  swaptick = 9;
                lastcbFile = ffb.cbFile;
                DosError(FERR_DISABLEHARDERR);
                if(!DosQueryFSInfo(toupper(*SwapperDat) - '@',
                                   FSIL_ALLOC,
                                   &fsa,
                                   sizeof(FSALLOCATE)))
                  nm = fsa.cUnitAvail * (fsa.cSectorUnit * fsa.cbSector);
                else
                  nm = 0;
                if(nm <= 32768 * 1024) {
                  swaptick = 10;
                  if(!warned) {
                    SetPresParams(hwnd,
                                  RGB_RED,
                                  -1,
                                  -1,
                                  NULL);
                    warned = TRUE;
                    DosBeep(250,15);
                  }
                }
                else if(warned) {
                  PostMsg(hwnd,
                          UM_SETUP,
                          MPVOID,
                          MPVOID);
                  warned = FALSE;
                }
                mb = MakeNumber(nm,
                                &nh,
                                &sl);
                *sss = 0;
                if(sl)
                  sprintf(sss,
                          ".%02lu",
                          sl);
                smb = MakeNumber(ffb.cbFile,
                                 &sh,
                                 &sl);
                *ss = 0;
                if(sl)
                  sprintf(ss,
                          ".%02lu",
                          sl);
                sprintf(s,
                        " Swap: %lu%s%cb ",
                        sh,
                        ss,
                        smb);
                if(fShowSwapFree)
                  sprintf(s + strlen(s),
                          "(%lu%s%cb free) ",
                          nh,
                          sss,
                          mb);
              }
            }
            SetMonitorSize(hwnd,
                           hwndMenu,
                           &swpSwap,
                           s);
          }
          break;

        case TSK_FRAME:
          {
            PROCESSINFO  *ppi;
            BUFFHEADER   *pbh = NULL;
            MODINFO      *pmi;
            long          numprocs = -1,numthreads = -1;
            char          s[80];

            if(!DosAllocMem((PVOID)&pbh,
                            USHRT_MAX + 4096,
                            PAG_COMMIT | OBJ_TILE | PAG_READ | PAG_WRITE)) {
              if(!DosQProcStatus(pbh,
                                 USHRT_MAX)) {
                numprocs = numthreads = 0;
                ppi = pbh->ppi;
                while(ppi->ulEndIndicator != PROCESS_END_INDICATOR ) {
                  pmi = pbh->pmi;
                  while(pmi &&
                        ppi->hModRef != pmi->hMod)
                    pmi = pmi->pNext;
                  if(pmi) {
                    numprocs++;
                    numthreads += ppi->usThreadCount;
                  }
                  ppi = (PPROCESSINFO)(ppi->ptiFirst + ppi->usThreadCount);
                }
              }
              DosFreeMem(pbh);
              sprintf(s,
                      " Procs: %ld  Thrds: %ld ",
                      numprocs,
                      numthreads);
              SetMonitorSize(hwnd,
                             hwndMenu,
                             &swpTask,
                             s);
            }
          }
          break;

        case MEM_FRAME:
          {
            ULONG sh,sl,nh,amem;
            char  s[128],ss[8],sss[8],mb,smb;

            strcpy(s,
                   "Can't check virtual memory.");
            MemHwnd = hwnd;
            if(!DosQuerySysInfo(QSV_TOTAVAILMEM,
                                QSV_TOTAVAILMEM,
                                &amem,
                                sizeof(amem))) {
              mb = MakeNumber(amem,
                              &nh,
                              &sl);
              *ss = 0;
              if(sl)
                sprintf(ss,
                        ".%02lu",
                        sl);
              sprintf(s,
                      " VMem: %lu%s%cb ",
                      nh,
                      ss,
                      mb);
            }
            if(fShowPMem) {
              if(!Dos16MemAvail(&amem)) {
                mb = MakeNumber(amem,
                                &nh,
                                &sl);
                *ss = 0;
                if(sl)
                  sprintf(ss,
                          ".%02lu",
                          sl);
                sprintf(s + strlen(s),
                        " PMem: %lu%s%cb ",
                        nh,
                        ss,
                        mb);
              }
            }
            SetMonitorSize(hwnd,
                           hwndMenu,
                           &swpMem,
                           s);
          }
          break;

        case CLOCK_FRAME:
          {
            char      s[80];
            DATETIME  dt;

            ClockHwnd = hwnd;
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
            else
              strcpy(s,"Unknown time");
            SetMonitorSize(hwnd,
                           hwndMenu,
                           &swpClock,
                           s);
          }
          break;
      }
      return 0;

    case WM_COMMAND:
      {
        BOOL ctrl,shift;

        ctrl = ((WinGetKeyState(HWND_DESKTOP,VK_CTRL) & 0x8000) != 0);
        shift = ((WinGetKeyState(HWND_DESKTOP,VK_SHIFT) & 0x8000) != 0);

        switch(SHORT1FROMMP(mp1)) {
          case CLP_CLEAR:
            if(WinOpenClipbrd(WinQueryAnchorBlock(hwnd))) {
              WinEmptyClipbrd(WinQueryAnchorBlock(hwnd));
              WinCloseClipbrd(WinQueryAnchorBlock(hwnd));
              PostMsg(hwnd,
                      UM_REFRESH,
                      MPVOID,
                      MPVOID);
            }
            break;

          case CLP_APPEND:
            fClipAppend = (fClipAppend) ? FALSE : TRUE;
            SavePrf("ClipAppend",
                    &fClipAppend,
                    sizeof(BOOL));
            PostMsg(hwnd,
                    UM_REFRESH,
                    MPVOID,
                    MPVOID);
            if(ClipSetHwnd)
              PostMsg(ClipSetHwnd,
                      UM_REFRESH,
                      MPVOID,
                      MPVOID);
            break;

          case MSE_HELP:
            ViewHelp(hwnd,
                     "Monitors page");
            break;

          case CPU_PULSE:
            {
              PROGDETAILS pgd;

              WinSetWindowPos(hwnd,
                              HWND_TOP,
                              0,
                              0,
                              0,
                              0,
                              SWP_ACTIVATE | SWP_FOCUSACTIVATE);
              memset(&pgd,
                     0,
                     sizeof(pgd));
              pgd.Length = sizeof(pgd);
              pgd.swpInitial.fl = SWP_ACTIVATE | SWP_ZORDER;
              pgd.swpInitial.hwndInsertBehind = HWND_TOP;
              pgd.progt.fbVisible = SHE_VISIBLE;
              pgd.progt.progc = PROG_DEFAULT;
              pgd.pszExecutable = "PULSE.EXE";
              if(WinStartApp(CPUHwnd,
                             &pgd,
                             NULL,
                             NULL,
                             0)) {
                if(CPUHwnd) {
                  closethreads = 1;
                  DosSleep(1);
                  PostMsg(CPUHwnd,
                          UM_REFRESH,
                          MPVOID,
                          MPFROMSHORT(TRUE));
                }
              }
            }
            break;

          case CPU_RESET:
            AveCPU = 0;
            NumAveCPU = 0;
            break;

          case CLOCK_VIRTUAL:
          case CLOCK_SWITCHLIST:
          case CLOCK_CMDLINE:
          case CLOCK_CLIPBOARD:
          case CLOCK_CALCULATOR:
          case HARD_FM2:
          case MSE_FRAME:
            {
              ULONG msg2 = UM_SHOWME;
              MPARAM mp12 = 0,mp22 = 0;
              HWND   hwndP = hwndConfig;

              switch(SHORT1FROMMP(mp1)) {
                case CLOCK_CMDLINE:
                  mp12 = MPFROMLONG(((WinGetKeyState(HWND_DESKTOP,VK_CTRL) & 0x8000) != 0));
                  msg2 = UM_CMDLINE;
                  break;
                case CLOCK_CLIPBOARD:
                  msg2 = UM_CLIPMGR;
                  hwndP = ObjectHwnd3;
                  break;
                case CLOCK_SWITCHLIST:
                  msg2 = UM_SWITCHLIST;
                  break;
                case CLOCK_VIRTUAL:
                  msg2 = UM_VIRTUAL;
                  break;
                case CLOCK_CALCULATOR:
                  msg2 = UM_CALC;
                  break;
                case HARD_FM2:
                  msg2 = UM_STARTWIN;
                  mp12 = MPFROM2SHORT(C_STARTFM2,0);
                  break;
              }
              PostMsg(hwndP,
                      msg2,
                      mp12,
                      mp22);
            }
            break;

          case CLOCK_SETTINGS:
          case CLOCK_CLOCK:
          case SWAP_LAUNCHPAD:
          case SWAP_WARPCENTER:
          case SWAP_CONNECTIONS:
          case SWAP_INFO:
          case SWAP_SETTINGS:
          case SWAP_SYSTEM:
          case SWAP_TEMPS:
          case SWAP_FM2:
          case HARD_OPEN:
          case CVR_COLORPALETTE:
          case CVR_HICOLORPALETTE:
          case CVR_FONTPALETTE:
          case CPU_KILLPROC:
          case CPU_HARDWARE:
            {
              char *p = "<WP_CLOCK>",*pp = defopen;
              char s[8];

              switch(SHORT1FROMMP(mp1)) {
                case HARD_OPEN:
                  sprintf(s,
                          "%c:\\",
                          (char)(ulDriveMon + '@'));
                  p = s;
                  break;
                case SWAP_FM2:
                  p = "<FM3_Folder>";
                  break;
                case SWAP_TEMPS:
                  p = "<WP_TEMPS>";
                  break;
                case SWAP_SYSTEM:
                  p = "<WP_OS2SYS>";
                  break;
                case SWAP_SETTINGS:
                  p = "<WP_CONFIG>";
                  break;
                case SWAP_INFO:
                  p = "<WP_INFO>";
                  break;
                case SWAP_CONNECTIONS:
                  p = "<WP_CONNECTIONSFOLDER>";
                  break;
                case SWAP_WARPCENTER:
                  p = "<WP_WARPCENTER>";
                  break;
                case SWAP_LAUNCHPAD:
                  p = "<WP_LAUNCHPAD>";
                  break;
                case CLOCK_SETTINGS:
                  pp = setopen;
                  break;
                case CVR_COLORPALETTE:
                  p = "<WP_LORESCLRPAL>";
                  break;
                case CVR_HICOLORPALETTE:
                  p = "<WP_HIRESCLRPAL>";
                  break;
                case CVR_FONTPALETTE:
                  p = "<WP_FNTPAL>";
                  break;
                case CPU_KILLPROC:
                  p = "<FM/2_KILLPROC>";
                  break;
                case CPU_HARDWARE:
                  p = "<WP_HWMGR>";
                  break;
              }
              OpenObject(p,
                         pp);
            }
            break;

          case HARD_VDIR:
            {
              PROGDETAILS pgd;
              char        s[36];

              sprintf(s,
                      "/C VDIR.CMD %c:\\",
                      (char)(ulDriveMon + '@'));
              memset(&pgd,
                     0,
                     sizeof(pgd));
              pgd.Length = sizeof(pgd);
              pgd.swpInitial.fl = SWP_MINIMIZE | SWP_ACTIVATE | SWP_ZORDER;
              pgd.swpInitial.hwndInsertBehind = HWND_TOP;
              pgd.progt.fbVisible = SHE_VISIBLE;
              pgd.progt.progc = PROG_DEFAULT;
              pgd.pszExecutable = "CMD.EXE";
              WinStartApp((HWND)0,
                          &pgd,
                          s,
                          NULL,
                          0);
            }
            break;

          case HARD_CHKDSK:
            {
              PROGDETAILS pgd;
              char        s[8];

              WinSetWindowPos(hwnd,
                              HWND_TOP,
                              0,
                              0,
                              0,
                              0,
                              SWP_ACTIVATE | SWP_FOCUSACTIVATE);
              sprintf(s,
                      "%c:",
                      (char)(ulDriveMon + '@'));
              memset(&pgd,
                     0,
                     sizeof(pgd));
              pgd.Length = sizeof(pgd);
              pgd.swpInitial.fl = SWP_ACTIVATE | SWP_ZORDER;
              pgd.swpInitial.hwndInsertBehind = HWND_TOP;
              pgd.progt.fbVisible = SHE_VISIBLE;
              pgd.progt.progc = PROG_DEFAULT;
              pgd.pszExecutable = "PMCHKDSK.EXE";
              WinStartApp((HWND)0,
                          &pgd,
                          s,
                          NULL,
                          0);
            }
            break;

          default:
            if(SHORT1FROMMP(mp1) >= HARD_C &&
               SHORT1FROMMP(mp1) <= HARD_C + 24) {
              ulDriveMon = (SHORT1FROMMP(mp1) - HARD_C) + 3;
              PostMsg(hwnd,
                      UM_TIMER,
                      MPVOID,
                      MPVOID);
              if(ctrl)
                PostMsg(hwnd,
                        WM_COMMAND,
                        MPFROM2SHORT(HARD_OPEN,0),
                        MPVOID);
              else if(shift)
                PostMsg(hwnd,
                        WM_COMMAND,
                        MPFROM2SHORT(HARD_VDIR,0),
                        MPVOID);
            }
            break;
        }
      }
      return 0;

    case WM_DESTROY:
      switch(WinQueryWindowUShort(hwnd,QWS_ID)) {
        case SWAP_FRAME:
          SwapHwnd = (HWND)0;
          break;
        case CLOCK_FRAME:
          ClockHwnd = (HWND)0;
          break;
        case HARD_FRAME:
          HardHwnd = (HWND)0;
          break;
        case CPU_FRAME:
          closethreads = 1;
          DosSleep(1);
          CPUHwnd = (HWND)0;
          break;
        case CLP_FRAME:
          ClipMonHwnd = (HWND)0;
          break;
        case MEM_FRAME:
          MemHwnd = (HWND)0;
          break;
        case TSK_FRAME:
          TaskHwnd = (HWND)0;
          break;
      }
      break;
  }
  return PFNWPStatic(hwnd,msg,mp1,mp2);
}


static void LoadSWP (char *name,SWP *swp) {

  ULONG size;

  size = sizeof(SWP);
  PrfQueryProfileData(prof,
                      appname,
                      name,
                      swp,
                      &size);
  if(swp->x + swp->cx > xScreen)
    swp->x = xScreen - swp->cx;
  if(swp->y + swp->cy > yScreen)
    swp->y = yScreen - swp->cy;
}


void StartSwapMon (ULONG id) {

  ULONG       x = swpClock.x,y = swpClock.y;
  static BOOL first = TRUE;

  if(first) {
    swpSwap.x = xScreen - (xScreen / 4);
    swpHard.x = xScreen / 3;
    swpCPU.x = xScreen - (xScreen / 3);
    swpClip.x = (xScreen / 2) - 24;
    swpMem.x = xScreen / 4;
    swpTask.x = (xScreen / 2) + 24;
    {
      SWP swp;

      if(WinGetMaxPosition(hwndConfig,&swp)) {
        if(swp.y < 0)
          swp.y = 0;
        swpHard.y = swpSwap.y = swpClock.y = swpCPU.y = swpClip.y =
          swpMem.y = swpTask.y = swp.y;
      }
    }
    LoadSWP("ClockSwp",
            &swpClock);
    LoadSWP("SwapSwp",
            &swpSwap);
    LoadSWP("HardSwp",
            &swpHard);
    LoadSWP("CPUSwp",
            &swpCPU);
    LoadSWP("ClipSwp",
            &swpClip);
    LoadSWP("MemSwp",
            &swpMem);
    LoadSWP("TaskSwp",
            &swpTask);
    first = FALSE;
  }

  switch(id) {
    case SWAP_FRAME:
      x = swpSwap.x;
      y = swpSwap.y;
      break;
    case HARD_FRAME:
      x = swpHard.x;
      y = swpHard.y;
      break;
    case CPU_FRAME:
      x = swpCPU.x;
      y = swpCPU.y;
      break;
    case CLP_FRAME:
      if(!ClipHwnd)
        return;
      x = swpClip.x;
      y = swpClip.y;
      break;
    case MEM_FRAME:
      x = swpMem.x;
      y = swpMem.y;
      break;
    case TSK_FRAME:
      x = swpTask.x;
      y = swpTask.y;
      break;
  }
  if(WinCreateWindow(HWND_DESKTOP,
                     monwinclass,
                     (PSZ)NULL,
                     WS_CLIPSIBLINGS |
                     SS_TEXT | DT_CENTER | DT_VCENTER,
                     x,
                     y,
                     0,
                     0,
                     HWND_DESKTOP,
                     HWND_TOP,
                     id,
                     MPVOID,
                     MPVOID)) {
    switch(id) {
      case CPU_FRAME:
        if(!StartCPUThreads())
          WinDestroyWindow(CPUHwnd);
        break;
    }
  }
  else
    WarbleBeep();
}

