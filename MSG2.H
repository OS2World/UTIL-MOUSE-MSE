/****************************************************************************
    msg2.h - Mesasge box defintions

    Copyright (c) 1997, 2001 by Mark Kimes
    Copyright (c) 2001 Steven Levine & Associates, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Revisions	26 Apr 97 MK - Release
		25 Sep 01 SHL - Drop duplicate definitions

*****************************************************************************/


   #define MAX_MBDTEXT               70
   #define MB_CUSTOMICON              0x0080

   /* NOINC */
   #pragma pack(4)
   
#  if 0 // fixme
   typedef struct _MB2D
   {
      CHAR   achText[MAX_MBDTEXT + 1]; /* Text of the button. eg. "~Cancel"    */
      ULONG  idButton;                  /* Button ID returned when user chooses */
      /*  button                              */
      LONG   flStyle;                   /* Button style or'ed with internal     */
      /*  styles                              */
   } MB2D;
   
   typedef MB2D * PMB2D;
   
   typedef struct _MB2INFO
   {
      ULONG    cb;                  /* Size of fixed part of structure          */
      HPOINTER hIcon;               /* Icon handle                              */
      ULONG    cButtons;            /* Number of buttons                        */
      ULONG    flStyle;             /* Icon style flags (MB_ICONQUESTION, etc...)*/
      HWND     hwndNotify;          /* Reserved                                 */
      MB2D     mb2d[1];             /* Array of button definitions              */
   } MB2INFO;
   
   typedef MB2INFO * PMB2INFO;
#  endif // fixme
   
   #pragma pack()
   
   /************************************************************************/
   /* WinMessageBox2                                                       */
   /*                                                                      */
   /* Parameters: HWND   hwndParent   - handle of the parent window.       */
   /*             HWND   hwndOwner    - handle of the owner window.        */
   /*             PSZ    pszText      - message text.                      */
   /*             PSZ    pszCaption   - title of the message box.          */
   /*             ULONG  idWindow     - Message box id                     */
   /*             PMB2INFO pmb2info   - pointer to button/icon info        */
   /************************************************************************/
   ULONG  EXPENTRY  WinMessageBox2(HWND     hwndParent,
   HWND     hwndOwner,
   PSZ      pszText,
   PSZ      pszCaption,
   ULONG    idWindow,
   PMB2INFO pmb2info);

