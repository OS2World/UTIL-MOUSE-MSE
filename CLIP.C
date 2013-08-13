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


void TakeClipboard (void) {

  if(ClipHwnd &&
     WinQueryClipbrdViewer(WinQueryAnchorBlock(ClipHwnd)) != ClipHwnd) {
    if(WinSetClipbrdViewer(WinQueryAnchorBlock(ClipHwnd),
                           ClipHwnd))
      WinSetWindowText(StatusHwnd,
                       " *MSE stole back the clipboard.");
    else
      WinSetWindowText(StatusHwnd,
                       " **MSE can't steal back the clipboard.");
  }
}


void StartClipMgr (void) {

  HWND  hwndClient;
  ULONG FrameFlags = FCF_TITLEBAR    | FCF_SYSMENU     |
                     FCF_SIZEBORDER  | FCF_MINMAX      |
                     FCF_NOBYTEALIGN | FCF_ICON        |
                     FCF_ACCELTABLE;

  WinCreateStdWindow(HWND_DESKTOP,
                     0,
                     &FrameFlags,
                     clipwinclass,
                     clipmgr,
                     0,
                     0,
                     CLIP_FRAME,
                     &hwndClient);
}


char * FullDrgName (PDRAGITEM pDItem,char *buffer,ULONG buflen) {

  register ULONG len,blen;

  *buffer = 0;
  blen = DrgQueryStrName(pDItem->hstrContainerName,
                         buflen,buffer);
  if(blen) {
    if(*(buffer + (blen - 1L)) != '\\') {
      *(buffer + blen) = '\\';
      blen++;
    }
  }
  buffer[blen] = 0;
  len = DrgQueryStrName(pDItem->hstrSourceName,
                        buflen - blen,buffer + blen);
  buffer[blen + len] = 0;
  return buffer;
}


MRESULT EXPENTRY DragProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  USHORT id;

  switch(msg) {
    case DM_DRAGOVER:
      id = WinQueryWindowUShort(hwnd,QWS_ID);
      if(id == 100 || id == 102 || id == 104) {

        PDRAGITEM pDItem;
        PDRAGINFO pDInfo;

        pDInfo = (PDRAGINFO)mp1;
        DrgAccessDraginfo(pDInfo);
        pDItem = DrgQueryDragitemPtr(pDInfo,0);
        if(DrgVerifyRMF(pDItem,"DRM_OS2FILE",NULL)) {
          DrgFreeDraginfo(pDInfo);
          return(MRFROM2SHORT(DOR_DROP,DO_COPY));
        }
        DrgFreeDraginfo(pDInfo);
      }
      return(MRFROM2SHORT(DOR_NEVERDROP,0));

    case DM_DROP:
      id = WinQueryWindowUShort(hwnd,QWS_ID);
      if(id == 100 || id == 102 || id == 104) {

        PDRAGITEM   pDItem;
        PDRAGINFO   pDInfo;
        ULONG       numitems,x;
        char       *p;
        static char szName[CCHMAXPATH + 2];
        USHORT      cmd;

        pDInfo = (PDRAGINFO)mp1;
        DrgAccessDraginfo(pDInfo);
        numitems = DrgQueryDragitemCount(pDInfo);
        pDItem = DrgQueryDragitemPtr(pDInfo,0);
        if(DrgVerifyRMF(pDItem,"DRM_OS2FILE",NULL) &&
           !(pDItem->fsControl & DC_PREPARE)) {
          *szName = 0;
          FullDrgName(pDItem,szName,CCHMAXPATH + 1);
          if(*szName) {
            p = strdup(szName);
            if(p) {
              if(id == 100)
                cmd = IDM_LOADCLIP;
              else if(id == 102)
                cmd = IDM_LOADBIT;
              else if(id == 104)
                cmd = IDM_LOADMET;
              if(!PostMsg(GObjectHwnd,
                          WM_COMMAND,
                          MPFROM2SHORT(cmd,0),
                          MPFROMP(p))) {
                free(p);
                _heapmin();
              }
            }
          }
          for(x = 0;x < numitems;x++) {
            pDItem = DrgQueryDragitemPtr(pDInfo,x);
            DrgSendTransferMsg(pDInfo->hwndSource,DM_ENDCONVERSATION,
                               MPFROMLONG(pDItem->ulItemID),
                               MPFROMLONG(DMFL_TARGETFAIL));
          }
        }
        DrgDeleteDraginfoStrHandles(pDInfo);
        DrgFreeDraginfo(pDInfo);
      }
      break;
  }
  return 0;
}


MRESULT EXPENTRY GraphicsProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_REALIZEPALETTE:
      {
        HPS   hps;
        ULONG cclr;

        hps = (HPS)WinQueryWindowULong(hwnd,4);
        if(hps)
          WinRealizePalette(hwnd,hps,&cclr);
      }
      break;

    case UM_SETUP:
      WinSetWindowULong(hwnd,0,(ULONG)mp1);
      {
        HDC   hdc = (HDC)0;
        HPS   hps = (HPS)0;
        SIZEL sizl = {0,0};

        hdc = WinOpenWindowDC(hwnd);
        if(hdc) {
          hps = GpiCreatePS(hwnd,
                            hdc,
                            &sizl,
                            PU_PELS | GPIF_LONG |
                            GPIT_NORMAL | GPIA_ASSOC);
          if(hps)
            WinSetWindowULong(hwnd,
                              4,
                              (ULONG)hps);
        }
        if(!hdc ||
           !hps)
          WinDestroyWindow(hwnd);
      }
      return 0;

    case DM_DROP:
      DragProc(hwnd,msg,mp1,mp2);
      break;

    case DM_DRAGOVER:
      return DragProc(hwnd,msg,mp1,mp2);

    case WM_CONTEXTMENU:
      PostMsg(WinQueryWindow(hwnd,QW_PARENT),
              UM_CONTEXTMENU,
              MPFROM2SHORT(WinQueryWindowUShort(hwnd,QWS_ID),0),
              mp2);
      return MRFROMSHORT(TRUE);

    case WM_MOUSEMOVE:
      if(hwnd != TipHwnd) {
        switch(WinQueryWindowULong(hwnd,0)) {
          case CF_BITMAP:
            WinSetWindowText(StatusHwnd,
                             " Bitmap data");
            break;
          case CF_DSPBITMAP:
            WinSetWindowText(StatusHwnd,
                             " Alternate Bitmap data");
            break;
          case CF_METAFILE:
            WinSetWindowText(StatusHwnd,
                             " Metafile data");
            break;
          case CF_DSPMETAFILE:
            WinSetWindowText(StatusHwnd,
                             " Alternate Metafile data");
            break;
        }
        TipHwnd = hwnd;
      }
      break;

    case WM_PAINT:
      {
        HPS   hps;
        RECTL rectl;
        HAB   hab;
        ULONG which;

        hps = (HPS)WinQueryWindowULong(hwnd,4);
        hps = WinBeginPaint(hwnd,hps,&rectl);
        if(hps) {
          WinFillRect(hps,
                      &rectl,
                      CLR_WHITE);
          WinQueryWindowRect(hwnd,
                             &rectl);
          if(rectl.xRight &&
             rectl.yTop &&
             WinIsWindowVisible(WinQueryWindow(hwnd,QW_PARENT))) {
            which = WinQueryWindowULong(hwnd,0);
            switch(which) {
              case CF_METAFILE:
              case CF_DSPMETAFILE:
                PostMsg(GObjectHwnd,
                        WM_DRAWCLIPBOARD,
                        MPFROM2SHORT(WinQueryWindowUShort(hwnd,QWS_ID),0),
                        MPVOID);
                goto SkipClip;
            }
            hab = WinQueryAnchorBlock(hwnd);
            if(which &&
               WinOpenClipbrd(hab)) {
              switch(which) {
                case CF_BITMAP:
                case CF_DSPBITMAP:
                  {
                    BITMAPINFOHEADER2 bmp2;
                    POINTL            aptl[4];
                    HBITMAP           hBitmap;
                    double            percent;

                    hBitmap = (HBITMAP)WinQueryClipbrdData(hab,which);
                    if(hBitmap) {
                      memset(&bmp2,0,sizeof(bmp2));
                      bmp2.cbFix = sizeof(bmp2);
                      if(GpiQueryBitmapInfoHeader(hBitmap,&bmp2)) {
                        aptl[0].x = 0;                /* target lower left */
                        aptl[0].y = 0;
                        aptl[1].x = rectl.xRight - 1;
                        aptl[1].y = rectl.yTop - 1;
                        percent = (double)bmp2.cx / (double)bmp2.cy;
                        aptl[1].y = (long)(((double)rectl.xRight - 1) /
                                             percent);
                        if(aptl[1].y > rectl.yTop - 1) {
                          aptl[1].y = rectl.yTop - 1;
                          aptl[1].x = (long)(((double)aptl[1].y) * percent);
                        }
                        aptl[2].x = 0;                /* source lower left */
                        aptl[2].y = 0;
                        aptl[3].x = bmp2.cx;          /* source upper right */
                        aptl[3].y = bmp2.cy;
                        GpiWCBitBlt(hps,hBitmap,4,aptl,ROP_SRCCOPY,
                                    BBO_IGNORE);
                      }
                    }
                  }
                  break;
              }
              WinCloseClipbrd(hab);
            }
          }
SkipClip:
          WinEndPaint(hps);
        }
      }
      return 0;

    case WM_DESTROY:
      {
        HPS  hps;
        HPAL hpal;

        hps = (HPS)WinQueryWindowULong(hwnd,4);
        hpal = (HPAL)WinQueryWindowULong(hwnd,8);
        if(hpal) {
          GpiSelectPalette(hps,(HPAL)0);
          GpiDeletePalette(hpal);
        }
        if(hps)
          GpiDestroyPS(hps);
      }
      break;
  }
  return WinDefWindowProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY SubProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case DM_DROP:
      DragProc(hwnd,msg,mp1,mp2);
      break;

    case DM_DRAGOVER:
      return DragProc(hwnd,msg,mp1,mp2);

    case WM_CONTEXTMENU:
      PostMsg(WinQueryWindow(hwnd,QW_PARENT),
              UM_CONTEXTMENU,
              MPFROM2SHORT(WinQueryWindowUShort(hwnd,QWS_ID),0),
              mp2);
      return MRFROMSHORT(TRUE);

    case WM_BUTTON1CLICK:
      if(WinQueryWindowUShort(hwnd,QWS_ID) == 99) {

        SWP swp;

        NormalizeWindow(hwndConfig,TRUE);
        WinQueryWindowPos(hwndConfig,&swp);
        if(swp.fl & (SWP_MINIMIZE | SWP_HIDE)) {
          WinSendMsg(hwndConfig,
                     WM_SYSCOMMAND,
                     MPFROM2SHORT(SC_RESTORE,0),
                     MPVOID);
          WinShowWindow(hwndConfig,
                        TRUE);
        }
        else
          WinSetWindowPos(hwndConfig,
                          HWND_TOP,
                          swp.x,swp.y,swp.cx,swp.cy,
                          SWP_MOVE | SWP_SHOW | SWP_SIZE | SWP_ACTIVATE |
                          SWP_ZORDER);
        WinSetFocus(HWND_DESKTOP,
                    hwndConfig);
      }
      return MRFROMSHORT(TRUE);

    case WM_MOUSEMOVE:
      if(hwnd != TipHwnd) {
        switch(WinQueryWindowUShort(hwnd,QWS_ID)) {
          case 99:
            WinSetWindowText(StatusHwnd,
                             " B1 here = MSE, B2 = settings menu");
            break;
          case 100:
            WinSetWindowText(StatusHwnd,
                             " Text data");
            break;
          case 101:
            WinSetWindowText(StatusHwnd,
                             " Alternate Text data");
            break;
        }
        TipHwnd = hwnd;
      }
      break;
  }
  return ((PFNWP)WinQueryWindowPtr(hwnd,0))(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY ClipProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  static HWND hwndMenu = (HWND)0,hwndMLE1,hwndMLE2,
              hwndBit1,hwndBit2,hwndMet1,hwndMet2;
  static char lasttext[CCHMAXPATH] = "",lastbmp[CCHMAXPATH] = "",
              lastmet[CCHMAXPATH] = "";

  switch(msg) {
    case WM_CREATE:
      if(fClipMon)
        StartSwapMon(CLP_FRAME);
      SetPresParams(hwnd,
                    -1,
                    -1,
                    -1,
                    helvtext);
      cliptext = malloc(numtext * sizeof(char *));
      if(cliptext)
        memset(cliptext,
               0,
               numtext * sizeof(char *));
      else
         WinDestroyWindow(WinQueryWindow(hwnd,QW_PARENT));
      if(WinOpenClipbrd(WinQueryAnchorBlock(hwnd))) {

        BOOL destroyme = FALSE;

        if(!WinQueryClipbrdViewer(WinQueryAnchorBlock(hwnd)) &&
           !WinSetClipbrdViewer(WinQueryAnchorBlock(hwnd),hwnd))
          destroyme = TRUE;
        WinCloseClipbrd(WinQueryAnchorBlock(hwnd));
        WinRegisterClass(WinQueryAnchorBlock(hwnd),
                         graphwinclass,
                         GraphicsProc,
                         CS_SIZEREDRAW,
                         sizeof(ULONG) + sizeof(HPS) + sizeof(HPAL));
        if(!destroyme) {
          StatusHwnd = WinCreateWindow(hwnd,
                                       WC_STATIC,
                                       (PSZ)NULL,
                                       WS_VISIBLE | SS_TEXT | DT_LEFT |
                                       DT_VCENTER,
                                       0,
                                       0,
                                       0,
                                       0,
                                       hwnd,
                                       HWND_TOP,
                                       99,
                                       MPVOID,
                                       MPVOID);
          WinCreateWindow(hwnd,
                          WC_BUTTON,
                          "~<",
                          WS_VISIBLE | BS_PUSHBUTTON | BS_NOPOINTERFOCUS,
                          0,
                          0,
                          0,
                          0,
                          hwnd,
                          HWND_TOP,
                          106,
                          MPVOID,
                          MPVOID);
          WinCreateWindow(hwnd,
                          WC_BUTTON,
                          "~>",
                          WS_VISIBLE | BS_PUSHBUTTON | BS_NOPOINTERFOCUS,
                          0,
                          0,
                          0,
                          0,
                          hwnd,
                          HWND_TOP,
                          107,
                          MPVOID,
                          MPVOID);
          hwndMLE1 = WinCreateWindow(hwnd,
                                     WC_MLE,
                                     (PSZ)NULL,
                                     MLS_HSCROLL | MLS_VSCROLL | MLS_BORDER |
                                     WS_VISIBLE,
                                     0L,
                                     0L,
                                     0L,
                                     0L,
                                     hwnd,
                                     HWND_TOP,
                                     100,
                                     MPVOID,
                                     MPVOID);
          hwndMLE2 = WinCreateWindow(hwnd,
                                     WC_MLE,
                                     (PSZ)NULL,
                                     MLS_HSCROLL | MLS_VSCROLL | MLS_BORDER |
                                     WS_VISIBLE,
                                     0L,
                                     0L,
                                     0L,
                                     0L,
                                     hwnd,
                                     HWND_TOP,
                                     101,
                                     MPVOID,
                                     MPVOID);
          hwndBit1 = WinCreateWindow(hwnd,
                                     graphwinclass,
                                     (PSZ)NULL,
                                     WS_VISIBLE,
                                     0L,
                                     0L,
                                     0L,
                                     0L,
                                     hwnd,
                                     HWND_TOP,
                                     102,
                                     MPVOID,
                                     MPVOID);
          hwndBit2 = WinCreateWindow(hwnd,
                                     graphwinclass,
                                     (PSZ)NULL,
                                     WS_VISIBLE,
                                     0L,
                                     0L,
                                     0L,
                                     0L,
                                     hwnd,
                                     HWND_TOP,
                                     103,
                                     MPVOID,
                                     MPVOID);
          hwndMet1 = WinCreateWindow(hwnd,
                                     graphwinclass,
                                     (PSZ)NULL,
                                     WS_VISIBLE,
                                     0L,
                                     0L,
                                     0L,
                                     0L,
                                     hwnd,
                                     HWND_TOP,
                                     104,
                                     MPVOID,
                                     MPVOID);
          hwndMet2 = WinCreateWindow(hwnd,
                                     graphwinclass,
                                     (PSZ)NULL,
                                     WS_VISIBLE,
                                     0L,
                                     0L,
                                     0L,
                                     0L,
                                     hwnd,
                                     HWND_TOP,
                                     105,
                                     MPVOID,
                                     MPVOID);
          if(!StatusHwnd || !hwndMLE1 || !hwndMLE2 || !hwndBit1 ||
             !hwndBit2 || !hwndMet1 || !hwndMet2)
            destroyme = TRUE;
          else {
            WinSetWindowPtr(hwndMLE1,
                            0,
                            (PVOID)WinSubclassWindow(hwndMLE1,
                                                     (PFNWP)SubProc));
            WinSetWindowPtr(hwndMLE2,
                            0,
                            (PVOID)WinSubclassWindow(hwndMLE2,
                                                     (PFNWP)SubProc));
            WinSetWindowPtr(StatusHwnd,
                            0,
                            (PVOID)WinSubclassWindow(StatusHwnd,
                                                     (PFNWP)SubProc));
            WinSendMsg(hwndMLE1,
                       MLM_SETWRAP,
                       MPVOID,
                       MPVOID);
            WinSendMsg(hwndMLE1,
                       MLM_SETREADONLY,
                       MPFROM2SHORT(TRUE,0),
                       MPVOID);
            WinSendMsg(hwndMLE1,
                       MLM_SETTEXTLIMIT,
                       MPFROMLONG(-1),
                       MPVOID);
            WinSendMsg(hwndMLE1,
                       MLM_FORMAT,
                       MPFROM2SHORT(MLFIE_CFTEXT,0),
                       MPVOID);
            WinSendMsg(hwndMLE2,
                       MLM_SETWRAP,
                       MPVOID,
                       MPVOID);
            WinSendMsg(hwndMLE2,
                       MLM_SETREADONLY,
                       MPFROM2SHORT(TRUE,0),
                       MPVOID);
            WinSendMsg(hwndMLE2,
                       MLM_SETTEXTLIMIT,
                       MPFROMLONG(-1),
                       MPVOID);
            WinSendMsg(hwndMLE2,
                       MLM_FORMAT,
                       MPFROM2SHORT(MLFIE_CFTEXT,0),
                       MPVOID);
            WinSendMsg(hwndBit1,
                       UM_SETUP,
                       MPFROMLONG(CF_BITMAP),
                       MPFROMLONG(StatusHwnd));
            WinSendMsg(hwndBit2,
                       UM_SETUP,
                       MPFROMLONG(CF_DSPBITMAP),
                       MPFROMLONG(StatusHwnd));
            WinSendMsg(hwndMet1,
                       UM_SETUP,
                       MPFROMLONG(CF_METAFILE),
                       MPFROMLONG(StatusHwnd));
            WinSendMsg(hwndMet2,
                       UM_SETUP,
                       MPFROMLONG(CF_DSPMETAFILE),
                       MPFROMLONG(StatusHwnd));
            SetPresParams(StatusHwnd,
                          RGB_BLACK,
                          RGB_PALEGRAY,
                          RGB_BLACK,
                          helvtext);
          }
        }
        if(!destroyme) {
          ClipHwnd = hwnd;
          PostMsg(hwnd,
                  UM_SETUP,
                  MPVOID,
                  MPVOID);
          PostMsg(hwnd,
                  WM_DRAWCLIPBOARD,
                  MPVOID,
                  MPVOID);
          break;
        }
      }
      WinDestroyWindow(WinQueryWindow(hwnd,QW_PARENT));
      break;

    case WM_SETFOCUS:
      if(mp2 &&
         WinOpenClipbrd(WinQueryAnchorBlock(hwnd))) {
        TakeClipboard();
        WinCloseClipbrd(WinQueryAnchorBlock(hwnd));
      }
      break;

    case UM_CONTEXTMENU:
      if(WinOpenClipbrd(WinQueryAnchorBlock(hwnd))) {
        TakeClipboard();
        WinCloseClipbrd(WinQueryAnchorBlock(hwnd));
      }
      {
        MENUITEM mi;
        USHORT   x;
        char     title[40],*p;

        hwndMenu = WinLoadMenu(HWND_DESKTOP,
                               0,
                               IDM_CLIPTEXT);
        if(hwndMenu) {
          mi.iPosition = MIT_END;
          mi.hwndSubMenu = (HWND)0;
          mi.hItem = 0L;
          if(SHORT1FROMMP(mp1) == 100) {
            mi.afStyle = MIS_TEXT;
            mi.afAttribute = 0;
            mi.id = IDM_SAVECLIP;
            if(!WinQueryWindowTextLength(hwndMLE1))
              mi.afAttribute = MIA_DISABLED;
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP("~Save clipboard to file"));
            mi.id = IDM_LOADCLIP;
            mi.afAttribute = 0;
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP("~Import file to clipboard"));
            mi.afStyle = MIS_SEPARATOR;
            mi.afAttribute = 0;
            mi.id = -1;
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPVOID);
            mi.afStyle = MIS_TEXT | MIS_STATIC;
            mi.afAttribute = MIA_FRAMED;
            mi.id = -1;
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP("Choose a clipboard (+Ctrl=Delete):"));
            for(x = 0;x < numtext;x++) {
              mi.afStyle = MIS_TEXT;
              mi.afAttribute = 0;
              mi.id = x + IDM_CLIPTEXT + 1;
              if(!(x % 25) && x)
                mi.afStyle |= MIS_BREAK;
              if(!cliptext[x]) {
                mi.afAttribute = MIA_DISABLED;
                strcpy(title,"n/a");
              }
              else {
                mi.afAttribute = 0;
                p = cliptext[x];
                while(*p && (*p == ' ' || *p == '\t' || *p == '\r' ||
                             *p == '\n' || *p == '~'))
                  p++;
                *title = '\"';
                strncpy(title + 1,p,38);
                title[38] = 0;
                p = title;
                while(*p) {
                  if(*p == '\t' || *p == '~' || *p == '\r' || *p == '\n')
                    *p = 0;
                  p++;
                }
                rstrip(title);
                if(strlen(cliptext[x]) >= 36)
                  strcpy(title + 35,"...");
                strcat(title,"\"");
              }
              if(x == nexttext)
                mi.afAttribute |= MIA_CHECKED;
              WinSendMsg(hwndMenu,
                         MM_INSERTITEM,
                         MPFROMP(&mi),
                         MPFROMP(title));
            }
          }
          else if(SHORT1FROMMP(mp1) == 102) {
            mi.afStyle = MIS_TEXT;
            mi.id = IDM_SAVEBIT;
            mi.afAttribute = MIA_DISABLED;
            if(WinOpenClipbrd(WinQueryAnchorBlock(hwnd))) {
              if(WinQueryClipbrdData(WinQueryAnchorBlock(hwnd),CF_BITMAP))
                mi.afAttribute = 0;
              WinCloseClipbrd(WinQueryAnchorBlock(hwnd));
            }
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP("~Save clipboard to file"));
            mi.id = IDM_BITINFO;
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP("Information on ~bitmap"));
            mi.id = IDM_LOADBIT;
            mi.afAttribute = 0;
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP("~Import file to clipboard"));
          }
          else if(SHORT1FROMMP(mp1) == 104) {
            mi.afStyle = MIS_TEXT;
            mi.id = IDM_SAVEMET;
            mi.afAttribute = MIA_DISABLED;
            if(WinOpenClipbrd(WinQueryAnchorBlock(hwnd))) {
              if(WinQueryClipbrdData(WinQueryAnchorBlock(hwnd),CF_METAFILE))
                mi.afAttribute = 0;
              WinCloseClipbrd(WinQueryAnchorBlock(hwnd));
            }
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP("~Save clipboard to file"));
            mi.id = IDM_LOADMET;
            mi.afAttribute = 0;
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP("~Import file to clipboard"));
          }
          else if(SHORT1FROMMP(mp1) == 101) {
            mi.afStyle = MIS_TEXT;
            mi.id = IDM_SAVECLIP2;
            mi.afAttribute = MIA_DISABLED;
            if(WinOpenClipbrd(WinQueryAnchorBlock(hwnd))) {
              if(WinQueryClipbrdData(WinQueryAnchorBlock(hwnd),CF_DSPTEXT))
                mi.afAttribute = 0;
              WinCloseClipbrd(WinQueryAnchorBlock(hwnd));
            }
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP("~Save clipboard to file"));
          }
          else if(SHORT1FROMMP(mp1) == 103) {
            mi.afStyle = MIS_TEXT;
            mi.id = IDM_SAVEBIT2;
            mi.afAttribute = MIA_DISABLED;
            if(WinOpenClipbrd(WinQueryAnchorBlock(hwnd))) {
              if(WinQueryClipbrdData(WinQueryAnchorBlock(hwnd),CF_DSPBITMAP))
                mi.afAttribute = 0;
              WinCloseClipbrd(WinQueryAnchorBlock(hwnd));
            }
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP("~Save clipboard to file"));
            mi.id = IDM_BITINFO2;
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP("Information on ~bitmap"));
          }
          else if(SHORT1FROMMP(mp1) == 105) {
            mi.afStyle = MIS_TEXT;
            mi.id = IDM_SAVEMET2;
            mi.afAttribute = MIA_DISABLED;
            if(WinOpenClipbrd(WinQueryAnchorBlock(hwnd))) {
              if(WinQueryClipbrdData(WinQueryAnchorBlock(hwnd),CF_DSPMETAFILE))
                mi.afAttribute = 0;
              WinCloseClipbrd(WinQueryAnchorBlock(hwnd));
            }
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP("~Save clipboard to file"));
          }
          else if(SHORT1FROMMP(mp1) == 99) {
            mi.afStyle = MIS_TEXT;
            mi.id = IDM_SHOWTEXT2;
            mi.afAttribute = (showText2) ? MIA_CHECKED : 0;
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP("Show alternate ~text data window"));
            mi.id = IDM_SHOWBIT2;
            mi.afAttribute = (showBit2) ? MIA_CHECKED : 0;
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP("Show alternate ~bitmap data window"));
            mi.id = IDM_SHOWMET;
            mi.afAttribute = (showMet) ? MIA_CHECKED : 0;
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP("Show ~metafile data window"));
            mi.id = IDM_SHOWMET2;
            mi.afAttribute = (showMet2) ? MIA_CHECKED : 0;
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP("Show ~alternate metafile data window"));
            mi.id = -1;
            mi.afAttribute = 0;
            mi.afStyle = MIS_SEPARATOR;
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPVOID);
            mi.afStyle = MIS_TEXT;
            mi.id = IDM_CLEARCLIP;
            mi.afAttribute = 0;
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP("~Clear clipboard\tDelete"));
            mi.id = -1;
            mi.afAttribute = 0;
            mi.afStyle = MIS_SEPARATOR;
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPVOID);
            mi.afStyle = MIS_TEXT;
            mi.id = IDM_CLIPSAVEALL;
            mi.afAttribute = 0;
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP("Save text clipboards to disk"));
            mi.id = IDM_CLIPLOADALL;
            mi.afAttribute = 0;
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP("Load text clipboards from disk"));
            mi.id = IDM_CLIPAUTOSAVE;
            mi.afAttribute = (fClipSave) ? MIA_CHECKED: 0;
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP("Autosave text clipboards to disk"));
            mi.id = IDM_CLIPFOLDER;
            mi.afAttribute = 0;
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP("Saved text clipboards folder"));
            mi.id = -1;
            mi.afAttribute = 0;
            mi.afStyle = MIS_SEPARATOR;
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPVOID);
            mi.afStyle = MIS_TEXT;
            mi.id = IDM_CLIPHELP;
            mi.afAttribute = 0;
            WinSendMsg(hwndMenu,
                       MM_INSERTITEM,
                       MPFROMP(&mi),
                       MPFROMP("~Help\tF1"));
          }
        }
      }
      if(hwndMenu) {

        POINTL ptl;

        WinQueryPointerPos(HWND_DESKTOP,&ptl);

        if(WinPopupMenu(HWND_DESKTOP,
                        hwnd,
                        hwndMenu,
                        ptl.x - 4,
                        ptl.y - 4,
                        0,
                        PU_HCONSTRAIN | PU_VCONSTRAIN |
                        PU_KEYBOARD   | PU_MOUSEBUTTON1))
          fSuspend = TRUE;
      }
      return 0;

    case WM_MENUEND:
      if(hwndMenu)
        WinDestroyWindow(hwndMenu);
      fSuspend = FALSE;
      return 0;

    case UM_SETUP:
      {
        SWP   swp;
        ULONG size;
        BOOL  fake = TRUE;

        if(prof) {
          size = sizeof(swp);
          if(PrfQueryProfileData(prof,
                                 appname,
                                 "ClipView.Pos",
                                 &swp,
                                 &size))
            fake = FALSE;
          if(!*lasttext) {
            size = sizeof(lasttext);
            PrfQueryProfileData(prof,
                                appname,
                                "LastFile",
                                lasttext,
                                &size);
          }
          if(!*lastbmp) {
            size = sizeof(lastbmp);
            PrfQueryProfileData(prof,
                                appname,
                                "LastBmp",
                                lastbmp,
                                &size);
          }
          if(!*lastmet) {
            size = sizeof(lastmet);
            PrfQueryProfileData(prof,
                                appname,
                                "LastMet",
                                lastmet,
                                &size);
          }
          size = sizeof(showText2);
          PrfQueryProfileData(prof,
                              appname,
                              "ShowText2",
                              &showText2,
                              &size);
          size = sizeof(showBit2);
          PrfQueryProfileData(prof,
                              appname,
                              "ShowBit2",
                              &showBit2,
                              &size);
          size = sizeof(showMet);
          PrfQueryProfileData(prof,
                              appname,
                              "ShowMet",
                              &showMet,
                              &size);
          size = sizeof(showMet2);
          PrfQueryProfileData(prof,
                              appname,
                              "ShowMet2",
                              &showMet2,
                              &size);
        }
        if(fake) {
          if(WinQueryTaskSizePos(WinQueryAnchorBlock(hwnd),0,&swp)) {
            swp.x = 50;
            swp.y = 100;
            swp.cx = 200;
            swp.cy = 100;
          }
        }
        SaveRestorePos(&swp,WinQueryWindow(hwnd,QW_PARENT));
        WinSetWindowPos(WinQueryWindow(hwnd,QW_PARENT),
                        HWND_TOP,
                        swp.x,
                        swp.y,
                        swp.cx,
                        swp.cy,
                        SWP_HIDE | SWP_SIZE | SWP_MOVE);
        WinSendMsg(WinQueryWindow(hwnd,QW_PARENT),
                   WM_SYSCOMMAND,
                   MPFROM2SHORT(SC_MINIMIZE,0),
                   MPVOID);
        ClipHwnd = hwnd;
        WinSendMsg(hwnd,
                   UM_REMIND,
                   MPVOID,
                   MPVOID);
      }
      return 0;

    case UM_CALC:
      if(cliptext) {

        int   x,z = 0;
        char  s[80];
        FILE *fp;

        DosEnterCritSec();
        DosCreateDir(".\\CLIPS",NULL);
        for(x = 0;
            x < 99;
            x++) {
          sprintf(s,
                  ".\\CLIPS\\CLIPBRD.%03x",
                  x);
          DosForceDelete(s);
        }
        for(x = 0;x < numtext;x++) {
          if(cliptext[x]) {
            sprintf(s,
                    ".\\CLIPS\\CLIPBRD.%03x",
                    z);
            fp = fopen(s,"wb");
            if(fp) {
              fwrite(cliptext[x],
                     1,
                     strlen(cliptext[x]),
                     fp);
              fclose(fp);
              z++;
            }
          }
        }
        DosExitCritSec();
      }
      return 0;

    case UM_REMIND:
      {
        char  s[80];
        FILE *fp;
        ULONG len;
        int   x,z = 0;

        DosEnterCritSec();
        for(x = 0;x < numtext;x++) {
          if(cliptext[x])
            free(cliptext[x]);
          cliptext[x] = NULL;
        }
        thistext = nexttext = 0;
        for(x = 0;x < numtext;x++) {
          sprintf(s,
                  ".\\CLIPS\\CLIPBRD.%03x",
                  x);
          fp = fopen(s,"rb");
          if(fp) {
            fseek(fp,
                  0,
                  SEEK_END);
            len = ftell(fp);
            fseek(fp,
                  0,
                  SEEK_SET);
            if(len) {
              if(cliptext[z]) {
                free(cliptext[z]);
                cliptext[z] = NULL;
              }
              cliptext[z] = malloc(len + 1);
              if(cliptext[z]) {
                cliptext[z][0] = 0;
                fread(cliptext[z],1,len,fp);
                cliptext[z][len] = 0;
                thistext = z;
                z++;
                nexttext = z;
                if(nexttext >= numtext)
                  nexttext = 0;
              }
            }
            fclose(fp);
          }
        }
        DosExitCritSec();
        _heapmin();
      }
      return 0;

    case WM_DESTROYCLIPBOARD:
    case WM_DRAWCLIPBOARD:
      ClipHwnd = hwnd;
      if(ClipMonHwnd)
        PostMsg(ClipMonHwnd,
                UM_REFRESH,
                MPVOID,
                MPVOID);
      if(WinOpenClipbrd(WinQueryAnchorBlock(hwnd))) {

        HAB        hab = WinQueryAnchorBlock(hwnd);
        PSZ        pszText;
        char      *p;
        ULONG      len,numb;
        IPT        insert;
        int        x;

        WinSetWindowText(hwndMLE1,  /* clear MLE */
                         "");
        if((pszText = (PSZ)WinQueryClipbrdData(hab,CF_TEXT)) != NULL) {
          p = pszText;
          insert = 0;
          WinSetPointer(HWND_DESKTOP,
                        hptrWait);
          while(*p) {
            len = min(32768,strlen(p));
            WinSendMsg(hwndMLE1,
                       MLM_SETIMPORTEXPORT,
                       MPFROMP(p),
                       MPFROMLONG(len));
            numb = (ULONG)WinSendMsg(hwndMLE1,
                                     MLM_IMPORT,
                                     MPFROMP(&insert),
                                     MPFROMLONG(len));
            if(!numb)
              break;
            p += len;
          }
          WinSetPointer(HWND_DESKTOP,
                        hptrArrow);
          if(!notnow &&
             (WinGetKeyState(HWND_DESKTOP,VK_SCRLLOCK) & 0x0001) == 0) {
            if(fClipAppend &&
               cliptext[thistext]) {

              char *test;

              if(strstr(cliptext[thistext], /* already contains this text */
                        pszText))
                goto Text2;
              len = strlen(pszText) + strlen(cliptext[thistext]) + 3;
              test = realloc(cliptext[thistext],len);
              if(test) {
                cliptext[thistext] = test;
                strcat(cliptext[thistext],"\r\n");
                strcat(cliptext[thistext],pszText);
                goto Text2;
              }
            }
            for(x = 0;x < numtext;x++) {
              if(cliptext[x] &&
                 !strcmp(cliptext[x],pszText))
                break;
            }
            if(x >= numtext) {
              cliptext[nexttext] = strdup(pszText);
              if(cliptext[nexttext]) {
                thistext = nexttext;
                nexttext++;
              }
              if(nexttext >= numtext)
                nexttext = 0;
            }
            else
              thistext = x;
          }
        }

Text2:
        if(!notnow &&
           (pszText = (PSZ)WinQueryClipbrdData(hab,CF_DSPTEXT)) != NULL) {
          p = pszText;
          insert = 0;
          WinSetPointer(HWND_DESKTOP,
                        hptrWait);
          while(*p) {
            len = min(32768,strlen(p));
            WinSendMsg(hwndMLE2,
                       MLM_SETIMPORTEXPORT,
                       MPFROMP(p),
                       MPFROMLONG(len));
            numb = (ULONG)WinSendMsg(hwndMLE2,
                                     MLM_IMPORT,
                                     MPFROMP(&insert),
                                     MPFROMLONG(len));
            if(!numb)
              break;
            p += len;
          }
          WinSetPointer(HWND_DESKTOP,
                        hptrArrow);
          if((WinGetKeyState(HWND_DESKTOP,VK_SCRLLOCK) & 0x0001) == 0) {
            for(x = 0;x < numtext;x++) {
              if(cliptext[x] &&
                 !strcmp(cliptext[x],pszText))
                break;
            }
            if(x >= numtext) {
              cliptext[nexttext] = strdup(pszText);
              if(cliptext[nexttext])
                nexttext++;
              if(nexttext >= numtext)
                nexttext = 0;
            }
          }
        }
        else if(!notnow)
          WinSetWindowText(hwndMLE2,"");
        WinCloseClipbrd(hab);
      }
      else {
        WinSetWindowText(hwndMLE1,"");
        WinSetWindowText(hwndMLE2,"");
      }
      if(!notnow &&
         WinIsWindowVisible(hwnd)) {
        WinInvalidateRect(hwndBit1,NULL,TRUE);
        if(WinIsWindowVisible(hwndBit2))
          WinInvalidateRect(hwndBit2,NULL,TRUE);
        if(WinIsWindowVisible(hwndMet1))
          WinInvalidateRect(hwndMet1,NULL,TRUE);
        if(WinIsWindowVisible(hwndMet2))
          WinInvalidateRect(hwndMet2,NULL,TRUE);
      }
      return 0;

    case UM_SIZE:
    case WM_SIZE:
      if(msg == UM_SIZE) {

        SWP swp;

        WinQueryWindowPos(hwnd,&swp);
        mp1 = MPFROM2SHORT(swp.cx,swp.cy);
        mp2 = MPFROM2SHORT(swp.cx,swp.cy);
      }
      {
        SHORT cx,cy,bx,btx,bty,btya,mtx;
        static SHORT by = 0;

        if(!by) {

          HPS hps;
          POINTL aptl[TXTBOX_COUNT];

          hps = WinGetPS(hwnd);
          GpiQueryTextBox(hps,5,"Wjq~,",TXTBOX_COUNT,aptl);
          WinReleasePS(hps);
          by = aptl[TXTBOX_TOPRIGHT].y + 4;
        }
        bx = btya = 0;
        cx = SHORT1FROMMP(mp2) - bx;
        cy = SHORT2FROMMP(mp2) - by;
        btx = (showBit2) ? (cx / 2) : cx;
        bty = (cy / 4) * 2;
        if(showMet || showMet2) {
          bty /= 2;
          btya = bty;
        }
        mtx = (showMet && showMet2) ? (cx / 2) : cx;

        WinSetWindowPos(StatusHwnd,HWND_TOP,
                        0,
                        0,
                        ((cx + bx) - (by * 2)) - 3,
                        by,
                        SWP_MOVE | SWP_SIZE | SWP_SHOW);
        WinSetWindowPos(WinWindowFromID(hwnd,106),HWND_TOP,
                        ((cx + bx) - (by * 2)) - 2,
                        0,
                        by,
                        by,
                        SWP_MOVE | SWP_SIZE | SWP_SHOW);
        WinSetWindowPos(WinWindowFromID(hwnd,107),HWND_TOP,
                        ((cx + bx) - by) - 1,
                        0,
                        by,
                        by,
                        SWP_MOVE | SWP_SIZE | SWP_SHOW);
        WinSetWindowPos(hwndMLE1,HWND_TOP,
                        bx,
                        (by + ((cy / 4)) * (2 + (showText2 != 0))),
                        cx,
                        (cy / 4) * (1 + (showText2 == 0)),
                        SWP_MOVE | SWP_SIZE | SWP_SHOW);
        if(showText2)
          WinSetWindowPos(hwndMLE2,HWND_TOP,
                          bx,
                          by + ((cy / 4) * 2),
                          cx,
                          cy / 4,
                          SWP_MOVE | SWP_SIZE | SWP_SHOW);
        else
          WinShowWindow(hwndMLE2,FALSE);
        WinSetWindowPos(hwndBit1,HWND_TOP,
                        bx + 1,
                        by + btya + 1,
                        btx - 2,
                        bty - 2,
                        SWP_MOVE | SWP_SIZE | SWP_SHOW);
        if(showBit2)
          WinSetWindowPos(hwndBit2,HWND_TOP,
                          bx + btx + 1,
                          by + btya + 1,
                          btx - 2,
                          bty - 2,
                          SWP_MOVE | SWP_SIZE | SWP_SHOW);
        else
          WinShowWindow(hwndBit2,FALSE);
        if(showMet)
          WinSetWindowPos(hwndMet1,HWND_TOP,
                          bx + 1,
                          by + 1,
                          mtx - 2,
                          bty - 2,
                          SWP_MOVE | SWP_SIZE | SWP_SHOW);
        else
          WinShowWindow(hwndMet1,FALSE);
        if(showMet2)
          WinSetWindowPos(hwndMet2,HWND_TOP,
                          bx + (cx / 2) + 1,
                          by + 1,
                          mtx - 2,
                          bty - 2,
                          SWP_MOVE | SWP_SIZE | SWP_SHOW);
        else
          WinShowWindow(hwndMet2,FALSE);
      }
      if(msg == UM_SIZE)
        return 0;
      break;

    case WM_PAINT:
      {
        HPS   hps;
        RECTL rcl;

        hps = WinBeginPaint(hwnd,(HPS)0,&rcl);
        if(hps) {
          WinFillRect(hps,&rcl,CLR_DARKGRAY);
          WinEndPaint(hps);
        }
      }
      return 0;

    case WM_CONTROL:

      break;

    case WM_COMMAND:
      {
        BOOL refocus = TRUE;

        switch(SHORT1FROMMP(mp1)) {
          case IDM_CLIPFOLDER:
            ShowFolder("CLIPS");
            refocus = FALSE;
            break;

          case IDM_CLIPAUTOSAVE:
            fClipSave = (fClipSave) ? FALSE : TRUE;
            SavePrf("SaveClips",
                    &fClipSave,
                    sizeof(BOOL));
            if(ClipSetHwnd)
              PostMsg(ClipSetHwnd,
                      UM_REFRESH,
                      MPVOID,
                      MPVOID);
            break;

          case IDM_CLIPLOADALL:
            WinSendMsg(hwnd,
                       UM_REMIND,
                       MPVOID,
                       MPVOID);
            PostMsg(hwnd,
                    WM_DRAWCLIPBOARD,
                    MPVOID,
                    MPVOID);
            break;

          case IDM_CLIPSAVEALL:
            WinSendMsg(hwnd,
                       UM_CALC,
                       MPVOID,
                       MPVOID);
            break;

          case IDM_CLIPHELP:
            ViewHelp(hwnd,
                     "Clipboard manager");
            refocus = FALSE;
            break;

          case 106:
            {
              int x = thistext;

              x--;
              if(x < 0)
                x = numtext - 1;
              while(x > 0) {
                if(cliptext[x])
                  break;
                x--;
              }
              if(x < 0) {
                x = numtext - 1;
                while(x > 0) {
                  if(cliptext[x])
                    break;
                  x--;
                }
              }
              if(x >= 0 &&
                 cliptext[x] &&
                 x != thistext) {
                thistext = x;
                PostMsg(hwnd,
                        WM_COMMAND,
                        MPFROM2SHORT(IDM_CLIPTEXT + 1 + thistext,0),
                        MPVOID);
              }
            }
            break;
          case 107:
            {
              int x = thistext;

              x++;
              if(x >= numtext)
                x = 0;
              while(x < numtext) {
                if(cliptext[x])
                  break;
                x++;
              }
              if(x >= numtext) {
                x = 0;
                while(x < numtext) {
                  if(cliptext[x])
                    break;
                  x++;
                }
              }
              if(x < numtext &&
                 cliptext[x] &&
                 x != thistext) {
                thistext = x;
                PostMsg(hwnd,
                        WM_COMMAND,
                        MPFROM2SHORT(IDM_CLIPTEXT + 1 + thistext,0),
                        MPVOID);
              }
            }
            break;
          case IDM_CLEARCLIP:
            {
              HAB hab = WinQueryAnchorBlock(hwnd);

              if(WinOpenClipbrd(hab)) {
                WinEmptyClipbrd(hab);
                WinCloseClipbrd(hab);
                if(ClipMonHwnd)
                  PostMsg(ClipMonHwnd,
                          UM_REFRESH,
                          MPVOID,
                          MPVOID);
              }
            }
            break;
          case IDM_BITINFO2:
          case IDM_BITINFO:
            {
              HAB               hab = WinQueryAnchorBlock(hwnd);
              BITMAPINFOHEADER2 bmp2;
              HBITMAP           hBitmap;
              ULONG             which = (SHORT1FROMMP(mp1) == IDM_BITINFO) ?
                                         CF_BITMAP : CF_DSPBITMAP;

              if(WinOpenClipbrd(hab)) {
                hBitmap = (HBITMAP)WinQueryClipbrdData(hab,which);
                if(hBitmap) {
                  memset(&bmp2,0,sizeof(bmp2));
                  bmp2.cbFix = sizeof(bmp2);
                  if(GpiQueryBitmapInfoHeader(hBitmap,
                                              &bmp2)) {

                    char s[128];

                    WinCloseClipbrd(hab);
                    sprintf(s,
                            "Size: %ldx%ld\r\r%lu colors\r\r%lu bytes",
                            bmp2.cx,
                            bmp2.cy,
                            (1 << bmp2.cBitCount),
                            ((((((bmp2.cBitCount * bmp2.cx) + 31) / 32) *
                             4) * bmp2.cy) * bmp2.cPlanes) + sizeof(bmp2) +
                              ((bmp2.cBitCount == 24) ? 0 : (1 << bmp2.cBitCount)));
                    WinMessageBox(HWND_DESKTOP,
                                  hwnd,
                                  s,
                                  "Bitmap information:",
                                  0,
                                  MB_ENTER | MB_MOVEABLE | MB_ICONASTERISK);
                    return 0;
                  }
                }
                WinCloseClipbrd(hab);
              }
            }
            break;
          case IDM_SAVEMET:
          case IDM_SAVEMET2:
          case IDM_SAVEBIT:
          case IDM_SAVEBIT2:
          case IDM_SAVECLIP:
          case IDM_SAVECLIP2:
            {
              FILEDLG       fdlg;
              char          drive[3] = " :",*pdrive = drive,*p,*lastfile;
              ULONG         bd = 0;

              lastfile = (SHORT1FROMMP(mp1) == IDM_SAVECLIP ||
                          SHORT1FROMMP(mp1) == IDM_SAVECLIP2) ? lasttext :
                         (SHORT1FROMMP(mp1) == IDM_SAVEBIT ||
                          SHORT1FROMMP(mp1) == IDM_SAVEBIT2) ? lastbmp : lastmet;
              memset(&fdlg,0,sizeof(FILEDLG));
              fdlg.cbSize =       (ULONG)sizeof(FILEDLG);
              fdlg.fl     =       FDS_CENTER | FDS_OPEN_DIALOG;
              fdlg.pszTitle =     "Select file to receive clipboard:";
              fdlg.pszOKButton =  "Okay";
              if(*lastfile &&
                 !IsInValidDir(lastfile))
                *lastfile = 0;
              if(isalpha(*lastfile))
                *drive = toupper(*lastfile);
              else {
                if(!DosQuerySysInfo(QSV_BOOT_DRIVE,
                                    QSV_BOOT_DRIVE,
                                    (PVOID)&bd,
                                    sizeof(bd)))
                  *drive = bd + '@';
                else
                  *drive = 'C';
              }
              fdlg.pszIDrive = pdrive;
              strcpy(fdlg.szFullFile,lastfile);
              p = strrchr(fdlg.szFullFile,'\\');
              if(p) {
                p++;
                *p = 0;
              }
              else
                strcat(fdlg.szFullFile,"\\");
              strcat(fdlg.szFullFile,"*");
              if(*lastfile) {
                p = strrchr(lastfile,'\\');
                if(p) {
                  p = strrchr(p,'.');
                  if(p) {
                    if(*(p + 1))
                      strcat(fdlg.szFullFile,p);
                  }
                }
              }
              if(SHORT1FROMMP(mp1) == IDM_SAVEBIT ||
                 SHORT1FROMMP(mp1) == IDM_SAVEBIT2) {
                p = strrchr(fdlg.szFullFile,'.');
                if(!p)
                  p = fdlg.szFullFile + strlen(fdlg.szFullFile);
                strcpy(p,".BMP");
              }
              else if(SHORT1FROMMP(mp1) == IDM_SAVEMET ||
                      SHORT1FROMMP(mp1) == IDM_SAVEMET2) {
                p = strrchr(fdlg.szFullFile,'.');
                if(!p)
                  p = fdlg.szFullFile + strlen(fdlg.szFullFile);
                strcpy(p,".MET");
              }
              else if(!strchr(fdlg.szFullFile,'.'))
                strcat(fdlg.szFullFile,".TXT");
              if(WinFileDlg(HWND_DESKTOP,
                            hwnd,
                            &fdlg)) {
                if(fdlg.lReturn != DID_CANCEL &&
                   !fdlg.lSRC) {

                  char *temp;

                  if(IsFile(fdlg.szFullFile) > 0) {

                    PMB2INFO pmb2;
                    ULONG    rt;
                    char     s[80];

                    sprintf(s,
                            "This file exists.  Do you want to overwrite%sit?",
                            (SHORT1FROMMP(mp1) == IDM_SAVECLIP ||
                             SHORT1FROMMP(mp1) == IDM_SAVECLIP2) ?
                             " or append to " : " ");
                    pmb2 = malloc(sizeof(MB2INFO) + (sizeof(MB2D) * 2));
                    if(pmb2) {
                      memset(pmb2,
                             0,
                             sizeof(MB2INFO) + (sizeof(MB2D) * 2));
                      pmb2->cb = sizeof(MB2INFO) + (sizeof(MB2D) * 2);
                      pmb2->cButtons = (SHORT1FROMMP(mp1) == IDM_SAVECLIP ||
                                        SHORT1FROMMP(mp1) == IDM_SAVECLIP2) ?
                                       3 : 2;
                      pmb2->hIcon = WinLoadPointer(HWND_DESKTOP,
                                                   0,
                                                   CLIP_FRAME);
                      pmb2->flStyle = MB_MOVEABLE | MB_CUSTOMICON;
                      strcpy(pmb2->mb2d[0].achText,"Cancel");
                      pmb2->mb2d[0].idButton = DID_CANCEL;
                      pmb2->mb2d[0].flStyle = BS_PUSHBUTTON | BS_DEFAULT;
                      strcpy(pmb2->mb2d[1].achText,"~Overwrite");
                      pmb2->mb2d[1].idButton = 101;
                      pmb2->mb2d[1].flStyle = BS_PUSHBUTTON;
                      strcpy(pmb2->mb2d[2].achText,"~Append");
                      pmb2->mb2d[2].idButton = 102;
                      pmb2->mb2d[2].flStyle = BS_PUSHBUTTON;
                      rt = WinMessageBox2(HWND_DESKTOP,
                                          hwnd,
                                          s,
                                          fdlg.szFullFile,
                                          10,
                                          pmb2);
                      WinDestroyPointer(pmb2->hIcon);
                      free(pmb2);
                      if(rt != 101 && rt != 102)
                        break;
                      if(rt == 102) {
                        switch(SHORT1FROMMP(mp1)) {
                          case IDM_SAVECLIP:
                            mp1 = MPFROM2SHORT(IDM_APPENDCLIP,0);
                            break;
                          case IDM_SAVECLIP2:
                            mp1 = MPFROM2SHORT(IDM_APPENDCLIP2,0);
                            break;
                          default:
                            return 0;
                        }
                      }
                      _heapmin();
                    }
                    else
                      break;
                  }
                  strcpy(lastfile,fdlg.szFullFile);
                  if(SHORT1FROMMP(mp1) == IDM_SAVEBIT ||
                     SHORT1FROMMP(mp1) == IDM_SAVEBIT2) {
                    p = strrchr(lastfile,'.');
                    if(!p)
                      p = lastfile + strlen(lastfile);
                    strcpy(p,".BMP");
                  }
                  else if(SHORT1FROMMP(mp1) == IDM_SAVEMET ||
                          SHORT1FROMMP(mp1) == IDM_SAVEMET2) {
                    if(!strchr(lastfile,'.'))
                      strcat(lastfile,".MET");
                  }
                  else if(!strchr(lastfile,'.'))
                    strcat(lastfile,".TXT");
                  SavePrf(((lastfile == lasttext) ? "LastFile" :
                           (lastfile == lastbmp) ? "LastBmp" : "LastMet"),
                          lastfile,
                          strlen(lastfile));
                  temp = strdup(lastfile);
                  if(temp) {
                    if(!PostMsg(GObjectHwnd,
                                msg,
                                mp1,
                                MPFROMP(temp))) {
                      free(temp);
                      _heapmin();
                    }
                  }
                }
              }
            }
            break;
          case IDM_LOADMET:
          case IDM_LOADBIT:
          case IDM_LOADCLIP:
            {
              FILEDLG fdlg;
              char    drive[3] = " :",*pdrive = drive,*p,*lastfile;
              ULONG   bd = 0;

              lastfile = (SHORT1FROMMP(mp1) == IDM_LOADCLIP) ? lasttext :
                         (SHORT1FROMMP(mp1) == IDM_LOADBIT) ? lastbmp : lastmet;
              if(*lastfile &&
                 !IsInValidDir(lastfile))
                *lastfile = 0;
              memset(&fdlg,0,sizeof(FILEDLG));
              fdlg.cbSize =       (ULONG)sizeof(FILEDLG);
              fdlg.fl     =       FDS_CENTER | FDS_OPEN_DIALOG;
              fdlg.pszTitle =     "Select file to import to clipboard:";
              fdlg.pszOKButton =  "Okay";
              if(isalpha(*lastfile))
                *drive = toupper(*lastfile);
              else {
                if(!DosQuerySysInfo(QSV_BOOT_DRIVE,
                                    QSV_BOOT_DRIVE,
                                    (PVOID)&bd,
                                    sizeof(bd)))
                  *drive = bd + '@';
                else
                  *drive = 'C';
              }
              fdlg.pszIDrive = pdrive;
              strcpy(fdlg.szFullFile,lastfile);
              p = strrchr(fdlg.szFullFile,'\\');
              if(p) {
                p++;
                *p = 0;
              }
              else
                strcat(fdlg.szFullFile,"\\");
              strcat(fdlg.szFullFile,"*");
              if(*lastfile) {
                p = strrchr(lastfile,'\\');
                if(p) {
                  p = strrchr(p,'.');
                  if(p) {
                    if(*(p + 1))
                      strcat(fdlg.szFullFile,p);
                  }
                }
              }
              if(SHORT1FROMMP(mp1) == IDM_LOADBIT) {
                p = strrchr(fdlg.szFullFile,'.');
                if(!p)
                  p = fdlg.szFullFile + strlen(fdlg.szFullFile);
                strcpy(p,".BMP");
              }
              else if(SHORT1FROMMP(mp1) == IDM_LOADMET) {
                if(!strchr(fdlg.szFullFile,'.'))
                  strcat(fdlg.szFullFile,".MET");
              }
              else if(!strchr(fdlg.szFullFile,'.'))
                strcat(fdlg.szFullFile,".TXT");
              if(WinFileDlg(HWND_DESKTOP,
                            hwnd,
                            &fdlg)) {
                if(fdlg.lReturn != DID_CANCEL &&
                   !fdlg.lSRC) {

                  char *temp;

                  strcpy(lastfile,fdlg.szFullFile);
                  if(SHORT1FROMMP(mp1) == IDM_LOADBIT) {
                    p = strrchr(lastfile,'.');
                    if(!p)
                      p = lastfile + strlen(lastfile);
                    strcpy(p,".BMP");
                  }
                  else if(SHORT1FROMMP(mp1) == IDM_LOADMET) {
                    if(!strchr(lastfile,'.'))
                      strcat(lastfile,".MET");
                  }
                  else if(!strchr(lastfile,'.'))
                    strcat(lastfile,".TXT");
                  SavePrf(((lastfile == lasttext) ? "LastFile" :
                           (lastfile == lastbmp) ? "LastBmp" : "LastMet"),
                          lastfile,
                          strlen(lastfile));
                  temp = strdup(lastfile);
                  if(temp) {
                    if(!PostMsg(GObjectHwnd,
                                msg,
                                mp1,
                                MPFROMP(temp))) {
                      free(temp);
                      _heapmin();
                    }
                  }
                }
              }
            }
            break;
          case IDM_SHOWTEXT2:
            showText2 = (showText2) ? FALSE : TRUE;
            SavePrf("ShowText2",
                    &showText2,
                    sizeof(BOOL));
            PostMsg(hwnd,
                    UM_SIZE,
                    MPVOID,
                    MPVOID);
            break;
          case IDM_SHOWBIT2:
            showBit2 = (showBit2) ? FALSE : TRUE;
            SavePrf("ShowBit2",
                    &showBit2,
                    sizeof(BOOL));
            PostMsg(hwnd,
                    UM_SIZE,
                    MPVOID,
                    MPVOID);
            break;
          case IDM_SHOWMET:
            showMet = (showMet) ? FALSE : TRUE;
            SavePrf("ShowMet",
                    &showMet,
                    sizeof(BOOL));
            PostMsg(hwnd,
                    UM_SIZE,
                    MPVOID,
                    MPVOID);
            break;
          case IDM_SHOWMET2:
            showMet2 = (showMet2) ? FALSE : TRUE;
            SavePrf("ShowMet2",
                    &showMet2,
                    sizeof(BOOL));
            PostMsg(hwnd,
                    UM_SIZE,
                    MPVOID,
                    MPVOID);
            break;
          default:
            if(SHORT1FROMMP(mp1) > IDM_CLIPTEXT &&
               SHORT1FROMMP(mp1) < IDM_CLIPTEXT + 1 + numtext) {
              /* is there text in this entry? */
              if(cliptext[SHORT1FROMMP(mp1) - (IDM_CLIPTEXT + 1)]) {
                /* yep, got text in it */
                if((WinGetKeyState(HWND_DESKTOP,VK_CTRL) & 0x8000) != 0) {

                  /* delete this entry */
                  int x,y;

                  y = SHORT1FROMMP(mp1) - (IDM_CLIPTEXT + 1);
                  free(cliptext[y]);
                  cliptext[y] = NULL;
                  for(x = y;x < numtext - 1;x++)
                    cliptext[x] = cliptext[x + 1];
                  if(nexttext > y)
                    nexttext--;
                  if(thistext > y)
                    thistext--;
                  break;
                }
                if(WinOpenClipbrd(WinQueryAnchorBlock(hwnd))) {
                  /* place this entry into the clipboard */

                  char *hold = NULL;
                  ULONG len;

                  len = strlen(cliptext[SHORT1FROMMP(mp1) - (IDM_CLIPTEXT + 1)]);
                  if(len) {
                    if(!DosAllocSharedMem((PPVOID)&hold,
                                          (PSZ)NULL,
                                          len + 2,
                                          PAG_COMMIT | OBJ_GIVEABLE |
                                          PAG_READ | PAG_WRITE)) {
                      TakeClipboard();
                      strcpy(hold,
                             cliptext[SHORT1FROMMP(mp1) - (IDM_CLIPTEXT + 1)]);
                      notnow = TRUE;
                      if(!WinSetClipbrdData(WinQueryAnchorBlock(hwnd),
                                            (ULONG)hold,
                                            CF_TEXT,
                                            CFI_POINTER))
                        DosFreeMem(hold);
                    }
                  }
                  WinCloseClipbrd(WinQueryAnchorBlock(hwnd));
                }
              }
              notnow = FALSE;
            }
            break;
        }
        if(refocus)
          WinSetFocus(HWND_DESKTOP,
                      hwnd);
      }
      return 0;

    case WM_SAVEAPPLICATION:
      {
        SWP swp;

        if(WinQueryWindowPos(WinQueryWindow(hwnd,QW_PARENT),&swp)) {
          if((swp.fl & (SWP_HIDE | SWP_MINIMIZE | SWP_MAXIMIZE)) != 0)
            LoadRestorePos(&swp,
                           WinQueryWindow(hwnd,QW_PARENT));
          SavePrf("ClipView.Pos",
                  &swp,
                  sizeof(SWP));
        }
      }
      break;

    case WM_CLOSE:
      WinSendMsg(hwnd,
                 WM_SAVEAPPLICATION,
                 MPVOID,
                 MPVOID);
      WinSendMsg(WinQueryWindow(hwnd,QW_PARENT),
                 WM_SYSCOMMAND,
                 MPFROM2SHORT(SC_MINIMIZE,0),
                 MPVOID);
      if(cliptext &&
         fClipSave)
        WinSendMsg(hwnd,
                   UM_CALC,
                   MPVOID,
                   MPVOID);
      return 0;

    case WM_DESTROY:
      if(ClipMonHwnd)
        PostMsg(ClipMonHwnd,
                WM_TIMER,
                MPVOID,
                MPVOID);
      ClipHwnd = (HWND)0;
      if(WinOpenClipbrd(WinQueryAnchorBlock(hwnd))) {
        if(WinQueryClipbrdViewer(WinQueryAnchorBlock(hwnd)) == hwnd)
          WinSetClipbrdViewer(WinQueryAnchorBlock(hwnd),(HWND)0);
        WinCloseClipbrd(WinQueryAnchorBlock(hwnd));
      }
      if(cliptext) {

        int x,z = 0;

        if(fClipSave)
          DosCreateDir(".\\CLIPS",NULL);
        for(x = 0;x < numtext;x++) {
          if(cliptext[x] &&
             fClipSave) {

            char  s[80];
            FILE *fp;

            sprintf(s,".\\CLIPS\\CLIPBRD.%03x",z);
            fp = fopen(s,"wb");
            if(fp) {
              fwrite(cliptext[x],
                     1,
                     strlen(cliptext[x]),fp);
              fclose(fp);
              z++;
            }
          }
          if(cliptext[x])
            free(cliptext[x]);
        }
        free(cliptext);
        cliptext = NULL;
        thistext = nexttext = 0;
        _heapmin();
      }
      break;
  }
  return WinDefWindowProc(hwnd,msg,mp1,mp2);
}

