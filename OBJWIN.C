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

static FRAMEPARMS *MyFrames;
static int         numFrames,numFramesAlloc;


static int AddFrame (HWND hwndFrame) {

  int         x,ret = -1;
  FRAMEPARMS *test;

  for(x = 0;x < numFrames;x++) {
    if(!MyFrames[x].hwndFrame) {
      memset(&MyFrames[x],
             0,
             sizeof(FRAMEPARMS));
      MyFrames[x].hwndFrame = hwndFrame;
      return x;
    }
    if(MyFrames[x].hwndFrame ==
       hwndFrame)
      return x;
  }
  if(numFrames == numFramesAlloc) {
    test = realloc(MyFrames,
                   sizeof(FRAMEPARMS) * (numFramesAlloc + 10));
    if(test) {
      numFramesAlloc += 10;
      MyFrames = test;
    }
  }
  if(numFramesAlloc > numFrames) {
    memset(&MyFrames[numFrames],
           0,
           sizeof(FRAMEPARMS));
    MyFrames[numFrames].hwndFrame = hwndFrame;
    ret = numFrames;
    numFrames++;
  }
  return ret;
}


static void RemoveFrame (HWND hwndFrame) {

  int         x;
  FRAMEPARMS *test;

  for(x = 0;x < numFrames;x++) {
    if(MyFrames[x].hwndFrame == hwndFrame) {
      if(x < numFrames - 1) {
        memmove(&MyFrames[x],
                &MyFrames[x + 1],
                sizeof(FRAMEPARMS) * ((numFrames - x) - 1));
        numFrames--;
        if(numFrames < numFramesAlloc - 20) {
          test = realloc(MyFrames,
                         sizeof(FRAMEPARMS) * numFrames);
          if(test) {
            MyFrames = test;
            numFramesAlloc = numFrames;
          }
        }
        else
          memset(&MyFrames[numFrames],
                 0,
                 sizeof(FRAMEPARMS));
      }
      return;
    }
  }
}


static int FindFrame (HWND hwndFrame) {

  int x,found = -1;

  for(x = 0;x < numFrames;x++) {
    if(MyFrames[x].hwndFrame == hwndFrame) {
      found = x;
      break;
    }
  }
  return found;
}


static BOOL TileBitmap (HWND hwnd,HBITMAP hbm,RECTL *cliprcl) {

  if(hwnd &&
     hbm) {

    HPS               hps;
    BITMAPINFOHEADER2 bmp2;
    POINTL            aptl[4];
    RECTL             rcl,rcl2,rclDest;

    hps = WinGetClipPS(hwnd,
                       (HWND)0,
                       0);
    if(hps) {
      if(cliprcl) {

        HRGN hrgn,hrgno;

        hrgn = GpiCreateRegion(hps,
                               1,
                               cliprcl);
        if(hrgn) {
          hrgno = (HRGN)0;
          GpiSetClipRegion(hps,
                           hrgn,
                           &hrgno);
          if(hrgno &&
             hrgno != HRGN_ERROR)
            GpiDestroyRegion(hps,
                             hrgno);
        }
      }
      memset(&bmp2,
             0,
             sizeof(bmp2));
      bmp2.cbFix = sizeof(bmp2);
      if(GpiQueryBitmapInfoHeader(hbm,
                                  &bmp2)) {
        WinQueryWindowRect(hwnd,
                           &rcl);
        aptl[1].x = 0;
        do {
          aptl[0].x  = aptl[1].x;
          aptl[0].y  = 0;
          aptl[1].x += (bmp2.cx - 1);
          if(aptl[1].x > rcl.xRight - 1)
            aptl[1].x  = rcl.xRight - 1;
          aptl[1].y  = rcl.yTop - 1;
          rcl2.xLeft   = aptl[0].x;
          rcl2.yBottom = aptl[0].y;
          rcl2.xRight  = aptl[1].x;
          rcl2.yTop    = aptl[1].y;
          if(!cliprcl ||
             WinIntersectRect(WinQueryAnchorBlock(hwnd),
                              &rclDest,
                              cliprcl,
                              &rcl2)) {
            aptl[2].x  = 0;
            aptl[2].y  = 0;
            aptl[3].x  = min(aptl[1].x - aptl[0].x,bmp2.cx);
            if(aptl[3].x > rcl.xRight)
              aptl[3].x = rcl.xRight;
            aptl[3].y  = bmp2.cy;
            if(aptl[3].y > rcl.yTop)
              aptl[3].y = rcl.yTop;
            GpiWCBitBlt(hps,
                        hbm,
                        4L,
                        aptl,
                        ROP_SRCCOPY,
                        BBO_IGNORE);
          }
        } while(aptl[1].x < rcl.xRight - 1);
        WinReleasePS(hps);
        return TRUE;
      }
      else {  /* reload bitmap */

        TBARPARMS tbp,*tbpp;

        if(WinQueryPresParam(hwnd,
                             TITLEBAR_PARM,
                             0,
                             NULL,
                             sizeof(tbp),
                             (PVOID)&tbp,
                             QPF_NOINHERIT) &&
           tbp.size == sizeof(TBARPARMS)) {
          if(tbp.hbm)
            GpiDeleteBitmap(tbp.hbm);
          tbp.hbm = (HBITMAP)0;
          if(tbp.bitmapname &&
             *tbp.bitmapname) {
            tbp.hbm = LoadBitmap(tbp.bitmapname);
            if(tbp.hbm)
              hbm = TrimBitmap((HAB)0,
                               tbp.hbm);
            if(hbm)
              tbp.hbm = hbm;
          }
          tbpp = malloc(sizeof(TBARPARMS));
          if(tbpp) {
            *tbpp = tbp;
            if(!PostMsg(ObjectHwnd2,
                        UM_SETPRESPARMS,
                        MPFROMLONG(hwnd),
                        MPFROMP(tbpp)))
              free(tbpp);
          }
        }
      }
      WinReleasePS(hps);
    }
  }
  return FALSE;
}


/*------------------------------------------------------------------------*/
/* function : ScaleMetafile.                                              */
/*                                                                        */
/* Description: Plays the specified MetaFile to the PS using the default  */
/* view matrix to scale and translate the output to fit a specified       */
/* rectangle in the PS page, while preserving the aspect ratio of the     */
/* of the picture.                                                        */
/*                                                                        */
/* inputs : hps          PS handle                                        */
/*          hmf          MetaFile handle                                  */
/*          prclTarget   Target rectangle in page coordinates.            */
/*                                                                        */
/* returns: TRUE or FALSE.                                                */
/*                                                                        */
/*------------------------------------------------------------------------*/
static BOOL ScaleMetafile (HPS hps, HMF hmf, PRECTL prclTarget) {

  BOOL     fret;
  LONG     lSegCount;
  UCHAR    abDesc[256];
  LONG     alOpns[PMF_DEFAULTS + 1];
  POINTL   ptlPosn;
  RECTL    rclBoundary;
  MATRIXLF matlfXform;
  FIXED    afxScale[2];

  /* Set display control off    */
  fret = GpiSetDrawControl(hps,
                           DCTL_DISPLAY,
                           DCTL_OFF);

  if(fret)
    /* Set bounds accumulation on */
    fret = GpiSetDrawControl(hps,
                             DCTL_BOUNDARY,
                             DCTL_ON);

  if(fret)
    fret = GpiResetBoundaryData(hps);

  alOpns[PMF_SEGBASE]         = 0L;
  alOpns[PMF_LOADTYPE]        = LT_NOMODIFY;
  alOpns[PMF_RESOLVE]         = 0L;
  alOpns[PMF_LCIDS]           = LC_LOADDISC;
  alOpns[PMF_RESET]           = RES_NORESET;
  alOpns[PMF_SUPPRESS]        = SUP_NOSUPPRESS;
  alOpns[PMF_COLORTABLES]     = CTAB_REPLACE;
  alOpns[PMF_COLORREALIZABLE] = CREA_NOREALIZE;
  alOpns[PMF_DEFAULTS]        = DDEF_LOADDISC;
  fret = (BOOL)GpiPlayMetaFile(hps,
                               hmf,
                               PMF_DEFAULTS + 1,
                               alOpns,
                               &lSegCount,
                               256L,
                               abDesc);

  fret = GpiQueryBoundaryData(hps,&rclBoundary);

  /*
   * Determine scale parameters to scale from boundary dimensions to targe
   * dimensions about bottom left of boundary. Ensure that both scale
   * factors are equal and set to the smaller of the two possible values
   * to preserve the aspect ratio of the picture.
   */

  afxScale[0] = (prclTarget->xRight - prclTarget->xLeft) * 0x10000 /
                  (rclBoundary.xRight - rclBoundary.xLeft);

  afxScale[1] = (prclTarget->yTop - prclTarget->yBottom) * 0x10000 /
                  (rclBoundary.yTop - rclBoundary.yBottom);

  if(afxScale[0] < afxScale[1])
    afxScale[1] = afxScale[0];
  else
    afxScale[0] = afxScale[1];

  ptlPosn.x = rclBoundary.xLeft;
  ptlPosn.y = rclBoundary.yBottom;

  fret = GpiScale(hps,
                  &matlfXform,
                  TRANSFORM_REPLACE,
                  afxScale,
                  &ptlPosn);

  ptlPosn.x = prclTarget->xLeft - rclBoundary.xLeft;
  ptlPosn.y = prclTarget->yBottom - rclBoundary.yBottom;

  fret = GpiTranslate(hps,
                      &matlfXform,
                      TRANSFORM_ADD,
                      &ptlPosn);

  fret = GpiSetDefaultViewMatrix(hps,
                                 9L,
                                 &matlfXform,
                                 TRANSFORM_REPLACE);

  fret = GpiSetDrawControl(hps,
                           DCTL_BOUNDARY,
                           DCTL_OFF);

  fret = GpiSetDrawControl(hps,
                           DCTL_DISPLAY,
                           DCTL_ON);

  DosSleep(0L);
  fret = (BOOL)GpiPlayMetaFile(hps,
                               hmf,
                               PMF_DEFAULTS + 1,
                               alOpns,
                               &lSegCount,
                               256L,
                               abDesc);

  afxScale[0] = 0x10000;
  afxScale[1] = 0x10000;

  fret = GpiScale(hps,
                  &matlfXform,
                  TRANSFORM_REPLACE,
                  afxScale,
                  &ptlPosn);

  return(fret);
}


MRESULT EXPENTRY GObjectProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_CREATE:
      GObjectHwnd = hwnd;
      break;

    case WM_DRAWCLIPBOARD:
      {
        HPS          hps;
        HMF          hmf;
        RECTL        rectl;
        ULONG        cmd;
        HAB          hab;
        HWND         hwndT;

        switch(SHORT1FROMMP(mp1)) {
          case 104:
            cmd = CF_METAFILE;
            break;
          case 105:
            cmd = CF_DSPMETAFILE;
            break;
        }
        hwndT = WinWindowFromID(ClipHwnd,SHORT1FROMMP(mp1));
        if(hwndT) {
          hps = WinQueryWindowULong(hwndT,4);
          WinQueryWindowRect(hwndT,&rectl);
          if(hps) {
            hab = WinQueryAnchorBlock(hwnd);
            if(WinOpenClipbrd(hab)) {
              hmf = (HMF)WinQueryClipbrdData(hab,cmd);
              if(hmf)
                ScaleMetafile(hps,
                              hmf,
                              &rectl);
              WinCloseClipbrd(hab);
            }
          }
        }
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_SAVEMET2:
        case IDM_SAVEMET:
          if(mp2) {

            char *filename = (char *)mp2;
            HMF   hmf;
            ULONG id = (SHORT1FROMMP(mp1) == IDM_SAVEMET) ? CF_METAFILE :
                                                            CF_DSPMETAFILE;

            if(WinOpenClipbrd(WinQueryAnchorBlock(hwnd))) {
              hmf = (HMF)WinQueryClipbrdData(WinQueryAnchorBlock(hwnd),id);
              if(hmf)
                GpiSaveMetaFile(hmf,
                                filename);
              free(filename);
              WinCloseClipbrd(WinQueryAnchorBlock(hwnd));
              _heapmin();
            }
          }
          break;

        case IDM_LOADMET:
          if(mp2) {

            HMF   hmf;
            char *filename = (char *)mp2;

            hmf = GpiLoadMetaFile(WinQueryAnchorBlock(hwnd),
                                  filename);
            free(filename);
            if(hmf) {
              if(WinOpenClipbrd(WinQueryAnchorBlock(hwnd))) {
                if(!WinSetClipbrdData(WinQueryAnchorBlock(hwnd),
                                      (ULONG)hmf,
                                      CF_METAFILE,CFI_HANDLE))
                  GpiDeleteMetaFile(hmf);
                WinCloseClipbrd(WinQueryAnchorBlock(hwnd));
              }
              else
                GpiDeleteMetaFile(hmf);
            }
            else
              WarbleBeep();
            _heapmin();
          }
          break;

        case IDM_LOADBIT:
          if(mp2) {

            HBITMAP hbm;
            char   *filename = (char *)mp2;

            hbm = LoadBitmap(filename);
            free(filename);
            if(hbm) {
              if(WinOpenClipbrd(WinQueryAnchorBlock(hwnd))) {
                if(!WinSetClipbrdData(WinQueryAnchorBlock(hwnd),
                                      (ULONG)hbm,
                                      CF_BITMAP,CFI_HANDLE))
                  GpiDeleteBitmap(hbm);
                WinCloseClipbrd(WinQueryAnchorBlock(hwnd));
              }
              else
                GpiDeleteBitmap(hbm);
            }
            else
              WarbleBeep();
            _heapmin();
          }
          break;

        case IDM_SAVEBIT2:
        case IDM_SAVEBIT:
          if(mp2) {

            char   *filename = (char *)mp2;
            HBITMAP hbm;
            ULONG   id = (SHORT1FROMMP(mp1) == IDM_SAVEBIT) ? CF_BITMAP :
                                                              CF_DSPBITMAP;

            if(WinOpenClipbrd(WinQueryAnchorBlock(hwnd))) {
              hbm = (HBITMAP)WinQueryClipbrdData(WinQueryAnchorBlock(hwnd),id);
              if(hbm)
                SaveBitmap(WinQueryAnchorBlock(hwnd),
                           hbm,
                           filename,
                           0);
              free(filename);
              WinCloseClipbrd(WinQueryAnchorBlock(hwnd));
              _heapmin();
            }
          }
          break;

        case IDM_APPENDCLIP:
        case IDM_APPENDCLIP2:
        case IDM_SAVECLIP2:
        case IDM_SAVECLIP:
          if(mp2) {

            FILE *fp;
            char *text,*filename = (char *)mp2,*opentype;
            ULONG id;

            switch(SHORT1FROMMP(mp1)) {
              case IDM_APPENDCLIP:
                id = CF_TEXT;
                opentype = "ab+";
                break;
              case IDM_APPENDCLIP2:
                id = CF_DSPTEXT;
                opentype = "ab+";
                break;
              case IDM_SAVECLIP2:
                id = CF_DSPTEXT;
                opentype = "wb";
                break;
              case IDM_SAVECLIP:
                id = CF_TEXT;
                opentype = "wb";
                break;
            }
            fp = fopen(filename,
                       opentype);
            free(filename);
            if(fp) {
              fseek(fp,
                    0,
                    SEEK_END);
              switch(SHORT1FROMMP(mp1)) {
                case IDM_APPENDCLIP:
                case IDM_APPENDCLIP2:
                  fwrite("\r\n",
                         1,
                         2,
                         fp);
                  break;
              }
              if(WinOpenClipbrd(WinQueryAnchorBlock(hwnd))) {
                text = (char *)WinQueryClipbrdData(WinQueryAnchorBlock(hwnd),id);
                if(text &&
                   *text)
                  fwrite(text,
                         1,
                         strlen(text),
                         fp);
                WinCloseClipbrd(WinQueryAnchorBlock(hwnd));
              }
              fclose(fp);
            }
            _heapmin();
          }
          break;

        case IDM_LOADCLIP:
          if(mp2) {

            FILE *fp;
            char *text = NULL,*filename = (char *)mp2;
            ULONG len;

            fp = fopen(filename,
                       "rb");
            free(filename);
            if(fp) {
              fseek(fp,
                    0,
                    SEEK_END);
              len = ftell(fp);
              fseek(fp,
                    0,
                    SEEK_SET);
              if(len) {
                if(!DosAllocSharedMem((PPVOID)&text,
                                      (PSZ)NULL,
                                      len + 1,
                                      PAG_COMMIT | OBJ_GIVEABLE |
                                      PAG_READ | PAG_WRITE)) {
                  if(fread(text,
                           1,
                           len,
                           fp) ==
                     len) {
                    if(WinOpenClipbrd(WinQueryAnchorBlock(hwnd))) {
                      TakeClipboard();
                      if(!WinSetClipbrdData(WinQueryAnchorBlock(hwnd),
                                            (ULONG)text,
                                            CF_TEXT,
                                            CFI_POINTER))
                        DosFreeMem(text);
                      WinCloseClipbrd(WinQueryAnchorBlock(hwnd));
                    }
                    else
                      DosFreeMem(text);
                  }
                }
              }
              fclose(fp);
            }
            _heapmin();
          }
          break;
      }
      return 0;

    case WM_DESTROY:
      GObjectHwnd = (HWND)0;
      break;
  }
  return WinDefWindowProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY ObjectProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  static HWND   hwndScroll = (HWND)0;
  static USHORT sID = 0;
  static ULONG  sMsg;
  static POINTL ptlOld;

  switch(msg) {
    case WM_CREATE:
      ObjectHwnd = hwnd;
      DosSetPriority(PRTYS_THREAD,
                     PRTYC_NOCHANGE,
                     2L,
                     0L);
      break;

    case UM_BITMAP:
      if(mp1) {
        if(!mp2)
          GpiDeleteBitmap((HBITMAP)mp1);
      }
      return 0;

    case UM_TILEBITMAP:
      if(mp1 &&
         mp2) {

        HBITMAP hbm   = (HBITMAP)mp2;
        HWND    hwndD = (HWND)mp1;
        RECTL   rcl   = ClipRectl;

        return (MRESULT)TileBitmap(hwndD,hbm,&rcl);
      }
      return 0;

    case UM_ALLOC:
      {
        QMSG *pqMsg = NULL,*sMsg = mp1;

        if(sMsg) {
          pqMsg = malloc(sizeof(QMSG));
          if(pqMsg)
            *pqMsg = *sMsg;
        }
        return MRFROMLONG((long)pqMsg);
      }

    case UM_FREE:
      {
        QMSG *pqMsg = mp1;

        if(pqMsg) {
          free(pqMsg);
          _heapmin();
        }
      }
      return 0;

    case UM_HOOK:
      if(mp2) {

        QMSG *pqMsg = mp2;

        switch(SHORT1FROMMP(mp1)) {
          case C_CHORD:
            PostMsg(pqMsg->hwnd,
                    WM_CHORD,
                    pqMsg->mp1,
                    MPFROM2SHORT(SHORT1FROMMP(pqMsg->mp2),
                                 KC_NONE));
            break;

          case C_B1DBLCLK:
            {
              HWND hwndFrame = WinQueryWindow(pqMsg->hwnd,QW_PARENT);

              if(pqMsg->hwnd == WinWindowFromID(hwndFrame,FID_TITLEBAR))
                PostMsg(pqMsg->hwnd,
                        WM_BUTTON1DBLCLK,
                        pqMsg->mp1,
                        MPFROM2SHORT(SHORT1FROMMP(pqMsg->mp2),
                                     KC_NONE));
              else {
                PostMsg(pqMsg->hwnd,
                        WM_BUTTON1DOWN,
                        pqMsg->mp1,
                        MPFROM2SHORT(SHORT1FROMMP(pqMsg->mp2),
                                     KC_NONE));
                DosSleep(33);
                PostMsg(pqMsg->hwnd,
                        WM_BUTTON1UP,
                        pqMsg->mp1,
                        MPFROM2SHORT(SHORT1FROMMP(pqMsg->mp2),
                                     KC_NONE));
                DosSleep(1);
                PostMsg(pqMsg->hwnd,
                        WM_BUTTON1CLICK,
                        pqMsg->mp1,
                        MPFROM2SHORT(SHORT1FROMMP(pqMsg->mp2),
                                     KC_NONE));
                PostMsg(pqMsg->hwnd,
                        WM_SINGLESELECT,
                        pqMsg->mp1,
                        MPFROM2SHORT(TRUE,0));
                DosSleep(WinQuerySysValue(HWND_DESKTOP,SV_DBLCLKTIME) / 4);
                PostMsg(pqMsg->hwnd,
                        WM_BUTTON1DBLCLK,
                        pqMsg->mp1,
                        MPFROM2SHORT(SHORT1FROMMP(pqMsg->mp2),
                                     KC_NONE));
                PostMsg(pqMsg->hwnd,
                        WM_OPEN,
                        pqMsg->mp1,
                        MPFROM2SHORT(TRUE,0));
                DosSleep(33);
                PostMsg(pqMsg->hwnd,
                        WM_BUTTON1UP,
                        pqMsg->mp1,
                        MPFROM2SHORT(TRUE,0));
              }
            }
            break;
        }
        free(pqMsg);
        _heapmin();
      }
      return 0;

    case WM_MOUSEMOVE:
      /* turn all mouse movements into scrollbar movements */
      {
        SHORT  y = SHORT2FROMMP(mp1),x = SHORT1FROMMP(mp1);
        BOOL   shift;
        int    cntr = 1,z;
        RECTL  rcl;
        POINTL ptl;

        if(!WinIsWindow(WinQueryAnchorBlock(hwnd),hwndScroll))
          PostMsg(hwnd,
                  WM_BUTTON1CLICK,
                  mp1,
                  MPVOID);
        shift = ((SHORT2FROMMP(mp2) & VK_SHIFT) != 0);
        WinQueryWindowRect(hwndScroll,
                           &rcl);
        ptl.x = (rcl.xRight - rcl.xLeft) / 2;
        ptl.y = (rcl.yTop - rcl.yBottom) / 2;
        WinMapWindowPoints(hwndScroll,
                           HWND_DESKTOP,
                           &ptl,
                           1L);
        switch(sMsg) {
          case WM_VSCROLL:
            if(y == ptl.y)
              return 0;
            if(!shift)
              cntr = abs(y - ptl.y) / 2;
            break;
          case WM_HSCROLL:
            if(x == ptl.x)
              return 0;
            if(!shift)
              cntr = abs(x - ptl.x) / 2;
            break;
        }
        cntr = min(max(cntr,1),lMaxRunahead);
        for(z = 0;z < cntr;z++) {
          switch(sMsg) {
            case WM_VSCROLL:
              if(y > ptl.y)
                WinPostMsg(WinQueryWindow(hwndScroll,QW_PARENT),
                           sMsg,
                           MPFROMSHORT(sID),
                           MPFROM2SHORT(0,
                                        (shift) ? SB_PAGEDOWN :
                                                  SB_LINEDOWN));
              else if(y < ptl.y)
                WinPostMsg(WinQueryWindow(hwndScroll,QW_PARENT),
                           sMsg,
                           MPFROMSHORT(sID),
                           MPFROM2SHORT(0,
                                        (shift) ? SB_PAGEUP :
                                                  SB_LINEUP));
              break;
            case WM_HSCROLL:
              if(x > ptl.x)
                WinPostMsg(WinQueryWindow(hwndScroll,QW_PARENT),
                           sMsg,
                           MPFROMSHORT(sID),
                           MPFROM2SHORT(0,
                                        (shift) ? SB_PAGELEFT :
                                                  SB_LINELEFT));
              else if(x < ptl.x)
                WinPostMsg(WinQueryWindow(hwndScroll,QW_PARENT),
                           sMsg,
                           MPFROMSHORT(sID),
                           MPFROM2SHORT(0,
                                        (shift) ? SB_PAGERIGHT :
                                                  SB_LINERIGHT));
              break;
          }
        }
        if(!WinQuerySysValue(HWND_DESKTOP,
                             SV_POINTERLEVEL))
          WinShowPointer(HWND_DESKTOP,
                         FALSE);
        WinSetPointerPos(HWND_DESKTOP,
                         ptl.x,
                         ptl.y);
      }
      return 0;

    case UM_ADDFRAME:
      if(mp1) {
        if(WinIsWindow(WinQueryAnchorBlock(hwnd),
                       (HWND)mp1))
          AddFrame((HWND)mp1);
      }
      return 0;

    case UM_REMOVEFRAME:
      if(mp1)
        RemoveFrame((HWND)mp1);
      return 0;

    case WM_BUTTON1CLICK:
    case WM_BUTTON2CLICK:
    case WM_BUTTON3CLICK:
      hwndScroll = (HWND)0;
      WinSetPointerPos(HWND_DESKTOP,
                       ptlOld.x,
                       ptlOld.y);
      WinShowPointer(HWND_DESKTOP,
                     TRUE);     /* show pointer */
      WinSetCapture(HWND_DESKTOP,
                    (HWND)0);   /* release mouse */
      return 0;

    case UM_HOOK2:
      switch(SHORT1FROMMP(mp1)) {
        case C_MOVEA:
        case C_SIZEA:
        case C_MAXIMIZEA:
        case C_MINIMIZEA:
        case C_MOVE:
        case C_SIZE:
        case C_MAXIMIZE:
        case C_MINIMIZE:
          {
            HWND  hwndFrame = (HWND)mp2;
            SHORT sc = SHORT2FROMMP(mp1);

            PostMsg(hwndFrame,
                    WM_SYSCOMMAND,
                    MPFROM2SHORT(sc,0),
                    MPVOID);
          }
          break;

        case C_VERTSCROLL:
          /* capture mouse if vertical scrollbar present */
          hwndScroll = (HWND)0;
          if(mp2) {

            char   ucClassname[9];
            RECTL  rcl;
            POINTL ptl;

            *ucClassname = 0;
            WinQueryClassName((HWND)mp2,
                              sizeof(ucClassname),
                              ucClassname);
            if(!strcmp(ucClassname,"#8")) {
              hwndScroll = (HWND)mp2;
              sID = WinQueryWindowUShort(hwndScroll,
                                         QWS_ID);
              sMsg = ((WinQueryWindowULong(hwndScroll,
                                           QWL_STYLE) & SBS_VERT) != 0) ?
                      WM_VSCROLL :
                      WM_HSCROLL;
              WinQueryWindowRect(hwndScroll,
                                 &rcl);
              ptl.x = (rcl.xRight - rcl.xLeft) / 2;
              ptl.y = (rcl.yTop - rcl.yBottom) / 2;
              WinMapWindowPoints(hwndScroll,
                                 HWND_DESKTOP,
                                 &ptl,
                                 1L);
              WinSetPointerPos(HWND_DESKTOP,
                               ptl.x,
                               ptl.y);
              WinShowPointer(HWND_DESKTOP,
                             FALSE);  /* hide pointer */
              WinQueryPointerPos(HWND_DESKTOP,
                                 &ptlOld);
              WinSetCapture(HWND_DESKTOP,
                            hwnd);    /* capture mouse */
            }
          }
          if(!hwndScroll)
            DosBeep(250,50);
          return 0;

        case C_NEXT:
          {
            HWND hwndFrame = (HWND)mp2;

            PostMsg(hwndFrame,
                    WM_SYSCOMMAND,
                    MPFROM2SHORT(SC_NEXT,0),
                    MPVOID);
          }
          break;

        case C_BACK:
        case C_BACKA:
          {
            HWND hwndFrame = (HWND)mp2;

            WinSetWindowPos(hwndFrame,
                            HWND_BOTTOM,
                            0,
                            0,
                            0,
                            0,
                            SWP_ZORDER | SWP_DEACTIVATE);
          }
          break;

        case C_CLOSE:
        case C_CLOSEA:
          {
            HWND hwndFrame = (HWND)mp2;

            PostMsg(hwndFrame,
                    WM_SYSCOMMAND,
                    MPFROM2SHORT(SC_CLOSE,0),
                    MPVOID);
          }
          break;

        case C_SHOW:
        case C_SHOWA:
          {
            SWP  swp;
            HWND hwndFrame = (HWND)mp2;

            WinQueryWindowPos(hwndFrame,&swp);
            if(swp.x + swp.cx > xScreen)
              swp.x = xScreen - swp.cx;
            if(swp.y + swp.cy > yScreen)
              swp.y = yScreen - swp.cy;
            if(swp.x < 0)
              swp.x = 0;
            if(swp.y < 0)
              swp.y = 0;
            if(swp.x + swp.cx > xScreen)
              swp.cx = xScreen - swp.x;
            if(swp.y + swp.cy > yScreen)
              swp.cy = yScreen - swp.y;
            WinSetWindowPos(hwndFrame,
                            HWND_TOP,
                            swp.x,
                            swp.y,
                            swp.cx,
                            swp.cy,
                            SWP_MOVE | SWP_SIZE);
          }
          break;

        case C_ROLLUP:
        case C_ROLLUPA:
          {
            SWP     swp,swpM,swpO;
            ULONG   fl;
            long    newsize;
            int     c;
            HWND    hwndFrame = (HWND)mp2;
            HSWITCH hswitch;
            SWCNTRL swcntrl;

            WinQueryWindowPos(hwndFrame,
                              &swpO);
            newsize = yTitleBar + (ySizeBorder * 2);
            c = FindFrame(hwndFrame);
            if(c == -1)
              c = AddFrame(hwndFrame);
            if(c != -1) {
              swp.x  = MyFrames[c].x;
              swp.y  = MyFrames[c].y;
              swp.cx = MyFrames[c].cx;
              swp.cy = MyFrames[c].cy;
            }
            if(swpO.cy <= newsize) {
              if((swpO.fl & SWP_MAXIMIZE) == 0) {
                if(c == -1 ||
                   swp.cx == 0 ||
                   swp.cy == 0)
                  LoadRestorePos(&swp,
                                 hwndFrame);
                if(swp.cy <= newsize) {

                  ULONG ulSession = 0;

                  swp.cy = yScreen / 2;
                  hswitch = WinQuerySwitchHandle(hwndFrame,0);
                  if(hswitch) {
                    if(!WinQuerySwitchEntry(hswitch,
                                            &swcntrl)) {
                      if(swcntrl.bProgType == PROG_WINDOWABLEVIO ||
                         swcntrl.bProgType == PROG_WINDOWEDVDM)
                        swp.cy = yScreen - 8;
                      else {
                        ulSession = swcntrl.idSession;
                        if(!WinQueryTaskSizePos(WinQueryAnchorBlock(hwnd),
                                                ulSession,
                                                &swpM))
                          swp.cy = swpM.cy;
                      }
                    }
                  }
                }
              }
              else {
                if(c == -1 ||
                   swp.cx == 0 ||
                   swp.cy == 0) {
                  WinSendMsg(hwndFrame,
                             WM_SYSCOMMAND,
                             MPFROM2SHORT(SC_RESTORE,0),
                             MPVOID);
                  WinSendMsg(hwndFrame,
                             WM_SYSCOMMAND,
                             MPFROM2SHORT(SC_MAXIMIZE,0),
                             MPVOID);
                  WinQueryWindowPos(hwndFrame,
                                    &swp);
                  if(swpO.y + swp.cy > yScreen)
                    swpO.y = yScreen - swp.cy;
                  if(swpO.y < 0)
                    swpO.y = 0;
                  if(swpO.x + swp.cx > xScreen)
                    swpO.x = xScreen - swp.cx;
                  if(swpO.x < 0)
                    swpO.x = 0;
                  if(swpO.x > 0 &&
                     swpO.y > 0 &&
                     (swpO.x != swp.x ||
                      swpO.y != swp.y)) {
                    WinSetWindowPos(hwndFrame,
                                    HWND_TOP,
                                    swpO.x,
                                    swpO.y,
                                    0,
                                    0,
                                    SWP_MOVE);
                  }
                  return 0;
                }
              }
              swpM.y = swpO.y;
              swpO.y -= (swp.cy - swpO.cy);
              if(swpO.y + swp.cy > yScreen)
                swpO.y = yScreen - swp.cy;
              if(swpO.y < 0)
                swpO.y = 0;
              fl = SWP_SIZE | SWP_MOVE;
              if(hwndFrame == WinQueryActiveWindow(HWND_DESKTOP))
                fl |= SWP_ACTIVATE | SWP_ZORDER;
              WinSetWindowPos(hwndFrame,
                              HWND_TOP,
                              swpO.x,
                              swpO.y,
                              swpO.cx,
                              swp.cy,
                              fl);
              WinQueryWindowPos(hwndFrame,&swp);
              swpO.y = swpM.y;
              swpO.y -= (swp.cy - swpO.cy);
              if(swpO.y + swp.cy > yScreen)
                swpO.y = yScreen - swp.cy;
              if(swpO.y < 0)
                swpO.y = 0;
              fl &= (~SWP_SIZE);
              if(swpO.y != swp.y) {
                WinSetWindowPos(hwndFrame,
                                HWND_TOP,
                                swp.x,
                                swpO.y,
                                0,
                                0,
                                fl);
                WinInvalidateRect(hwndFrame,
                                  NULL,
                                  TRUE);
              }
            }
            else {
              WinSetWindowPos(hwndFrame,
                              HWND_TOP,
                              swpO.x,
                              swpO.y + (swpO.cy - newsize),
                              swpO.cx,
                              newsize,
                              SWP_SIZE | SWP_MOVE);
              if(c != -1) {
                MyFrames[c].x  = swpO.x;
                MyFrames[c].y  = swpO.y;
                MyFrames[c].cx = swpO.cx;
                MyFrames[c].cy = swpO.cy;
              }
              else {
                if((swpO.fl & SWP_MAXIMIZE) == 0)
                  SaveRestorePos(&swpO,
                                 hwndFrame);
              }
            }
          }
          break;
      }
      return 0;

    case UM_SHOWME:
      if(mp1) {

        HWND hwndT = (HWND)mp1;

        ShowWindow(WinQueryAnchorBlock(hwnd),
                   hwndT);
      }
      return 0;

    case UM_POSMOUSE:
      {
        SWP    swp;
        POINTL ptl;
        ULONG  cntr = 0;
        HWND   hwndT = (HWND)mp1,hwndO = (HWND)0;
        HAB    habO = WinQueryAnchorBlock(hwnd);

        if(WinIsChild(hwndT,hwndConfig))
          return 0;
        while(WinIsWindow(habO,
                          hwndT) &&
              !WinIsWindowVisible(hwndT) &&
              cntr++ < 181)
          DosSleep((cntr % 20) == 0);
        if(!WinIsWindow(habO,
                        hwndT) ||
           !WinIsWindowShowing(hwndT))
          return 0;
        if(fDefButton)
          hwndO = WinQueryWindowULong(hwndT,
                                      QWL_DEFBUTTON);
        if(!hwndO) {
          if(fCenterDlg)
            hwndO = hwndT;
          else
            return 0;
        }
        if(hwndO &&
           WinQueryWindowPos(hwndO,&swp) &&
           swp.cx &&
           swp.cy) {
          ptl.x = swp.x + (swp.cx / 2);
          ptl.y = swp.y + (swp.cy / 2);
          WinMapWindowPoints(WinQueryWindow(hwndO,QW_PARENT),
                             HWND_DESKTOP,
                             &ptl,
                             1L);
          WinSetPointerPos(HWND_DESKTOP,
                           ptl.x,
                           ptl.y);
        }
      }
      return 0;

    case WM_DESTROY:
      ObjectHwnd = (HWND)0;
      break;
  }
  return WinDefWindowProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY ObjectProc2 (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_CREATE:
      ObjectHwnd2 = hwnd;
      break;

    case UM_SETPRESPARMS:
      if(mp2) {

        TBARPARMS *tbpp = (TBARPARMS *)mp2;

        if(mp1) {

          HWND hwndTbar = (HWND)mp1;

          tbpp->hbm = (HBITMAP)0;
          tbpp->hwnd = WinQueryWindow(hwndTbar,QW_PARENT);
          if(tbpp->bitmapname &&
             *tbpp->bitmapname) {

            HBITMAP hbm = (HBITMAP)0;

            tbpp->hbm = LoadBitmap(tbpp->bitmapname);
            if(tbpp->hbm)
              hbm = TrimBitmap(WinQueryAnchorBlock(hwnd),
                               tbpp->hbm);
            if(hbm)
              tbpp->hbm = hbm;
          }
          WinSetPresParam(hwndTbar,
                          TITLEBAR_PARM,
                          sizeof(TBARPARMS),
                          (PVOID)tbpp);
          WinInvalidateRect(hwndTbar,
                            NULL,
                            FALSE);
        }
        free(tbpp);
      }
      return 0;

    case UM_CHECKTTLS:
      if(mp2) {

        TBARPARMS *info,tbp;
        char       s[CCHMAXPATH];
        HWND       hwndTbar = (HWND)mp2;
        long       cntr = (long)mp1,wascntr = (long)mp1;

        DosSleep(150);
        if(amclosing ||
           !WinIsWindow(WinQueryAnchorBlock(hwnd),hwndTbar))
          return 0;
        WinQueryWindowText(hwndTbar,
                           sizeof(s),
                           s);
        if(strcmp(s,"Untitled")) {
          if(!fEnhanceTitlebars)
            return 0;
           info = tbHead;
           while(info) {
             if(info->istitletext &&
                !strnicmp(info->schemename,
                          s,
                          strlen(info->schemename)))
               break;
             info = info->next;
           }
           if(info &&
              !info->exclude) {
             tbp = *info;
             tbp.hwnd = WinQueryWindow(hwndTbar,QW_PARENT);
             tbp.hbm = (HBITMAP)0;
             if(tbp.bitmapname &&
                *tbp.bitmapname) {

               HBITMAP hbm;

               tbp.hbm = LoadBitmap(tbp.bitmapname);
               hbm = TrimBitmap(WinQueryAnchorBlock(hwnd),
                                tbp.hbm);
               if(hbm)
                 tbp.hbm = hbm;
             }
             WinSetPresParam(hwndTbar,
                             TITLEBAR_PARM,
                             sizeof(TBARPARMS),
                             (PVOID)&tbp);
             WinInvalidateRect(hwndTbar,
                               NULL,
                               FALSE);
           }
           else if(info &&
                   info->exclude) {
             if(WinQueryPresParam(hwndTbar,
                                  TITLEBAR_PARM,
                                  0,
                                  NULL,
                                  sizeof(tbp),
                                  (PVOID)&tbp,
                                  QPF_NOINHERIT) &&
                tbp.size == sizeof(TBARPARMS) &&
                tbp.hbm)
               GpiDeleteBitmap(tbp.hbm);
             tbp.hbm = (HBITMAP)0;
             WinSetPresParam(hwndTbar,
                             TITLEBAR_PARM,
                             sizeof(TBARPARMS),
                             (PVOID)&tbp);
             WinRemovePresParam(hwndTbar,
                                TITLEBAR_PARM);
             WinInvalidateRect(hwndTbar,
                               NULL,
                               FALSE);
           }
           if(!info &&
              cntr < 1000)
            cntr++;
        }
        else if(cntr < 250)
          cntr++;
        if(*s &&
           strcmp(s,"Untitled"))
          return 0;
        DosSleep(0);
        if(cntr > wascntr)
          PostMsg(hwnd,
                  msg,
                  MPFROMLONG(cntr),
                  mp2);
      }
      return 0;

    case WM_DESTROY:
      ObjectHwnd2 = (HWND)0;
      break;
  }
  return WinDefWindowProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY ObjectProc3 (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_CREATE:
      ObjectHwnd3 = hwnd;
      break;

    case UM_BUMPDESKS:
      {
        long  vx,vy;
        SHORT xb,yb;

        xb = SHORT1FROMMP(mp1);
        yb = SHORT2FROMMP(mp1);
        vx = (xVirtual / xScreen) + 2;
        vy = (yVirtual / yScreen) + 2;
        vx += xb;
        vy -= yb;
        if(vx >= 1 &&
           vx <= 3 &&
           vy >= 1 &&
           vy <= 3) {
          ChangeDesktop(WinQueryAnchorBlock(hwnd),
                        vx,
                        vy);
          if(lBumpDesks < 0) {

            long   x,y;
            POINTL ptl;

            WinQueryPointerPos(HWND_DESKTOP,
                               &ptl);
            x = (xb < 0) ? xScreen - 2 : (xb > 0) ? 2 : ptl.x;
            y = (yb > 0 ) ? yScreen - 2 : (yb < 0) ? 2 : ptl.y;
            WinSetPointerPos(HWND_DESKTOP,
                             x,
                             y);
          }
        }
      }
      return 0;

    case UM_SAVESCREEN:
      SaveScreen(WinQueryAnchorBlock(hwnd),
                 ((fActiveSave) ?
                   WinQueryActiveWindow(HWND_DESKTOP) :
                   HWND_DESKTOP),
                 TRUE);
      if(mp1)
        PostMsg(hwndConfig,
                WM_SYSCOMMAND,
                MPFROM2SHORT(SC_RESTORE,0),
                MPVOID);
      return 0;

    case UM_MINALL:
      WinBroadcastMsg(HWND_DESKTOP,
                      WM_SYSCOMMAND,
                      MPFROM2SHORT(SC_MINIMIZE,0),
                      MPVOID,
                      BMSG_POST | BMSG_DESCENDANTS);
      return 0;

    case UM_CLIPMGR:
      if(ClipHwnd)
        ShowWindow(WinQueryAnchorBlock(hwnd),
                   WinQueryWindow(ClipHwnd,QW_PARENT));
      else
        DosBeep(50,100);
      return 0;

    case UM_SETUP:
      if(fEnhanceTitlebars)
        RedrawTitlebars(FALSE,HWND_DESKTOP);
      return 0;

    case UM_COMMAND:
      {
        HWND useHwnd = (HWND)mp1;
        SWP  swp;

        if(useHwnd &&
           useHwnd != (HWND)-1) {
          if(mp2)
            NormalizeWindow(useHwnd,TRUE);
          else {
            WinSetWindowPos(useHwnd,
                            HWND_TOP,
                            0,
                            0,
                            0,
                            0,
                            SWP_ZORDER | SWP_ACTIVATE);
            if(fMoveMouse &&
               WinQueryWindowPos(useHwnd,&swp) &&
               swp.x + (swp.cx / 2) < xScreen &&
               swp.y + (swp.cy / 2) < yScreen &&
               swp.x + (swp.cx / 2) > 0 &&
               swp.y + (swp.cy / 2) > 0)
              WinSetPointerPos(HWND_DESKTOP,
                               swp.x + (swp.cx / 2),
                               swp.y + (swp.cy / 2));
          }
        }
      }
      return 0;

    case WM_DESTROY:
      ObjectHwnd3 = (HWND)0;
      break;
  }
  return WinDefWindowProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY ObjectProc4 (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_CREATE:
      ObjectHwnd4 = hwnd;
      break;

    case UM_SETUP2:
      if(mp1) {

        int  x,y,z;
        HWND hwndD = (HWND)mp1;

        for(z = 0;z < 3;z++) {
          for(x = 0;x < 8;x++) {
            if(cmds[z][x] > C_MAXCMDS)
              cmds[z][x] = C_NONE;
            WinSetDlgItemText(hwndD,
                              101 + (x + (z * 8)),
                              cmdnames[cmds[z][x]]);
            DosSleep(0);
          }
          DosSleep(1);
        }
        for(z = 0;z < 3;z++) {
          for(x = 0;x < 8;x++) {
            for(y = 0;y <= C_MAXCMDS;y++) {
              WinSendDlgItemMsg(hwndD,
                                101 + (x + (z * 8)),
                                LM_INSERTITEM,
                                MPFROM2SHORT(LIT_END,0),
                                cmdnames[y]);
              DosSleep(0);
            }
            DosSleep(0);
          }
          DosSleep(1);
        }
      }
      return 0;

    case UM_TASKLIST:
      if(mp1) {

        HWND hwndT = (HWND)mp1;
        int  x;

        for(x = 0;x < 16;x++) {
          DosSleep(500);
          if(!WinIsWindow(WinQueryAnchorBlock(hwnd),hwndT))
            break;
        }
        if(WinIsWindow(WinQueryAnchorBlock(hwnd),hwndT)) {
          PostMsg(hwndT,
                  WM_QUIT,
                  MPVOID,
                  MPVOID);
          for(x = 0;x < 16;x++) {
            DosSleep(500);
            if(!WinIsWindow(WinQueryAnchorBlock(hwnd),hwndT))
              break;
          }
          if(WinIsWindow(WinQueryAnchorBlock(hwnd),hwndT)) {

            PID pid;
            TID tid;

            if(WinQueryWindowProcess(hwndT,
                                     &pid,
                                     &tid))
              DosKillProcess(DKP_PROCESS,
                             pid);
          }
        }
      }
      return 0;

    case WM_DESTROY:
      ObjectHwnd4 = (HWND)0;
      break;
  }
  return WinDefWindowProc(hwnd,msg,mp1,mp2);
}


static void MakeObjWin (void *args) {

  HAB   hab2;
  HMQ   hmq2;
  QMSG  qmsg2;
  HWND  hwndFrame = (HWND)0;
  ULONG which = (ULONG)args;

  hab2 = WinInitialize(0);
  if(hab2) {
    hmq2 = WinCreateMsgQueue(hab2,
                             256);
    if(hmq2) {
      WinCancelShutdown(hmq2,
                        TRUE);
      switch(which) {
        case 0:
          WinRegisterClass(hab2,
                           objwinclass,
                           ObjectProc,
                           0,
                           sizeof(PVOID));
          hwndFrame = WinCreateWindow(HWND_OBJECT,
                                      objwinclass,
                                      (PSZ)NULL,
                                      0,
                                      0L,
                                      0L,
                                      0L,
                                      0L,
                                      0L,
                                      HWND_TOP,
                                      32767,
                                      NULL,
                                      NULL);
          break;
        case 1:
          WinRegisterClass(hab2,
                           gobjwinclass,
                           GObjectProc,
                           0,
                           sizeof(PVOID));
          hwndFrame = WinCreateWindow(HWND_OBJECT,
                                      gobjwinclass,
                                      (PSZ)NULL,
                                      0,
                                      0L,
                                      0L,
                                      0L,
                                      0L,
                                      0L,
                                      HWND_TOP,
                                      32766,
                                      NULL,
                                      NULL);
          break;
        case 2:
          WinRegisterClass(hab2,
                           objwinclass2,
                           ObjectProc2,
                           0,
                           sizeof(PVOID));
          hwndFrame = WinCreateWindow(HWND_OBJECT,
                                      objwinclass2,
                                      (PSZ)NULL,
                                      0,
                                      0L,
                                      0L,
                                      0L,
                                      0L,
                                      0L,
                                      HWND_TOP,
                                      32765,
                                      NULL,
                                      NULL);
          break;
        case 3:
          WinRegisterClass(hab2,
                           objwinclass3,
                           ObjectProc3,
                           0,
                           sizeof(PVOID));
          hwndFrame = WinCreateWindow(HWND_OBJECT,
                                      objwinclass3,
                                      (PSZ)NULL,
                                      0,
                                      0L,
                                      0L,
                                      0L,
                                      0L,
                                      0L,
                                      HWND_TOP,
                                      32764,
                                      NULL,
                                      NULL);
          break;
        case 4:
          WinRegisterClass(hab2,
                           objwinclass4,
                           ObjectProc4,
                           0,
                           sizeof(PVOID));
          hwndFrame = WinCreateWindow(HWND_OBJECT,
                                      objwinclass4,
                                      (PSZ)NULL,
                                      0,
                                      0L,
                                      0L,
                                      0L,
                                      0L,
                                      0L,
                                      HWND_TOP,
                                      32764,
                                      NULL,
                                      NULL);
          break;
      }
      if(hwndFrame) {
        while(WinGetMsg(hab2,
                        &qmsg2,
                        (HWND)0,
                        0,
                        0))
          WinDispatchMsg(hab2,
                         &qmsg2);
        WinDestroyWindow(hwndFrame);
      }
      WinDestroyMsgQueue(hmq2);
    }
    WinTerminate(hab2);
  }
}


void StartObjWins (void) {

  _beginthread(MakeObjWin,
               NULL,
               41952,
               (PVOID)0);
  DosSleep(1);
  _beginthread(MakeObjWin,
               NULL,
               41952,
               (PVOID)1);
  DosSleep(1);
  _beginthread(MakeObjWin,
               NULL,
               41952,
               (PVOID)2);
  DosSleep(1);
  _beginthread(MakeObjWin,
               NULL,
               41952,
               (PVOID)3);
  DosSleep(1);
  _beginthread(MakeObjWin,
               NULL,
               41952,
               (PVOID)4);
  DosSleep(1);
}

