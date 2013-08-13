#include "msg2.h"

#ifndef DP_MINI
 #define DP_MINI  0x0004
#endif

typedef struct {
  HPOINTER hptr;
  HWND     hwnd;
  HSWITCH  hswitch;
  USHORT   id;
  char     text[128];
} MENUPARMS;

/* bitmap.c */
HBITMAP LoadBitmap (char *pszFileName);
HBITMAP TrimBitmap (HAB hab,HBITMAP hbmIn);
BOOL    SaveBitmap (HAB hab,HBITMAP hbmIn,char *pszFileName,USHORT bitcount);
HBITMAP SaveScreen (HAB hab,HWND hwndCap,BOOL saveit);

/* titlebar.c */
void             ReadSchemes (void);
MRESULT EXPENTRY FitBitProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);
void             RedrawTitlebars (BOOL all,HWND hwndTop);
MRESULT EXPENTRY SetTitleDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);

/* calc.c */
MRESULT EXPENTRY CalcProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);

/* clip.c */
void             TakeClipboard (void);
void             StartClipMgr (void);
char *           FullDrgName (PDRAGITEM pDItem,char *buffer,ULONG buflen);
MRESULT EXPENTRY DragProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);
MRESULT EXPENTRY GraphicsProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);
MRESULT EXPENTRY ClipProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);

/* objwin.c */
void             StartObjWins (void);

/* desktops.c */
MRESULT EXPENTRY DeskProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);
void             ChangeDesktop (HAB hab,long virtx,long virty);
BOOL             WhichDesk (HWND hwnd,SWP *pswp,long *lx,long *ly);
void             NormalizeWindow (HWND hwnd,BOOL activate);
void             NormalizeAll (void);
MRESULT EXPENTRY OviewProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);

/* misc.c */
char *  strip_trail_char (char *strip,char *a);
char *  strip_lead_char (char *strip,char *a);
BOOL    IsInValidDir (char *name);
void    SavePrf (char *name,void *data,ULONG size);
void    LoadPrf (void);
VOID    PresParamChanged (HWND hwnd,CHAR *keyroot,MPARAM mp1,MPARAM mp2);
BOOL    RestorePresParams (HWND hwnd,CHAR *keyroot);
void    ViewHelp (HWND hwnd,char *topic);
HOBJECT OpenObject (char *text,char *ostr);
void    ShowFolder (char *name);
void    ShowWindow (HAB hab,HWND hwnd);
void    WarbleBeep (void);

/* msemon.c */
void MonitorIdleLoopThread (void *args);
void MonitorLoopThread (void *args);

/* msemon2.c */
void             FindSwapperDat (char *SwapperDat);
MRESULT EXPENTRY SwapProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);
void             StartSwapMon (ULONG id);


/* mse.c */
int main (int argc,char *argv[]);

/* mainwnd.c */
void             TimerThread (void *args);
MRESULT EXPENTRY MainProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);

/* settings.c */
MRESULT EXPENTRY FitBitProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);
MRESULT EXPENTRY OuttieProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);
MRESULT EXPENTRY MseSetProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);
MRESULT EXPENTRY Mse2SetProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);
MRESULT EXPENTRY FDlgSetProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);
MRESULT EXPENTRY VirtSetProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);
MRESULT EXPENTRY ClipSetProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);
MRESULT EXPENTRY ScrnSetProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);
MRESULT EXPENTRY MonSetProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);
MRESULT EXPENTRY TtlSetProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);
MRESULT EXPENTRY ConfigProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);
MRESULT EXPENTRY CoverProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);

/* runproc.c */
MRESULT EXPENTRY RunProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);

/* excludes.c */
void FreeExcludes (char ***ex,int *nex);
void LoadExcludes (char ***ex,int *nex,char *filename);
BOOL IsExcluded (char *titletext);


#define lstrip(s)    strip_lead_char(" \t",(s))
#define rstrip(s)    strip_trail_char(" \t",(s))
#define stripcr(s)   strip_trail_char("\r\n",(s))
#define stripzero(s) strip_trail_char("0",(s))

#pragma data_seg(DATAN)

extern  char *cmdnames[];
extern  char *crnrnames[];
extern  char *objwinclass,
             *gobjwinclass,
             *graphwinclass,
             *helvtext,
             *defopen,
             *setopen,
             *eexe,
             *clipwinclass,
             *deskwinclass,
             *shelvtext,
             *clipmgr,
             *desknoteclass,
             *monwinclass,
             *objwinclass2,
             *objwinclass3,
             *objwinclass4,
             *appname;

#ifdef DEFINE_GLOBALS
  #define DATADEF
#else
  #define DATADEF extern
#endif

DATADEF char        tBitmap[CCHMAXPATH];
DATADEF HINI        prof;
DATADEF HWND        GObjectHwnd,TipHwnd,StatusHwnd,SwapHwnd,ClockHwnd,
                    DeskHwnd,CalcHwnd,VirtHwnd,lastHwnd,hwndCover,HardHwnd,
                    CPUHwnd,ClipMonHwnd,ClipSetHwnd,MemHwnd,TaskHwnd;
DATADEF char      **excludes,**cliptext,**fexcludes;
DATADEF int         numexcludes,numtext,numbits,nummets,nexttext,thistext,
                    swaptick,desktick,hardtick,numfexcludes,numtbars,
                    closethreads;
DATADEF HPOINTER    hptrApp,hptrWait,hptrArrow;
DATADEF ULONG       ulMouseMins,ulDriveMon;
DATADEF SWP         swpClock,swpSwap,swpHard,swpCPU,swpClip,BigSwp,swpMem,
                    swpTask;
DATADEF long        xVirtual,yVirtual,lMaxRunahead,yIcon;
DATADEF HWND        hwndMenus[7],hwndTasklist,hwndPick;
DATADEF BOOL        minimizeme,fNoTasklist,fUseClipMgr,notnow,
                    showBit2,showMet,showMet2,showText2,firsttime,fSwapMon,
                    fSwapFloat,fClipSave,fMoveMouse,fCenterDlg,
                    fDefButton,fNoNormalize,fConfirmExit,fLowMemory,
                    hookset,showhardmon,fActiveSave,fVirtText,fVirtWPS,
                    fCPUMon,fNoMonClick,fAverage,fShowMem,fShowFreeInMenus,
                    fFractions,fClipMon,fClipAppend,oldstyle,fMSEInTaskList,
                    fShowPMem,fShowSwapFree,fShowTasks;
DATADEF USHORT      scrnbitcount;
DATADEF TBARPARMS  *tbHead;
DATADEF ULONG       AveCPU,LastPercent;
DATADEF USHORT      NumAveCPU;
DATADEF ULONG       IdleCounter,MaxCount,MilliSecs;
DATADEF TID         IdleTID,MonTID;

#if defined DATAN
 #pragma data_seg(DATAN)
#elif defined DATAS
 #pragma data_seg(DATAS)
#elif defined DATAC
 #pragma data_seg(DATAC)
#elif defined DATAM
 #pragma data_seg(DATAM)
#else
 #pragma data_seg(DATAP)
#endif

#pragma alloc_text(ONCE,main,CleanUp)
#pragma alloc_text(ONCE2,LoadPrf,FindSwapperDat)
#pragma alloc_text(SELDOM,ViewHelp,ShowFolder,IsInValidDir)
#pragma alloc_text(SELDOM,LoadExcludes,FreeExcludes)
#pragma alloc_text(DESKTOPS,NormalizeWindow,NormalizeAll,ChangeDesktop,DeskProc)
#pragma alloc_text(DESKTOPS,IsExcluded,WhichDesk)
#pragma alloc_text(DESKTOP2,OviewProc,OFrameSubProc,OTextSubProc)
#pragma alloc_text(OBJWIN,GObjectProc,MakeObjWin,StartObjWins)
#pragma alloc_text(OBJWIN2,ObjectProc,ObjectProc2,ObjectProc3)
#pragma alloc_text(CLIPBRD,ClipProc)
#pragma alloc_text(CLIPBRD2,GraphicsProc,SubProc)
#pragma alloc_text(CLIPBRD2,StartClipMgr,FullDrgName,SaveBitmap)
#pragma alloc_text(CLIPBRD2,DragProc,ScaleMetafile,TakeClipboard)
#pragma alloc_text(SAVESCRN,SaveScreen,SaveName)
#pragma alloc_text(SWAPMON,SwapProc,MakeNumber,SetDriveText,SetMonitorSize)
#pragma alloc_text(SWAPMON2,RestorePresParams,PresParamChanged,StartSwapMon)
#pragma alloc_text(SWAPMON2,MonitorLoopThread,MonitorIdleLoopThread,LoadSWP)
#pragma alloc_text(CALC,CalcProc,CalcSubProc)
#pragma alloc_text(OFTEN,strip_trail_char,strip_lead_char,SavePrf,ShowWindow)
#pragma alloc_text(OFTEN,OpenObject,TimerThread,LoadBitmap,WarbleBeep)
#pragma alloc_text(OFTEN2,MainProc)
#pragma alloc_text(TITLEBARS,WriteSchemes,ReadSchemes,RedrawTitlebars)
#pragma alloc_text(TITLEBARS,FreeSchemes,TileBitmap,TrimBitmap)
#pragma alloc_text(TITLEBARS2,ColorBlockProc,GetVars)
#pragma alloc_text(TITLEBARS2,FakeTitleProc,SetTitleDlgProc)
#pragma alloc_text(SETTINGS,ConfigProc,CoverProc,MonSetProc,TtlSetProc)
#pragma alloc_text(SETTINGS2,MseSetProc,FDlgSetProc,VirtSetProc)
#pragma alloc_text(SETTINGS3,ClipSetProc,ScrnSetProc,FitBitProc,OuttieProc)
#pragma alloc_text(SETTINGS,Mse2SetProc)
#pragma alloc_text(RUNPROC,RunProc)
#pragma alloc_text(FRAMES,AddFrame,RemoveFrame,FindFrame)
