#define INCL_DOS
#define INCL_WIN
#define INCL_GPI

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


MRESULT EXPENTRY FitBitProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_PAINT:
      {
        HPS     hps;
        HBITMAP hbm;

        hps = WinBeginPaint(hwnd,
                            (HPS)0,
                            NULL);
        if(hps) {

          RECTL rcl;

          WinQueryWindowRect(hwnd,
                             &rcl);
          hbm = (HBITMAP)WinQueryWindowULong(hwnd,0);
          if(!hbm) {
            hbm = GpiLoadBitmap(hps,
                                0,
                                (ULONG)WinQueryWindowUShort(hwnd,QWS_ID),
                                0,
                                0);
            WinSetWindowULong(hwnd,
                              0,
                              (ULONG)hbm);
          }
          if(hbm) {

            BITMAPINFOHEADER2 bmp2;
            POINTL            aptl[4];

            memset(&bmp2,0,sizeof(bmp2));
            bmp2.cbFix = sizeof(bmp2);
            if(GpiQueryBitmapInfoHeader(hbm,&bmp2)) {
              aptl[0].x = 1;
              aptl[0].y = 1;
              aptl[1].x = rcl.xRight - 2;
              aptl[1].y = rcl.yTop - 2;
              aptl[2].x = 0;
              aptl[2].y = 0;
              aptl[3].x = bmp2.cx;
              aptl[3].y = bmp2.cy;
              GpiWCBitBlt(hps,
                          hbm,
                          4L,
                          aptl,
                          ROP_SRCCOPY,
                          BBO_IGNORE);
            }
          }
          {
            POINTL ptl;

            GpiSetColor(hps,
                        CLR_DARKGRAY);
            ptl.x = ptl.y = 0;
            GpiMove(hps,
                    &ptl);
            ptl.y = rcl.yTop - 1;
            GpiLine(hps,
                    &ptl);
            ptl.x = rcl.xRight - 1;
            GpiLine(hps,
                    &ptl);
            GpiSetColor(hps,
                        CLR_WHITE);
            ptl.y = 0;
            GpiLine(hps,
                    &ptl);
            ptl.x = 0;
            GpiLine(hps,
                    &ptl);
          }
          WinEndPaint(hps);
        }
      }
      return 0;

    case WM_DESTROY:
      {
        HBITMAP hbm;

        hbm = (HBITMAP)WinQueryWindowULong(hwnd,0);
        if(hbm)
          GpiDeleteBitmap(hbm);
      }
      break;
  }
  return PFNWPStatic(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY OuttieProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  MRESULT rc;
  HPS     hps;
  POINTL  ptl;
  RECTL   rcl;

  rc =  WinDefDlgProc(hwnd,
                      msg,
                      mp1,
                      mp2);
  hps = WinGetPS(hwnd);
  if(hps) {
    WinQueryWindowRect(hwnd,
                       &rcl);
    GpiSetColor(hps,
                CLR_DARKGRAY);
    ptl.y = rcl.yTop - 2;
    ptl.x = rcl.xRight - 2;
    GpiMove(hps,
            &ptl);
    ptl.y = 1;
    GpiLine(hps,
            &ptl);
    ptl.x = 1;
    GpiLine(hps,
            &ptl);
    ptl.y = rcl.yTop - 1;
    ptl.x = rcl.xRight - 1;
    GpiMove(hps,
            &ptl);
    ptl.y = 0;
    GpiLine(hps,
            &ptl);
    ptl.x = 0;
    GpiLine(hps,
            &ptl);
    GpiSetColor(hps,
                CLR_WHITE);
    ptl.x = ptl.y = 1;
    GpiMove(hps,
            &ptl);
    ptl.y = rcl.yTop - 2;
    GpiLine(hps,
            &ptl);
    ptl.x = rcl.xRight - 2;
    GpiLine(hps,
            &ptl);
    ptl.x = ptl.y = 0;
    GpiMove(hps,
            &ptl);
    ptl.y = rcl.yTop - 1;
    GpiLine(hps,
            &ptl);
    ptl.x = rcl.xRight - 1;
    GpiLine(hps,
            &ptl);
    WinReleasePS(hps);
  }
  return rc;
}


MRESULT EXPENTRY MseSetProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  static USHORT id;

  switch(msg) {
    case WM_INITDLG:
      PostMsg(hwnd,
              UM_REFRESH,
              MPVOID,
              MPVOID);
      break;

    case UM_REFRESH:
      {
        int x;

        for(x = 0;x < 4;x++)
          if(crnrcmds[x] > CR_MAXCMDS)
            crnrcmds[x] = CR_NONE;
        for(x = 0;
            x <= CR_MAXCMDS;
            x++)
          WinSendDlgItemMsg(hwnd,
                            CRNR_LISTBOX,
                            LM_INSERTITEM,
                            MPFROM2SHORT(LIT_END,0),
                            MPFROMP(crnrnames[x]));
        WinSendDlgItemMsg(hwnd,
                          CRNR_LISTBOX,
                          LM_SELECTITEM,
                          MPFROM2SHORT((SHORT)crnrcmds[0],0),
                          MPFROMSHORT(TRUE));
        WinCheckButton(hwnd,
                       CRNR_TOPLEFT,
                       TRUE);
        id = 0;
      }
      WinCheckButton(hwnd,
                     MSA_MOVEMOUSE,
                     fMoveMouse);
      WinCheckButton(hwnd,
                     MSA_DEFBUTTON,
                     fDefButton);
      WinCheckButton(hwnd,
                     MSA_CENTERDLG,
                     fCenterDlg);
      WinCheckButton(hwnd,
                     MSA_SLIDINGFOCUS,
                     fSlidingFocus);
      WinCheckButton(hwnd,
                     MSA_NOZORDER,
                     fNoZorderChange);
      WinCheckButton(hwnd,
                     MSA_WRAPSCREEN,
                     fWrapMouse);
      WinCheckButton(hwnd,
                     MS1_AUTODROP,
                     fAutoDropCombos);
      {
        char s[13];

        WinSendDlgItemMsg(hwnd,
                          MSA_MOUSEMINS,
                          EM_SETTEXTLIMIT,
                          MPFROM2SHORT(5,0),
                          MPVOID);
        sprintf(s,
                "%lu",
                ulMouseMins);
        WinSetDlgItemText(hwnd,
                          MSA_MOUSEMINS,
                          s);
        WinSendDlgItemMsg(hwnd,
                          MSA_DELAY,
                          EM_SETTEXTLIMIT,
                          MPFROM2SHORT(5,0),
                          MPVOID);
        sprintf(s,
                "%lu",
                ulDelay);
        WinSetDlgItemText(hwnd,
                          MSA_DELAY,
                          s);
      }
      return 0;

    case WM_PAINT:
      return OuttieProc(hwnd,msg,mp1,mp2);

    case WM_SETFOCUS:
      if(!mp2) {
        PostMsg(hwnd,
                WM_CONTROL,
                MPFROM2SHORT(MSA_MOUSEMINS,EN_KILLFOCUS),
                MPVOID);
        PostMsg(hwnd,
                WM_CONTROL,
                MPFROM2SHORT(MSA_DELAY,EN_KILLFOCUS),
                MPVOID);
      }
      break;

    case WM_CONTROL:
      switch(SHORT2FROMMP(mp1)) {
        case EN_KILLFOCUS:
          switch(SHORT1FROMMP(mp1)) {
            case MSA_MOUSEMINS:
              {
                char s[13];

                WinQueryDlgItemText(hwnd,
                                    MSA_MOUSEMINS,
                                    5,
                                    s);
                ulMouseMins = atol(s);
                if(ulMouseMins > 99999)
                  ulMouseMins = 0;
                SavePrf("MouseMins",
                        &ulMouseMins,
                        sizeof(ULONG));
              }
              break;

            case MSA_DELAY:
              {
                char s[13];

                WinQueryDlgItemText(hwnd,
                                    MSA_DELAY,
                                    5,
                                    s);
                ulDelay = atol(s);
                if(ulDelay > 99999)
                  ulDelay = 100;
                SavePrf("Delay",
                        &ulDelay,
                        sizeof(ULONG));
              }
              break;
          }
          break;

        case LN_ENTER:
          if(SHORT1FROMMP(mp1) == CRNR_LISTBOX) {

            SHORT sSelect;
            int   x;

            sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
                                               CRNR_LISTBOX,
                                               LM_QUERYSELECTION,
                                               MPVOID,
                                               MPVOID);
            if(sSelect >= 0) {
              crnrcmds[id] = sSelect;
              SavePrf("CornerCmds",
                      crnrcmds,
                      sizeof(crnrcmds));
            }
            if(sSelect == 0) {
              for(x = 0;x < 4;x++) {
                if(crnrcmds[x] > CR_NONE &&
                   crnrcmds[x] <= CR_MAXCMDS)
                  break;
              }
              if(x < 4)
                fUsingCorners = TRUE;
            }
            else
              fUsingCorners = TRUE;
          }
          break;

        case BN_CLICKED:
          switch(SHORT1FROMMP(mp1)) {
            case CRNR_TOPLEFT:
            case CRNR_TOPRIGHT:
            case CRNR_BOTLEFT:
            case CRNR_BOTRIGHT:
              id = SHORT1FROMMP(mp1) - CRNR_TOPLEFT;
              WinSendDlgItemMsg(hwnd,
                                CRNR_LISTBOX,
                                LM_SELECTITEM,
                                MPFROM2SHORT((SHORT)crnrcmds[id],0),
                                MPFROMSHORT(TRUE));
              break;

            case MS1_AUTODROP:
              fAutoDropCombos = (fAutoDropCombos) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MS1_AUTODROP,
                             fAutoDropCombos);
              SavePrf("AutoDropCombos",
                      &fAutoDropCombos,
                      sizeof(BOOL));
              break;

            case MSA_NONORMALIZE:
              fNoNormalize = (fNoNormalize) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_NONORMALIZE,
                             fNoNormalize);
              SavePrf("NoNormalizeOnExit",
                      &fNoNormalize,
                      sizeof(BOOL));
              break;

            case MSA_MOVEMOUSE:
              fMoveMouse = (fMoveMouse) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_MOVEMOUSE,
                             fMoveMouse);
              SavePrf("MoveMouse",
                      &fMoveMouse,
                      sizeof(BOOL));
              break;

            case MSA_DEFBUTTON:
              fDefButton = (fDefButton) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_DEFBUTTON,
                             fDefButton);
              SavePrf("DefButton",
                      &fDefButton,
                      sizeof(BOOL));
              break;

            case MSA_CENTERDLG:
              fCenterDlg = (fCenterDlg) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_CENTERDLG,
                             fCenterDlg);
              SavePrf("CenterDlg",
                      &fCenterDlg,
                      sizeof(BOOL));
              break;


            case MSA_SLIDINGFOCUS:
              fSlidingFocus = (fSlidingFocus) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_SLIDINGFOCUS,
                             fSlidingFocus);
              SavePrf("SlidingFocus",
                      &fSlidingFocus,
                      sizeof(BOOL));
              break;

            case MSA_NOZORDER:
              fNoZorderChange = (fNoZorderChange) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_NOZORDER,
                             fNoZorderChange);
              SavePrf("NoZOrder",
                      &fNoZorderChange,
                      sizeof(BOOL));
              break;

            case MSA_WRAPSCREEN:
              fWrapMouse = (fWrapMouse) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_WRAPSCREEN,
                             fWrapMouse);
              SavePrf("WrapMouse",
                      &fWrapMouse,
                      sizeof(BOOL));
              break;

        }
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case MSE_HELP:
          ViewHelp(hwnd,
                   "Mouse page");
          break;
        default:
          break;
      }
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY Mse2SetProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_INITDLG:
      PostMsg(hwnd,
              UM_REFRESH,
              MPVOID,
              MPVOID);
      break;

    case UM_REFRESH:
      WinCheckButton(hwnd,
                     MSC_MSEINTASKLIST,
                     fMSEInTaskList);
      WinSendDlgItemMsg(hwnd,
                        MSA_RUNAHEAD,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(3,0),
                        MPVOID);
      WinCheckButton(hwnd,
                     MSA_CHORDS,
                     fChords);
      {
        char s[13];

        sprintf(s,
                "%lu",
                lMaxRunahead);
        WinSetDlgItemText(hwnd,
                          MSA_RUNAHEAD,
                          s);
      }
      return 0;

    case WM_PAINT:
      return OuttieProc(hwnd,msg,mp1,mp2);

    case WM_SETFOCUS:
      if(!mp2) {
        PostMsg(hwnd,
                WM_CONTROL,
                MPFROM2SHORT(MSA_RUNAHEAD,EN_KILLFOCUS),
                MPVOID);
      }
      break;

    case WM_CONTROL:
      switch(SHORT2FROMMP(mp1)) {
        case EN_KILLFOCUS:
          switch(SHORT1FROMMP(mp1)) {
            case MSA_RUNAHEAD:
              {
                char s[6];

                WinQueryDlgItemText(hwnd,
                                    MSA_RUNAHEAD,
                                    4,
                                    s);
                lMaxRunahead = atol(s);
                lMaxRunahead = min(999,max(lMaxRunahead,1));
                SavePrf("MaxRunahead",
                        &lMaxRunahead,
                        sizeof(long));
              }
              break;
          }
          break;

        case BN_CLICKED:
          switch(SHORT1FROMMP(mp1)) {
            case MSC_MSEINTASKLIST:
              fMSEInTaskList = (fMSEInTaskList) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSC_MSEINTASKLIST,
                             fMSEInTaskList);
              SavePrf("MSEInTaskList",
                      &fMSEInTaskList,
                      sizeof(BOOL));
              break;

            case MSA_CHORDS:
              fChords = (fChords) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_CHORDS,
                             fChords);
              SavePrf("Chords",
                      &fChords,
                      sizeof(BOOL));
              break;

        }
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case MSE_HELP:
          ViewHelp(hwnd,
                   "Buttons page 2");
          break;
        default:
          break;
      }
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY FDlgSetProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_INITDLG:
      PostMsg(hwnd,
              UM_REFRESH,
              MPVOID,
              MPVOID);
      break;

    case UM_REFRESH:
      WinCheckButton(hwnd,
                     MSA_ENHANCEFILEDLG,
                     fEnhanceFileDlg);
      WinCheckButton(hwnd,
                     MS2_CONFIRMFILEDEL,
                     fConfirmFileDel);
      WinCheckButton(hwnd,
                     MS2_CONFIRMDIRDEL,
                     fConfirmDirDel);
      WinCheckButton(hwnd,
                     MS2_REMEMBER,
                     fRememberDirs);
      WinCheckButton(hwnd,
                     MS2_REMEMBERF,
                     fRememberFiles);
      WinCheckButton(hwnd,
                     MS2_AGGRESSIVE,
                     fAggressive);
      WinSendDlgItemMsg(hwnd,
                        MS2_VIEWER,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(CCHMAXPATH,0),
                        MPVOID);
      WinSetDlgItemText(hwnd,
                        MS2_VIEWER,
                        viewer);
      WinSendDlgItemMsg(hwnd,
                        MS2_EDITOR,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(CCHMAXPATH,0),
                        MPVOID);
      WinSetDlgItemText(hwnd,
                        MS2_EDITOR,
                        editor);
      return 0;

    case WM_PAINT:
      return OuttieProc(hwnd,msg,mp1,mp2);

    case WM_SETFOCUS:
      if(!mp2) {
        PostMsg(hwnd,
                WM_CONTROL,
                MPFROM2SHORT(MS2_VIEWER,EN_KILLFOCUS),
                MPVOID);
        PostMsg(hwnd,
                WM_CONTROL,
                MPFROM2SHORT(MS2_EDITOR,EN_KILLFOCUS),
                MPVOID);
      }
      break;

    case WM_CONTROL:
      switch(SHORT2FROMMP(mp1)) {
        case EN_KILLFOCUS:
          switch(SHORT1FROMMP(mp1)) {
            case MS2_VIEWER:
              {
                *viewer = 0;
                WinQueryDlgItemText(hwnd,
                                    MS2_VIEWER,
                                    sizeof(viewer),
                                    viewer);
                SavePrf("Viewer",
                        viewer,
                        strlen(viewer));
              }
              break;

            case MS2_EDITOR:
              {
                *editor = 0;
                WinQueryDlgItemText(hwnd,
                                    MS2_EDITOR,
                                    sizeof(editor),
                                    editor);
                SavePrf("Editor",
                        editor,
                        strlen(editor));
              }
              break;
          }
          break;

        case BN_CLICKED:
          switch(SHORT1FROMMP(mp1)) {
            case MSA_ENHANCEFILEDLG:
              fEnhanceFileDlg = (fEnhanceFileDlg) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_ENHANCEFILEDLG,
                             fEnhanceFileDlg);
              SavePrf("EnhanceFileDlg",
                      &fEnhanceFileDlg,
                      sizeof(BOOL));
              break;

            case MS2_REMEMBER:
              fRememberDirs = (fRememberDirs) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MS2_REMEMBER,
                             fRememberDirs);
              SavePrf("RememberDirs",
                      &fRememberDirs,
                      sizeof(BOOL));
              break;

            case MS2_REMEMBERF:
              fRememberFiles = (fRememberFiles) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MS2_REMEMBERF,
                             fRememberFiles);
              SavePrf("RememberFiles",
                      &fRememberFiles,
                      sizeof(BOOL));
              break;

            case MS2_AGGRESSIVE:
              fAggressive = (fAggressive) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MS2_AGGRESSIVE,
                             fAggressive);
              SavePrf("AggressiveFileDlg",
                      &fAggressive,
                      sizeof(BOOL));
              break;

            case MS2_CONFIRMFILEDEL:
              fConfirmFileDel = (fConfirmFileDel) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MS2_CONFIRMFILEDEL,
                             fConfirmFileDel);
              SavePrf("ConfirmFileDel",
                      &fConfirmFileDel,
                      sizeof(BOOL));
              break;

            case MS2_CONFIRMDIRDEL:
              fConfirmDirDel = (fConfirmDirDel) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MS2_CONFIRMDIRDEL,
                             fConfirmDirDel);
              SavePrf("ConfirmDirDel",
                      &fConfirmDirDel,
                      sizeof(BOOL));
              break;
          }
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case MSE_HELP:
          ViewHelp(hwnd,
                   "FileDlg page");
          break;

        case MSA_EDITFEXCLUDE:
          PostMsg((HWND)WinQueryWindowULong(hwnd,0),
                  msg,
                  mp1,
                  mp2);
          break;
        default:
          break;
      }
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY VirtSetProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_INITDLG:
      PostMsg(hwnd,
              UM_REFRESH,
              MPVOID,
              MPVOID);
      break;

    case UM_REFRESH:
      WinCheckButton(hwnd,
                     MSA_VIRTUAL,
                     fDesktops);
      WinCheckButton(hwnd,
                     MSA_VIRTTEXT,
                     fVirtText);
      WinCheckButton(hwnd,
                     MSA_NOVIRTWPS,
                     fVirtWPS);
      WinCheckButton(hwnd,
                     MSA_BUMPDESKS,
                     lBumpDesks);
      WinCheckButton(hwnd,
                     MSA_NONORMALIZE,
                     fNoNormalize);
      WinCheckButton(hwnd,
                     MSA_SHIFTBUMP,
                     fShiftBump);
      return 0;

    case WM_PAINT:
      return OuttieProc(hwnd,msg,mp1,mp2);

    case WM_CONTROL:
      switch(SHORT2FROMMP(mp1)) {
        case BN_CLICKED:
          switch(SHORT1FROMMP(mp1)) {
            case MSA_NONORMALIZE:
              fNoNormalize = (fNoNormalize) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_NONORMALIZE,
                             fNoNormalize);
              SavePrf("NoNormalizeOnExit",
                      &fNoNormalize,
                      sizeof(BOOL));
              break;

            case MSA_VIRTTEXT:
              fVirtText = (fVirtText) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_VIRTTEXT,
                             fVirtText);
              SavePrf("VirtText",
                      &fVirtText,
                      sizeof(BOOL));
              break;

            case MSA_NOVIRTWPS:
              fVirtWPS = (fVirtWPS) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_NOVIRTWPS,
                             fVirtWPS);
              SavePrf("VirtWPS",
                      &fVirtWPS,
                      sizeof(BOOL));
              break;

            case MSA_VIRTUAL:
              fDesktops = (fDesktops) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_VIRTUAL,
                             fDesktops);
              SavePrf("Desktops",
                      &fDesktops,
                      sizeof(BOOL));
              if(!fDesktops) {
                NormalizeAll();
                xVirtual = yVirtual = 0;
              }
              break;

            case MSA_SHIFTBUMP:
              fShiftBump = (fShiftBump) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_SHIFTBUMP,
                             fShiftBump);
              SavePrf("ShiftBump",
                      &fShiftBump,
                      sizeof(BOOL));
              break;

            case MSA_BUMPDESKS:
              lBumpDesks = (lBumpDesks == -1) ? 0 :
                            (lBumpDesks == 1) ? -1 : 1;
              WinCheckButton(hwnd,
                             MSA_BUMPDESKS,
                             lBumpDesks);
              SavePrf("BumpDesks",
                      &lBumpDesks,
                      sizeof(BOOL));
              break;
          }
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case MSE_HELP:
          ViewHelp(hwnd,
                   "Virtual page");
          break;
        case MSE_VIRTUAL:
          PostMsg(hwndCover,
                  msg,
                  mp1,
                  mp2);
          break;

        case MSA_RESET:
          NormalizeAll();
          xVirtual = yVirtual = 0;
          LoadExcludes(&excludes,
                       &numexcludes,
                       "EXCLUDE.LST");
          DosBeep(1000,10);
          break;

        case MSA_EDITEXCLUDE:
          PostMsg((HWND)WinQueryWindowULong(hwnd,0),
                  msg,
                  mp1,
                  mp2);
          break;

        default:
          break;
      }
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY ClipSetProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_INITDLG:
      ClipSetHwnd = hwnd;
      PostMsg(hwnd,
              UM_REFRESH,
              MPVOID,
              MPVOID);
      break;

    case UM_REFRESH:
      WinCheckButton(hwnd,
                     MSA_CLIPSAVE,
                     fClipSave);
      WinCheckButton(hwnd,
                     MSA_USECLIPMGR,
                     fUseClipMgr);
      WinCheckButton(hwnd,
                     MSA_CLIPAPPEND,
                     fClipAppend);
      return 0;

    case WM_PAINT:
      return OuttieProc(hwnd,msg,mp1,mp2);

    case WM_CONTROL:
      switch(SHORT2FROMMP(mp1)) {
        case BN_CLICKED:
          switch(SHORT1FROMMP(mp1)) {
            case MSA_CLIPSAVE:
              fClipSave = (fClipSave) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_CLIPSAVE,
                             fClipSave);
              SavePrf("SaveClips",
                      &fClipSave,
                      sizeof(BOOL));
              break;

            case MSA_CLIPAPPEND:
              fClipAppend = (fClipAppend) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_CLIPAPPEND,
                             fClipAppend);
              SavePrf("ClipAppend",
                      &fClipAppend,
                      sizeof(BOOL));
              if(ClipMonHwnd)
                PostMsg(ClipMonHwnd,
                        UM_REFRESH,
                        MPVOID,
                        MPVOID);
              break;

            case MSA_USECLIPMGR:
              fUseClipMgr = (fUseClipMgr) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_USECLIPMGR,
                             fUseClipMgr);
              SavePrf("UseClipMgr",
                      &fUseClipMgr,
                      sizeof(BOOL));
              if(fUseClipMgr)
                StartClipMgr();
              else
                WinDestroyWindow(WinQueryWindow(ClipHwnd,QW_PARENT));
              break;
        }
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case MSE_HELP:
          ViewHelp(hwnd,
                   "Clipbrd page");
          break;
        case MSA_CLIPSFLDR:
          ShowFolder("CLIPS");
          break;
        case MSE_CLIP:
          PostMsg(hwndCover,
                  msg,
                  mp1,
                  mp2);
          break;
        default:
          break;
      }
      return 0;

    case WM_DESTROY:
      ClipSetHwnd = (HWND)0;
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY ScrnSetProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_INITDLG:
      PostMsg(hwnd,
              UM_REFRESH,
              MPVOID,
              MPVOID);
      break;

    case UM_REFRESH:
      WinCheckButton(hwnd,
                     MSA_SAVESCREEN,
                     fScreenSave);
      WinCheckButton(hwnd,
                     MSA_SAVEACTIVE,
                     fActiveSave);
      {
        char   s[13];
        USHORT bitarray[] = {1,4,8,24,0},x;

        for(x = 0;bitarray[x];x++) {
          sprintf(s,"%u",bitarray[x]);
          WinSendDlgItemMsg(hwnd,
                            MSA_SCRNBITCOUNT,
                            LM_INSERTITEM,
                            MPFROM2SHORT(LIT_END,0),
                            MPFROMP(s));
        }
        sprintf(s,
                "%u",
                scrnbitcount);
        WinSetDlgItemText(hwnd,
                          MSA_SCRNBITCOUNT,
                          s);
      }
      return 0;

    case WM_PAINT:
      return OuttieProc(hwnd,msg,mp1,mp2);

    case WM_SETFOCUS:
      if(!mp2) {
        PostMsg(hwnd,
                WM_CONTROL,
                MPFROM2SHORT(MSA_SCRNBITCOUNT,EN_KILLFOCUS),
                MPVOID);
      }
      break;

    case WM_CONTROL:
      switch(SHORT2FROMMP(mp1)) {
        case EN_KILLFOCUS:
          switch(SHORT1FROMMP(mp1)) {
            case MSA_SCRNBITCOUNT:
              {
                char s[13];

                WinQueryDlgItemText(hwnd,
                                    MSA_SCRNBITCOUNT,
                                    3,
                                    s);
                scrnbitcount = (USHORT)atoi(s);
                SavePrf("ScrnBits",
                        &scrnbitcount,
                        sizeof(USHORT));
              }
              break;
          }
          break;

        case BN_CLICKED:
          switch(SHORT1FROMMP(mp1)) {
            case MSA_SAVESCREEN:
              fScreenSave = (fScreenSave) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_SAVESCREEN,
                             fScreenSave);
              SavePrf("ScreenSave",
                      &fScreenSave,
                      sizeof(BOOL));
              break;

            case MSA_SAVEACTIVE:
              fActiveSave = (fActiveSave) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_SAVEACTIVE,
                             fActiveSave);
              SavePrf("ActiveSave",
                      &fActiveSave,
                      sizeof(BOOL));
              break;
          }
          break;
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case MSE_HELP:
          ViewHelp(hwnd,
                   "Screen page");
          break;

        case MSA_SCRNFLDR:
          ShowFolder("SCRNSHTS");
          break;

        case MS5_EDITAFTER:
          {
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
            WinStartApp((HWND)0,
                        &pgd,
                        "AFTRSCRN.CMD",
                        NULL,
                        0);
          }
          break;

        default:
          break;
      }
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY MonSetProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_INITDLG:
      PostMsg(hwnd,
              UM_REFRESH,
              MPVOID,
              MPVOID);
      break;

    case UM_REFRESH:
      WinCheckButton(hwnd,
                     MSA_SWAPMON,
                     fSwapMon);
      WinCheckButton(hwnd,
                     MSA_MEMORY,
                     fShowMem);
      WinCheckButton(hwnd,
                     MSA_PROCESS,
                     fShowTasks);
      WinCheckButton(hwnd,
                     MSA_PMEMORY,
                     fShowPMem);
      WinCheckButton(hwnd,
                     MSA_SHOWSWAPFREE,
                     fShowSwapFree);
      WinCheckButton(hwnd,
                     MSA_SWAPFLOAT,
                     fSwapFloat);
      WinCheckButton(hwnd,
                     MSA_CLOCK,
                     showclock);
      WinCheckButton(hwnd,
                     MSA_AMPM,
                     ampm);
      WinCheckButton(hwnd,
                     MSA_DATE,
                     showdate);
      WinCheckButton(hwnd,
                     MSA_HARDMON,
                     showhardmon);
      WinCheckButton(hwnd,
                     MSA_CPUMON,
                     fCPUMon);
      WinCheckButton(hwnd,
                     MSA_NOMONCLICK,
                     fNoMonClick);
      WinCheckButton(hwnd,
                     MSA_AVERAGE,
                     fAverage);
      WinCheckButton(hwnd,
                     MSA_SHOWFREEINMENUS,
                     fShowFreeInMenus);
      WinCheckButton(hwnd,
                     MSA_FRACTIONS,
                     fFractions);
      WinCheckButton(hwnd,
                     MSA_CLIPMON,
                     fClipMon);
      WinSendDlgItemMsg(hwnd,
                        MSA_TICKS,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(6,0),
                        MPVOID);
      {
        char s[33];

        sprintf(s,
                "%lu",
                MilliSecs);
        WinSetDlgItemText(hwnd,
                          MSA_TICKS,
                          s);
      }
      return 0;

    case WM_SETFOCUS:
      if(!mp2) {
        PostMsg(hwnd,
                WM_CONTROL,
                MPFROM2SHORT(MSA_TICKS,EN_KILLFOCUS),
                MPVOID);
      }
      break;

    case WM_PAINT:
      return OuttieProc(hwnd,msg,mp1,mp2);

    case WM_CONTROL:
      switch(SHORT2FROMMP(mp1)) {
        case EN_KILLFOCUS:
          switch(SHORT1FROMMP(mp1)) {
            case MSA_TICKS:
              {
                char s[33];

                *s = 0;
                WinQueryDlgItemText(hwnd,
                                    MSA_TICKS,
                                    sizeof(s),
                                    s);
                if(atol(s) > 100 &&
                   atol(s) < 1000000 &&
                   atol(s) != MilliSecs) {
                  if(MonTID) {
                    DosSuspendThread(IdleTID);
                    DosSuspendThread(MonTID);
                  }
                  MilliSecs = atol(s);
                  MaxCount = 1;
                  if(MonTID) {
                    DosResumeThread(MonTID);
                    DosResumeThread(IdleTID);
                  }
                  SavePrf("MilliSecs",
                          &MilliSecs,
                          sizeof(MilliSecs));
                }
                else {
                  sprintf(s,
                          "%lu",
                          MilliSecs);
                  WinSetDlgItemText(hwnd,
                                    MSA_TICKS,
                                    s);
                }
              }
              break;
          }
          break;

        case BN_CLICKED:
          switch(SHORT1FROMMP(mp1)) {
            case MSA_NOMONCLICK:
              fNoMonClick = (fNoMonClick) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_NOMONCLICK,
                             fNoMonClick);
              SavePrf("NoMonClick",
                      &fNoMonClick,
                      sizeof(BOOL));
              break;

            case MSA_FRACTIONS:
              fFractions = (fFractions) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_FRACTIONS,
                             fFractions);
              SavePrf("Fractions",
                      &fFractions,
                      sizeof(BOOL));
              break;

            case MSA_PROCESS:
              fShowTasks = (fShowTasks) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_PROCESS,
                             fShowTasks);
              SavePrf("ShowTasks",
                      &fShowTasks,
                      sizeof(BOOL));
              if(fShowTasks)
                StartSwapMon(TSK_FRAME);
              else
                WinDestroyWindow(TaskHwnd);
              break;

            case MSA_MEMORY:
              fShowMem = (fShowMem) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_MEMORY,
                             fShowMem);
              SavePrf("ShowMem",
                      &fShowMem,
                      sizeof(BOOL));
              if(fShowMem)
                StartSwapMon(MEM_FRAME);
              else
                WinDestroyWindow(MemHwnd);
              break;

            case MSA_PMEMORY:
              fShowPMem = (fShowPMem) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_PMEMORY,
                             fShowPMem);
              SavePrf("ShowPMem",
                      &fShowPMem,
                      sizeof(BOOL));
              if(MemHwnd)
                WinSendMsg(MemHwnd,
                           UM_TIMER,
                           MPVOID,
                           MPVOID);
              break;

            case MSA_SWAPFLOAT:
              fSwapFloat = (fSwapFloat) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_SWAPFLOAT,
                             fSwapFloat);
              SavePrf("SwapFloat",
                      &fSwapFloat,
                      sizeof(BOOL));
              break;

            case MSA_AVERAGE:
              fAverage = (fAverage) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_AVERAGE,
                             fAverage);
              SavePrf("Average",
                      &fAverage,
                      sizeof(BOOL));
              break;

            case MSA_SWAPMON:
              fSwapMon = (fSwapMon) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_SWAPMON,
                             fSwapMon);
              SavePrf("SwapMon",
                      &fSwapMon,
                      sizeof(BOOL));
              if(fSwapMon)
                StartSwapMon(SWAP_FRAME);
              else
                WinDestroyWindow(SwapHwnd);
              break;

            case MSA_SHOWSWAPFREE:
              fShowSwapFree = (fShowSwapFree) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_SHOWSWAPFREE,
                             fShowSwapFree);
              SavePrf("ShowSwapFree",
                      &fShowSwapFree,
                      sizeof(BOOL));
              if(SwapHwnd)
                WinSendMsg(SwapHwnd,
                           UM_TIMER,
                           MPVOID,
                           MPVOID);
              break;

            case MSA_CLIPMON:
              fClipMon = (fClipMon) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_CLIPMON,
                             fClipMon);
              SavePrf("ClipMon",
                      &fClipMon,
                      sizeof(BOOL));
              if(fClipMon)
                StartSwapMon(CLP_FRAME);
              else
                WinDestroyWindow(ClipMonHwnd);
              break;

            case MSA_AMPM:
              ampm = (ampm) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_AMPM,
                             ampm);
              SavePrf("ShowAmpm",
                      &ampm,
                      sizeof(BOOL));
              if(ClockHwnd)
                PostMsg(ClockHwnd,
                        UM_TIMER,
                        MPVOID,
                        MPVOID);
              break;

            case MSA_CLOCK:
              showclock = (showclock) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_CLOCK,
                             showclock);
              SavePrf("ShowClock",
                      &showclock,
                      sizeof(BOOL));
              if(showclock)
                StartSwapMon(CLOCK_FRAME);
              else
                WinDestroyWindow(ClockHwnd);
              break;

            case MSA_DATE:
              showdate = (showdate) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_DATE,
                             showdate);
              SavePrf("ShowDate",
                      &showdate,
                      sizeof(BOOL));
              if(ClockHwnd)
                WinSendMsg(ClockHwnd,
                           UM_TIMER,
                           MPVOID,
                           MPVOID);
              break;

            case MSA_HARDMON:
              showhardmon = (showhardmon) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_HARDMON,
                             showhardmon);
              SavePrf("ShowHard",
                      &showhardmon,
                      sizeof(BOOL));
              if(showhardmon)
                StartSwapMon(HARD_FRAME);
              else
                WinDestroyWindow(HardHwnd);
              break;

            case MSA_SHOWFREEINMENUS:
              fShowFreeInMenus = (fShowFreeInMenus) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_SHOWFREEINMENUS,
                             fShowFreeInMenus);
              SavePrf("ShowFreeInMenus",
                      &fShowFreeInMenus,
                      sizeof(BOOL));
              break;

            case MSA_CPUMON:
              fCPUMon = (fCPUMon) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_CPUMON,
                             fCPUMon);
              SavePrf("CPUMon",
                      &fCPUMon,
                      sizeof(BOOL));
              if(fCPUMon)
                StartSwapMon(CPU_FRAME);
              else
                WinDestroyWindow(CPUHwnd);
              break;
          }
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case MSE_HELP:
          ViewHelp(hwnd,
                   "Monitors page");
          break;

        default:
          break;
      }
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY TtlSetProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_INITDLG:
      PostMsg(hwnd,
              UM_REFRESH,
              MPVOID,
              MPVOID);
      break;

    case UM_REFRESH:
      WinCheckButton(hwnd,
                     MSA_ENHANCETITLEBARS,
                     fEnhanceTitlebars);
      WinCheckButton(hwnd,
                     MSA_TITLEBARMENUS,
                     fEnableTitlebarMenus);
      WinCheckButton(hwnd,
                     MSA_EXTRATITLEBARMENUS,
                     fExtraMenus);
      WinCheckButton(hwnd,
                     TTL_CLOCKINTITLEBARS,
                     fClockInTitlebars);
      return 0;

    case WM_PAINT:
      return OuttieProc(hwnd,msg,mp1,mp2);

    case WM_CONTROL:
      switch(SHORT2FROMMP(mp1)) {
        case BN_CLICKED:
          switch(SHORT1FROMMP(mp1)) {
            case MSA_ENHANCETITLEBARS:
              fEnhanceTitlebars = (fEnhanceTitlebars) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_ENHANCETITLEBARS,
                             fEnhanceTitlebars);
              SavePrf("EnhanceTitlebars",
                      &fEnhanceTitlebars,
                      sizeof(BOOL));
              RedrawTitlebars(FALSE,
                              HWND_DESKTOP);
              break;

            case TTL_CLOCKINTITLEBARS:
              fClockInTitlebars = (fClockInTitlebars) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             TTL_CLOCKINTITLEBARS,
                             fClockInTitlebars);
              SavePrf("ClockInTitlebars",
                      &fClockInTitlebars,
                      sizeof(BOOL));
              RedrawTitlebars(FALSE,
                              HWND_DESKTOP);
              break;

            case MSA_TITLEBARMENUS:
              fEnableTitlebarMenus = (fEnableTitlebarMenus) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_TITLEBARMENUS,
                             fEnableTitlebarMenus);
              SavePrf("EnableTitlebarMenus",
                      &fEnableTitlebarMenus,
                      sizeof(BOOL));
              break;

            case MSA_EXTRATITLEBARMENUS:
              fExtraMenus = (fExtraMenus) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_EXTRATITLEBARMENUS,
                             fExtraMenus);
              SavePrf("ExtraMenus",
                      &fExtraMenus,
                      sizeof(BOOL));
              break;
          }
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case MSE_HELP:
          ViewHelp(hwnd,
                   "Titlebars page");
          break;
        case MSA_TITLEBAREDIT:
          WinDlgBox(HWND_DESKTOP,
                    hwnd,
                    SetTitleDlgProc,
                    0,
                    TTL_FRAME,
                    0);
          break;

        default:
          break;
      }
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY ConfigProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_INITDLG:
      PostMsg(hwnd,
              UM_REFRESH,
              MPVOID,
              MPVOID);
      PostMsg(hwnd,
              UM_SETUP,
              MPVOID,
              MPVOID);
      break;

    case UM_REFRESH:
      return 0;

    case WM_PAINT:
      return OuttieProc(hwnd,msg,mp1,mp2);

    case UM_SETUP:
      PostMsg(ObjectHwnd4,
              UM_SETUP2,
              MPFROMLONG((ULONG)hwnd),
              MPVOID);
      return 0;

    case WM_CONTROL:
      if(SHORT1FROMMP(mp1) >= MSE_M1NONE &&
         SHORT1FROMMP(mp1) <= MSE_M3ALL) {

        SHORT sSelect;
        int   b,x;

        switch(SHORT2FROMMP(mp1)) {
          case CBN_ENTER:
          case CBN_LBSELECT:
            sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
                                               SHORT1FROMMP(mp1),
                                               LM_QUERYSELECTION,
                                               MPVOID,MPVOID);
            if(sSelect >= 0) {
              b = (SHORT1FROMMP(mp1) >= MSE_M1NONE &&
                   SHORT1FROMMP(mp1) <= MSE_M1ALL) ? 0 :
                   (SHORT1FROMMP(mp1) >= MSE_M2NONE &&
                    SHORT1FROMMP(mp1) <= MSE_M2ALL) ? 1 :
                     2;
              x = ((SHORT1FROMMP(mp1) - 101) % 8);
              cmds[b][x] = sSelect;
              SavePrf("Commands",
                      cmds,
                      sizeof(cmds));
            }
            break;
        }
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case MSE_CLEAR:
          if(WinMessageBox(HWND_DESKTOP,
                           hwnd,
                           "Are you sure you want to clear all MSE mouse "
                           "button definitions?",
                           "MSE:  WARNING!  This is a non-recoverable operation!",
                           0,
                           MB_YESNOCANCEL | MB_MOVEABLE | MB_ICONEXCLAMATION) ==
            MBID_YES) {

            int x,z;

            for(z = 0;z < 3;z++) {
              for(x = 0;x < 8;x++) {
                cmds[z][x] = C_NONE;
                WinSetDlgItemText(hwnd,
                                  101 + (x + (z * 8)),
                                  cmdnames[cmds[z][x]]);
              }
            }
          }
          break;

        case MSE_HELP:
          ViewHelp(hwnd,
                   "Buttons page");
          break;
      }
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY CoverProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_INITDLG:
      hwndCover = hwnd;
      PostMsg(hwnd,
              UM_REFRESH,
              MPVOID,
              MPVOID);
      break;

    case UM_REFRESH:
      WinCheckButton(hwnd,
                     MSE_LOWMEMORY,
                     fLowMemory);
      WinCheckButton(hwnd,
                     MSA_CONFIRMEXIT,
                     fConfirmExit);
      WinCheckButton(hwnd,
                     MSE_DISABLED,
                     fDisabled);
      WinCheckButton(hwnd,
                     MSE_NOTASKLIST,
                     fNoTasklist);
      WinSubclassWindow(WinWindowFromID(hwnd,CVR_BITMAP2),
                        (PFNWP)FitBitProc);

      break;

    case WM_PAINT:
      return OuttieProc(hwnd,msg,mp1,mp2);

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case MSE_HELP:
          ViewHelp(hwnd,
                   "Cover page");
          break;

        case MSE_CLIPS:
          ShowFolder("CLIPS");
          break;

        case MSE_SNAPSHOTS:
          ShowFolder("SCRNSHTS");
          break;

        case CVR_FONTPALETTE:
        case CVR_COLORPALETTE:
        case CVR_HICOLORPALETTE:
        case CVR_SYSTEMOBJECT:
        case CVR_MOUSEOBJECT:
        case CVR_SYSTEMSETUP:
          {
            char *p = NULL;

            switch(SHORT1FROMMP(mp1)) {
              case CVR_FONTPALETTE:
                p = "<WP_FNTPAL>";
                break;
              case CVR_COLORPALETTE:
                p = "<WP_LORESCLRPAL>";
                break;
              case CVR_HICOLORPALETTE:
                p = "<WP_HIRESCLRPAL>";
                break;
              case CVR_SYSTEMOBJECT:
                p = "<WP_SYSTEM>";
                break;
              case CVR_MOUSEOBJECT:
                p = "<WP_MOUSE>";
                break;
              case CVR_SYSTEMSETUP:
                p = "<WP_CONFIG>";
                break;
            }
            if(p)
              OpenObject(p,
                         defopen);
          }
          break;

        case MSE_VIRTUAL:
        case MSE_CALC:
        case MSE_FM2:
        case MSE_OPEN:
        case MSE_MINALL:
        case MSE_SAVESCREEN:
        case MSE_SWITCH:
        case MSE_CLIP:
        case MSE_CMDLINE:
        case MSE_DOSCMDLINE:
        case MSE_LAUNCHPAD:
          {
            ULONG  msg2;
            MPARAM mp12 = 0,mp22 = 0;
            SWP    swp;
            HWND   hwndP = hwndConfig;

            switch(SHORT1FROMMP(mp1)) {
              case MSE_CMDLINE:
                msg2 = UM_CMDLINE;
                break;
              case MSE_DOSCMDLINE:
                msg2 = UM_CMDLINE;
                mp12 = MPFROMLONG(1);
                break;
              case MSE_LAUNCHPAD:
                msg2 = UM_CMDLINE;
                mp12 = MPFROMLONG(2);
                break;
              case MSE_CLIP:
                msg2 = UM_CLIPMGR;
                hwndP = ObjectHwnd3;
                break;
              case MSE_SWITCH:
                msg2 = UM_SWITCHLIST;
                break;
              case MSE_VIRTUAL:
                msg2 = UM_VIRTUAL;
                break;
              case MSE_CALC:
                msg2 = UM_CALC;
                break;
              case MSE_FM2:
                msg2 = UM_STARTWIN;
                mp12 = MPFROM2SHORT(C_STARTFM2,0);
                break;
              case MSE_OPEN:
                msg2 = UM_OPEN;
                break;
              case MSE_MINALL:
                msg2 = UM_MINALL;
                hwndP = ObjectHwnd3;
                break;
              case MSE_SAVESCREEN:
                WinQueryWindowPos(hwndConfig,&swp);
                if(!(swp.fl & (SWP_MINIMIZE | SWP_HIDE)))
                  WinSendMsg(hwndConfig,
                             WM_SYSCOMMAND,
                             MPFROM2SHORT(SC_MINIMIZE,0),
                             MPVOID);
                msg2 = UM_SAVESCREEN;
                mp12 = MPFROMSHORT(TRUE);
                hwndP = ObjectHwnd3;
                break;
            }
            PostMsg(hwndP,
                    msg2,
                    mp12,
                    mp22);
          }
          break;

        case MSE_RELOAD:
          {
            int x;

            InitVars();
            ReadSchemes();
            for(x = 0;x < 7;x++) {
              if(hwndMenus[x])
                WinDestroyWindow(hwndMenus[x]);
              hwndMenus[x] = (HWND)0;
            }
            if(hwndPick)
              WinDestroyWindow(hwndPick);
            hwndPick = (HWND)0;
            LoadExcludes(&excludes,
                         &numexcludes,
                         "EXCLUDE.LST");
            LoadExcludes(&fexcludes,
                         &numfexcludes,
                         "FEXCLUDE.LST");
            DosBeep(1000,10);
          }
          break;

        case MSE_ABOUT:
          {
            static char s[516];
            MB2INFO     mb2;

            sprintf(s,
                    "MSE is free software from Mark Kimes.\r\r"
                    "MSE is a utility that enhances the "
                    "appearance and functionality of the OS/2 user "
                    "interface in several ways."
                    "\r\rCompiled:"
                    "  %s  %s\rVersion:  %lu.%02lu",
                    __DATE__,
                    __TIME__,
                    VERMAJOR,
                    VERMINOR);
            memset(&mb2,
                   0,
                   sizeof(MB2INFO));
            mb2.cb = sizeof(MB2INFO);
            mb2.cButtons = 1;
            mb2.hIcon = WinLoadPointer(HWND_DESKTOP,
                                       0,
                                       MSE_FRAME);
            mb2.flStyle = MB_MOVEABLE | MB_CUSTOMICON;
            strcpy(mb2.mb2d[0].achText,
                   "Okay");
            mb2.mb2d[0].idButton = DID_CANCEL;
            mb2.mb2d[0].flStyle = BS_PUSHBUTTON | BS_DEFAULT;
            WinMessageBox2(HWND_DESKTOP,
                           hwnd,
                           s,
                           "                         About MSE                         ",
                           10,
                           &mb2);
            WinDestroyPointer(mb2.hIcon);
          }
          break;
      }
      return 0;

    case WM_CONTROL:
      switch(SHORT2FROMMP(mp1)) {
        case BN_CLICKED:
          switch(SHORT1FROMMP(mp1)) {
            case MSE_LOWMEMORY:
              fLowMemory = (fLowMemory) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSE_LOWMEMORY,
                             fLowMemory);
              SavePrf("LowMemory",
                      &fLowMemory,
                      sizeof(BOOL));
              break;

            case MSA_CONFIRMEXIT:
              fConfirmExit = (fConfirmExit) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSA_CONFIRMEXIT,
                             fConfirmExit);
              SavePrf("ConfirmExit",
                      &fConfirmExit,
                      sizeof(BOOL));
              break;

            case MSE_NOTASKLIST:
              fNoTasklist = (fNoTasklist) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSE_NOTASKLIST,
                             fNoTasklist);
              SavePrf("NoTasklist",
                      &fNoTasklist,
                      sizeof(BOOL));
              PostMsg(hwndConfig,
                      UM_TASKLIST,
                      MPVOID,
                      MPVOID);
              if(fNoTasklist) {
                fDisabled = FALSE;
                WinCheckButton(hwnd,
                               MSE_DISABLED,
                               fDisabled);
                SavePrf("Disabled",
                        &fDisabled,
                        sizeof(BOOL));
              }
              break;

            case MSE_DISABLED:
              fDisabled = (fDisabled) ? FALSE : TRUE;
              WinCheckButton(hwnd,
                             MSE_DISABLED,
                             fDisabled);
              SavePrf("Disabled",
                      &fDisabled,sizeof(BOOL));
              WinSendMsg(hwndConfig,
                         UM_TIMER,
                         MPVOID,
                         MPVOID);
              fSuspend = FALSE;
              if(fDisabled) {
                fNoTasklist = FALSE;
                WinCheckButton(hwnd,
                               MSE_NOTASKLIST,
                               fNoTasklist);
                SavePrf("NoTasklist",
                        &fNoTasklist,
                        sizeof(BOOL));
                PostMsg(hwndConfig,
                        UM_TASKLIST,
                        MPVOID,
                        MPVOID);
              }
              break;
          }
          break;
      }
      return 0;

    case WM_DESTROY:
      hwndCover = (HWND)0;
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}

