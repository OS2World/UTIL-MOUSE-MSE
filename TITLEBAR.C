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


static void FreeSchemes (void) {

  TBARPARMS *info,*next;

  DosEnterCritSec();
   info = tbHead;
   tbHead = NULL;
   while(info) {
     next = info->next;
     free(info->schemename);
     if(info->bitmapname)
       free(info->bitmapname);
     if(info->hbm)
       GpiDeleteBitmap(info->hbm);
     free(info);
     info = next;
   }
  DosExitCritSec();
}


static void WriteSchemes (void) {

  TBARPARMS *info;
  TBARDISK   tb;
  FILE      *fp;

  fp = fopen("TTLBARS.DAT","wb");
  if(fp) {
    info = tbHead;
    while(info) {
      memset(&tb,0,sizeof(TBARDISK));
      tb.size = sizeof(TBARDISK);
      strcpy(tb.schemename,info->schemename);
      if(info->bitmapname)
        strcpy(tb.bitmapname,info->bitmapname);
      tb.isprogname = info->isprogname;
      tb.istitletext = info->istitletext;
      tb.redStart = info->redStart;
      tb.greenStart = info->greenStart;
      tb.blueStart = info->blueStart;
      tb.redEnd = info->redEnd;
      tb.greenEnd = info->greenEnd;
      tb.blueEnd = info->blueEnd;
      tb.sSteps = info->sSteps;
      tb.vGrade = info->vGrade;
      tb.cGrade = info->cGrade;
      tb.tBorder = info->tBorder;
      tb.exclude = info->exclude;
      tb.fTexture = info->fTexture;
      fwrite(&tb,sizeof(tb),1,fp);
      info = info->next;
    }
    fclose(fp);
  }
}


void ReadSchemes (void) {

  TBARPARMS *info,*last = NULL;
  TBARDISK   tb;
  FILE      *fp;

  FreeSchemes();
  fp = fopen("TTLBARS.DAT","rb");
  if(fp) {
    DosEnterCritSec();
     while(!feof(fp)) {
       if(fread(&tb,sizeof(tb),1,fp) != 1)
         break;
       if(tb.size != sizeof(TBARDISK)) {
         fclose(fp);
         DosDelete("TTLBARS.DAT");
         DosExitCritSec();
       }
       info = malloc(sizeof(TBARPARMS));
       if(info) {
         memset(info,0,sizeof(TBARPARMS));
         info->size = sizeof(TBARPARMS);
         info->schemename = strdup(tb.schemename);
         if(info->schemename) {
           if(*tb.bitmapname)
             info->bitmapname = strdup(tb.bitmapname);
           info->isprogname = tb.isprogname;
           info->istitletext = tb.istitletext;
           info->redStart = tb.redStart;
           info->greenStart = tb.greenStart;
           info->blueStart = tb.blueStart;
           info->redEnd = tb.redEnd;
           info->greenEnd = tb.greenEnd;
           info->blueEnd = tb.blueEnd;
           info->redText = tb.redText;
           info->greenText = tb.greenText;
           info->blueText = tb.blueText;
           info->sSteps = tb.sSteps;
           info->sSteps = min(255,max(16,info->sSteps));
           info->vGrade = tb.vGrade;
           info->cGrade = tb.cGrade;
           info->tBorder = tb.tBorder;
           info->exclude = tb.exclude;
           info->fTexture = tb.fTexture;
           if(!tbHead)
             tbHead = info;
           else
             last->next = info;
           last = info;
         }
         else
           free(info);
       }
     }
    DosExitCritSec();
    fclose(fp);
  }
}


void RedrawTitlebars (BOOL all,HWND hwndTop) {

  HENUM        henum;
  HWND         hwndChild;
  static char  Class[9];

  henum = WinBeginEnumWindows(hwndTop);
  while((hwndChild = WinGetNextWindow(henum)) != NULLHANDLE) {
    WinQueryClassName(hwndChild,
                      sizeof(Class),
                      (PCH)Class);
    if(!strcmp(Class,"#9")) {
      WinSendMsg(hwndChild,
                 TBM_QUERYHILITE,
                 MPVOID,
                 MPVOID);
      WinInvalidateRect(hwndChild,
                        NULL,
                        FALSE);
    }
    else {
#ifdef TESTING
      if(all) {
        if(!strcmp(Class,"#1") ||
           !strcmp(Class,"wpFolder") ||
           !strcmp(Class,"#4"))
          WinSendMsg(hwndChild,
                     TBM_QUERYHILITE,
                     MPVOID,
                     MPVOID);
      }
#endif
      RedrawTitlebars(all,
                      hwndChild);
    }
  }
  WinEndEnumWindows(henum);
}


static void GetVars (HWND hwnd,UCHAR *redS,UCHAR *greenS,UCHAR *blueS,
                     UCHAR *redE,UCHAR *greenE,UCHAR *blueE,
                     UCHAR *redT,UCHAR *greenT,UCHAR *blueT,
                     UCHAR *Steps,UCHAR *vG,UCHAR *cG,UCHAR *tB,
                     UCHAR *fT) {

  *vG = (UCHAR)WinQueryButtonCheckstate(hwnd,
                                        TTL_VERTICAL);
  *cG = (UCHAR)WinQueryButtonCheckstate(hwnd,
                                        TTL_CENTERED);
  *tB = (UCHAR)WinQueryButtonCheckstate(hwnd,
                                        TTL_BORDER);
  *fT = (UCHAR)WinQueryButtonCheckstate(hwnd,
                                        TTL_TEXTURE);
  {
    char s[12];

    WinQueryDlgItemText(hwnd,
                        TTL_REDENTRY,
                        sizeof(s),
                        s);
    *redS = (UCHAR)atoi(s);
    WinQueryDlgItemText(hwnd,
                        TTL_GREENENTRY,
                        sizeof(s),
                        s);
    *greenS = (UCHAR)atoi(s);
    WinQueryDlgItemText(hwnd,
                        TTL_BLUEENTRY,
                        sizeof(s),
                        s);
    *blueS = (UCHAR)atoi(s);
    WinQueryDlgItemText(hwnd,
                        TTL_REDEENTRY,
                        sizeof(s),
                        s);
    *redE = (UCHAR)atoi(s);
    WinQueryDlgItemText(hwnd,
                        TTL_GREENEENTRY,
                        sizeof(s),
                        s);
    *greenE = (UCHAR)atoi(s);
    WinQueryDlgItemText(hwnd,
                        TTL_BLUEEENTRY,
                        sizeof(s),
                        s);
    *blueE = (UCHAR)atoi(s);
    WinSendDlgItemMsg(hwnd,
                      TTL_STEPS,
                      SPBM_QUERYVALUE,
                      MPFROMP(s),
                      MPFROM2SHORT(sizeof(s),
                                   SPBQ_ALWAYSUPDATE));
    *Steps = (UCHAR)atoi(s);
    *Steps = max(16,min(*Steps,255));
  }
  /* temporary */
  *redT = redText;
  *greenT = greenText;
  *blueT = blueText;
}


MRESULT EXPENTRY FakeTitleProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_BUTTON1CLICK:
      PostMsg(hwnd,
              UM_COMMAND,
              MPVOID,
              MPVOID);
      return MRFROMSHORT(TRUE);

    case UM_COMMAND:
      {
        ULONG ret;
        char  s[256];

        ret = WinQuerySysValue(HWND_DESKTOP,
                               SV_CYTITLEBAR);
        sprintf(s,
                "Titlebars are %lu pixels tall.\r\r"
                "For minimum memory usage and speed of "
                "loading, trim bitmaps used for titlebars "
                "with MSE to something around this height.",
                ret);
        WinMessageBox(HWND_DESKTOP,
                      WinQueryWindow(hwnd,QW_PARENT),
                      s,
                      "FYI",
                      0,
                      MB_ENTER | MB_ICONASTERISK | MB_MOVEABLE);
      }
      return 0;

    case WM_PAINT:
      {
        HPS   hps;
        RECTL cliprcl;

        hps = WinBeginPaint(hwnd,
                            (HPS)0,
                            &cliprcl);
        if(hps) {

          UCHAR   redS,greenS,blueS,
                  redE,greenE,blueE,
                  redT,greenT,blueT,
                  Steps,tB,vG,cG,fT;
          long    tColor;
          HBITMAP hbm = (HBITMAP)0;

          GpiCreateLogColorTable(hps,
                                 0,
                                 LCOLF_RGB,
                                 0,
                                 0,
                                 0);
          GetVars(WinQueryWindow(hwnd,QW_PARENT),
                  &redS,
                  &greenS,
                  &blueS,
                  &redE,
                  &greenE,
                  &blueE,
                  &redT,
                  &greenT,
                  &blueT,
                  &Steps,
                  &vG,
                  &cG,
                  &tB,
                  &fT);
          if(WinQueryButtonCheckstate(WinQueryWindow(hwnd,QW_PARENT),
                                      TTL_USEBITMAP))
            hbm = WinQueryWindowULong(WinQueryWindow(hwnd,QW_PARENT),0);
          tColor = (redS << 16) + (greenS << 8) + blueS;
          WinSetPresParam(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                                          TTL_STARTCOLOR),
                          PP_FOREGROUNDCOLOR,
                          sizeof(tColor),
                          (PVOID)&tColor);
          tColor = (redE << 16) + (greenE << 8) + blueE;
          WinSetPresParam(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                                          TTL_ENDCOLOR),
                          PP_FOREGROUNDCOLOR,
                          sizeof(tColor),
                          (PVOID)&tColor);
          PaintGradientWindow(hwnd,
                              hps,
                              &cliprcl,
                              TRUE,
                              redS,
                              greenS,
                              blueS,
                              redE,
                              greenE,
                              blueE,
                              redT,
                              greenT,
                              blueT,
                              Steps,
                              vG,
                              cG,
                              tB,
                              fT,
                              hbm);
        }
      }
      return 0;
  }
  return PFNWPStatic(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY ColorBlockProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_PAINT:
      {
        HPS hps;

        hps = WinBeginPaint(hwnd,
                            (HPS)0,
                            NULL);
        if(hps) {

          long   tcolor;
          RECTL  rcl;
          POINTL ptl;

          GpiCreateLogColorTable(hps,
                                 0,
                                 LCOLF_RGB,
                                 0,
                                 0,
                                 0);
          WinQueryWindowRect(hwnd,
                             &rcl);
          if(!WinQueryPresParam(hwnd,
                                PP_FOREGROUNDCOLOR,
                                0,
                                NULL,
                                sizeof(LONG),
                                (PVOID)&tcolor,
                                QPF_PURERGBCOLOR))
            tcolor = WinQuerySysColor(HWND_DESKTOP,
                                      SYSCLR_WINDOWFRAME,
                                      0);
          ptl.x = 0;
          ptl.y = 0;
          GpiMove(hps,
                  &ptl);
          ptl.x = rcl.xRight;
          ptl.y = rcl.yTop;
          GpiSetColor(hps,
                      tcolor);
          GpiBox(hps,
                 DRO_FILL,
                 &ptl,
                 0,
                 0);
          WinEndPaint(hps);
        }
      }
      return 0;
  }
  return PFNWPStatic(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY SetTitleDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  HBITMAP     hbm;
  static char lastbmp[CCHMAXPATH] = "";

  switch(msg) {
    case WM_INITDLG:
      if(!*lastbmp) {

        ULONG size;

        size = sizeof(lastbmp);
        PrfQueryProfileData(prof,
                            appname,
                            "Titlebar.LastBmp",
                            lastbmp,
                            &size);
      }
      WinSubclassWindow(WinWindowFromID(hwnd,TTL_FAKETITLEBAR),
                        (PFNWP)FakeTitleProc);
      WinSubclassWindow(WinWindowFromID(hwnd,TTL_STARTCOLOR),
                        (PFNWP)ColorBlockProc);
      WinSubclassWindow(WinWindowFromID(hwnd,TTL_ENDCOLOR),
                        (PFNWP)ColorBlockProc);
      WinSendDlgItemMsg(hwnd,
                        TTL_REDENTRY,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(3,0),
                        MPVOID);
      WinSendDlgItemMsg(hwnd,
                        TTL_GREENENTRY,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(3,0),
                        MPVOID);
      WinSendDlgItemMsg(hwnd,
                        TTL_BLUEENTRY,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(3,0),
                        MPVOID);
      WinSendDlgItemMsg(hwnd,
                        TTL_REDEENTRY,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(3,0),
                        MPVOID);
      WinSendDlgItemMsg(hwnd,
                        TTL_GREENEENTRY,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(3,0),
                        MPVOID);
      WinSendDlgItemMsg(hwnd,
                        TTL_BLUEEENTRY,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(3,0),
                        MPVOID);
      WinSendDlgItemMsg(hwnd,
                        TTL_STEPS,
                        SPBM_SETLIMITS,
                        MPFROMSHORT(255),
                        MPFROMSHORT(16));
      WinSendDlgItemMsg(hwnd,
                        TTL_STEPS,
                        SPBM_SETTEXTLIMIT,
                        MPFROMSHORT(3),
                        MPVOID);
      WinSendDlgItemMsg(hwnd,
                        TTL_SCHEMENAME,
                        EM_SETTEXTLIMIT,
                        MPFROMSHORT(CCHMAXPATH),
                        MPVOID);
      WinSendDlgItemMsg(hwnd,
                        TTL_BITMAPNAME,
                        EM_SETTEXTLIMIT,
                        MPFROMSHORT(CCHMAXPATH),
                        MPVOID);
      {
        TBARPARMS *info;
        SHORT      x;

        info = tbHead;
        while(info) {
          x = (SHORT)WinSendDlgItemMsg(hwnd,
                                       TTL_LISTBOX,
                                       LM_INSERTITEM,
                                       MPFROM2SHORT(LIT_END,0),
                                       MPFROMP(info->schemename));
          if(x >= 0)
            WinSendDlgItemMsg(hwnd,
                              TTL_LISTBOX,
                              LM_SETITEMHANDLE,
                              MPFROMSHORT(x),
                              MPFROMP(info));
          info = info->next;
        }
      }
      PostMsg(hwnd,
              WM_COMMAND,
              MPFROM2SHORT(TTL_UNDO,0),
              MPVOID);
      break;

    case UM_SETPRESPARMS:
    case WM_PRESPARAMCHANGED:
      {
        long tcolor = 0,add = 0;
        char s[12];

        if(msg == UM_SETPRESPARMS ||
           WinQueryPresParam(hwnd,
                             (ULONG)mp1,
                             0,
                             NULL,
                             sizeof(long),
                             (PVOID)&tcolor,
                             QPF_PURERGBCOLOR)) {
          if(msg == UM_SETPRESPARMS)
            tcolor = (ULONG)mp2;
          if((ULONG)mp1 == PP_FOREGROUNDCOLOR ||
             (ULONG)mp1 == PP_BACKGROUNDCOLOR) {
            switch((ULONG)mp1) {
              case PP_BACKGROUNDCOLOR:
                add = TTL_REDESLIDER - TTL_REDSLIDER;
                WinSetPresParam(WinWindowFromID(hwnd,TTL_ENDCOLOR),
                                PP_FOREGROUNDCOLOR,
                                sizeof(tcolor),
                                (PVOID)&tcolor);
                break;
              case PP_FOREGROUNDCOLOR:
                WinSetPresParam(WinWindowFromID(hwnd,TTL_STARTCOLOR),
                                PP_FOREGROUNDCOLOR,
                                sizeof(tcolor),
                                (PVOID)&tcolor);
                break;
            }
            sprintf(s,
                    "%d",
                    ((tcolor & 0x00ff0000) >> 16));
            WinSetDlgItemText(hwnd,
                              TTL_REDENTRY + add,
                              s);
            sprintf(s,
                    "%d",
                    ((tcolor & 0x0000ff00) >> 8));
            WinSetDlgItemText(hwnd,
                              TTL_GREENENTRY + add,
                              s);
            sprintf(s,
                    "%d",
                    (tcolor & 0x000000ff));
            WinSetDlgItemText(hwnd,
                              TTL_BLUEENTRY + add,
                              s);
            WinSendDlgItemMsg(hwnd,
                              TTL_REDSLIDER + add,
                              SLM_SETSLIDERINFO,
                              MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                           SMA_INCREMENTVALUE),
                              MPFROMLONG((tcolor & 0x00ff0000) >> 16));
            WinSendDlgItemMsg(hwnd,
                              TTL_GREENSLIDER + add,
                              SLM_SETSLIDERINFO,
                              MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                           SMA_INCREMENTVALUE),
                              MPFROMLONG((tcolor & 0x0000ff00) >> 8));
            WinSendDlgItemMsg(hwnd,
                              TTL_BLUESLIDER + add,
                              SLM_SETSLIDERINFO,
                              MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                           SMA_INCREMENTVALUE),
                              MPFROMLONG(tcolor & 0x000000ff));
          }
          if(msg == WM_PRESPARAMCHANGED)
            WinRemovePresParam(hwnd,
                               (ULONG)mp1);
        }
      }
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case TTL_LISTBOX:
          switch(SHORT2FROMMP(mp1)) {
            case LN_ENTER:
              {
                TBARPARMS *info;
                SHORT      x;
                long       tB;
                char       s[12];

                x = (SHORT)WinSendDlgItemMsg(hwnd,
                                             TTL_LISTBOX,
                                             LM_QUERYSELECTION,
                                             MPFROMSHORT(LIT_FIRST),
                                             MPVOID);
                if(x >= 0) {
                  info = (TBARPARMS *)WinSendDlgItemMsg(hwnd,
                                                        TTL_LISTBOX,
                                                        LM_QUERYITEMHANDLE,
                                                        MPFROMSHORT(x),
                                                        MPVOID);
                  if(info) {
                    WinSetDlgItemText(hwnd,
                                      TTL_SCHEMENAME,
                                      info->schemename);
                    WinCheckButton(hwnd,
                                   TTL_SCHEMEPROG,
                                   info->isprogname);
                    WinCheckButton(hwnd,
                                   TTL_SCHEMETITLE,
                                   info->istitletext);
                    WinCheckButton(hwnd,
                                   TTL_EXCLUDE,
                                   info->exclude);
                    sprintf(s,
                            "%d",
                            info->redStart);
                    WinSetDlgItemText(hwnd,
                                      TTL_REDENTRY,
                                      s);
                    sprintf(s,
                            "%d",
                            info->greenStart);
                    WinSetDlgItemText(hwnd,
                                      TTL_GREENENTRY,
                                      s);
                    sprintf(s,
                            "%d",
                            info->blueStart);
                    WinSetDlgItemText(hwnd,
                                      TTL_BLUEENTRY,
                                      s);
                    WinSendDlgItemMsg(hwnd,
                                      TTL_REDSLIDER,
                                      SLM_SETSLIDERINFO,
                                      MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                                   SMA_INCREMENTVALUE),
                                      MPFROMSHORT(info->redStart));
                    WinSendDlgItemMsg(hwnd,
                                      TTL_GREENSLIDER,
                                      SLM_SETSLIDERINFO,
                                      MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                                   SMA_INCREMENTVALUE),
                                      MPFROMSHORT(info->greenStart));
                    WinSendDlgItemMsg(hwnd,
                                      TTL_BLUESLIDER,
                                      SLM_SETSLIDERINFO,
                                      MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                                   SMA_INCREMENTVALUE),
                                      MPFROMSHORT(info->blueStart));
                    sprintf(s,
                            "%d",
                            info->redEnd);
                    WinSetDlgItemText(hwnd,
                                      TTL_REDEENTRY,
                                      s);
                    sprintf(s,
                            "%d",
                            info->greenEnd);
                    WinSetDlgItemText(hwnd,
                                      TTL_GREENEENTRY,
                                      s);
                    sprintf(s,
                            "%d",
                            info->blueEnd);
                    WinSetDlgItemText(hwnd,
                                      TTL_BLUEEENTRY,
                                      s);
                    WinSendDlgItemMsg(hwnd,
                                      TTL_REDESLIDER,
                                      SLM_SETSLIDERINFO,
                                      MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                                   SMA_INCREMENTVALUE),
                                      MPFROMSHORT(info->redEnd));
                    WinSendDlgItemMsg(hwnd,
                                      TTL_GREENESLIDER,
                                      SLM_SETSLIDERINFO,
                                      MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                                   SMA_INCREMENTVALUE),
                                      MPFROMSHORT(info->greenEnd));
                    WinSendDlgItemMsg(hwnd,
                                      TTL_BLUEESLIDER,
                                      SLM_SETSLIDERINFO,
                                      MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                                   SMA_INCREMENTVALUE),
                                      MPFROMSHORT(info->blueEnd));
                    WinSendDlgItemMsg(hwnd,
                                      TTL_STEPS,
                                      SPBM_SETCURRENTVALUE,
                                      MPFROMSHORT(info->sSteps),
                                      MPVOID);
                    WinCheckButton(hwnd,
                                   TTL_VERTICAL,
                                   info->vGrade);
                    WinCheckButton(hwnd,
                                   TTL_CENTERED,
                                   info->cGrade);
                    WinCheckButton(hwnd,
                                   TTL_TEXTURE,
                                   info->fTexture);
                    tB = (info->tBorder == 0) ?
                          2 :
                          (info->tBorder == 1) ?
                           0 :
                           1;
                    WinCheckButton(hwnd,
                                   TTL_BORDER,
                                   tB);
                    WinSendMsg(hwnd,
                               WM_CONTROL,
                               MPFROM2SHORT(TTL_BORDER,BN_CLICKED),
                               MPFROMLONG(WinWindowFromID(hwnd,TTL_BORDER)));
                    WinSetDlgItemText(hwnd,
                                      TTL_BITMAPNAME,
                                      info->bitmapname);
                    if(info->bitmapname &&
                       *info->bitmapname)
                      WinCheckButton(hwnd,
                                     TTL_USEBITMAP,
                                     TRUE);
                    WinSendMsg(hwnd,
                               WM_CONTROL,
                               MPFROM2SHORT(TTL_BITMAPNAME,EN_KILLFOCUS),
                               MPVOID);
                  }
                }
              }
              break;
          }
          break;

        case TTL_EXCLUDE:
        case TTL_TEXTURE:
        case TTL_SCHEMEPROG:
        case TTL_SCHEMETITLE:
        case TTL_VERTICAL:
        case TTL_CENTERED:
        case TTL_BORDER:
        case TTL_USEBITMAP:
          switch(SHORT2FROMMP(mp1)) {
            case BN_CLICKED:
              {
                long tB = WinQueryButtonCheckstate(hwnd,
                                                   SHORT1FROMMP(mp1));

                switch(SHORT1FROMMP(mp1)) {
                  case TTL_BORDER:
                    {
                      char *p = "No ~border";

                      tB = (tB == 0) ? 1 : (tB == 1) ? 2 : 0;
                      switch(tB) {
                        case 1:
                          p = "\"Innie\" ~border";
                          break;
                        case 2:
                          p = "\"Outtie\" ~border";
                          break;
                      }
                      WinSetDlgItemText(hwnd,
                                        TTL_BORDER,
                                        p);
                    }
                    break;

                  case TTL_USEBITMAP:
                    tB = (tB == 0) ? 1 : 0;
                    PostMsg(hwnd,
                            WM_CONTROL,
                            MPFROM2SHORT(TTL_BITMAPNAME,EN_KILLFOCUS),
                            MPVOID);
                    break;

                  case TTL_EXCLUDE:
                  case TTL_VERTICAL:
                  case TTL_CENTERED:
                  case TTL_TEXTURE:
                    tB = (tB == 0) ? 1 : 0;
                    break;
                  case TTL_SCHEMEPROG:
                    tB = (tB == 0) ? 1 : 0;
                    if(tB)
                      WinCheckButton(hwnd,
                                     TTL_SCHEMETITLE,
                                     FALSE);
                    break;
                  case TTL_SCHEMETITLE:
                    tB = (tB == 0) ? 1 : 0;
                    if(tB)
                      WinCheckButton(hwnd,
                                     TTL_SCHEMEPROG,
                                     FALSE);
                    break;
                }
                WinCheckButton(hwnd,
                               SHORT1FROMMP(mp1),
                               tB);
              }
              WinInvalidateRect(WinWindowFromID(hwnd,TTL_FAKETITLEBAR),
                                NULL,
                                FALSE);
              break;
          }
          break;

        case TTL_REDENTRY:
        case TTL_GREENENTRY:
        case TTL_BLUEENTRY:
        case TTL_REDEENTRY:
        case TTL_GREENEENTRY:
        case TTL_BLUEEENTRY:
          switch(SHORT2FROMMP(mp1)) {
            case EN_KILLFOCUS:
              {
                char s[12];
                long ret;

                WinQueryDlgItemText(hwnd,
                                    SHORT1FROMMP(mp1),
                                    sizeof(s),
                                    s);
                ret = atol(s);
                WinSendDlgItemMsg(hwnd,
                                  SHORT1FROMMP(mp1) - 1,
                                  SLM_SETSLIDERINFO,
                                  MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                               SMA_INCREMENTVALUE),
                                  MPFROMLONG(ret));
                WinInvalidateRect(WinWindowFromID(hwnd,TTL_FAKETITLEBAR),
                                  NULL,
                                  FALSE);

              }
              break;
          }
          break;

        case TTL_REDSLIDER:
        case TTL_GREENSLIDER:
        case TTL_BLUESLIDER:
        case TTL_REDESLIDER:
        case TTL_GREENESLIDER:
        case TTL_BLUEESLIDER:
          switch(SHORT2FROMMP(mp1)) {
            case SLN_CHANGE:
            case SLN_SLIDERTRACK:
              {
                char  s[12];
                ULONG ret;

                ret = (ULONG)WinSendDlgItemMsg(hwnd,
                                               SHORT1FROMMP(mp1),
                                               SLM_QUERYSLIDERINFO,
                                               MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                                            SMA_INCREMENTVALUE),
                                               MPVOID);
                sprintf(s,
                        "%lu",
                        ret);
                WinSetDlgItemText(hwnd,
                                  SHORT1FROMMP(mp1) + 1,
                                  s);
                WinInvalidateRect(WinWindowFromID(hwnd,TTL_FAKETITLEBAR),
                                  NULL,
                                  FALSE);
              }
              break;
          }
          break;

        case TTL_BITMAPNAME:
          switch(SHORT2FROMMP(mp1)) {
            case EN_KILLFOCUS:
              {
                char s[CCHMAXPATH];

                *s = 0;
                WinQueryDlgItemText(hwnd,
                                    TTL_BITMAPNAME,
                                    sizeof(s),
                                    s);
                if(*s) {

                  char fullname[CCHMAXPATH];

                  if(!DosQueryPathInfo(s,
                                       FIL_QUERYFULLNAME,
                                       fullname,
                                       sizeof(fullname))) {
                    strcpy(s,fullname);
                    WinSetDlgItemText(hwnd,
                                      TTL_BITMAPNAME,
                                      s);
                  }
                }
                hbm = WinQueryWindowULong(hwnd,0);
                if(hbm) {
                  GpiDeleteBitmap(hbm);
                  hbm = (HBITMAP)0;
                }
                if(*s) {

                  HBITMAP hbm2;

                  hbm = LoadBitmap(s);
                  hbm2 = TrimBitmap(WinQueryAnchorBlock(hwnd),
                                    hbm);
                  if(hbm2)
                    hbm = hbm2;
                }
                WinSetWindowULong(hwnd,0,hbm);
                if(WinQueryButtonCheckstate(hwnd,
                                            TTL_USEBITMAP))
                  WinInvalidateRect(WinWindowFromID(hwnd,TTL_FAKETITLEBAR),
                                    NULL,
                                    FALSE);
              }
              break;
          }
          break;

        case TTL_STEPS:
          switch(SHORT2FROMMP(mp1)) {
            case SPBN_ENDSPIN:
            case SPBN_KILLFOCUS:
              {
                char s[12];

                if(WinSendDlgItemMsg(hwnd,
                                     TTL_STEPS,
                                     SPBM_QUERYVALUE,
                                     MPFROMP(s),
                                     MPFROM2SHORT(sizeof(s),
                                                  SPBQ_ALWAYSUPDATE)))
                  WinInvalidateRect(WinWindowFromID(hwnd,TTL_FAKETITLEBAR),
                                    NULL,
                                    FALSE);
              }
              break;
          }
          break;
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case TTL_INVERTGRAD:
          {
            char  s[12];
            long  tB;
            UCHAR redS,greenS,blueS,
                  redE,greenE,blueE,
                  redT,greenT,blueT,
                  Steps,tB2,vG,cG,fT;

            GetVars(hwnd,
                    &redS,
                    &greenS,
                    &blueS,
                    &redE,
                    &greenE,
                    &blueE,
                    &redT,
                    &greenT,
                    &blueT,
                    &Steps,
                    &vG,
                    &cG,
                    &tB2,
                    &fT);
            sprintf(s,
                    "%hu",
                    redE);
            WinSetDlgItemText(hwnd,
                              TTL_REDENTRY,
                              s);
            sprintf(s,
                    "%hu",
                    greenE);
            WinSetDlgItemText(hwnd,
                              TTL_GREENENTRY,
                              s);
            sprintf(s,
                    "%hu",
                    blueE);
            WinSetDlgItemText(hwnd,
                              TTL_BLUEENTRY,
                              s);
            WinSendDlgItemMsg(hwnd,
                              TTL_REDSLIDER,
                              SLM_SETSLIDERINFO,
                              MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                           SMA_INCREMENTVALUE),
                              MPFROMSHORT(redE));
            WinSendDlgItemMsg(hwnd,
                              TTL_GREENSLIDER,
                              SLM_SETSLIDERINFO,
                              MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                           SMA_INCREMENTVALUE),
                              MPFROMSHORT(greenE));
            WinSendDlgItemMsg(hwnd,
                              TTL_BLUESLIDER,
                              SLM_SETSLIDERINFO,
                              MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                           SMA_INCREMENTVALUE),
                              MPFROMSHORT(blueE));
            sprintf(s,
                    "%hu",
                    redS);
            WinSetDlgItemText(hwnd,
                              TTL_REDEENTRY,
                              s);
            sprintf(s,
                    "%hu",
                    greenS);
            WinSetDlgItemText(hwnd,
                              TTL_GREENEENTRY,
                              s);
            sprintf(s,
                    "%hu",
                    blueS);
            WinSetDlgItemText(hwnd,
                              TTL_BLUEEENTRY,
                              s);
            WinSendDlgItemMsg(hwnd,
                              TTL_REDESLIDER,
                              SLM_SETSLIDERINFO,
                              MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                           SMA_INCREMENTVALUE),
                              MPFROMSHORT(redS));
            WinSendDlgItemMsg(hwnd,
                              TTL_GREENESLIDER,
                              SLM_SETSLIDERINFO,
                              MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                           SMA_INCREMENTVALUE),
                              MPFROMSHORT(greenS));
            WinSendDlgItemMsg(hwnd,
                              TTL_BLUEESLIDER,
                              SLM_SETSLIDERINFO,
                              MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                           SMA_INCREMENTVALUE),
                              MPFROMSHORT(blueS));
            WinInvalidateRect(WinWindowFromID(hwnd,TTL_FAKETITLEBAR),
                              NULL,
                              FALSE);
          }
          break;

        case TTL_FINDBITMAP:
          {
            FILEDLG     fdlg;
            char        drive[3] = " :",*pdrive = drive,*p;
            ULONG       drv = 0;

            memset(&fdlg,
                   0,
                   sizeof(FILEDLG));
            fdlg.cbSize =       (ULONG)sizeof(FILEDLG);
            fdlg.fl     =       FDS_CENTER | FDS_OPEN_DIALOG;
            fdlg.pszTitle =     "Select a bitmap file";
            fdlg.pszOKButton =  "Okay";

            if(isalpha(*lastbmp))
              *drive = toupper(*lastbmp);
            else {
              if(!DosQuerySysInfo(QSV_BOOT_DRIVE,
                                  QSV_BOOT_DRIVE,
                                  (PVOID)&drv,
                                  sizeof(drv)))
                *drive = drv + '@';
              else
                *drive = 'C';
            }
            fdlg.pszIDrive = pdrive;
            strcpy(fdlg.szFullFile,lastbmp);
            p = strrchr(fdlg.szFullFile,'\\');
            if(p) {
              p++;
              *p = 0;
            }
            else
              strcat(fdlg.szFullFile,"\\");
            strcat(fdlg.szFullFile,"*.BMP");
            if(WinFileDlg(HWND_DESKTOP,
                          hwnd,
                          &fdlg)) {
              if(fdlg.lReturn != DID_CANCEL &&
                 !fdlg.lSRC) {
                WinSetDlgItemText(hwnd,
                                  TTL_BITMAPNAME,
                                  fdlg.szFullFile);
                strcpy(lastbmp,
                       fdlg.szFullFile);
                SavePrf("Titlebar.LastBmp",
                        lastbmp,
                        strlen(lastbmp));
                WinSendMsg(hwnd,
                           WM_CONTROL,
                           MPFROM2SHORT(TTL_BITMAPNAME,EN_KILLFOCUS),
                           MPVOID);
              }
            }
          }
          break;

        case TTL_UP:
        case TTL_DOWN:
          {
            TBARPARMS *info,*last,*temp;
            SHORT      x,y;

            DosEnterCritSec();
            x = (SHORT)WinSendDlgItemMsg(hwnd,
                                         TTL_LISTBOX,
                                         LM_QUERYSELECTION,
                                         MPFROMSHORT(LIT_FIRST),
                                         MPVOID);
            if(x >= 0) {
              info = (TBARPARMS *)WinSendDlgItemMsg(hwnd,
                                                    TTL_LISTBOX,
                                                    LM_QUERYITEMHANDLE,
                                                    MPFROMSHORT(x),
                                                    MPVOID);
              if(info) {
                y = ((SHORT1FROMMP(mp1) == TTL_UP) ? x - 1 : x + 1);
                if(y >= 0) {
                  last = (TBARPARMS *)WinSendDlgItemMsg(hwnd,
                                                        TTL_LISTBOX,
                                                        LM_QUERYITEMHANDLE,
                                                        MPFROMSHORT(y),
                                                        MPVOID);
                  if(last) {
                    WinSendDlgItemMsg(hwnd,
                                      TTL_LISTBOX,
                                      LM_SETITEMTEXT,
                                      MPFROMSHORT(x),
                                      MPFROMP(last->schemename));
                    WinSendDlgItemMsg(hwnd,
                                      TTL_LISTBOX,
                                      LM_SETITEMTEXT,
                                      MPFROMSHORT(y),
                                      MPFROMP(info->schemename));
                    WinSendDlgItemMsg(hwnd,
                                      TTL_LISTBOX,
                                      LM_SETITEMHANDLE,
                                      MPFROMSHORT(x),
                                      MPFROMP(last));
                    WinSendDlgItemMsg(hwnd,
                                      TTL_LISTBOX,
                                      LM_SETITEMHANDLE,
                                      MPFROMSHORT(y),
                                      MPFROMP(info));
                    WinSendDlgItemMsg(hwnd,
                                      TTL_LISTBOX,
                                      LM_SELECTITEM,
                                      MPFROMSHORT(y),
                                      MPFROMSHORT(TRUE));
                    temp = tbHead;
                    if(y < x) {
                      while(temp && temp->next != last)
                        temp = temp->next;
                      if(temp)
                        temp->next = info;
                      last->next = info->next;
                      info->next = last;
                      if(tbHead == last)
                        tbHead = info;
                    }
                    else {
                      while(temp && temp->next != info)
                        temp = temp->next;
                      if(temp)
                        temp->next = last;
                      info->next = last->next;
                      last->next = info;
                      if(tbHead == info)
                        tbHead = last;
                    }
                    WriteSchemes();
                  }
                }
              }
            }
            DosExitCritSec();
          }
          break;

        case TTL_ADDSCHEME:
          {
            TBARPARMS *info,*last;
            char       s[CCHMAXPATH];
            SHORT      x;

            *s = 0;
            WinQueryDlgItemText(hwnd,
                                TTL_SCHEMENAME,
                                sizeof(s),
                                s);
            if(*s) {
              last = tbHead;
              while(last && last->next)
                last = last->next;
              info = malloc(sizeof(TBARPARMS));
              if(info) {
                memset(info,
                       0,
                       sizeof(TBARPARMS));
                info->size = sizeof(TBARPARMS);
                info->schemename = strdup(s);
                if(info->schemename) {
                  GetVars(hwnd,
                          &info->redStart,
                          &info->greenStart,
                          &info->blueStart,
                          &info->redEnd,
                          &info->greenEnd,
                          &info->blueEnd,
                          &info->redText,
                          &info->greenText,
                          &info->blueText,
                          &info->sSteps,
                          &info->vGrade,
                          &info->cGrade,
                          &info->tBorder,
                          &info->fTexture);
                  if(WinQueryButtonCheckstate(hwnd,
                                              TTL_USEBITMAP)) {
                    *s = 0;
                    WinQueryDlgItemText(hwnd,
                                        TTL_BITMAPNAME,
                                        sizeof(s),
                                        s);
                    info->bitmapname = strdup(s);
                  }
                  info->isprogname = (UCHAR)WinQueryButtonCheckstate(hwnd,
                                                                     TTL_SCHEMEPROG);
                  info->istitletext = (UCHAR)WinQueryButtonCheckstate(hwnd,
                                                                      TTL_SCHEMETITLE);
                  info->exclude = (UCHAR)WinQueryButtonCheckstate(hwnd,
                                                                  TTL_EXCLUDE);
                  x = (SHORT)WinSendDlgItemMsg(hwnd,
                                               TTL_LISTBOX,
                                               LM_INSERTITEM,
                                               MPFROM2SHORT(LIT_END,0),
                                               MPFROMP(info->schemename));
                  if(x >= 0)
                    WinSendDlgItemMsg(hwnd,
                                      TTL_LISTBOX,
                                      LM_SETITEMHANDLE,
                                      MPFROMSHORT(x),
                                      MPFROMP(info));
                  DosEnterCritSec();
                   if(last)
                     last->next = info;
                   else
                     tbHead = info;
                  DosExitCritSec();
                  WriteSchemes();
                }
                else
                  free(info);
              }
            }
          }
          break;

        case TTL_DELSCHEME:
          {
            TBARPARMS *info,*last;
            SHORT      x;

            x = (SHORT)WinSendDlgItemMsg(hwnd,
                                         TTL_LISTBOX,
                                         LM_QUERYSELECTION,
                                         MPFROMSHORT(LIT_FIRST),
                                         MPVOID);
            if(x >= 0) {
              info = (TBARPARMS *)WinSendDlgItemMsg(hwnd,
                                                    TTL_LISTBOX,
                                                    LM_QUERYITEMHANDLE,
                                                    MPFROMSHORT(x),
                                                    MPVOID);
              if(info) {
                WinSendDlgItemMsg(hwnd,
                                  TTL_LISTBOX,
                                  LM_DELETEITEM,
                                  MPFROMSHORT(x),
                                  MPVOID);

                DosEnterCritSec();
                 last = tbHead;
                 while(last && last->next != info)
                   last = last->next;
                 if(last)
                   last->next = info->next;
                 if(tbHead == info)
                   tbHead = info->next;
                 free(info->schemename);
                 if(info->bitmapname)
                   free(info->bitmapname);
                 if(info->hbm)
                   GpiDeleteBitmap(info->hbm);
                 free(info);
                DosExitCritSec();
                WriteSchemes();
              }
            }
          }
          break;

        case TTL_HELP:
          ViewHelp(hwnd,
                   "Titlebar enhancement parameters");
          break;

        case DID_OK:
          WinSendMsg(hwnd,
                     WM_COMMAND,
                     MPFROM2SHORT(TTL_APPLY,0),
                     MPVOID);
          WinDismissDlg(hwnd,0);
          break;

        case TTL_UNDO:
          {
            char s[12];
            long tB;

            sprintf(s,
                    "%hu",
                    redStart);
            WinSetDlgItemText(hwnd,
                              TTL_REDENTRY,
                              s);
            sprintf(s,
                    "%hu",
                    greenStart);
            WinSetDlgItemText(hwnd,
                              TTL_GREENENTRY,
                              s);
            sprintf(s,
                    "%hu",
                    blueStart);
            WinSetDlgItemText(hwnd,
                              TTL_BLUEENTRY,
                              s);
            WinSendDlgItemMsg(hwnd,
                              TTL_REDSLIDER,
                              SLM_SETSLIDERINFO,
                              MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                           SMA_INCREMENTVALUE),
                              MPFROMSHORT(redStart));
            WinSendDlgItemMsg(hwnd,
                              TTL_GREENSLIDER,
                              SLM_SETSLIDERINFO,
                              MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                           SMA_INCREMENTVALUE),
                              MPFROMSHORT(greenStart));
            WinSendDlgItemMsg(hwnd,
                              TTL_BLUESLIDER,
                              SLM_SETSLIDERINFO,
                              MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                           SMA_INCREMENTVALUE),
                              MPFROMSHORT(blueStart));
            sprintf(s,
                    "%hu",
                    redEnd);
            WinSetDlgItemText(hwnd,
                              TTL_REDEENTRY,
                              s);
            sprintf(s,
                    "%hu",
                    greenEnd);
            WinSetDlgItemText(hwnd,
                              TTL_GREENEENTRY,
                              s);
            sprintf(s,
                    "%hu",
                    blueEnd);
            WinSetDlgItemText(hwnd,
                              TTL_BLUEEENTRY,
                              s);
            WinSendDlgItemMsg(hwnd,
                              TTL_REDESLIDER,
                              SLM_SETSLIDERINFO,
                              MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                           SMA_INCREMENTVALUE),
                              MPFROMSHORT(redEnd));
            WinSendDlgItemMsg(hwnd,
                              TTL_GREENESLIDER,
                              SLM_SETSLIDERINFO,
                              MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                           SMA_INCREMENTVALUE),
                              MPFROMSHORT(greenEnd));
            WinSendDlgItemMsg(hwnd,
                              TTL_BLUEESLIDER,
                              SLM_SETSLIDERINFO,
                              MPFROM2SHORT(SMA_SLIDERARMPOSITION,
                                           SMA_INCREMENTVALUE),
                              MPFROMSHORT(blueEnd));
            WinSendDlgItemMsg(hwnd,
                              TTL_STEPS,
                              SPBM_SETCURRENTVALUE,
                              MPFROMSHORT(sSteps),
                              MPVOID);
            WinCheckButton(hwnd,
                           TTL_VERTICAL,
                           vGrade);
            WinCheckButton(hwnd,
                           TTL_CENTERED,
                           cGrade);
            tB = (tBorder == 0) ? 2 : (tBorder == 1) ? 0 : 1;
            WinCheckButton(hwnd,
                           TTL_BORDER,
                           tB);
            WinCheckButton(hwnd,
                           TTL_TEXTURE,
                           fTexture);
            WinSetDlgItemText(hwnd,
                              TTL_SCHEMENAME,
                              "");
            WinCheckButton(hwnd,
                           TTL_SCHEMEPROG,
                           0);
            WinCheckButton(hwnd,
                           TTL_SCHEMETITLE,
                           0);
            WinCheckButton(hwnd,
                           TTL_EXCLUDE,
                           0);
            WinSendMsg(hwnd,
                       WM_CONTROL,
                       MPFROM2SHORT(TTL_BORDER,BN_CLICKED),
                       MPFROMLONG(WinWindowFromID(hwnd,TTL_BORDER)));
          }
          WinCheckButton(hwnd,
                         TTL_USEBITMAP,
                         (*tBitmap != 0));
          WinSetDlgItemText(hwnd,
                            TTL_BITMAPNAME,
                            tBitmap);
          WinSendMsg(hwnd,
                     WM_CONTROL,
                     MPFROM2SHORT(TTL_BITMAPNAME,EN_KILLFOCUS),
                     MPVOID);
          WinInvalidateRect(WinWindowFromID(hwnd,TTL_FAKETITLEBAR),
                            NULL,
                            FALSE);
          break;

        case TTL_APPLY:
          WinSendMsg(hwnd,
                     WM_CONTROL,
                     MPFROM2SHORT(TTL_BITMAPNAME,EN_KILLFOCUS),
                     MPVOID);
          GetVars(hwnd,
                  &redStart,
                  &greenStart,
                  &blueStart,
                  &redEnd,
                  &greenEnd,
                  &blueEnd,
                  &redText,
                  &greenText,
                  &blueText,
                  &sSteps,
                  &vGrade,
                  &cGrade,
                  &tBorder,
                  &fTexture);
          if(hbmTitles)
            GpiDeleteBitmap(hbmTitles);
          hbmTitles = (HBITMAP)0;
          if(WinQueryButtonCheckstate(hwnd,
                                      TTL_USEBITMAP)) {
            hbm = WinQueryWindowULong(hwnd,0);
            hbmTitles = hbm;
          }
          hbm = (HBITMAP)0;
          WinSetWindowULong(hwnd,0,hbm);
          SavePrf("Titlebar.redS",
                  &redStart,
                  sizeof(redStart));
          SavePrf("Titlebar.redE",
                  &redEnd,
                  sizeof(redEnd));
          SavePrf("Titlebar.redT",
                  &redText,
                  sizeof(redText));
          SavePrf("Titlebar.greenS",
                  &greenStart,
                  sizeof(greenStart));
          SavePrf("Titlebar.greenE",
                  &greenEnd,
                  sizeof(greenEnd));
          SavePrf("Titlebar.greenT",
                  &greenText,
                  sizeof(greenText));
          SavePrf("Titlebar.blueS",
                  &blueStart,
                  sizeof(blueStart));
          SavePrf("Titlebar.blueE",
                  &blueEnd,
                  sizeof(blueEnd));
          SavePrf("Titlebar.blueT",
                  &blueText,
                  sizeof(blueText));
          SavePrf("Titlebar.steps",
                  &sSteps,
                  sizeof(sSteps));
          SavePrf("Titlebar.vgrade",
                  &vGrade,
                  sizeof(vGrade));
          SavePrf("Titlebar.cgrade",
                  &cGrade,
                  sizeof(cGrade));
          SavePrf("Titlebar.border",
                  &tBorder,
                  sizeof(tBorder));
          SavePrf("Titlebar.texture",
                  &fTexture,
                  sizeof(fTexture));
          *tBitmap = 0;
          if(WinQueryButtonCheckstate(hwnd,
                                      TTL_USEBITMAP))
            WinQueryDlgItemText(hwnd,
                                TTL_BITMAPNAME,
                                sizeof(tBitmap),
                                tBitmap);
          SavePrf("Titlebar.Bitmap",
                  &tBitmap,
                  sizeof(tBitmap));
          if(fEnhanceTitlebars)
            RedrawTitlebars(FALSE,HWND_DESKTOP);
          break;

        case DID_CANCEL:
          WinDismissDlg(hwnd,0);
          break;
      }
      return 0;

    case WM_DESTROY:
      hbm = WinQueryWindowULong(hwnd,0);
      if(hbm)
        GpiDeleteBitmap(hbm);
      hbm = (HBITMAP)0;
      WinSetWindowULong(hwnd,0,hbm);
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}

