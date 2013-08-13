#define FID_TITLEBAREXTRA 0x8004

#define C_MAXCMDS         41

#define C_NONE            0
#define C_SHOWME          1
#define C_CLOSE           2
#define C_CLOSEA          3
#define C_BACK            4
#define C_BACKA           5
#define C_SHOW            6
#define C_SHOWA           7
#define C_ROLLUP          8
#define C_ROLLUPA         9
#define C_MOVE            10
#define C_MOVEA           11
#define C_SIZE            12
#define C_SIZEA           13
#define C_MAXIMIZE        14
#define C_MAXIMIZEA       15
#define C_MINIMIZE        16
#define C_MINIMIZEA       17
#define C_WINDOWLIST      18
#define C_TASKLIST        19
#define C_NEXT            20
#define C_B1DBLCLK        21
#define C_CHORD           22
#define C_WINCLIP         23
#define C_WINCLIPA        24
#define C_CLIPWIN         25
#define C_STARTFM2        26
#define C_OPEN            27
#define C_MINALL          28
#define C_MENU1           29
#define C_MENU2           30
#define C_MENU3           31
#define C_MENU4           32
#define C_MENU5           33
#define C_MENU6           34
#define C_MENU7           35
#define C_VIRTUAL         36
#define C_CLIPMGR         37
#define C_CALC            38
#define C_CMDLINE         39
#define C_VERTSCROLL      40
#define C_HANDICAP        41

#define CR_MAXCMDS        13

#define CR_NONE            0
#define CR_SHOWME          1
#define CR_TASKLIST        2
#define CR_OPEN            3
#define CR_MENU1           4
#define CR_MENU2           5
#define CR_MENU3           6
#define CR_MENU4           7
#define CR_MENU5           8
#define CR_MENU6           9
#define CR_MENU7          10
#define CR_VIRTUAL        11
#define CR_CLIPMGR        12
#define CR_CALC           13

#define UM_SETUP          (WM_USER + 1)
#define UM_TASKLIST       (WM_USER + 2)
#define UM_STARTWIN       (WM_USER + 3)
#define UM_STARTMENU      (WM_USER + 4)
#define UM_OPEN           (WM_USER + 5)
#define UM_TIMER          (WM_USER + 6)
#define UM_SWITCHLIST     (WM_USER + 7)
#define UM_MINALL         (WM_USER + 8)
#define UM_VIRTUAL        (WM_USER + 9)
#define UM_HANDICAP       (WM_USER + 10)
#define UM_BUMPDESKS      (WM_USER + 11)
#define UM_UNSUSPEND      (WM_USER + 12)
#define UM_TELLDESKTOP    (WM_USER + 13)
#define UM_CLIPMGR        (WM_USER + 14)
#define UM_CONTEXTMENU    (WM_USER + 15)
#define UM_SIZE           (WM_USER + 16)
#define UM_SAVESCREEN     (WM_USER + 17)
#define UM_REMIND         (WM_USER + 18)
#define UM_SHOWME         (WM_USER + 19)
#define UM_CHECKEXCLUDE   (WM_USER + 20)
#define UM_CALC           (WM_USER + 21)
#define UM_POSMOUSE       (WM_USER + 22)
#define UM_CMDLINE        (WM_USER + 23)
#define UM_SETUP2         (WM_USER + 24)
#define UM_HOOK           (WM_USER + 25)
#define UM_HOOK2          (WM_USER + 26)
#define UM_ALLOC          (WM_USER + 27)
#define UM_FREE           (WM_USER + 28)
#define UM_CHECKTTLS      (WM_USER + 29)
#define UM_SETPRESPARMS   (WM_USER + 30)
#define UM_TEST           (WM_USER + 31)
#define UM_INSERTPAGE     (WM_USER + 32)
#define UM_FOCUSME        (WM_USER + 33)
#define UM_TILEBITMAP     (WM_USER + 34)
#define UM_BITMAP         (WM_USER + 35)
#define UM_COMMAND        (WM_USER + 36)
#define UM_REFRESH        (WM_USER + 37)
#define UM_DEBUG          (WM_USER + 38)
#define UM_ADDFRAME       (WM_USER + 39)
#define UM_REMOVEFRAME    (WM_USER + 40)
#define UM_EMPTYMENU      (WM_USER + 41)

#define MSEOBJ_FRAME      1000

#define TITLEBAR_PARM     0xf001L

#define IDF_FILES         4096
#define IDF_DIRS          4097
#define IDF_HELP          4098
#define IDF_FILEDATA      4099

#define ID_TIMER          900

#ifndef RGB_GRAY
  #define RGB_GRAY      0x00aaaaaaL
#endif
#ifndef RGB_PALEGRAY
  #define RGB_PALEGRAY      0x00ccccccL
#endif
#ifndef RGB_DARKGRAY
  #define RGB_DARKGRAY  0x00808080L
#endif
#ifndef RGB_CHARCOAL
  #define RGB_CHARCOAL  0x00404040L
#endif
#ifndef RGB_NEONGREEN
  #define RGB_NEONGREEN  0x0000ea00L
#endif
#ifndef RGB_DARKBLUE
  #define RGB_DARKBLUE  0x00000080L
#endif

#pragma pack(1)

typedef struct TBARPARMS {
  USHORT            size;
  HWND              hwnd;
  char             *schemename;
  char             *bitmapname;
  UCHAR             isprogname,
                    istitletext,
                    vGrade,
                    cGrade,
                    tBorder,
                    exclude,
                    sSteps,
                    redStart,
                    greenStart,
                    blueStart,
                    redEnd,
                    greenEnd,
                    blueEnd,
                    redText,
                    greenText,
                    blueText,
                    fTexture;
  HBITMAP           hbm;
  struct TBARPARMS *next;
} TBARPARMS;

typedef struct {
  USHORT size;
  char   schemename[CCHMAXPATH];
  char   bitmapname[CCHMAXPATH];
  UCHAR  isprogname,
         istitletext,
         vGrade,
         cGrade,
         tBorder,
         exclude,
         sSteps,
         redStart,
         greenStart,
         blueStart,
         redEnd,
         greenEnd,
         blueEnd,
         diagonal,
         redText,
         greenText,
         blueText;
  UCHAR  fTexture,
         dummy2,
         dummy3,
         dummy4;
  ULONG  dummy;
} TBARDISK;

typedef struct FRAMEPARMS {
  HWND      hwndFrame;
  long      x,    // restore positions for rollup function
            y,
            cx,
            cy;
} FRAMEPARMS;

#pragma pack()

int   EXPENTRY InitDLL     (HAB hab,HINI prof,ULONG vmajor,ULONG vminor);
BOOL  EXPENTRY StartHook   (void);
BOOL  EXPENTRY StopHook    (void);
void  InitVars             (void);
BOOL  PostMsg              (HWND h, ULONG msg, MPARAM mp1, MPARAM mp2);
void  SaveRestorePos       (SWP *swp,HWND hwnd);
void  LoadRestorePos       (SWP *swp,HWND hwnd);
BOOL  IsRoot               (char *filename);
int   IsFile               (char *filename);
void  SetPresParams        (HWND hwnd,long back,long fore,
                            long border,char *font);
void PaintGradientWindow   (HWND hwnd,HPS hps,RECTL *cliprcl,BOOL hilite,
                            UCHAR redStart,UCHAR greenStart,UCHAR blueStart,
                            UCHAR redEnd,UCHAR greenEnd,UCHAR blueEnd,
                            UCHAR redText,UCHAR greenText,UCHAR blueText,
                            UCHAR sSteps,UCHAR vGrade,UCHAR cGrade,
                            UCHAR tBorder,UCHAR fTexture,HBITMAP hbm);
MRESULT EXPENTRY MyFileDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);
APIRET SaveDir             (char *curdir);

extern USHORT      cmds[3][8],nextcommand,crnrcmds[4];
extern HWND        hwndConfig,ClipHwnd;
extern BOOL        fSlidingFocus,fNoZorderChange,fWrapMouse,fDisabled,
                   fSuspend,showclock,fNextIsCommand,fDesktops,fShiftBump,
                   fChords,fScreenSave,fUsingCorners,fEnhanceFileDlg,
                   fEnhanceTitlebars,fConfirmFileDel,fConfirmDirDel,
                   fRememberDirs,fRememberFiles,fAutoDropCombos,fAggressive,
                   fEnableTitlebarMenus,fExtraMenus,showdate,ampm,
                   fClockInTitlebars;
extern HWND        hwndDesktop,hwndWPS,hwndBottom,ObjectHwnd,ObjectHwnd2,
                   ObjectHwnd3,ObjectHwnd4;
extern ULONG       ulLastMouse,ulPointerHidden,ulDelay,ulNumFileDlg,
                   ClockFore,ClockBack;
extern long        xScreen,yScreen,ySizeBorder,yTitleBar,lBumpDesks;
extern PID         dtPid,myPid;
extern QMSG        pickqMsg;
extern char        mydir[CCHMAXPATH],viewer[CCHMAXPATH],editor[CCHMAXPATH],
                   amclosing;
extern UCHAR       redStart,greenStart,blueStart,
                   redEnd,greenEnd,blueEnd,
                   redText,greenText,blueText,
                   sSteps,tBorder,vGrade,cGrade,fTexture;
extern PFNWP       PFNWPTitle,PFNWPStatic,PFNWPMenu,PFNWPFrame;
extern HBITMAP     hbmTitles;
extern RECTL       ClipRectl;
