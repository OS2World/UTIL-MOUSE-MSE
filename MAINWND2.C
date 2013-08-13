#define INCL_DOS
#define INCL_DOSERRORS
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
#include "notebook.h"

static HEV hevTimerSem;


static void TimerThread (void *args) {

  HAB   hab2;
  HMQ   hmq2;
  int   clocktick = 0,cputick = 0;
  ULONG ulTime;

  hab2 = WinInitialize(0);
  if(hab2) {
    hmq2 = WinCreateMsgQueue(hab2,
                             0);
    if(hmq2) {
      WinCancelShutdown(hmq2,
                        TRUE);
      if(!DosCreateEventSem(NULL,
                            &hevTimerSem,
                            0,
                            FALSE)) {
        for(;;) {
          if(DosWaitEventSem(hevTimerSem,
                             10000) !=
             ERROR_TIMEOUT)
            break;
          clocktick++;
          if(ClockHwnd) {
            if(!(clocktick % 5))
              PostMsg(ClockHwnd,
                      UM_TIMER,
                      MPVOID,
                      MPVOID);
            else if(fSwapFloat)
              PostMsg(ClockHwnd,
                      WM_TIMER,
                      MPVOID,
                      MPVOID);
          }
          if(SwapHwnd) {
            swaptick++;
            if(!(swaptick % 11))
              PostMsg(SwapHwnd,
                      UM_TIMER,
                      MPVOID,
                      MPVOID);
            else if(fSwapFloat)
              PostMsg(SwapHwnd,
                      WM_TIMER,
                      MPVOID,
                      MPVOID);
          }
          if(DeskHwnd) {
            desktick++;
            if(desktick > 1)
              PostMsg(DeskHwnd,
                      WM_TIMER,
                      MPVOID,
                      MPVOID);
          }
          if(HardHwnd) {
            hardtick++;
            if(!(hardtick % 10))
              PostMsg(HardHwnd,
                      UM_TIMER,
                      MPVOID,
                      MPVOID);
            else if(fSwapFloat)
              PostMsg(HardHwnd,
                      WM_TIMER,
                      MPVOID,
                      MPVOID);
          }
          if(CPUHwnd &&
             fSwapFloat)
            PostMsg(CPUHwnd,
                    WM_TIMER,
                    MPVOID,
                    MPVOID);
          if(ClipMonHwnd &&
             fSwapFloat)
            PostMsg(ClipMonHwnd,
                    WM_TIMER,
                    MPVOID,
                    MPVOID);
          if(ulMouseMins &&
             !ulPointerHidden &&
             !fDisabled &&
             !fSuspend) {
            if(!DosQuerySysInfo(QSV_MS_COUNT,
                                QSV_MS_COUNT,
                                &ulTime,
                                sizeof(ulTime))) {
              if(ulTime > ulLastMouse + (ulMouseMins * 1000))
                PostMsg(hwndConfig,
                        WM_TIMER,
                        MPVOID,
                        MPVOID);
            }
          }
        }
        DosCloseEventSem(hevTimerSem);
      }
      WinDestroyMsgQueue(hmq2);
    }
    WinTerminate(hab2);
  }
}


MRESULT EXPENTRY MainProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  static long lastPage = 0;

  switch(msg) {
    case WM_INITDLG:
      _beginthread(TimerThread,
                   NULL,
                   32768,
                   (PVOID)0);
      DosSleep(1);
      hwndConfig = hwnd;
      if(oldstyle) {

        ULONG style;
        SWP   swp;

        style = WinQueryWindowULong(WinWindowFromID(hwnd,NTE_NOTEBOOK),
                                    QWL_STYLE);
        style &= 0xffff0000;
        style |= (BKS_SPIRALBIND       |
                  BKS_MAJORTABTOP      |
                  BKS_BACKPAGESTR      |
                  BKS_STATUSTEXTLEFT   |
                  BKS_SQUARETABS       |
                  BKS_TABTEXTLEFT);
        WinSetWindowULong(WinWindowFromID(hwnd,NTE_NOTEBOOK),
                          QWL_STYLE,
                          style);
        PostMsg(WinWindowFromID(hwnd,NTE_NOTEBOOK),
                BKM_SETDIMENSIONS,
                MPFROM2SHORT(78,22),
                MPFROMLONG(BKA_MAJORTAB));
        PostMsg(WinWindowFromID(hwnd,NTE_NOTEBOOK),
                BKM_SETDIMENSIONS,
                MPFROM2SHORT(20,20),
                MPFROMLONG(BKA_PAGEBUTTON));
        WinQueryWindowPos(hwnd,&swp);
        swp.cx += 8;
        swp.cy += 18;
        WinSetWindowPos(hwnd,
                        HWND_TOP,
                        swp.x,
                        swp.y,
                        swp.cx,
                        swp.cy,
                        SWP_SIZE);
        WinQueryWindowPos(WinWindowFromID(hwnd,NTE_NOTEBOOK),&swp);
        swp.x += 4;
        swp.y += 2;
        WinSetWindowPos(WinWindowFromID(hwnd,NTE_NOTEBOOK),
                        HWND_TOP,
                        swp.x,
                        swp.y,
                        swp.cx,
                        swp.cy,
                        SWP_MOVE | SWP_SIZE);
        WinInvalidateRect(WinWindowFromID(hwnd,NTE_NOTEBOOK),
                          NULL,
                          TRUE);
      }
      if(minimizeme)
        PostMsg(hwnd,
                WM_SYSCOMMAND,
                MPFROM2SHORT(SC_MINIMIZE,0),
                MPVOID);
      {
        TID tid;

        WinQueryWindowProcess(hwnd,
                              &myPid,
                              &tid);
      }
      {
        HAB hab = *(HAB *)mp2;

        WinSetWindowULong(hwnd,
                          0,
                          hab);
        hptrApp = WinLoadPointer(HWND_DESKTOP,
                                 0,
                                 MSE_FRAME);
        if(hptrApp)
          WinSendMsg(hwnd,
                     WM_SETICON,
                     MPFROMLONG(hptrApp),
                     MPVOID);
        hptrArrow = WinQuerySysPointer(HWND_DESKTOP,
                                       SPTR_ARROW,
                                       FALSE);
        hptrWait = WinQuerySysPointer(HWND_DESKTOP,
                                      SPTR_WAIT,
                                      FALSE);
      }
      DosSleep(1);
      PostMsg(hwnd,
              UM_SETUP,
              MPVOID,
              MPVOID);
      PostMsg(hwnd,
              UM_TIMER,
              MPVOID,
              MPVOID);
      DosSetPriority(PRTYS_THREAD,
                     PRTYC_NOCHANGE,
                     1L,
                     0L);
      break;

    case UM_DEBUG:
      fprintf(stderr,"%04hx\n",SHORT1FROMMP(mp1));
      return 0;

    case WM_FORMATFRAME:
      PostMsg(hwnd,
              UM_FOCUSME,
              MPFROMLONG(1),
              MPVOID);
      break;

    case WM_ADJUSTWINDOWPOS:
      if(fLowMemory) {

        SWP *swp = (SWP *)mp1;
        int  x;

        if(swp->fl & SWP_MINIMIZE) {
          for(x = 0;np[x].frameid;x++) {
            if(np[x].hwnd)
              PostMsg(np[x].hwnd,
                      WM_CLOSE,
                      MPVOID,
                      MPVOID);
            np[x].hwnd = (HWND)0;
            np[x].pageID = 0;
          }
          PostMsg(WinWindowFromID(hwnd,NTE_NOTEBOOK),
                  BKM_DELETEPAGE,
                  MPVOID,
                  MPFROMSHORT(BKA_ALL));
        }
        else if(swp->fl & SWP_RESTORE) {
          for(x = 0;np[x].frameid;x++) {
            PostMsg(hwnd,
                    UM_INSERTPAGE,
                    MPFROMLONG(x),
                    MPVOID);
            DosSleep(0);
          }
          PostMsg(hwnd,
                  UM_FOCUSME,
                  MPVOID,
                  MPVOID);
        }
      }
      break;

    case WM_SETFOCUS:
      if(mp2)
        PostMsg(hwnd,
                UM_FOCUSME,
                MPVOID,
                MPFROMLONG(1));
      break;

    case UM_FOCUSME:
      if(mp1) {

        SWP swp;

        WinQueryWindowPos(hwnd,&swp);
        WinShowWindow(WinWindowFromID(hwnd,NTE_NOTEBOOK),
                      (swp.cy > yTitleBar + (ySizeBorder * 2)));
      }
      else if(mp2)
        WinSetFocus(HWND_DESKTOP,
                    WinWindowFromID(hwnd,NTE_NOTEBOOK));
      else {
        WinSetFocus(HWND_DESKTOP,
                    HWND_DESKTOP);
        WinSetFocus(HWND_DESKTOP,
                    WinWindowFromID(hwnd,NTE_NOTEBOOK));
        WinInvalidateRect(WinWindowFromID(hwnd,NTE_NOTEBOOK),
                          NULL,
                          FALSE);
        PostMsg(WinWindowFromID(hwnd,NTE_NOTEBOOK),
                BKM_TURNTOPAGE,
                MPFROMLONG(np[lastPage].pageID),
                MPVOID);
      }
      return 0;

    case UM_CHECKTTLS:
      if(mp2) {

        TBARPARMS *info,*tbpp;
        char      *progname = (char *)mp1,s[CCHMAXPATH];
        HWND       hwndTbar = (HWND)mp2;

        WinQueryWindowText(hwndTbar,
                           sizeof(s),
                           s);
        info = tbHead;
        while(info) {
          if((progname &&
              info->isprogname &&
              !stricmp(info->schemename,
                       progname)) ||
             (info->istitletext &&
              !strnicmp(info->schemename,
                        s,
                        strlen(info->schemename))))
            break;
          info = info->next;
        }
        if(info &&
           !info->exclude) {
          tbpp = malloc(sizeof(TBARPARMS));
          if(tbpp) {
            *tbpp = *info;
            if(!PostMsg(ObjectHwnd2,
                        UM_SETPRESPARMS,
                        MPFROMLONG(hwndTbar),
                        MPFROMP(tbpp)))
              free(tbpp);
            else
              return MRFROMSHORT(2);
          }
        }
        if(info &&
           info->exclude)
          return 0;
        if(!info &&
           (!*s ||
            !stricmp(s,"Untitled")))
          /* maybe title text not set yet -- defer */
          PostMsg(ObjectHwnd2,
                  UM_CHECKTTLS,
                  MPVOID,
                  mp2);
        return MRFROMSHORT(1);
      }
      return 0;

    case UM_CHECKEXCLUDE:
      if(mp1) {

        int   x;
        char *progname = (char *)mp1;

        if(fexcludes) {
          for(x = 0;fexcludes[x];x++) {
            if(!stricmp(fexcludes[x],
                        progname))
              return MRFROMSHORT(TRUE);
          }
        }
      }
      return 0;

    case WM_APPTERMINATENOTIFY:
      PostMsg(hwndCover,
              WM_COMMAND,
              MPFROM2SHORT(MSE_RELOAD,0),
              MPVOID);
      break;

    case UM_SHOWME:
      {
        SWP    swp;
        POINTL ptl;

        ptl.x = SHORT1FROMMP(mp1);
        ptl.y = SHORT2FROMMP(mp1);
        WinQueryWindowPos(hwnd,&swp);
        if(swp.fl & (SWP_MINIMIZE | SWP_HIDE))
          LoadRestorePos(&swp,hwnd);
        swp.x = ptl.x - (swp.cx / 2);
        swp.y = ptl.y - (swp.cy / 2);
        if(swp.x + swp.cx > xScreen)
          swp.x = xScreen - swp.cx;
        if(swp.y + swp.cy > yScreen)
          swp.y = yScreen - swp.cy;
        if(swp.x < 0)
          swp.x = 0;
        if(swp.y < 0)
          swp.y = 0;
        if(swp.fl & (SWP_MINIMIZE | SWP_HIDE)) {
          SaveRestorePos(&swp,hwnd);
          WinSendMsg(hwnd,
                     WM_SYSCOMMAND,
                     MPFROM2SHORT(SC_RESTORE,0),
                     MPVOID);
        }
        else
          WinSetWindowPos(hwnd,
                          HWND_TOP,
                          swp.x,
                          swp.y,
                          swp.cx,
                          swp.cy,
                          SWP_MOVE);
        WinSetFocus(HWND_DESKTOP,
                    hwnd);
      }
      return 0;

    case UM_TIMER:
    case WM_TIMER:
      if(!ulPointerHidden &&
         ulMouseMins &&
         !fDisabled &&
         !fSuspend) {

        ULONG ulTime;

        if(!DosQuerySysInfo(QSV_MS_COUNT,
                            QSV_MS_COUNT,
                            &ulTime,
                            sizeof(ulTime))) {
          if(ulTime > ulLastMouse + (ulMouseMins * 1000)) {
            WinShowPointer(HWND_DESKTOP,
                           FALSE);
            ulPointerHidden = 2;
          }
        }
      }
      return 0;

    case WM_MENUEND:
      fSuspend = FALSE;
      return 0;

    case UM_OPEN:
      WinDlgBox(HWND_DESKTOP,
                HWND_DESKTOP,
                RunProc,
                0,
                RUN_FRAME,
                0);
      break;

    case UM_CALC:
      if(!CalcHwnd)
        WinDlgBox(HWND_DESKTOP,
                  HWND_DESKTOP,
                  CalcProc,
                  0,
                  CLC_FRAME,
                  0);
      else {

        SWP    swp;
        POINTL ptl;

        NormalizeWindow(CalcHwnd,TRUE);
        WinQueryWindowPos(CalcHwnd,&swp);
        WinQueryPointerPos(HWND_DESKTOP,
                           &ptl);
        swp.x = ptl.x - (swp.cx / 2);
        swp.y = ptl.y - (swp.cy / 2);
        if(swp.y < 0)
          swp.y = 0;
        if(swp.x < 0)
          swp.x = 0;
        if(swp.x + swp.cx > xScreen)
          swp.x = xScreen - swp.cx;
        if(swp.y + swp.cy > yScreen)
          swp.y = yScreen - swp.cy;
        WinSetWindowPos(CalcHwnd,
                        HWND_TOP,
                        swp.x,
                        swp.y,
                        0,
                        0,
                        SWP_RESTORE | SWP_MOVE | SWP_SHOW |
                        SWP_ZORDER | SWP_ACTIVATE);
      }
      break;

    case UM_HANDICAP:
      if(!hwndPick) {

        MENUITEM mi;
        USHORT   x;

        hwndPick = WinLoadMenu(HWND_DESKTOP,
                               0,
                               IDM_HANDICAP);
        if(hwndPick) {
          mi.iPosition = MIT_END;
          mi.hwndSubMenu = (HWND)0;
          mi.hItem = 0L;
          mi.afStyle = MIS_TEXT | MIS_STATIC;
          mi.afAttribute = MIA_FRAMED;
          mi.id = -1;
          WinSendMsg(hwndPick,
                     MM_INSERTITEM,
                     MPFROMP(&mi),
                     MPFROMP("Choose an MSE command:"));
          mi.afAttribute = 0;
          for(x = 1;x < C_MAXCMDS;x++) {
            mi.id = x + IDM_HANDICAP;
            mi.afStyle = MIS_TEXT;
            if(!(x % 25) && x)
              mi.afStyle |= MIS_BREAK;
            WinSendMsg(hwndPick,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP(cmdnames[x]));
          }
        }
      }
      if(hwndPick) {

        POINTL ptl;

        WinQueryPointerPos(HWND_DESKTOP,
                           &ptl);
        if(WinPopupMenu(HWND_DESKTOP,
                        hwnd,
                        hwndPick,
                        ptl.x - 4,
                        ptl.y - 4,
                        0,
                        PU_HCONSTRAIN | PU_VCONSTRAIN |
                        PU_KEYBOARD   | PU_MOUSEBUTTON1))
          fSuspend = TRUE;
      }
      break;

    case UM_STARTMENU:
      {
        USHORT which = SHORT1FROMMP(mp1) - C_MENU1;

        if(!hwndMenus[which]) {

          MENUITEM    mi;
          USHORT      x = 0;
          static char s[1002];
          FILE       *fp;

          sprintf(s,
                  "MSEMENU%u.DAT",
                  which + 1);
          fp = fopen(s,"r");
          if(fp) {
            hwndMenus[which] = WinLoadMenu(HWND_DESKTOP,
                                           0,
                                           (2000 + ((1000 * (which + 1)) - 1)));
            if(hwndMenus[which]) {
              mi.iPosition = MIT_END;
              mi.hwndSubMenu = (HWND)0;
              mi.hItem = 0L;
              mi.afStyle = MIS_TEXT | MIS_STATIC;
              mi.afAttribute = MIA_FRAMED;
              mi.id = -1;
              sprintf(s,
                      "MSE Menu #%u (+Ctrl=edit menu)",
                      which + 1);
              WinSendMsg(hwndMenus[which],
                         MM_INSERTITEM,
                         MPFROMP(&mi),
                         MPFROMP(s));
              mi.afAttribute = 0;
              while(!feof(fp)) {
                if(!fgets(s,1002,fp))
                  break;
                s[1001] = 0;
                stripcr(s);
                lstrip(rstrip(s));
                if(*s && *s != ';') {
                  mi.id = 2000 + (1000 * (which + 1)) + x;
                  mi.afStyle = MIS_TEXT;
                  if(!(x % 25) && x)
                    mi.afStyle |= MIS_BREAK;
                  WinSendMsg(hwndMenus[which],
                             MM_INSERTITEM,
                             MPFROMP(&mi),
                             MPFROMP(s));
                  x++;
                }
              }
            }
            fclose(fp);
            if(!x) {
              WinDestroyWindow(hwndMenus[which]);
              hwndMenus[which] = (HWND)0;
            }
          }
        }
        if(hwndMenus[which]) {

          POINTL ptl;

          WinQueryPointerPos(HWND_DESKTOP,
                             &ptl);
          if(WinPopupMenu(HWND_DESKTOP,
                          hwnd,
                          hwndMenus[which],
                          ptl.x - 4,
                          ptl.y - 4,
                          0,
                          PU_HCONSTRAIN | PU_VCONSTRAIN |
                          PU_KEYBOARD   | PU_MOUSEBUTTON1))
            fSuspend = TRUE;
        }
        else {

          char s[80];

          sprintf(s,
                  "MSEMENU%u.DAT",
                  which + 1);
          if(WinMessageBox(HWND_DESKTOP,
                           hwnd,
                           "Couldn't find that menu file, or it contained "
                           "no menu items.  Would you like to edit/create "
                           "the menu definition file named in the title?",
                           s,
                           0,
                           MB_YESNOCANCEL | MB_MOVEABLE | MB_ICONEXCLAMATION) ==
            MBID_YES) {

            PROGDETAILS pgd;

            WinSetWindowPos(hwnd,
                            HWND_TOP,
                            0,
                            0,
                            0,
                            0,
                            SWP_ACTIVATE);
            memset(&pgd,
                   0,
                   sizeof(pgd));
            pgd.Length = sizeof(pgd);
            pgd.swpInitial.fl = SWP_ACTIVATE | SWP_ZORDER;
            pgd.swpInitial.hwndInsertBehind = HWND_TOP;
            pgd.progt.fbVisible = SHE_VISIBLE;
            pgd.progt.progc = PROG_DEFAULT;
            pgd.pszExecutable = (*editor) ? editor : eexe;
            WinStartApp(hwndConfig,
                        &pgd,
                        s,
                        NULL,
                        0);
          }
        }
      }
      return 0;

    case UM_STARTWIN:
      switch(SHORT1FROMMP(mp1)) {
        case C_STARTFM2:
          if(!OpenObject("<FM/2>",
                         defopen))
            WinMessageBox(HWND_DESKTOP,
                          hwnd,
                          "To correct this condition, ensure that FM/2 is "
                          "installed, then run its INSTALL.CMD and allow "
                          "it to build program objects for you.",
                          "FM/2 program object not found.",
                          0,
                          MB_CANCEL | MB_MOVEABLE | MB_ICONEXCLAMATION);
          break;
      }
      return 0;

    case UM_SWITCHLIST:
      if(!hwndTasklist)
        hwndTasklist = WinLoadMenu(HWND_DESKTOP,
                                   0,
                                   IDM_TASK);
      if(hwndTasklist) {

        MENUITEM  mi;
        USHORT    x,c;
        PSWBLOCK  pswb;
        ULONG     ulSize,ulCount;
        long      lx,ly;
        char      s[120],*p;

        /* delete any old entries */
        WinSendMsg(hwndTasklist,
                   MM_DELETEITEM,
                   MPFROMSHORT(-1),
                   MPVOID);
        c = (SHORT)WinSendMsg(hwndTasklist,
                              MM_QUERYITEMCOUNT,
                              MPVOID,
                              MPVOID);
        for(x = 0;x < c;x++)
          WinSendMsg(hwndTasklist,
                     MM_DELETEITEM,
                     MPFROMSHORT((SHORT)(x + IDM_TASK + 1)),
                     MPVOID);
        /* add new entries */
        mi.iPosition = MIT_END;
        mi.hwndSubMenu = (HWND)0;
        mi.hItem = 0L;
        mi.afStyle = MIS_TEXT | MIS_STATIC;
        mi.afAttribute = MIA_FRAMED;
        mi.id = -1;
        sprintf(s,
                "MSE switch list (+Ctrl=Kill, %s+Alt=Hide)",
                (fDesktops) ? "+Shift=ToDesk " : "");
        WinSendMsg(hwndTasklist,
                   MM_INSERTITEM,
                   MPFROMP(&mi),
                   MPFROMP(s));
        mi.afAttribute = 0;
        x = 0;
        if(ClipHwnd) {
          mi.id = IDM_TASK + x + 1;
          mi.afStyle = MIS_TEXT;
          sprintf(s,
                  "%s\t(%lx)",
                  clipmgr,
                  (ULONG)WinQueryWindow(ClipHwnd,QW_PARENT));
          if(fDesktops) {
            if(WhichDesk(WinQueryWindow(ClipHwnd,QW_PARENT),
                         NULL,
                         &lx,
                         &ly))
              sprintf(s + strlen(s),
                      " Desk #%lu",
                      (lx + ((ly - 1) * 3)));
          }
          WinSendMsg(hwndTasklist,
                     MM_INSERTITEM,
                     MPFROMP(&mi),
                     MPFROMP(s));
          x++;
        }
        if(CalcHwnd) {
          mi.id = IDM_TASK + x + 1;
          mi.afStyle = MIS_TEXT;
          sprintf(s,
                  "MSE Calculator\t(%lx)",
                  CalcHwnd);
          if(fDesktops) {
            if(WhichDesk(CalcHwnd,NULL,&lx,&ly))
              sprintf(s + strlen(s),
                      " Desk #%lu",
                      (lx + ((ly - 1) * 3)));
          }
          WinSendMsg(hwndTasklist,
                     MM_INSERTITEM,
                     MPFROMP(&mi),
                     MPFROMP(s));
          x++;
        }
        /* Get the switch list information */
        ulCount = WinQuerySwitchList(0,NULL,0);
        ulSize = sizeof(SWBLOCK) + sizeof(HSWITCH) + (ulCount + 4L) *
                 (long)sizeof(SWENTRY);
        /* Allocate memory for list */
        if((pswb = malloc(ulSize)) != NULL) {
          WinQuerySwitchList(0,pswb,ulSize - sizeof(SWENTRY));
          /* do the dirty deed */
          for(c = 0;c < pswb->cswentry;c++) {
            if(pswb->aswentry[c].swctl.uchVisibility == SWL_VISIBLE &&
               pswb->aswentry[c].swctl.idProcess != myPid &&
               pswb->aswentry[c].swctl.hwnd &&
               pswb->aswentry[c].swctl.hwnd != hwndWPS &&
               pswb->aswentry[c].swctl.hwnd != hwndDesktop &&
               pswb->aswentry[c].swctl.hwnd != hwndBottom &&
               WinIsWindow(WinQueryAnchorBlock(hwnd),
                           pswb->aswentry[c].swctl.hwnd)) {
              mi.id = IDM_TASK + x + 1;
              mi.afStyle = MIS_TEXT;
              if(!(x % 25) && x)
                mi.afStyle |= MIS_BREAK;
              sprintf(s,
                      "%0.64s\t(%lx)",
                      pswb->aswentry[c].swctl.szSwtitle,
                      (ULONG)pswb->aswentry[c].swctl.hwnd);
              p = s;
              while(*p) {
                if(*p == '\r' || *p == '\n')
                  *p = ' ';
                p++;
              }
              if(fDesktops) {
                if(WhichDesk(pswb->aswentry[c].swctl.hwnd,
                             NULL,
                             &lx,
                             &ly))
                  sprintf(s + strlen(s),
                          " Desk #%lu",
                          (lx + ((ly - 1) * 3)));
              }
              WinSendMsg(hwndTasklist,
                         MM_INSERTITEM,
                         MPFROMP(&mi),
                         MPFROMP(s));
              x++;
            }
          }
          free(pswb);
          _heapmin();
        }
        if(!x) {
          WinDestroyWindow(hwndTasklist);
          hwndTasklist = (HWND)0;
          WarbleBeep();
        }
      }
      if(hwndTasklist) {

        POINTL ptl;

        WinQueryPointerPos(HWND_DESKTOP,&ptl);
        if(WinPopupMenu(HWND_DESKTOP,
                        hwnd,
                        hwndTasklist,
                        ptl.x - 4,
                        ptl.y - 4,
                        0,
                        PU_HCONSTRAIN | PU_VCONSTRAIN |
                        PU_KEYBOARD   | PU_MOUSEBUTTON1))
          fSuspend = TRUE;
      }
      return 0;

    case UM_VIRTUAL:
      if(fDesktops &&
         (!VirtHwnd ||
          !WinIsWindow(WinQueryAnchorBlock(hwnd),VirtHwnd))) {

        HWND  hwndClient;
        ULONG FrameFlags = FCF_TITLEBAR    | FCF_SYSMENU     |
                           FCF_SIZEBORDER  | FCF_NOBYTEALIGN |
                           FCF_ICON        | FCF_ACCELTABLE  |
                           FCF_MAXBUTTON;

        if(WinCreateStdWindow(HWND_DESKTOP,
                              WS_VISIBLE,
                              &FrameFlags,
                              deskwinclass,
                              "MSE:  Virtual desktops",
                              0,
                              0,
                              BIG_FRAME,
                              &hwndClient))
          fSuspend = TRUE;
      }
      break;

    case UM_CMDLINE:
      {
        PROGDETAILS pgd;
        char       *env;

        if((ULONG)mp1 == 1) {
          env = getenv("DOS_SHELL");
          if(!env)
            env = "COMMAND.COM";
        }
        else if(!mp1) {
          env = getenv("COMSPEC");
          if(!env)
            env = getenv("OS2_SHELL");
          if(!env)
            env = "CMD.EXE";
        }
        else {
          OpenObject("<WP_LAUNCHPAD>",
                     defopen);
          return 0;
        }
        memset(&pgd,
               0,
               sizeof(pgd));
        WinSetWindowPos(hwnd,
                        HWND_TOP,
                        0,
                        0,
                        0,
                        0,
                        SWP_ACTIVATE);
        pgd.Length = sizeof(pgd);
        pgd.swpInitial.fl = SWP_ACTIVATE | SWP_ZORDER;
        pgd.swpInitial.hwndInsertBehind = HWND_TOP;
        pgd.progt.fbVisible = SHE_VISIBLE;
        if(mp1)
          pgd.progt.progc = PROG_WINDOWEDVDM;
        else
          pgd.progt.progc = PROG_WINDOWABLEVIO;
        pgd.pszExecutable = env;
        WinStartApp((HWND)0,
                    &pgd,
                    NULL,
                    NULL,
                    0);
      }
      return 0;

    case UM_TASKLIST:
      {
        HSWITCH hsw;
        SWCNTRL swc;

        hsw = WinQuerySwitchHandle(hwnd,0);
        if(hsw)
          WinQuerySwitchEntry(hsw,&swc);
        else
          memset(&swc,0,sizeof(swc));
        swc.hwnd = hwnd;
        swc.uchVisibility = (fNoTasklist) ? SWL_INVISIBLE : SWL_VISIBLE;
        swc.fbJump = SWL_JUMPABLE;
        strcpy(swc.szSwtitle,"MSE");
        if(!hsw)
          WinCreateSwitchEntry(WinQueryAnchorBlock(hwnd),
                                                   &swc);
        else
          WinChangeSwitchEntry(hsw,
                               &swc);
      }
      return 0;

    case UM_SETUP:
      PostMsg(hwnd,
              UM_TASKLIST,
              MPVOID,
              MPVOID);
      if(fUseClipMgr)
        StartClipMgr();
      if(!fLowMemory ||
         !minimizeme) {

        long x = 0;

        while((!ObjectHwnd2 ||
               !ObjectHwnd) &&
              x < 250) {
          DosSleep(1);
          x++;
        }
        for(x = 0;np[x].frameid;x++) {
          PostMsg(hwnd,
                  UM_INSERTPAGE,
                  MPFROMLONG(x),
                  MPVOID);
          DosSleep(0);
        }
      }
      PostMsg(hwnd,
              UM_SETUP2,
              MPVOID,
              MPVOID);
      return 0;

    case UM_SETUP2:
      if(!PostMsg(hwnd,
                  UM_UNSUSPEND,
                  MPVOID,
                  MPVOID))
        fSuspend = FALSE;
      if(!minimizeme)
        PostMsg(hwnd,
                UM_FOCUSME,
                MPVOID,
                MPVOID);
      if(showclock)
        StartSwapMon(CLOCK_FRAME);
      if(fSwapMon)
        StartSwapMon(SWAP_FRAME);
      if(showhardmon)
        StartSwapMon(HARD_FRAME);
      if(fCPUMon)
        StartSwapMon(CPU_FRAME);
      if(fClipMon)
        StartSwapMon(CLP_FRAME);
      PostMsg(ObjectHwnd3,
              UM_SETUP,
              MPVOID,
              MPVOID);
      RedrawTitlebars(TRUE,
                      HWND_DESKTOP);
      return 0;

    case UM_INSERTPAGE:
      {
        HWND hwndPage;
        long x = (long)mp1;

        hwndPage = WinLoadDlg(HWND_DESKTOP,
                              HWND_DESKTOP,
                              np[x].proc,
                              0,
                              np[x].frameid,
                              NULL);
        if(hwndPage) {
          WinSetWindowULong(hwndPage,
                            0,
                            (ULONG)hwnd);
          np[x].hwnd = hwndPage;
          np[x].pageID = (ULONG)WinSendDlgItemMsg(hwnd,
                                                  NTE_NOTEBOOK,
                                                  BKM_INSERTPAGE,
                                                  MPFROMLONG(BKA_FIRST),
                                                  MPFROM2SHORT(BKA_STATUSTEXTON |
                                                               BKA_AUTOPAGESIZE |
                                                               BKA_MAJOR,
                                                               BKA_LAST));
          WinSendDlgItemMsg(hwnd,
                            NTE_NOTEBOOK,
                            BKM_SETPAGEWINDOWHWND,
                            MPFROMLONG(np[x].pageID),
                            MPFROMLONG(np[x].hwnd));
          WinSendDlgItemMsg(hwnd,
                            NTE_NOTEBOOK,
                            BKM_SETSTATUSLINETEXT,
                            MPFROMLONG(np[x].pageID),
                            MPFROMP(np[x].title));
          WinSendDlgItemMsg(hwnd,
                            NTE_NOTEBOOK,
                            BKM_SETTABTEXT,
                            MPFROMLONG(np[x].pageID),
                            MPFROMP(np[x].tab));
        }
        DosSleep(0);
      }
      return 0;

    case UM_UNSUSPEND:
      fSuspend = FALSE;
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case NTE_NOTEBOOK:
          switch(SHORT2FROMMP(mp1)) {
            case BKN_PAGESELECTED:
              {
                PAGESELECTNOTIFY *psn = (PAGESELECTNOTIFY *)mp2;
                long              x;

                for(x = 0;np[x].frameid;x++) {
                  if(psn->ulPageIdNew == np[x].pageID) {
                    lastPage = x;
                    break;
                  }
                }
              }
              break;
          }
          break;
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case MSE_EXIT:
          if(fConfirmExit) {
            if(WinMessageBox(HWND_DESKTOP,
                             hwnd,
                             "Are you sure you want to exit MSE?",
                             "Confirm:",
                             0,
                             MB_YESNOCANCEL | MB_ICONQUESTION |
                              MB_MOVEABLE) != MBID_YES)
              break;
          }
          /* intentional fallthru */
        case MSE_EXIT2:
          WinDismissDlg(hwnd,0);
          break;

        case MSA_EDITFEXCLUDE:
        case MSA_EDITEXCLUDE:
          {
            char        fullname[CCHMAXPATH];
            PROGDETAILS pgd;

            WinSetWindowPos(hwnd,
                            HWND_TOP,
                            0,
                            0,
                            0,
                            0,
                            SWP_ACTIVATE);
            sprintf(fullname,
                    "%s%sEXCLUDE.LST",mydir,
                    (SHORT1FROMMP(mp1) == MSA_EDITEXCLUDE) ? "" : "F");
            memset(&pgd,
                   0,
                   sizeof(pgd));
            pgd.Length = sizeof(pgd);
            pgd.swpInitial.fl = SWP_ACTIVATE | SWP_ZORDER;
            pgd.swpInitial.hwndInsertBehind = HWND_TOP;
            pgd.progt.fbVisible = SHE_VISIBLE;
            pgd.progt.progc = PROG_DEFAULT;
            pgd.pszExecutable = (*editor) ? editor : eexe;
            WinStartApp(hwndConfig,
                        &pgd,
                        fullname,
                        NULL,
                        0);
          }
          break;

        default:
          /* a switch list entry was selected? */
          if(SHORT1FROMMP(mp1) > IDM_TASK &&
             SHORT1FROMMP(mp1) < IDM_HANDICAP) {

            USHORT len;
            char  *text,*p;
            HWND   hwndT;
            BOOL   closeit = FALSE,switchit = FALSE,minimizeit = FALSE;

            if(fDesktops &&
               (WinGetKeyState(HWND_DESKTOP,VK_SHIFT) & 0x8000) != 0)
              switchit = TRUE;
            else if((WinGetKeyState(HWND_DESKTOP,VK_ALT) & 0x8000) != 0)
              minimizeit = TRUE;
            else if((WinGetKeyState(HWND_DESKTOP,VK_CTRL) & 0x8000) != 0)
              closeit = TRUE;
            len = (USHORT)WinSendMsg(hwndTasklist,
                                     MM_QUERYITEMTEXTLENGTH,
                                     MPFROMSHORT(SHORT1FROMMP(mp1)),
                                     MPVOID);
            if(len) {
              text = malloc(len + 2);
              if(text) {
                *text = 0;
                if(WinSendMsg(hwndTasklist,
                              MM_QUERYITEMTEXT,
                              MPFROM2SHORT(SHORT1FROMMP(mp1),len + 1),
                              MPFROMP(text))) {
                  p = strrchr(text,'(');
                  if(p) {
                    p++;
                    hwndT = (HWND)strtoul(p,&p,16);
                    if(WinIsWindow(WinQueryAnchorBlock(hwnd),hwndT)) {
                      if(closeit) {
                        if(ClipHwnd &&
                           hwndT == WinQueryWindow(ClipHwnd,QW_PARENT))
                          PostMsg(ClipHwnd,
                                  WM_CLOSE,
                                  MPVOID,
                                  MPVOID);
                        else {
                          PostMsg(hwndT,
                                  WM_SAVEAPPLICATION,
                                  MPVOID,
                                  MPVOID);
                          DosSleep(100);
                          PostMsg(hwndT,
                                  WM_SYSCOMMAND,
                                  MPFROM2SHORT(SC_CLOSE,0),
                                  MPVOID);
                          if(ObjectHwnd2)
                            PostMsg(ObjectHwnd2,
                                    UM_TASKLIST,
                                    MPFROMLONG(hwndT),
                                    MPVOID);
                        }
                      }
                      else {
                        if(switchit)
                          NormalizeWindow(hwndT,TRUE);
                        if(minimizeit)
                          PostMsg(hwndT,
                                  WM_SYSCOMMAND,
                                  MPFROM2SHORT(SC_MINIMIZE,0),
                                  MPVOID);
                        else
                          PostMsg(ObjectHwnd,
                                  UM_SHOWME,
                                  MPFROMLONG(hwndT),
                                  MPVOID);
                      }
                    }
                  }
                }
                free(text);
                _heapmin();
              }
            }
            break;
          }
          else if(SHORT1FROMMP(mp1) > IDM_HANDICAP) {

            POINTL ptl;

            fNextIsCommand = TRUE;
            nextcommand = SHORT1FROMMP(mp1) - IDM_HANDICAP;
            ptl.x = (long)SHORT1FROMMP(pickqMsg.mp1);
            ptl.y = (long)SHORT2FROMMP(pickqMsg.mp1);
            WinMapWindowPoints(pickqMsg.
                               hwnd,
                               HWND_DESKTOP,
                               &ptl,
                               1L);
            WinSetPointerPos(HWND_DESKTOP,
                             ptl.x,
                             ptl.y);
            PostMsg(pickqMsg.hwnd,
                    pickqMsg.msg,
                    pickqMsg.mp1,
                    pickqMsg.mp2);
            break;
          }
          /* one of our custom menu commands was selected */
          else {

            USHORT menu,len;
            char  *text;

            menu = ((SHORT1FROMMP(mp1) - 2000) / 1000) - 1;
            if((WinGetKeyState(HWND_DESKTOP,VK_CTRL) & 0x8000) != 0) {

              /* CTRL key was down, so just edit menu */

              PROGDETAILS pgd;
              char        s[80];

              WinSetWindowPos(hwnd,
                              HWND_TOP,
                              0,
                              0,
                              0,
                              0,
                              SWP_ACTIVATE);
              sprintf(s,
                      "MSEMENU%u.DAT",
                      menu + 1);
              memset(&pgd,
                     0,
                     sizeof(pgd));
              pgd.Length = sizeof(pgd);
              pgd.swpInitial.fl = SWP_ACTIVATE | SWP_ZORDER;
              pgd.swpInitial.hwndInsertBehind = HWND_TOP;
              pgd.progt.fbVisible = SHE_VISIBLE;
              pgd.progt.progc = PROG_DEFAULT;
              pgd.pszExecutable = (*editor) ? editor : eexe;
              WinStartApp(hwndConfig,
                          &pgd,
                          s,
                          NULL,
                          0);
              break;
            }
            /* else it's a command, run it */
            len = (USHORT)WinSendMsg(hwndMenus[menu],
                                     MM_QUERYITEMTEXTLENGTH,
                                     MPFROMSHORT(SHORT1FROMMP(mp1)),
                                     MPVOID);
            if(len) {
              text = malloc(len + 2);
              if(text) {
                *text = 0;
                if(WinSendMsg(hwndMenus[menu],
                              MM_QUERYITEMTEXT,
                              MPFROM2SHORT(SHORT1FROMMP(mp1),len + 1),
                              MPFROMP(text))) {
                  if(*text != '<' &&
                     *text != '&') {  /* is a program to run */

                    PROGDETAILS pgd;
                    ULONG       apptype;
                    char       *p,c,*sd = NULL,*d = NULL;
                    BOOL        quote = FALSE;

                    if(*text == '\"') {
                      quote = TRUE;
                      memmove(text,
                              text + 1,
                              strlen(text));
                    }
                    p = text;
                    while(*p &&
                          ((quote) ?
                           (*p != '\"') :
                           (*p != ' ')))
                      p++;
                    if(*p) {
                      *p = 0;
                      p++;
                      if(*p)
                        d = p;
                    }
                    p = strrchr(text,'\\');
                    if(p) {
                      if(*(p - 1) == ':')
                        p++;
                      c = *p;
                      *p = 0;
                      sd = strdup(text);
                      if(!sd) {
                        free(text);
                        WarbleBeep();
                        break;
                      }
                      *p = c;
                    }
                    if(IsFile(text) == 0 ||
                       DosQueryAppType(text,
                                       &apptype) != 0 ||
                       (apptype & (FAPPTYP_DLL     |
                                   FAPPTYP_PHYSDRV |
                                   FAPPTYP_PROTDLL |
                                   FAPPTYP_VIRTDRV)) != 0)
                      goto OpenObject;
                    else {
                      WinSetWindowPos(hwnd,
                                      HWND_TOP,
                                      0,
                                      0,
                                      0,
                                      0,
                                      SWP_ACTIVATE);
                      memset(&pgd,0,sizeof(pgd));
                      pgd.Length = sizeof(pgd);
                      pgd.swpInitial.fl = SWP_ACTIVATE | SWP_ZORDER;
                      pgd.swpInitial.hwndInsertBehind = HWND_TOP;
                      pgd.progt.fbVisible = SHE_VISIBLE;
                      pgd.progt.progc = PROG_DEFAULT;
                      pgd.pszStartupDir = sd;
                      pgd.pszExecutable = text;
                      if(!WinStartApp((HWND)0,
                                      &pgd,
                                      d,
                                      NULL,
                                      0))
                        WarbleBeep();
                      free(sd);
                    }
                  }
                  else if(*text == '<') { /* is a WPS object */
OpenObject:
                    if(!OpenObject(text,
                                   defopen))
                      WarbleBeep();
                  }
                  else {  /* is a menu to invoke */

                    char  *p = text;

                    while(*p) {
                      if(*p > '0' &&
                         *p < '8')
                        break;
                      p++;
                    }
                    if(*p)
                      PostMsg(hwnd,
                              UM_STARTMENU,
                              MPFROM2SHORT(C_MENU1 + (*p - '1'),0),
                              MPVOID);
                    else
                      WarbleBeep();
                  }
                }
                free(text);
                _heapmin();
              }
            }
          }
          break;
      }
      return 0;

    case WM_CLOSE:
      PostMsg(hwnd,
              WM_COMMAND,
              MPFROM2SHORT(MSE_EXIT,0),
              MPVOID);
      return 0;

    case WM_DESTROY:
      amclosing = TRUE;
      PrfWriteProfileData(prof,
                          appname,
                          "SwapperDat",
                          NULL,
                          0);
      fEnhanceTitlebars = fEnhanceFileDlg = FALSE;
      RedrawTitlebars(TRUE,
                      HWND_DESKTOP);
      fDisabled = fSuspend = TRUE;
      hwndConfig = (HWND)0;
      DosPostEventSem(hevTimerSem);
      DosSleep(100);
      if(ClipHwnd) {
        WinSendMsg(ClipHwnd,
                   WM_SAVEAPPLICATION,
                   MPVOID,
                   MPVOID);
        WinDestroyWindow(WinQueryWindow(ClipHwnd,QW_PARENT));
      }
      if(CalcHwnd)
        WinDestroyWindow(CalcHwnd);
      if(SwapHwnd)
        WinDestroyWindow(SwapHwnd);
      if(ClockHwnd)
        WinDestroyWindow(ClockHwnd);
      if(DeskHwnd)
        WinDestroyWindow(DeskHwnd);
      if(HardHwnd)
        WinDestroyWindow(HardHwnd);
      if(CPUHwnd)
        WinDestroyWindow(CPUHwnd);
      if(ClipMonHwnd)
        WinDestroyWindow(ClipMonHwnd);
      if(VirtHwnd)
        WinDestroyWindow(VirtHwnd);
      if(ClipHwnd)
        WinDestroyWindow(ClipHwnd);
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}

