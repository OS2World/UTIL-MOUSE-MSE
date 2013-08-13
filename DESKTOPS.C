#define INCL_DOS
#define INCL_WIN
#define INCL_GPI

#define DATAS 1

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "msehook.h"
#include "mse.h"
#include "dialog.h"


MRESULT EXPENTRY DeskProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_CREATE:
      {
        MRESULT mr;

        mr = PFNWPStatic(hwnd,msg,mp1,mp2);
        SetPresParams(hwnd,
                      RGB_BLACK,
                      RGB_YELLOW,
                      RGB_BLACK,
                      helvtext);
        return mr;
      }

    case UM_TELLDESKTOP:
      {
        char s[32];
        long virtx = (long)mp1,virty = (long)mp2,cx,cy;
        HPS  hps;
        POINTL aptl[TXTBOX_COUNT];

        sprintf(s,"Desktop #%u",(virtx + ((virty - 1) * 3)));
        WinSetWindowText(hwnd,s);
        hps = WinGetPS(hwnd);
        GpiQueryTextBox(hps,strlen(s),s,TXTBOX_COUNT,aptl);
        WinReleasePS(hps);
        cx = aptl[TXTBOX_TOPRIGHT].x + 4;
        cy = aptl[TXTBOX_TOPRIGHT].y + 4;
        WinSetWindowPos(hwnd,
                        HWND_TOP,
                        (xScreen / 2) - (cx / 2),
                        yScreen - cy,
                        cx,
                        cy,
                        SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_ZORDER);
      }
      return 0;

    case WM_MOUSEMOVE:
    case WM_BUTTON1CLICK:
    case WM_TIMER:
      WinDestroyWindow(hwnd);
      return MRFROMSHORT(TRUE);

    case WM_DESTROY:
      if(DeskHwnd == hwnd)
        DeskHwnd = (HWND)0;
      break;
  }
  return PFNWPStatic(hwnd,msg,mp1,mp2);
}


void ChangeDesktop (HAB hab,long virtx,long virty) {

  /* move desktop */
  HENUM     henum;            /* Window handle of WC_FRAME class Desktop */
  HWND      hwndApplication;  /* Window handles of enumerated application */
  char      ucClassname[9];   /* Class name of enumerated application */
  char      ucWindowText[64]; /* Window text of enumerated application */
  ULONG     ulAppCount = 0,ulNumSwps = 64,ulCount,ulSize,c,x,ulOrgCount;
  long      vx,vy;
  SWP      *swpApps,*test;
  PSWBLOCK  pswb;

  if(virtx < 1)
    virtx = 1;
  if(virtx > 3)
    virtx = 3;
  if(virty < 1)
    virty = 1;
  if(virty > 3)
    virty = 3;
  
  /*
   * virtx and virty represent the matrix of the new desktop.
   * now we calculate the matrix of the present desktop in vx and vy.
   */
  vx = (xVirtual / xScreen) + 2;
  vy = (yVirtual / yScreen) + 2;

  if(vx == virtx &&
     vy == virty)  /* no need to move anything */
    return;

  swpApps = malloc(sizeof(SWP) * ulNumSwps);
  if(!swpApps)
    return;

  /* Begin with offset 0 */
  ulAppCount = 0;
  /* Enumerate all descendants of HWND_DESKTOP,
     which are the frame windows seen on Desktop,
     but not having necessarily the class WC_FRAME */
  henum = WinBeginEnumWindows(HWND_DESKTOP);
  while((hwndApplication = WinGetNextWindow(henum)) != (HWND)0) {
    /* Don't move desktop */
    if(hwndApplication != hwndDesktop &&
       hwndApplication != hwndWPS &&
       hwndApplication != hwndBottom &&
       WinIsWindow(hab,hwndApplication)) {
      *ucClassname = *ucWindowText = 0;
      WinQueryClassName(hwndApplication,
                        sizeof(ucClassname),
                        ucClassname);
      WinQueryWindowText(hwndApplication,
                         sizeof(ucWindowText),
                         ucWindowText);
      /* Only move WC_FRAME class (#1) and wpFolder class windows.
       * If it is such a window, overwrite current offset
       * with next enumeration so that array only contains
       * windows that must be moved
       */
      if(*ucClassname &&
         (!strcmp(ucClassname,"#1") ||
          (!strcmp(ucClassname,"wpFolder") &&
           !fVirtWPS)) &&
          *ucWindowText &&
          strcmp(ucWindowText,"Window List") &&
          !IsExcluded(ucWindowText)) {
        if(WinQueryWindowPos(hwndApplication,&swpApps[ulAppCount]) &&
           !(swpApps[ulAppCount].fl & (SWP_HIDE | SWP_MINIMIZE))) {
          swpApps[ulAppCount].fl = SWP_MOVE | SWP_NOADJUST;
          if(virtx > vx)
            swpApps[ulAppCount].x -= ((virtx - vx) * xScreen);
          else if(virtx < vx)
            swpApps[ulAppCount].x += ((vx - virtx) * xScreen);
          if(virty > vy)
            swpApps[ulAppCount].y -= ((virty - vy) * yScreen);
          else if(virty < vy)
            swpApps[ulAppCount].y += ((vy - virty) * yScreen);
          if(ulAppCount + 1 >= ulNumSwps) {
            test = realloc(swpApps,sizeof(SWP) * (ulNumSwps + 64));
            if(test) {
              swpApps = test;
              ulNumSwps += 64;
            }
            else
              break;
          }
          ulAppCount++;
        }
      }
    }
  }
  WinEndEnumWindows(henum);    /* End enumeration */
  ulOrgCount = ulAppCount;

  /* Also process windows in switch list */
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
         pswb->aswentry[c].swctl.hwnd &&
         pswb->aswentry[c].swctl.hwnd != hwndWPS &&
         pswb->aswentry[c].swctl.hwnd != hwndDesktop &&
         pswb->aswentry[c].swctl.hwnd != hwndBottom &&
         WinIsWindow(hab,pswb->aswentry[c].swctl.hwnd)) {
        *ucClassname = *ucWindowText = 0;
        WinQueryClassName(pswb->aswentry[c].swctl.hwnd,
                          sizeof(ucClassname),
                          ucClassname);
        WinQueryWindowText(pswb->aswentry[c].swctl.hwnd,
                          sizeof(ucWindowText),
                          ucWindowText);
        if(*ucClassname &&
           (!strcmp(ucClassname,"#1") ||
            (!strcmp(ucClassname,"wpFolder") &&
             !fVirtWPS)) &&
           *ucWindowText &&
           strcmp(ucWindowText,"Window List") &&
           !IsExcluded(ucWindowText)) {
          for(x = 0;x < ulOrgCount;x++) {
            if(swpApps[x].hwnd == pswb->aswentry[c].swctl.hwnd)
              break;    /* we've already done this one */
          }
          if(x >= ulOrgCount &&
             WinQueryWindowPos(pswb->aswentry[c].swctl.hwnd,
                               &swpApps[ulAppCount]) &&
             (!(swpApps[ulAppCount].fl & (SWP_HIDE | SWP_MINIMIZE)))) {
            swpApps[ulAppCount].fl = SWP_MOVE | SWP_NOADJUST;
            if(virtx > vx)
              swpApps[ulAppCount].x -= ((virtx - vx) * xScreen);
            else if(virtx < vx)
              swpApps[ulAppCount].x += ((vx - virtx) * xScreen);
            if(virty > vy)
              swpApps[ulAppCount].y -= ((virty - vy) * yScreen);
            else if(virty < vy)
              swpApps[ulAppCount].y += ((vy - virty) * yScreen);
            if(ulAppCount + 1 >= ulNumSwps)
              test = realloc(swpApps,sizeof(SWP) * (ulNumSwps + 64));
            if(test) {
              swpApps = test;
              ulNumSwps += 64;
            }
            else
              break;
            ulAppCount++;
          }
        }
      }
    }
    free(pswb);
    _heapmin();
  }

  /* Now move all windows */
  if(ulAppCount) {
    if(!WinSetMultWindowPos(hab,swpApps,ulAppCount)) {
      for(x = 0;x < ulAppCount;x++)
        if(!WinSetWindowPos(swpApps[x].hwnd,
                            HWND_TOP,
                            swpApps[x].x,
                            swpApps[x].y,
                            swpApps[x].cx,
                            swpApps[x].cy,
                            swpApps[x].fl))
          WinSetWindowPos(swpApps[x].hwnd,
                          HWND_TOP,
                          swpApps[x].x,
                          swpApps[x].y,
                          swpApps[x].cx,
                          swpApps[x].cy,
                          swpApps[x].fl);
    }
  }
  free(swpApps);
  _heapmin();

  xVirtual = (virtx - 2) * xScreen;
  yVirtual = (virty - 2) * yScreen;

  desktick = 0;
  if(!DeskHwnd)
    DeskHwnd = WinCreateWindow(HWND_DESKTOP,
                               desknoteclass,
                               (PSZ)NULL,
                               WS_CLIPSIBLINGS |
                               SS_TEXT | DT_CENTER | DT_VCENTER,
                               0,0,0,0,
                               HWND_DESKTOP,
                               HWND_TOP,
                               DESK_FRAME,
                               MPVOID,
                               MPVOID);
  if(DeskHwnd)
    PostMsg(DeskHwnd,
            UM_TELLDESKTOP,
            MPFROMLONG(virtx),
            MPFROMLONG(virty));
}


BOOL WhichDesk (HWND hwnd,SWP *pswp,long *lx,long *ly) {

  /* calculate the "desktop" in which the window lies */
  SWP    swp;
  long   offx,offy;
  double dx,dy;

  if(!pswp) {
    pswp = &swp;
    if(!WinQueryWindowPos(hwnd,&swp))
      return FALSE;
  }
  if(swp.fl & (SWP_MINIMIZE | SWP_HIDE))
    return FALSE;
  /* where is it from current desktop? */
  dx = (double)(swp.x + (swp.cx / 2)) / (double)xScreen;
  dy = (double)(swp.y + (swp.cy / 2)) / (double)yScreen;
  *lx = floor(dx);
  *ly = floor(dy);
  /* "quadrant" of current desktop? */
  offx = (xVirtual / xScreen) + 2;
  offy = (yVirtual / yScreen) + 2;
  /* add together and we have new desktop we want */
  *lx += offx;
  *ly += offy;
  return TRUE;
}


void NormalizeWindow (HWND hwnd,BOOL activate) {

  SWP  swp;
  long oldx,oldy,fl = SWP_MOVE | SWP_NOADJUST;

  if(activate)
    fl |= SWP_ACTIVATE;
  if(WinQueryWindowPos(hwnd,&swp)) {
    if(swp.fl & (SWP_HIDE | SWP_MINIMIZE)) {
      swp.x = WinQueryWindowUShort(hwnd,QWS_XRESTORE);
      swp.y = WinQueryWindowUShort(hwnd,QWS_YRESTORE);
    }
    oldx = swp.x;
    oldy = swp.y;
    while(swp.x + (swp.cx / 2) > xScreen)
      swp.x -= xScreen;
    while(swp.y + (swp.cy / 2) > yScreen)
      swp.y -= yScreen;
    while(swp.x + swp.cx < (WinQuerySysValue(HWND_DESKTOP,SV_CYSIZEBORDER) * 2))
      swp.x += xScreen;
    while(swp.y + swp.cy < (WinQuerySysValue(HWND_DESKTOP,SV_CYSIZEBORDER) * 2))
      swp.y += yScreen;
    if(oldx != swp.x ||
       oldy != swp.y) {
      if(swp.fl & (SWP_HIDE | SWP_MINIMIZE)) {
        WinSetWindowUShort(hwnd,
                           QWS_XRESTORE,
                           swp.x);
        WinSetWindowUShort(hwnd,
                           QWS_YRESTORE,
                           swp.y);
      }
      else
        WinSetWindowPos(hwnd,
                        HWND_TOP,
                        swp.x,
                        swp.y,
                        0,
                        0,
                        fl);
    }
  }
}


void NormalizeAll (void) {

  HENUM     henum;            /* Window handle of WC_FRAME class Desktop */
  HWND      hwnd;             /* Window handles of enumerated application */
  char      ucClassname[9];  /* Class name of enumerated application */
  char      ucWindowText[64]; /* Window text of enumerated application */
  PSWBLOCK  pswb;
  ULONG     ulSize,ulCount,c;

  /* Enumerate all descendants of HWND_DESKTOP,
     which are the frame windows seen on Desktop,
     but not having necessarily the class WC_FRAME */
  henum = WinBeginEnumWindows(HWND_DESKTOP);
  if(henum != (HENUM)0) {
    while((hwnd = WinGetNextWindow(henum)) != (HWND)0) {
      WinQueryClassName(hwnd,
                        sizeof(ucClassname),
                        ucClassname);
      WinQueryWindowText(hwnd,
                         sizeof(ucWindowText),
                         ucWindowText);
      /* Only move WC_FRAME class (#1) windows. */
      if(strcmp(ucClassname,"#1") &&
         strcmp(ucClassname,"wpFolder"))
        continue;
      NormalizeWindow(hwnd,FALSE);
    }
    WinEndEnumWindows(henum);    /* End enumeration */
  }

  /* Normalize all windows in switch list */
  /* Get the switch list information */
  ulCount = WinQuerySwitchList(0,NULL,0);
  ulSize = sizeof(SWBLOCK) + sizeof(HSWITCH) + (ulCount + 4L) *
           (long)sizeof(SWENTRY);
  /* Allocate memory for list */
  if((pswb = malloc(ulSize)) != NULL) {
    WinQuerySwitchList(0,pswb,ulSize - sizeof(SWENTRY));
    /* do the dirty deed */
    for(c = 0;c < pswb->cswentry;c++) {
      if(pswb->aswentry[c].swctl.hwnd &&
         pswb->aswentry[c].swctl.hwnd != hwndWPS &&
         pswb->aswentry[c].swctl.hwnd != hwndDesktop &&
         pswb->aswentry[c].swctl.hwnd != hwndBottom)
        NormalizeWindow(pswb->aswentry[c].swctl.hwnd,FALSE);
    }
    free(pswb);
    _heapmin();
  }
}


MRESULT EXPENTRY OTextSubProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_PRESPARAMCHANGED:
      PresParamChanged(hwnd,
                       "BigVirtStat",
                       mp1,
                       mp2);
      break;

    case WM_CONTEXTMENU:
      OpenObject("<WP_VIEWER>",
                 defopen);
      return MRFROMSHORT(TRUE);

    case WM_BUTTON1CLICK:
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
      WinDestroyWindow(WinQueryWindow(hwnd,QW_PARENT));
      return MRFROMSHORT(TRUE);

    case WM_MOUSEMOVE:
      if(lastHwnd != (HWND)-1) {
        WinSetWindowText(hwnd,
                         "B1 here = MSE, B2 here = MinWinViewer");
        lastHwnd = (HWND)-1;
      }
      break;
  }
  return PFNWPStatic(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY OFrameSubProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_CALCFRAMERECT:
      {
        MRESULT mr;
        PRECTL  prectl;
        long    by;
        HPS     hps;
        POINTL  aptl[TXTBOX_COUNT];

        mr = PFNWPFrame(hwnd,msg,mp1,mp2);
        hps = WinGetPS(WinWindowFromID(hwnd,99));
        GpiQueryTextBox(hps,5,"Wjq~,",TXTBOX_COUNT,aptl);
        WinReleasePS(hps);
        by = aptl[TXTBOX_TOPRIGHT].y + 4;
        prectl = (PRECTL)mp1;
        prectl->yBottom += by;
        prectl->yTop -= by;
        return mr;
      }

    case WM_FORMATFRAME:
      {
        SHORT   x,sCount = (SHORT)PFNWPFrame(hwnd,msg,mp1,mp2);
        PSWP    pswp,pswpClient = (PSWP)mp1;
        long    by;
        HPS     hps;
        POINTL  aptl[TXTBOX_COUNT];

        hps = WinGetPS(WinWindowFromID(hwnd,99));
        GpiQueryTextBox(hps,
                        5,
                        "Wjq~,",
                        TXTBOX_COUNT,
                        aptl);
        WinReleasePS(hps);
        by = aptl[TXTBOX_TOPRIGHT].y + 4;
        for(x = 0;x < sCount;x++) {
          if(WinQueryWindowUShort(pswpClient->hwnd,QWS_ID) == FID_CLIENT)
            break;
          pswpClient++;
        }
        pswp = (PSWP)mp1 + sCount;
        memcpy(pswp,pswpClient,sizeof(SWP));
        pswpClient->y += by;
        pswpClient->cy -= by;
        pswp->hwnd = WinWindowFromID(hwnd,99);
        pswp->cy = by;
        sCount++;
        return MRFROMSHORT(sCount);
      }

    case WM_QUERYFRAMECTLCOUNT:
      {
        SHORT sCount = (SHORT)PFNWPFrame(hwnd,msg,mp1,mp2);

        sCount++;
        return MRFROMSHORT(sCount);
      }
  }
  return PFNWPFrame(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY OviewProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  static SWP  *swpApps    = NULL;
  static ULONG ulAppCount = 0;

  switch(msg) {
    case WM_CREATE:
      if(!fDesktops ||
         (VirtHwnd &&
          WinIsWindow(WinQueryAnchorBlock(hwnd),
                      VirtHwnd))) {
        if(!fDesktops &&
           !VirtHwnd)
          DosBeep(50,100);
        WinDestroyWindow(WinQueryWindow(hwnd,QW_PARENT));
      }
      else {

        HWND hwndTemp;

        lastHwnd = (HWND)0;
        if(!RestorePresParams(hwnd,
                              "BigVirtual"))
          SetPresParams(hwnd,
                        -1,
                        -1,
                        -1,
                        shelvtext);
        hwndTemp = WinCreateWindow(WinQueryWindow(hwnd,QW_PARENT),
                                   WC_STATIC,
                                   (PSZ)NULL,
                                   WS_VISIBLE | SS_TEXT | DT_LEFT | DT_VCENTER,
                                   0,
                                   0,
                                   0,
                                   0,
                                   WinQueryWindow(hwnd,QW_PARENT),
                                   HWND_TOP,
                                   99,
                                   MPVOID,
                                   MPVOID);
        if(!RestorePresParams(hwndTemp,
                              "BigVirtStat"))
          SetPresParams(hwndTemp,
                        RGB_BLACK,
                        RGB_PALEGRAY,
                        RGB_BLACK,
                        shelvtext);
        WinSubclassWindow(WinQueryWindow(hwnd,QW_PARENT),
                          (PFNWP)OFrameSubProc);
        WinSubclassWindow(hwndTemp,
                          (PFNWP)OTextSubProc);
        WinSendMsg(hwnd,
                   UM_SETUP,
                   MPVOID,
                   MPVOID);
      }
      break;

    case WM_SIZE:
      PostMsg(hwnd,
              WM_SAVEAPPLICATION,
              MPVOID,
              MPVOID);
      break;

    case WM_PRESPARAMCHANGED:
      PresParamChanged(hwnd,
                       "BigVirtual",
                       mp1,
                       mp2);
      break;

    case UM_SETUP:
      {
        ULONG size = sizeof(SWP);

        if(!BigSwp.hwnd &&
           !PrfQueryProfileData(prof,
                                appname,
                                "BigDesk.Pos",
                                &BigSwp,
                                &size)) {
          BigSwp.hwnd = WinQueryWindow(hwnd,QW_PARENT);
          if(WinQueryTaskSizePos(WinQueryAnchorBlock(hwnd),
                                 0,
                                 &BigSwp)) {
            BigSwp.x = 50;
            BigSwp.y = 100;
            BigSwp.cx = 200;
            BigSwp.cy = 100;
          }
        }
        while(BigSwp.x > xScreen)
          BigSwp.x -= xScreen;
        while(BigSwp.y > yScreen)
          BigSwp.y -= yScreen;
        while(BigSwp.x + BigSwp.cx < 4)
          BigSwp.x += xScreen;
        while(BigSwp.y + BigSwp.cy < 4)
          BigSwp.y += yScreen;
        WinSetWindowPos(WinQueryWindow(hwnd,QW_PARENT),
                        HWND_TOP,
                        BigSwp.x,
                        BigSwp.y,
                        BigSwp.cx,
                        BigSwp.cy,
                        SWP_SHOW | SWP_SIZE | SWP_MOVE | SWP_ACTIVATE);
        if(fMoveMouse)
          WinSetPointerPos(HWND_DESKTOP,
                           BigSwp.x + (BigSwp.cx / 2),
                           BigSwp.y + (BigSwp.cy / 2));
        VirtHwnd = WinQueryWindow(hwnd,QW_PARENT);
      }
      return 0;

    case WM_FOCUSCHANGE:
      PostMsg(hwnd,
              UM_TIMER,
              MPVOID,
              MPVOID);
      break;

    case UM_TIMER:
      {
        HWND hwndFrame;

        hwndFrame = WinQueryActiveWindow(HWND_DESKTOP);
        if(hwndFrame != WinQueryWindow(hwnd,QW_PARENT))
          WinDestroyWindow(WinQueryWindow(hwnd,QW_PARENT));
      }
      return 0;

    case WM_CONTEXTMENU:
      PostMsg(hwndConfig,
              UM_SWITCHLIST,
              MPVOID,
              MPVOID);
      WinDestroyWindow(WinQueryWindow(hwnd,QW_PARENT));
      return MRFROMSHORT(TRUE);

    case WM_BUTTON1CLICK:
      {
        long   xpos,ypos,virtx,virty;
        double px,py;
        SWP    swp;

        WinSetActiveWindow(HWND_DESKTOP,
                           HWND_DESKTOP);
        WinSetFocus(HWND_DESKTOP,
                    HWND_DESKTOP);
        WinSendMsg(hwnd,
                   WM_SAVEAPPLICATION,
                   MPVOID,
                   MPVOID);
        xpos = SHORT1FROMMP(mp1);
        ypos = SHORT2FROMMP(mp1);
        WinQueryWindowPos(hwnd, &swp);
        if(xpos < 0 ||
           xpos > swp.cx ||
           ypos < 0 ||
           ypos > swp.cy)
          break;
        if(SHORT2FROMMP(mp2) & KC_SHIFT) {
          if(lastHwnd &&
             lastHwnd != (HWND)-1 &&
             PostMsg(ObjectHwnd3,
                     UM_COMMAND,
                     MPFROMLONG((ULONG)lastHwnd),
                     MPFROMSHORT(TRUE)))
            WinDestroyWindow(WinQueryWindow(hwnd,QW_PARENT));
          else
            DosBeep(250,100);
        }
        else {
          /* determine "desktop" picked */
          px = (double)xpos / (double)swp.cx;
          py = (double)ypos / (double)swp.cy;
          virtx = (px > .33333) ? ((px > .66666) ? 3 : 2) : 1;
          virty = (py > .33333) ? ((py > .66666) ? 3 : 2) : 1;
          ChangeDesktop(WinQueryAnchorBlock(hwnd),
                        virtx,
                        virty);
          if(lastHwnd &&
             lastHwnd != (HWND)-1)
            PostMsg(ObjectHwnd3,
                    UM_COMMAND,
                    MPFROMLONG((ULONG)lastHwnd),
                    MPVOID);
          WinDestroyWindow(WinQueryWindow(hwnd,QW_PARENT));
        }
      }
      return MRFROMSHORT(TRUE);

    case WM_MOUSEMOVE:
      {
        SWP    swp;
        double fScaleX,fScaleY;
        HWND   hwndFrame = (HWND)0;
        POINTL ptl;
        char   text[80],*p;
        ULONG  x;

        WinQueryWindowPos(hwnd,&swp);
        fScaleX = (3.0 * xScreen) / ((double)swp.cx - 1.0);
        fScaleY = (3.0 * yScreen) / ((double)swp.cy - 1.0);
        ptl.x = (long)((double)SHORT1FROMMP(mp1) * fScaleX);
        ptl.y = (long)((double)SHORT2FROMMP(mp1) * fScaleY);
        ptl.x -= (xVirtual + xScreen);
        ptl.y -= (yVirtual + yScreen);
/*
sprintf(text,"%ld %ld",ptl.x,ptl.y);
WinSetWindowText(WinQueryWindow(hwnd,QW_PARENT),text);
*/
        for(x = 0;x < ulAppCount;x++) {
          if(swpApps[x].x <= ptl.x &&
             swpApps[x].x + swpApps[x].cx >= ptl.x &&
             swpApps[x].y <= ptl.y &&
             swpApps[x].y + swpApps[x].cy >= ptl.y) {
            hwndFrame = swpApps[x].hwnd;
            break;
          }
        }
        if(hwndFrame &&
           lastHwnd != hwndFrame) {
          lastHwnd = hwndFrame;
          *text = 0;
          WinQueryWindowText(hwndFrame,
                             sizeof(text),
                             text);
          p = text;
          while(*p) {
            if(*p == '\r' || *p == '\n')
              *p = ' ';
            p++;
          }
          lstrip(rstrip(text));
          WinSetWindowText(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),99),
                           (*text) ? text : "<No window text>");
        }
        else if(!hwndFrame) {
          lastHwnd = (HWND)0;
          WinSetWindowText(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),99),
                           "");
        }
      }
      break;

    case WM_PAINT:            /* Draw overview of virtual Desktop */
      {
        HPS     hps;
        SWP     swp;
        double  fScaleX;      /* Reduce factor to reduce horizontal size of
                                 virtual Desktop to horizonal client window
                                 size */
        double  fScaleY;
        POINTL  ptl[4];       /* Point of lines,... */
        POINTL  ptlOrigin;    /* Coordinates (0|0) within client area */
        ULONG   ulColor = 1;  /* Use least significant 7 bits for window
                                 colors */
        long    lTemp,lTemp2;
        HENUM   henum;           /* Window handle of WC_FRAME class Desktop */
        HWND    hwndApplication; /* Window handles of enumerated application */
        ULONG   ulNumAlloc;
        SWP    *swpTemp;
        CHAR    ucClassname[9];     /* Class name of enumerated application */
        CHAR    ucWindowText[64];   /* Window text of enumerated application */
        RECTL   rectl;
        HWND    hwndFrame,hwndObject;

        hwndFrame = WinQueryWindow(hwnd,QW_PARENT);
        hwndObject = WinQueryObjectWindow(HWND_DESKTOP);
        hps = WinBeginPaint(hwnd,
                            (HPS)0,
                            &rectl);
        if(hps) {
          WinFillRect(hps,
                      &rectl,
                      CLR_PALEGRAY);
          /* Get the client area size */
          WinQueryWindowPos(hwnd,
                            &swp);
          /* Now get scale factor to scale virtual Desktop to client area */
          fScaleX = ((double)swp.cx - 1.0) / (3.0 * xScreen);
          fScaleY = (double)(swp.cy - 1.0) / (3.0 * yScreen);
          /* Get coordinates (0|0) origin */
          ptlOrigin.x = xScreen * fScaleX;
          ptlOrigin.y = yScreen * fScaleY;
          /* Get physical Desktop origin */
          ptlOrigin.x += (double)xVirtual * fScaleX;
          ptlOrigin.y += (double)yVirtual * fScaleY;
          /* Display the physical Desktop */
          GpiSetColor(hps,
                      CLR_WHITE);
          ptl[0].x = ptl[1].x = ptlOrigin.x;
          ptl[0].y = ptl[1].y = ptlOrigin.y;
          ptl[1].x += (double)xScreen * fScaleX;
          ptl[1].y += (double)yScreen * fScaleY;
          ptl[0].x++;
          ptl[0].y++;
          ptl[1].x--;
          ptl[1].y--;
          GpiMove(hps,
                  &ptl[0]);
          GpiBox(hps,
                 DRO_OUTLINEFILL,
                 &ptl[1],
                 0,
                 0);
          if(!swpApps) {
            swpApps = malloc(48 * sizeof(SWP));
            if(swpApps) {
              ulNumAlloc = 48;
              /* enumerate all desktop windows */
              henum = WinBeginEnumWindows(HWND_DESKTOP);
              /* Begin with offset 0 in first iteration */
              ulAppCount = 0;
              while((hwndApplication = WinGetNextWindow(henum)) != (HWND)0) {
                if(hwndApplication != hwndDesktop &&
                   hwndApplication != hwndWPS &&
                   hwndApplication != hwndBottom &&
                   hwndApplication != hwndFrame &&
                   WinIsWindowVisible(hwndApplication) &&
                   WinQueryWindow(hwndApplication,QW_OWNER) != hwndObject) {
                  *ucWindowText = 0;
                  WinQueryWindowText(hwndApplication,
                                     sizeof(ucWindowText),
                                     ucWindowText);
                  if(*ucWindowText &&
                     strcmp(ucWindowText,"Window List")) {
                    *ucClassname = 0;
                    WinQueryClassName(hwndApplication,
                                      sizeof(ucClassname),
                                      ucClassname);
                    if(*ucClassname &&
                       (!strcmp(ucClassname,"#1") ||
                        !strcmp(ucClassname,"wpFolder"))) {
                      WinQueryWindowPos(hwndApplication,
                                        &swpApps[ulAppCount]);
                      if(!(swp.fl & (SWP_HIDE | SWP_MINIMIZE)) &&
                         swp.cx && swp.cy) {
                        swpApps[ulAppCount].hwnd = hwndApplication;
                        ulAppCount++;
                        if(ulAppCount >= ulNumAlloc) {
                          swpTemp = realloc(swpApps,
                                            ((ulNumAlloc + 48) * sizeof(SWP)));
                          if(swpTemp) {
                            swpApps = swpTemp;
                            ulNumAlloc += 48;
                          }
                          else
                            break;
                        }
                      }
                    }
                  }
                }
              }
              WinEndEnumWindows(henum);    /* End enumeration */
            }
            else
              WinDestroyWindow(WinQueryWindow(hwnd,QW_PARENT));
          }
          if(ulAppCount) {
            /* Now display the windows from topmost to bottommost */
            for(lTemp = ulAppCount - 1; lTemp >= 0; lTemp--) {
              while((ulColor & 15) == CLR_PALEGRAY ||
                    (ulColor & 15) == CLR_BACKGROUND)
                ulColor++;
              GpiSetColor(hps,
                          1 + (ulColor & 15));
              ptl[0].x = (double)ptlOrigin.x +
                         ((double)swpApps[lTemp].x * fScaleX);
              ptl[0].y = (double)ptlOrigin.y +
                         ((double)swpApps[lTemp].y * fScaleY);
              ptl[1].x = ptl[0].x +
                         ((double)swpApps[lTemp].cx * fScaleX);
              ptl[1].y = ptl[0].y + ((double)swpApps[lTemp].cy *
                                     fScaleY);
              GpiMove(hps,
                      &ptl[0]);
              GpiBox(hps,
                     DRO_OUTLINE,
                     &ptl[1],
                     0,
                     0);
              if(fVirtText) {
                *ucWindowText = 0;
                ptl[0].x += 2;
                ptl[0].y += 2;
                rectl.xLeft = ptl[0].x;
                rectl.xRight = ptl[1].x;
                rectl.yBottom = ptl[0].y;
                rectl.yTop = ptl[1].y;
                WinQueryWindowText(swpApps[lTemp].hwnd,
                                   sizeof(ucWindowText),
                                   ucWindowText);
                if(ucWindowText)
                  GpiCharStringPosAt(hps,
                                     &ptl[0],
                                     &rectl,
                                     CHS_CLIP,
                                     strlen(ucWindowText),
                                     ucWindowText,
                                     NULL);
              }
              ulColor++;
            }
          }
          /* Get coordinates (0|0) origin */
          ptlOrigin.x = xScreen * fScaleX;
          ptlOrigin.y = yScreen * fScaleY;
          /* Draw the borders of the 3 * 3 Desktops */
          GpiSetColor(hps,
                      CLR_BLACK);
          for(lTemp = 0; lTemp <= 3; lTemp++)  {
            ptl[0].x = 0;
            ptl[0].y = ptlOrigin.y * lTemp;
            GpiMove(hps,
                    &ptl[0]);
            ptl[0].x = swp.cx;
            GpiLine(hps,
                    &ptl[0]);
            ptl[0].x = ptlOrigin.x * lTemp;
            ptl[0].y = 0;
            GpiMove(hps,
                    &ptl[0]);
            ptl[0].y = swp.cy;
            GpiLine(hps,
                    &ptl[0]);
          }
          /* number the desktops */
          for(lTemp = 1;lTemp < 4;lTemp++) {
            for(lTemp2 = 1;lTemp2 < 4;lTemp2++) {
              sprintf(ucWindowText,
                      "%lu",
                      (lTemp + ((lTemp2 - 1) * 3)));
              ptl[0].x = ((double)xScreen * fScaleX) * (lTemp - 1);
              ptl[0].y = ((double)yScreen * fScaleY) * (lTemp2 - 1);
              ptl[0].x += 2;
              ptl[0].y += 2;
              GpiCharStringAt(hps,
                              &ptl[0],
                              1,
                              ucWindowText);
            }
          }
          /* Get physical Desktop origin */
          ptlOrigin.x += (double)xVirtual * fScaleX;
          ptlOrigin.y += (double)yVirtual * fScaleY;
          /* Display the physical Desktop (just an outline)*/
          GpiSetMix(hps,
                    FM_OVERPAINT);
          GpiSetColor(hps,
                      CLR_WHITE);
          ptl[0].x = ptl[1].x = ptlOrigin.x;
          ptl[0].y = ptl[1].y = ptlOrigin.y;
          ptl[1].x += (double)xScreen * fScaleX;
          ptl[1].y += (double)yScreen * fScaleY;
          ptl[0].x++;
          ptl[0].y++;
          ptl[1].x--;
          ptl[1].y--;
          GpiMove(hps,
                  &ptl[0]);
          GpiBox(hps,
                 DRO_OUTLINE,
                 &ptl[1],
                 0,
                 0);
          WinEndPaint(hps);
        }
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_CLIPHELP:
          ViewHelp(hwnd,
                   "Virtual page");
          break;
      }
      return 0;

    case WM_SAVEAPPLICATION:
      if(VirtHwnd == WinQueryWindow(hwnd,QW_PARENT)) {

        SWP swp;

        if(WinQueryWindowPos(WinQueryWindow(hwnd,QW_PARENT),&swp)) {
          if((swp.fl & (SWP_HIDE | SWP_MINIMIZE | SWP_MAXIMIZE)) != 0)
            LoadRestorePos(&swp,
                           WinQueryWindow(hwnd,QW_PARENT));
          BigSwp = swp;
          SavePrf("BigDesk.Pos",
                  &BigSwp,
                  sizeof(SWP));
        }
      }
      break;

    case WM_CLOSE:
      WinSendMsg(hwnd,
                 WM_SAVEAPPLICATION,
                 MPVOID,
                 MPVOID);
      WinDestroyWindow(WinQueryWindow(hwnd,QW_PARENT));
      return 0;

    case WM_DESTROY:
      if(VirtHwnd == WinQueryWindow(hwnd,QW_PARENT)) {
        VirtHwnd = (HWND)0;
        if(swpApps) {
          free(swpApps);
          swpApps = NULL;
          ulAppCount = 0;
          lastHwnd = (HWND)0;
          _heapmin();
        }
        fSuspend = FALSE;
      }
      break;
  }
  return WinDefWindowProc(hwnd,msg,mp1,mp2);
}

