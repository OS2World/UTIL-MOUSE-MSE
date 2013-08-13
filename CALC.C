#define INCL_DOS
#define INCL_WIN
#define INCL_GPI

#define DATAC 1

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "msehook.h"
#include "mse.h"
#include "dialog.h"

static double clcmem;


MRESULT EXPENTRY CalcSubProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_CHAR:
      if(!(SHORT1FROMMP(mp1) & KC_KEYUP) &&
         (SHORT1FROMMP(mp1) & KC_CHAR))
        PostMsg(WinQueryWindow(WinQueryWindow(hwnd,QW_PARENT),QW_PARENT),
                   msg,
                   mp1,
                   mp2);
      break;
  }
  return ((PFNWP)WinQueryWindowPtr(hwnd,0))(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY CalcProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  static double lastnum;
  static USHORT lastop;
  static char   s[4096];

  switch(msg) {
    case WM_INITDLG:
      if(CalcHwnd) {
        NormalizeWindow(CalcHwnd,TRUE);
        WinDismissDlg(hwnd,0);
        break;
      }
      CalcHwnd = hwnd;
      lastnum = 0.0;
      lastop = 0;
      WinSetWindowPtr(WinWindowFromID(WinWindowFromID(hwnd,CLC_RESULTS),
                      CBID_EDIT),0,
                      (PVOID)WinSubclassWindow(WinWindowFromID(
                                               WinWindowFromID(hwnd,
                                                               CLC_RESULTS),
                                                               CBID_EDIT),
                                               (PFNWP)CalcSubProc));
      break;

    case WM_CHAR:
      if(SHORT1FROMMP(mp1) & KC_CHAR) {
        switch(SHORT1FROMMP(mp2)) {
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            PostMsg(hwnd,
                    WM_COMMAND,
                    MPFROM2SHORT(CLC_FRAME + 1 +
                                 (SHORT1FROMMP(mp2) - '0'),0),
                    MPVOID);
            break;

          case 'x':
            *s = 0;
            WinQueryDlgItemText(hwnd,
                                CLC_RESULTS,
                                4000,
                                s);
            if(!strcmp(s,"0")) {
              strcat(s,"x");
              WinSetDlgItemText(hwnd,
                                CLC_RESULTS,
                                s);
            }
            break;

          case 'a':
          case 'A':
          case 'b':
          case 'B':
          case 'c':
          case 'C':
          case 'D':
          case 'd':
          case 'e':
          case 'E':
          case 'f':
          case 'F':
            *s = 0;
            WinQueryDlgItemText(hwnd,
                                CLC_RESULTS,
                                4000,
                                s);
            if(!strncmp(s,"0x",2)) {
              s[strlen(s) + 1] = 0;
              s[strlen(s)] = tolower(SHORT1FROMMP(mp2));
              WinSetDlgItemText(hwnd,
                                CLC_RESULTS,
                                s);
            }
            break;

          case '\b':
            *s = 0;
            WinQueryDlgItemText(hwnd,
                                CLC_RESULTS,
                                4000,
                                s);
            if(*s) {
              s[strlen(s) - 1] = 0;
              WinSetDlgItemText(hwnd,
                                CLC_RESULTS,
                                s);
            }
            break;

          case '.':
          case '+':
          case '-':
          case '\r':
          case '=':
          case '/':
          case '*':
            {
              USHORT cmd;

              switch(SHORT1FROMMP(mp2)) {
                case '.':
                  cmd = CLC_DECIMAL;
                  break;
                case '+':
                  cmd = CLC_ADD;
                  break;
                case '-':
                  cmd = CLC_SUB;
                  break;
                case '\r':
                case '=':
                  cmd = CLC_EQ;
                  break;
                case '/':
                  cmd = CLC_DIV;
                  break;
                case '*':
                  cmd = CLC_MUL;
                  break;
              }
              PostMsg(hwnd,
                      WM_COMMAND,
                      MPFROM2SHORT(cmd,0),
                      MPVOID);
            }
            break;
        }
      }
      else if(SHORT1FROMMP(mp1) & KC_VIRTUALKEY) {

        USHORT cmd = 0;

        switch(SHORT2FROMMP(mp2)) {
          case VK_F2:
            cmd = CLC_MCLR;
            break;
          case VK_F3:
            cmd = CLC_MREC;
            break;
          case VK_F4:
            cmd = CLC_MADD;
            break;
          case VK_F5:
            cmd = CLC_MSUB;
            break;
          case VK_F6:
            cmd = CLC_MMUL;
            break;
          case VK_F7:
            cmd = CLC_MDIV;
            break;
          case VK_INSERT:
            cmd = CLC_PASTE;
            break;
          case VK_DELETE:
            cmd = CLC_COPY;
            break;
          case VK_F8:
            cmd = CLC_CLR;
            break;
          case VK_F9:
            cmd = CLC_ECLR;
            break;
          case VK_BACKSPACE:
            *s = 0;
            WinQueryDlgItemText(hwnd,
                                CLC_RESULTS,
                                4000,
                                s);
            if(*s) {
              s[strlen(s) - 1] = 0;
              WinSetDlgItemText(hwnd,
                                CLC_RESULTS,
                                s);
            }
            break;
        }
        if(cmd)
          PostMsg(hwnd,
                  WM_COMMAND,
                  MPFROM2SHORT(cmd,0),
                  MPVOID);
      }
      break;

    case WM_CONTROL:
      return 0;

    case WM_COMMAND:
      if(SHORT1FROMMP(mp1) == DID_CANCEL)
        WinDismissDlg(hwnd,0);
      else {

        double sd = 0;

        *s = 0;
        WinQueryDlgItemText(hwnd,
                            CLC_RESULTS,
                            4000,
                            s);
        if(*s &&
           strncmp(s,"0x",2))
          sd = _atold(s);
        else if(*s) {

          char *p;

          sd = (double)strtoul(s,&p,0);
        }
        switch(SHORT1FROMMP(mp1)) {
          case CLC_CLR:
            lastnum = 0.0;
            lastop = 0;
            WinSetDlgItemText(hwnd,
                              CLC_HELP,
                              "");
            /* intentional fallthru */
          case CLC_ECLR:
            WinSetDlgItemText(hwnd,
                              CLC_RESULTS,
                              "");
            break;

          case CLC_ADD:
          case CLC_SUB:
          case CLC_MUL:
          case CLC_DIV:
            if(lastop) {
              WinSendMsg(hwnd,
                         WM_COMMAND,
                         MPFROM2SHORT(CLC_EQ,0),
                         MPVOID);
              *s = 0;
              WinQueryDlgItemText(hwnd,
                                  CLC_RESULTS,
                                  4000,
                                  s);
              if(*s &&
                 strncmp(s,"0x",2))
                sd = _atold(s);
              else if(*s) {

                char *p;

                sd = (double)strtoul(s,&p,0);
              }
            }
            if(!*s) {
              DosBeep(250,100);
              break;
            }
            if(sd == _LHUGE_VAL) {
              DosBeep(50,100);
              WinSetDlgItemText(hwnd,
                                CLC_HELP,
                                "Bad input.");
              break;
            }
            lastnum = sd;
            lastop = SHORT1FROMMP(mp1);
            sprintf(s,
                    "%lf",
                    sd);
            if(strchr(s,'.'))
              stripzero(s);
            switch(lastop) {
              case CLC_ADD:
                strcat(s," [+]");
                break;
              case CLC_SUB:
                strcat(s," [-]");
                break;
              case CLC_MUL:
                strcat(s," [*]");
                break;
              case CLC_DIV:
                strcat(s," [/]");
                break;
            }
            WinSetDlgItemText(hwnd,
                              CLC_HELP,
                              s);
            WinSetDlgItemText(hwnd,
                              CLC_RESULTS,
                              "");
            break;

          case CLC_EQ:
            if(!*s) {
              DosBeep(250,100);
              break;
            }
            if(sd == _LHUGE_VAL) {
              DosBeep(50,100);
              WinSetDlgItemText(hwnd,
                                CLC_HELP,
                                "Bad input");
              break;
            }
            WinSetDlgItemText(hwnd,
                              CLC_HELP,
                              "");
            switch(lastop) {
              case CLC_ADD:
                sd += lastnum;
                break;
              case CLC_SUB:
                sd = lastnum - sd;
                break;
              case CLC_MUL:
                sd *= lastnum;
                break;
              case CLC_DIV:
                if(sd != 0.0)
                  sd = lastnum / sd;
                else {
                  DosBeep(50,100);
                  WinSetDlgItemText(hwnd,
                                    CLC_HELP,
                                    "Divide by zero error");
                  return 0;
                }
                break;
            }
            if(sd != _LHUGE_VAL) {
              sprintf(s,
                      "%lf",
                      sd);
              if(strchr(s,'.'))
                stripzero(s);
              WinSetDlgItemText(hwnd,
                                CLC_RESULTS,
                                s);
              if((SHORT)WinSendDlgItemMsg(hwnd,
                                          CLC_RESULTS,
                                          LM_INSERTITEM,
                                          MPFROM2SHORT(LIT_END,0),
                                          MPFROMP(s)) < 0) {
                WinSendDlgItemMsg(hwnd,
                                  CLC_RESULTS,
                                  LM_DELETEITEM,
                                  MPFROM2SHORT(0,0),
                                  MPVOID);
                WinSendDlgItemMsg(hwnd,
                                  CLC_RESULTS,
                                  LM_DELETEITEM,
                                  MPFROM2SHORT(0,0),
                                  MPVOID);
                WinSendDlgItemMsg(hwnd,
                                  CLC_RESULTS,
                                  LM_DELETEITEM,
                                  MPFROM2SHORT(0,0),
                                  MPVOID);
                WinSendDlgItemMsg(hwnd,
                                  CLC_RESULTS,
                                  LM_INSERTITEM,
                                  MPFROM2SHORT(LIT_END,0),
                                  MPFROMP(s));
              }
            }
            else {
              WinSetDlgItemText(hwnd,
                                CLC_HELP,
                                "Overflow");
              DosBeep(50,100);
            }
            lastop = 0;
            break;

          case CLC_DECIMAL:
            if(!strchr(s,'.')) {
              strcat(s,".");
              WinSetDlgItemText(hwnd,
                                CLC_RESULTS,
                                s);
            }
            else
              DosBeep(250,100);
            break;

          case CLC_MCLR:
            clcmem = 0.0;
            break;

          case CLC_MADD:
          case CLC_MSUB:
          case CLC_MMUL:
          case CLC_MDIV:
            if(sd == _LHUGE_VAL) {
              DosBeep(50,100);
              WinSetDlgItemText(hwnd,
                                CLC_HELP,
                                "Bad input");
              break;
            }
            switch(SHORT1FROMMP(mp1)) {
              case CLC_MADD:
                clcmem += sd;
                break;
              case CLC_MSUB:
                clcmem -= sd;
                break;
              case CLC_MMUL:
                clcmem *= sd;
                break;
              case CLC_MDIV:
                if(sd != 0.0)
                  clcmem /= sd;
                else {
                  DosBeep(50,100);
                  WinSetDlgItemText(hwnd,
                                    CLC_HELP,
                                    "Memory divide by zero error");
                  return 0;
                }
                break;
            }
            if(clcmem == _LHUGE_VAL) {
              clcmem = 0.0;
              DosBeep(50,100);
              WinSetDlgItemText(hwnd,
                                CLC_HELP,
                                "Memory overflow");
            }
            break;

          case CLC_MREC:
            sprintf(s,"%lf",clcmem);
            if(strchr(s,'.'))
              stripzero(s);
            WinSetDlgItemText(hwnd,
                              CLC_RESULTS,
                              s);
            break;

          case CLC_PASTE:
            if(WinOpenClipbrd(WinQueryAnchorBlock(hwnd))) {

              char *p = (char *)WinQueryClipbrdData(WinQueryAnchorBlock(hwnd),
                                                    CF_TEXT);

              if(p && *p)
                WinSetDlgItemText(hwnd,
                                  CLC_RESULTS,
                                  p);
              WinCloseClipbrd(WinQueryAnchorBlock(hwnd));
            }
            break;

          case CLC_COPY:
            if(WinOpenClipbrd(WinQueryAnchorBlock(hwnd))) {

              char *hold = NULL;
              long  len;

              len = WinQueryWindowTextLength(WinWindowFromID(hwnd,CLC_RESULTS));
              if(len) {
                if(!DosAllocSharedMem((PPVOID)&hold,
                                      (PSZ)NULL,
                                      len + 1,
                                      PAG_COMMIT | OBJ_GIVEABLE |
                                      PAG_READ | PAG_WRITE)) {
                  TakeClipboard();
                  if(WinQueryDlgItemText(hwnd,
                                         CLC_RESULTS,
                                         len + 1,
                                         hold))
                    WinSetClipbrdData(WinQueryAnchorBlock(hwnd),
                                      (ULONG)hold,
                                      CF_TEXT,
                                      CFI_POINTER);
                  else
                    DosFreeMem(hold);
                }
              }
              WinCloseClipbrd(WinQueryAnchorBlock(hwnd));
            }
            break;

          default:
            if(SHORT1FROMMP(mp1) > CLC_FRAME &&
               SHORT1FROMMP(mp1) < CLC_FRAME + 11) {
              s[strlen(s) + 1] = 0;
              s[strlen(s)] = '0' + (SHORT1FROMMP(mp1) - (CLC_FRAME + 1));
              WinSetDlgItemText(hwnd,
                                CLC_RESULTS,
                                s);
            }
            break;
        }
      }
      return 0;

    case WM_DESTROY:
      if(CalcHwnd == hwnd)
        CalcHwnd = (HWND)0;
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}

