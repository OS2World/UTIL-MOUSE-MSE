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


char * strip_trail_char (char *strip,char *a) {

  register char *p;

  if(a &&
     *a &&
     strip &&
     *strip) {
    p = &a[strlen(a) - 1];
    while (*a &&
           strchr(strip,*p) != NULL) {
      *p = 0;
      p--;
    }
  }
  return a;
}


char * strip_lead_char (char *strip,char *a) {

  register char *p = a;

  if(a &&
     *a &&
     strip &&
     *strip) {
    while(*p &&
          strchr(strip,*p) != NULL)
      p++;
    if(p != a)
      memmove(a,
              p,
              strlen(p) + 1);
  }
  return a;
}


BOOL IsInValidDir (char *name) {

  char *p,was;
  int   ret;

  if(IsFile(name) == -1) {
    p = strrchr(name,'\\');
    if(p) {
      if(p < name + 2)
        return TRUE;
      was = *p;
      ret = IsFile(name);
      *p = was;
      return (ret == 0);
    }
    return FALSE;
  }
  return TRUE;
}


void SavePrf (char *name,void *data,ULONG size) {

  if(prof)
    PrfWriteProfileData(prof,
                        appname,
                        name,
                        data,
                        size);
}


void LoadPrf (void) {

  ULONG size;

  if(IsFile("mse.ini") < 0)
    firsttime = TRUE;
  prof = PrfOpenProfile((HAB)0,
                        "mse.ini");
  if(prof) {
    size = sizeof(cmds);
    PrfQueryProfileData(prof,
                        appname,
                        "Commands",
                        cmds,
                        &size);
    size = sizeof(crnrcmds);
    PrfQueryProfileData(prof,
                        appname,
                        "CornerCmds",
                        crnrcmds,
                        &size);
    {
      int x;

      for(x = 0;x < 4;x++) {
        if(crnrcmds[x] > CR_NONE && crnrcmds[x] <= CR_MAXCMDS)
          break;
      }
      if(x < 4)
        fUsingCorners = TRUE;
    }
    fConfirmExit = TRUE;
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "ConfirmExit",
                        &fConfirmExit,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "LowMemory",
                        &fLowMemory,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "AutoDropCombos",
                        &fAutoDropCombos,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "MSEInTaskList",
                        &fMSEInTaskList,
                        &size);
    size = sizeof(long);
    PrfQueryProfileData(prof,
                        appname,
                        "MaxRunahead",
                        &lMaxRunahead,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "RememberDirs",
                        &fRememberDirs,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "RememberFiles",
                        &fRememberFiles,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "AggressiveFileDlg",
                        &fAggressive,
                        &size);
    fConfirmFileDel = fConfirmDirDel = TRUE;
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "ConfirmFileDel",
                        &fConfirmFileDel,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "ConfirmDirDel",
                        &fConfirmDirDel,
                        &size);
    *viewer = 0;
    size = sizeof(viewer);
    PrfQueryProfileData(prof,
                        appname,
                        "Viewer",
                        viewer,
                        &size);
    *editor = 0;
    size = sizeof(editor);
    PrfQueryProfileData(prof,
                        appname,
                        "Editor",
                        editor,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "SlidingFocus",
                        &fSlidingFocus,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "NoZOrder",
                        &fNoZorderChange,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "WrapMouse",
                        &fWrapMouse,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "MoveMouse",
                        &fMoveMouse,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "CenterDlg",
                        &fCenterDlg,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "DefButton",
                        &fDefButton,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "NoTasklist",
                        &fNoTasklist,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "Disabled",
                        &fDisabled,
                        &size);
    if(fDisabled)
      fNoTasklist = FALSE;
    if(DosQuerySysInfo(QSV_BOOT_DRIVE,
                       QSV_BOOT_DRIVE,
                       (PVOID)&ulDriveMon,
                       sizeof(ulDriveMon)))
      ulDriveMon = 'C' - '@';
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "Fractions",
                        &fFractions,
                        &size);
    size = sizeof(ULONG);
    PrfQueryProfileData(prof,
                        appname,
                        "MonDrive",
                        &ulDriveMon,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "ShowHard",
                        &showhardmon,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "ShowFreeInMenus",
                        &fShowFreeInMenus,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "ShowMem",
                        &fShowMem,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "ShowTasks",
                        &fShowTasks,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "ShowPMem",
                        &fShowPMem,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "ShowSwapFree",
                        &fShowSwapFree,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "CPUMon",
                        &fCPUMon,
                        &size);
    MilliSecs = 1000;
    size = sizeof(ULONG);
    PrfQueryProfileData(prof,
                        appname,
                        "MilliSecs",
                        &MilliSecs,
                        &size);
    MilliSecs = max(100,MilliSecs);
    MilliSecs = min(999999,MilliSecs);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "ShowClock",
                        &showclock,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "ShowAmpm",
                        &ampm,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "ShowDate",
                        &showdate,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "Average",
                        &fAverage,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "NoMonClick",
                        &fNoMonClick,
                        &size);
    size = sizeof(ULONG);
    PrfQueryProfileData(prof,
                        appname,
                        "MouseMins",
                        &ulMouseMins,
                        &size);
    size = sizeof(ULONG);
    PrfQueryProfileData(prof,
                        appname,
                        "Delay",
                        &ulDelay,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "ScreenSave",
                        &fScreenSave,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "ActiveSave",
                        &fActiveSave,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "EnhanceFileDlg",
                        &fEnhanceFileDlg,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "EnhanceTitlebars",
                        &fEnhanceTitlebars,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "ClockInTitlebars",
                        &fClockInTitlebars,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "EnableTitlebarMenus",
                        &fEnableTitlebarMenus,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "ExtraMenus",
                        &fExtraMenus,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "Desktops",
                        &fDesktops,
                        &size);
    fVirtText = TRUE;
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "VirtText",
                        &fVirtText,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "VirtWPS",
                        &fVirtWPS,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "NoNormalizeOnExit",
                        &fNoNormalize,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "ShiftBump",
                        &fShiftBump,
                        &size);
    size = sizeof(long);
    PrfQueryProfileData(prof,
                        appname,
                        "BumpDesks",
                        &lBumpDesks,
                        &size);
    if(fWrapMouse &&
       lBumpDesks)
      fShiftBump = TRUE;
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "Chords",
                        &fChords,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "ClipMon",
                        &fClipMon,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "ClipAppend",
                        &fClipAppend,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "SwapMon",
                        &fSwapMon,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "SwapFloat",
                        &fSwapFloat,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "UseClipMgr",
                        &fUseClipMgr,
                        &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(prof,
                        appname,
                        "SaveClips",
                        &fClipSave,
                        &size);
    size = sizeof(USHORT);
    PrfQueryProfileData(prof,
                        appname,
                        "ScrnBits",
                        &scrnbitcount,
                        &size);
    *tBitmap = 0;
    size = sizeof(tBitmap);
    PrfQueryProfileData(prof,
                        appname,
                        "Titlebar.Bitmap",
                        tBitmap,
                        &size);
    if(*tBitmap) {

      HBITMAP hbm;

      if(hbmTitles)
        GpiDeleteBitmap(hbmTitles);
      hbmTitles = LoadBitmap(tBitmap);
      hbm = TrimBitmap((HAB)0,
                       hbmTitles);
      if(hbm)
        hbmTitles = hbm;
    }
    size = sizeof(UCHAR);
    PrfQueryProfileData(prof,
                        appname,
                        "Titlebar.redS",
                        &redStart,
                        &size);
    size = sizeof(UCHAR);
    PrfQueryProfileData(prof,
                        appname,
                        "Titlebar.redE",
                        &redEnd,
                        &size);
    size = sizeof(UCHAR);
    PrfQueryProfileData(prof,
                        appname,
                        "Titlebar.redT",
                        &redText,
                        &size);
    size = sizeof(UCHAR);
    PrfQueryProfileData(prof,
                        appname,
                        "Titlebar.greenS",
                        &greenStart,
                        &size);
    size = sizeof(UCHAR);
    PrfQueryProfileData(prof,
                        appname,
                        "Titlebar.greenE",
                        &greenEnd,
                        &size);
    size = sizeof(UCHAR);
    PrfQueryProfileData(prof,
                        appname,
                        "Titlebar.greenT",
                        &greenText,
                        &size);
    size = sizeof(UCHAR);
    PrfQueryProfileData(prof,
                        appname,
                        "Titlebar.blueS",
                        &blueStart,
                        &size);
    size = sizeof(UCHAR);
    PrfQueryProfileData(prof,
                        appname,
                        "Titlebar.blueE",
                        &blueEnd,
                        &size);
    size = sizeof(UCHAR);
    PrfQueryProfileData(prof,
                        appname,
                        "Titlebar.blueT",
                        &blueText,
                        &size);
    size = sizeof(UCHAR);
    PrfQueryProfileData(prof,
                        appname,
                        "Titlebar.steps",
                        &sSteps,
                        &size);
    size = sizeof(UCHAR);
    PrfQueryProfileData(prof,
                        appname,
                        "Titlebar.vgrade",
                        &vGrade,
                        &size);
    size = sizeof(UCHAR);
    PrfQueryProfileData(prof,
                        appname,
                        "Titlebar.cgrade",
                        &cGrade,
                        &size);
    size = sizeof(UCHAR);
    PrfQueryProfileData(prof,
                        appname,
                        "Titlebar.border",
                        &tBorder,
                        &size);
    size = sizeof(UCHAR);
    PrfQueryProfileData(prof,
                        appname,
                        "Titlebar.texture",
                        &fTexture,
                        &size);
    if(firsttime) {

      DATETIME dt;

      SavePrf("Commands",
              cmds,
              sizeof(cmds));
      if(!DosGetDateTime(&dt))
        SavePrf("Cmds",
                &dt,
                sizeof(dt));
    }
  }
  LoadExcludes(&excludes,
               &numexcludes,
               "EXCLUDE.LST");
  LoadExcludes(&fexcludes,
               &numfexcludes,
               "FEXCLUDE.LST");
  RestorePresParams((HWND)0,
                    "Clock");
}


VOID PresParamChanged (HWND hwnd,char *keyroot,MPARAM mp1,MPARAM mp2) {

  ULONG AttrFound,AttrValue[64],cbRetLen;

  cbRetLen = WinQueryPresParam(hwnd,(ULONG)mp1,0,&AttrFound,
                               (ULONG)sizeof(AttrValue),
                               &AttrValue,0);
  if(cbRetLen) {

    char s[133];

    *s = 0;
    switch(AttrFound) {
      case PP_BACKGROUNDCOLOR:
        if(hwnd == ClockHwnd)
          ClockBack = AttrValue[0];
        sprintf(s,
                "%s.Backgroundcolor",
                keyroot);
        break;
      case PP_FOREGROUNDCOLOR:
        if(hwnd == ClockHwnd)
          ClockFore = AttrValue[0];
        sprintf(s,
                "%s.Foregroundcolor",
                keyroot);
        break;
      case PP_FONTNAMESIZE:
        sprintf(s,
                "%s.Fontnamesize",
                keyroot);
        break;
      default:
        break;
    }
    if(*s)
      SavePrf(s,
              (PVOID)AttrValue,
              cbRetLen);
  }
}


BOOL RestorePresParams (HWND hwnd,char *keyroot) {

  char  s[81];
  ULONG AttrValue[64],size;
  BOOL  ret = FALSE;

  size = sizeof(AttrValue);
  sprintf(s,
          "%s.Backgroundcolor",
          keyroot);
  if(PrfQueryProfileData(prof,
                         appname,
                         s,
                         (PVOID)AttrValue,
                         &size) &&
     size) {
    if(hwnd)
      WinSetPresParam(hwnd,
                      PP_BACKGROUNDCOLOR,
                      size,
                      (PVOID)AttrValue);
    if(!strcmp(keyroot,"Clock"))
      ClockBack = AttrValue[0];
    ret = TRUE;
  }
  size = sizeof(AttrValue);
  sprintf(s,
          "%s.Foregroundcolor",
          keyroot);
  if(PrfQueryProfileData(prof,
                         appname,
                         s,
                         (PVOID)AttrValue,
                         &size) &&
     size) {
    if(hwnd)
      WinSetPresParam(hwnd,
                      PP_FOREGROUNDCOLOR,
                      size,
                      (PVOID)AttrValue);
    if(!strcmp(keyroot,"Clock"))
      ClockFore = AttrValue[0];
    ret = TRUE;
  }
  size = sizeof(AttrValue);
  sprintf(s,
          "%s.Fontnamesize",
          keyroot);
  if(PrfQueryProfileData(prof,
                         appname,
                         s,
                         (PVOID)AttrValue,
                         &size) &&
     size) {
    if(hwnd)
      WinSetPresParam(hwnd,
                      PP_FONTNAMESIZE,
                      size,
                      (PVOID)AttrValue);
    ret = TRUE;
  }
  return ret;
}


void ViewHelp (HWND hwnd,char *topic) {

  PROGDETAILS pgd;
  static char args[256];

  WinSetWindowPos(hwnd,
                  HWND_TOP,
                  0,
                  0,
                  0,
                  0,
                  SWP_ACTIVATE);
  sprintf(args,
          "MSE.INF%s%s",(topic) ? " " : "",
          (topic) ? topic : "");
  memset(&pgd,
         0,
         sizeof(pgd));
  pgd.Length = sizeof(pgd);
  pgd.swpInitial.fl = SWP_ACTIVATE | SWP_ZORDER;
  pgd.swpInitial.hwndInsertBehind = HWND_TOP;
  pgd.progt.fbVisible = SHE_VISIBLE;
  pgd.progt.progc = PROG_DEFAULT;
  pgd.pszExecutable = "VIEW.EXE";
  WinStartApp(hwndCover,
              &pgd,
              args,
              NULL,
              0);
}


HOBJECT OpenObject (char *text,char *ostr) {

  HOBJECT hWPSObject = (HOBJECT)0;

  if(text &&
     *text) {
    if(!ostr)
      ostr = defopen;
    hWPSObject = WinQueryObject(text);
    if(hWPSObject != NULLHANDLE) {
      WinSetFocus(HWND_DESKTOP,
                  HWND_DESKTOP);
      WinSetObjectData(hWPSObject,
                       ostr);
    }
  }
  return hWPSObject;
}


void ShowFolder (char *name) {

  HOBJECT hWPSObject;
  char    fullname[CCHMAXPATH];

  if(!strchr(name,'\\') &&
     !strchr(name,'/') &&
     !strchr(name,':')) {
    sprintf(fullname,
            "%s%s",
            mydir,
            name);
    name = fullname;
  }

  hWPSObject = WinQueryObject(name);
  if(name == fullname &&
     hWPSObject == NULLHANDLE) {
    DosCreateDir(name,NULL);
    hWPSObject = WinQueryObject(name);
    if(hWPSObject)
      WinSetObjectData(hWPSObject,
                       "NORENAME=YES");
  }
  if(hWPSObject != NULLHANDLE) {
    WinSetFocus(HWND_DESKTOP,
                HWND_DESKTOP);
    WinSetObjectData(hWPSObject,
                     defopen);
  }
  else
    DosBeep(50,100);
}


void ShowWindow (HAB hab,HWND hwnd) {

  SWP   swp;
  ULONG fl = SWP_SHOW | SWP_ZORDER | SWP_ACTIVATE;

  if(WinQueryWindowPos(hwnd,&swp)) {
    if(swp.fl & SWP_MINIMIZE)
      fl |= SWP_RESTORE;
    DosSleep(1);
    WinSetWindowPos(hwnd,
                    HWND_TOP,
                    0,
                    0,
                    0,
                    0,
                    fl);
    if(fDesktops) {

      long lx,ly;

      if(WhichDesk(hwnd,
                   NULL,
                   &lx,
                   &ly))
        ChangeDesktop(hab,
                      lx,
                      ly);
    }
    if(fMoveMouse &&
       WinQueryWindowPos(hwnd,&swp) &&
       swp.x + (swp.cx / 2) < xScreen &&
       swp.y + (swp.cy / 2) < yScreen &&
       swp.x + (swp.cx / 2) > 0 &&
       swp.y + (swp.cy / 2) > 0)
      WinSetPointerPos(HWND_DESKTOP,
                       swp.x + (swp.cx / 2),
                       swp.y + (swp.cy / 2));
  }
}


void WarbleBeep (void) {

  DosBeep(50,5);
  DosBeep(250,10);
  DosBeep(500,20);
  DosBeep(1000,45);
  DosBeep(500,20);
  DosBeep(250,10);
  DosBeep(50,5);
}
