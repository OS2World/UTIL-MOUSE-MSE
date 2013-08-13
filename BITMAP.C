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


HBITMAP LoadBitmap (char *pszFileName) {

  HBITMAP                 hBmp = (HBITMAP)0;
  FILE                   *File;
  ULONG                   rc;
  USHORT                  usType = 0;           // #@!!! compiler warnings
  PBITMAPARRAYFILEHEADER2 pbafh2 = NULL;        // (MAM) chng init values to NULL instead of ptr to 0
  PBITMAPFILEHEADER2      pbfh2  = NULL;                       
  PBITMAPINFOHEADER2      pbih2  = NULL;                       
  PBITMAPINFO2            pbmi2  = NULL;                       
  PBITMAPARRAYFILEHEADER  pbafh  = NULL;                       
  PBITMAPFILEHEADER       pbfh   = NULL;                       
  PBITMAPINFOHEADER       pbih   = NULL;                       
  BOOL                    f2;                   // format 1.x or 2.x
  ULONG                   ulOffset;
  PBYTE                   pData  = NULL;
  ULONG                   ulDataSize;
  SIZEL                   sizel;
  HPS                     hPS = WinGetPS(HWND_DESKTOP);

  //--- open the file
  File = fopen(pszFileName,"rb");
  if(!File)
    goto ExitLoadBMP;

  /* Read image type, and reset the stream...................................*/
  /* The image type is a USHORT, so we only read that........................*/
  rc = fread(&usType,1,sizeof(usType),File);
  if(rc != sizeof(usType))
    goto ExitLoadBMP;

  /* Next read the bitmap info header........................................*/
  // we allocate enough to hold a complete bitmap array file header
  pbafh2 = (PBITMAPARRAYFILEHEADER2)malloc(sizeof(*pbafh2) +
                                             256 * sizeof(RGB2));
  if(!pbafh2)
    goto ExitLoadBMP;
  /* Next we assign pointers to the file header and bitmap info header...*/
  /* Both the 1.x and 2.x structures are assigned just in case...........*/
  pbfh2 = &pbafh2->bfh2;
  pbih2 = &pbfh2->bmp2;
  pbmi2 = (PBITMAPINFO2)pbih2;
  pbafh = (PBITMAPARRAYFILEHEADER)pbafh2;
  pbfh  = &pbafh->bfh;
  pbih  = &pbfh->bmp;
  switch (usType) {
    case BFT_BMAP:
    case BFT_ICON:
    case BFT_POINTER:
    case BFT_COLORICON:
    case BFT_COLORPOINTER:
      {
        /* Now we assume the image is a 2.0 image and so we read a bitmap-file-    */
        /* Now we reset the stream, next we'll read the bitmap info header. To do .*/
        /* this we need to reset the stream to 0...................................*/
        fseek(File,
              0,
              SEEK_SET);
        /*-header-2 structure..................................................*/
        rc = fread(pbfh2,
                   1,
                   sizeof(*pbfh2),
                   File);
        if(rc != sizeof(*pbfh2))
          goto ExitLoadBMP;

        f2 = pbih2->cbFix > sizeof(*pbih); // 1.x or 2.x bitmap
        /* We will need to read the color table. Thus we position the stream...*/
        /* so that the next read will read IT. This, of course, depends on the.*/
        /* type of the bitmap (old vs new), note that in the NEW case, we can..*/
        /* not be certain of the size of the bitmap header.....................*/
        ulOffset = (f2) ? sizeof(*pbfh2) + pbih->cbFix - sizeof(*pbih2) :
                          sizeof(*pbfh);
      }
      break;

    case BFT_BITMAPARRAY:
      {
        /* Now we are dealing with a bitmap array. This is a collection of ....*/
        /* bitmap files and each has its own file header.......................*/

        BOOL   bBest = FALSE;
        ULONG  ulCurOffset, ulOffsetTemp = 0;
        LONG   lScreenWidth;
        LONG   lScreenHeight;
        LONG   lClrsDev, lClrsTemp = 0;
        LONG   lClrs;
        ULONG  ulSizeDiff, ulSizeDiffTemp = 0xffffffff;
        HDC    hdc;

        // -- We will browse through the array and chose the bitmap best suited
        // -- for the current display size and color capacities.
        hdc = GpiQueryDevice(hPS);
        DevQueryCaps(hdc,
                     CAPS_COLORS,
                     1,
                     &lClrsDev);
        DevQueryCaps(hdc,
                     CAPS_WIDTH,
                     1,
                     &lScreenWidth);
        DevQueryCaps(hdc,
                     CAPS_HEIGHT,
                     1,
                     &lScreenHeight);
        pbafh2->offNext = 0;
        do {
           ulCurOffset = pbafh2->offNext;
           rc = fseek(File,
                      pbafh2->offNext,
                      SEEK_SET);
           if(rc)
             goto ExitLoadBMP;
           rc = fread(pbafh2,
                      1,
                      sizeof(*pbafh2),
                      File);
           if(rc != sizeof(*pbafh2))
             goto ExitLoadBMP;
           f2 = pbih2->cbFix > sizeof(*pbih);
           if(f2)
             lClrs = 1 << (pbafh2->bfh2.bmp2.cBitCount *
                           pbafh2->bfh2.bmp2.cPlanes);
           else
             lClrs = 1 << (pbafh->bfh.bmp.cBitCount *
                           pbafh->bfh.bmp.cPlanes);
           if((pbafh2->cxDisplay == 0) &&
              (pbafh2->cyDisplay == 0)) {
             // This is a device independant bitmap
             // Process it as a VGA
             pbafh2->cxDisplay = 640;
             pbafh2->cyDisplay = 480;
           } // endif
           ulSizeDiff = abs(pbafh2->cxDisplay - lScreenWidth) +
                            abs(pbafh2->cyDisplay - lScreenHeight);
           if((lClrsDev == lClrs) &&
              (ulSizeDiff == 0)) {
             // We found the perfect match
             bBest = TRUE;
             ulOffsetTemp = ulCurOffset;
           }
           else {
             if((ulOffsetTemp == 0) ||           // First time thru
                (ulSizeDiff < ulSizeDiffTemp) || // Better fit than any previous
                  ((lClrs > lClrsTemp) && (lClrs < lClrsDev)) || // More colors than prev & less than device
                  ((lClrs < lClrsTemp) && (lClrs > lClrsDev))) {
               ulOffsetTemp = ulCurOffset;       // Make this our current pick
               lClrsTemp   = lClrs;
               ulSizeDiffTemp = ulSizeDiff;
             } // endif
           } // endif
        } while((pbafh2->offNext != 0) && !bBest); // enddo

        // Now retrieve the best bitmap
        rc = fseek(File,
                   ulOffsetTemp,
                   SEEK_SET);
        if(rc)
          goto ExitLoadBMP;
        rc = fread(pbafh2,
                   1,
                   sizeof(*pbafh2),
                   File);
        if(rc != sizeof(*pbafh2))
          goto ExitLoadBMP;

        f2 = pbih2->cbFix > sizeof(*pbih);
        /* as before, we calculate where to position the stream in order to ...*/
        /* read the color table information....................................*/
        ulOffset = ulOffsetTemp;
        ulOffset += (f2) ? sizeof(*pbafh2) + pbih2->cbFix - sizeof(*pbih2):
                           sizeof(*pbafh);
      }
      break;

    default:
       goto ExitLoadBMP;
  } /* endswitch */

  /* We now position the stream on the color table information...............*/
  rc = fseek(File,
             ulOffset,
             SEEK_SET);
  if(rc)
    goto ExitLoadBMP;

  /* Read the color table....................................................*/
  if(f2) {
    /* For a 2.0 bitmap, all we need to do is read the color table...........*/
    /* The bitmap info structure is just the header + color table............*/
    // If we have 24 bits per pel, there is usually no color table, unless
    // pbih2->cclrUsed or pbih2->cclrImportant are non zero, we should
    // test that !
    if(pbih2->cBitCount < 24) {

      ULONG ul = (1 << pbih2->cBitCount) * sizeof(RGB2);

      rc = fread(&pbmi2->argbColor[0],
                 1,
                 ul,
                 File);
      if(rc != ul)
        goto ExitLoadBMP;
    } // endif
    /* remember the bitmap info we mentioned just above?.....................*/
    pbmi2 = (PBITMAPINFO2)pbih2;
  }
  else {
    /* This is an old format bitmap. Since the common format is the 2.0......*/
    /* We have to convert all the RGB entries to RGB2........................*/

    ULONG ul, cColors;
    RGB   rgb;

    if(pbih->cBitCount <24)
      cColors = 1 << pbih->cBitCount;
    else
    // If there are 24 bits per pel, the 24 bits are assumed to be a RGB value
      cColors = 0;
    /* Loop over the original table and create the new table, the extra byte.*/
    /* has to be 0...........................................................*/
    for(ul = 0; ul < cColors; ul++) {
      fread(&rgb,1,sizeof(rgb),File);
      pbmi2->argbColor[ul].bRed      = rgb.bRed;
      pbmi2->argbColor[ul].bGreen    = rgb.bGreen;
      pbmi2->argbColor[ul].bBlue     = rgb.bBlue;
      pbmi2->argbColor[ul].fcOptions = 0;
    } /* endfor */

    // we have to convert the old to the new version header
    pbmi2->cbFix = sizeof(*pbih2);
    pbmi2->cBitCount = pbih->cBitCount;
    pbmi2->cPlanes = pbih->cPlanes;
    pbmi2->cy = pbih->cy;
    pbmi2->cx = pbih->cx;
    // set rest to zero
    memset((PCHAR)pbmi2 + 16,
           0,
           sizeof(*pbih2) - 16);
  } /* endif */

  /* We have the 2.0 bitmap info structure set...............................*/
  /* move to the stream to the start of the bitmap data......................*/
  rc = fseek(File,
             pbfh2->offBits,
             SEEK_SET);
  if(rc)
    goto ExitLoadBMP;

  /* Read the bitmap data, the read size is derived using the magic formula..*/
  /* The bitmap scan line is aligned on a doubleword boundary................*/
  /* The size of the scan line is the number of pels times the bpp...........*/
  /* After aligning it, we divide by 4 to get the number of bytes, and.......*/
  /* multiply by the number of scan lines and the number of pel planes.......*/
  if(pbmi2->ulCompression)
    ulDataSize = pbmi2->cbImage;
  else
    ulDataSize = (((pbmi2->cBitCount * pbmi2->cx) + 31) / 32) * 4 *
                  pbmi2->cy * pbmi2->cPlanes;
  pData = (PBYTE)malloc(ulDataSize);
  if(!pData)
    goto ExitLoadBMP;
  rc = fread(pData,
             1,
             ulDataSize,
             File);
  if(rc != ulDataSize)
    goto ExitLoadBMP;

  /* Now, we create the bitmap...............................................*/
  sizel.cx = pbmi2->cx;
  sizel.cy = pbmi2->cy;

  hBmp = GpiCreateBitmap(hPS,
                         (PBITMAPINFOHEADER2)pbmi2,
                         CBM_INIT,
                         pData,
                         pbmi2);

ExitLoadBMP:
  if(pData)
    free(pData);
  if(pbafh2)
    free(pbafh2);
  fclose(File);
  WinReleasePS(hPS);
  _heapmin();
  return(hBmp);
}


HBITMAP TrimBitmap (HAB hab,HBITMAP hbmIn) {

  /*
   *  Trim a bitmap in height to match the height of a titlebar and
   *  in width to be no more than the width of the screen.
   *  Return the HBITMAP of the copy.
   *  The original bitmap is freed on success (return != (HBITMAP)0).
   */

  HPS               hpsMemory;
  HDC               hdcMemory;
  HBITMAP           hbm = (HBITMAP)0;
  SIZEL             ImageSize;
  long              temp;
  POINTL            aptl[4];
  BITMAPINFOHEADER2 bmp2;

  if(!hbmIn)
    return hbm;
  memset(&bmp2,
         0,
         sizeof(bmp2));
  bmp2.cbFix = sizeof(bmp2);
  bmp2.cbImage = 0;
  if(GpiQueryBitmapInfoHeader(hbmIn,
                              &bmp2)) {
    if(bmp2.cy <= WinQuerySysValue(HWND_DESKTOP,
                                   SV_CYTITLEBAR))
      return hbm;
    hdcMemory = DevOpenDC(hab,
                          OD_MEMORY,
                          "*",
                          0L,
                          NULL,
                          0);
    if(hdcMemory) {
      ImageSize.cx = min(WinQuerySysValue(HWND_DESKTOP,
                                          SV_CXSCREEN),
                         bmp2.cx) + 1;
      ImageSize.cy = WinQuerySysValue(HWND_DESKTOP,
                                      SV_CYTITLEBAR) + 1;
      hpsMemory = GpiCreatePS(hab,
                              hdcMemory,
                              &ImageSize,
                              PU_PELS | GPIF_DEFAULT | GPIT_MICRO |
                              GPIA_ASSOC);
      if(hpsMemory) {
        temp = ImageSize.cx;
        bmp2.cx = temp;
        ImageSize.cx = bmp2.cx;
        temp = ImageSize.cy;
        ImageSize.cy = bmp2.cy;
        bmp2.cy = temp;
        hbm = GpiCreateBitmap(hpsMemory,
                              &bmp2,
                              0,
                              NULL,
                              NULL);
        if(hbm) {
          if(GpiSetBitmap(hpsMemory,
                          hbm) !=
             HBM_ERROR) {
            aptl[0].x = 0;
            aptl[0].y = 0;
            aptl[1].x = min(WinQuerySysValue(HWND_DESKTOP,
                                             SV_CXSCREEN),
                            ImageSize.cx);
            aptl[1].y = WinQuerySysValue(HWND_DESKTOP,
                                         SV_CYTITLEBAR);
            aptl[2].x = 0;
            aptl[2].y = 0;
            aptl[3].x = min(WinQuerySysValue(HWND_DESKTOP,
                                             SV_CXSCREEN),
                            ImageSize.cx);
            aptl[3].y = min(WinQuerySysValue(HWND_DESKTOP,
                                             SV_CYTITLEBAR),
                            ImageSize.cy);
            if(GpiWCBitBlt(hpsMemory,
                           hbmIn,
                           4L,
                           aptl,
                           ROP_SRCCOPY,
                           BBO_IGNORE) ==
               GPI_ERROR) {
              GpiDeleteBitmap(hbm);
              hbm = (HBITMAP)0;
            }
            else
              GpiDeleteBitmap(hbmIn);
            GpiSetBitmap(hpsMemory,
                         0);
          }
          else {
            GpiDeleteBitmap(hbm);
            hbm = (HBITMAP)0;
          }
        }
        GpiAssociate(hpsMemory,
                     NULLHANDLE);
        GpiDestroyPS(hpsMemory);
      }
      DevCloseDC(hdcMemory);
    }
  }
  return hbm;
}


BOOL SaveBitmap (HAB hab,HBITMAP hbmIn,char *pszFileName,USHORT bitcount) {

  HDC               hdcMemory;
  HPS               hpsMemory;
  SIZEL             ImageSize;
  POINTL            aptl[4];
  BITMAPFILEHEADER2 bfh;
  HBITMAP           hbm;
  ULONG             rc;
  BOOL              ret = FALSE;
  PBITMAPINFO2      pbmi = NULL;
  PBYTE             pbBuffer = NULL;
  ULONG             cbBuffer,cbBitmapInfo;

  if(!pszFileName)
    return ret;

  hdcMemory = DevOpenDC(hab,
                        OD_MEMORY,
                        "*",
                        0L,
                        NULL,
                        0);
  if(hdcMemory == DEV_ERROR)
    return ret;

  memset(&bfh,0,sizeof(bfh));
  bfh.usType = BFT_BMAP;
  bfh.xHotspot = 0;
  bfh.yHotspot = 0;
  bfh.bmp2.cbFix = sizeof(BITMAPINFOHEADER2);
  if(GpiQueryBitmapInfoHeader(hbmIn,
                              &bfh.bmp2)) {
    ImageSize.cx = bfh.bmp2.cx;
    ImageSize.cy = bfh.bmp2.cy;
    if(bitcount &&
       bitcount < bfh.bmp2.cBitCount)
      bfh.bmp2.usRendering = BRH_ERRORDIFFUSION;
    if(bitcount)
      bfh.bmp2.cBitCount = bitcount;
    hpsMemory = GpiCreatePS(hab,
                            hdcMemory,
                            &ImageSize,
                            PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC);
    if(hpsMemory) {
      hbm = GpiCreateBitmap(hpsMemory,
                            &bfh.bmp2,
                            0,
                            NULL,
                            NULL);
      if(hbm) {
        if(GpiSetBitmap(hpsMemory,
                        hbm) !=
           HBM_ERROR) {
          aptl[0].x = 0;                /* target lower left */
          aptl[0].y = 0;
          aptl[1].x = bfh.bmp2.cx - 1;  /* target upper right */
          aptl[1].y = bfh.bmp2.cy - 1;
          aptl[2].x = 0;                /* source lower left */
          aptl[2].y = 0;
          aptl[3].x = bfh.bmp2.cx;      /* source upper right */
          aptl[3].y = bfh.bmp2.cy;
          if(GpiWCBitBlt(hpsMemory,
                         hbmIn,
                         4L,
                         aptl,
                         ROP_SRCCOPY,
                         BBO_IGNORE) !=  GPI_ERROR) {
            cbBuffer = (((((bfh.bmp2.cBitCount * bfh.bmp2.cx) + 31) / 32) *
                            4) * bfh.bmp2.cy) * bfh.bmp2.cPlanes;
            cbBitmapInfo = sizeof(BITMAPINFOHEADER2);
            if(bfh.bmp2.cBitCount != 24)
              cbBitmapInfo += (sizeof(RGB2) *
                               (1 << (bfh.bmp2.cPlanes * bfh.bmp2.cBitCount)));
            bfh.offBits = cbBitmapInfo +
                          (sizeof(BITMAPFILEHEADER2) -
                           sizeof(BITMAPINFOHEADER2));
            if(!DosAllocMem((PPVOID)&pbBuffer,
                            cbBuffer,
                            PAG_COMMIT | PAG_READ | PAG_WRITE)) {
              if(!DosAllocMem((PPVOID)&pbmi,
                              cbBitmapInfo,
                              PAG_COMMIT | PAG_READ | PAG_WRITE)) {
                memset(pbmi,
                       0,
                       cbBitmapInfo);
                memcpy(pbmi,
                       &bfh.bmp2,
                       sizeof(BITMAPINFOHEADER2));
                rc = GpiQueryBitmapBits(hpsMemory,
                                        0,
                                        bfh.bmp2.cy,
                                        pbBuffer,
                                        pbmi);
                if(rc != (ULONG)GPI_ALTERROR) {

                  FILE *fp;

                  bfh.cbSize = (sizeof(BITMAPFILEHEADER2) -
                                sizeof(BITMAPINFOHEADER2)) +
                                cbBitmapInfo + cbBuffer;
                  fp = fopen(pszFileName,"wb");
                  if(fp) {
                    fwrite(&bfh,
                           sizeof(bfh) - sizeof(bfh.bmp2),
                           1,
                           fp);
                    fwrite(pbmi,
                           cbBitmapInfo,
                           1,
                           fp);
                    fwrite(pbBuffer,
                           cbBuffer,
                           1,
                           fp);
                    fclose(fp);
                    ret = TRUE;
                  }
                }
                DosFreeMem(pbmi);
              }
              DosFreeMem(pbBuffer);
            }
          }
        }
        GpiDeleteBitmap(hbm);
      }
      GpiDestroyPS(hpsMemory);
    }
  }
  DevCloseDC(hdcMemory);
  return ret;
}


static void SaveName (char *root,char *ext) {

  char *p = root + strlen(root);
  ULONG x = 1;

  do {
    sprintf(p,
            "%03x%s",
            x,
            ext);
    x++;
  } while(x &&
          IsFile(root) != -1);
}


HBITMAP SaveScreen (HAB hab,HWND hwndCap,BOOL saveit) {

  HDC               hdcMem;
  PSZ               pszData[4] = { "Display", NULL, NULL, NULL };
  HPS               hpsMem, hps;
  SIZEL             sizlPage = {0, 0};
  BITMAPINFOHEADER2 bmp;
  PBITMAPINFO2      pbmi = NULL;
  HBITMAP           hbm = (HBITMAP)0;
  POINTL            aptl[4];
  LONG              alData[2];
  char              name[24] = "SCRNSHTS\\SCRN";
  BOOL              success = FALSE;
  RECTL             rcl;

  if(saveit)
    DosBeep(500,100);

  rcl.xLeft   = 0;
  rcl.yBottom = 0;
  rcl.xRight  = xScreen;
  rcl.yTop    = yScreen;
  if(hwndCap &&
     WinIsWindow(hab,hwndCap) &&
     WinIsWindowVisible(hwndCap))
    WinQueryWindowRect(hwndCap,&rcl);
  else
    hwndCap = HWND_DESKTOP;

  /*
   * Create the memory device context and presentation space so they
   * are compatible with the screen device context and presentation space.
   */
  hdcMem = DevOpenDC(hab,
                     OD_MEMORY,
                     "*",
                     4,
                     (PDEVOPENDATA)pszData,
                     NULLHANDLE);
  if(hdcMem != HDC_ERROR) {
    hpsMem = GpiCreatePS(hab,
                         hdcMem,
                         &sizlPage,
                         PU_PELS | GPIA_ASSOC | GPIT_MICRO);
    if(hpsMem != GPI_ERROR) {
      /* Determine the device's plane/bit-count format. */
      if(GpiQueryDeviceBitmapFormats(hpsMem,
                                     2,
                                     alData)) {
        /*
         * Load the BITMAPINFOHEADER2 and BITMAPINFO2 structures.
           */
        memset(&bmp,0,sizeof(bmp));
        bmp.cbFix = (ULONG)sizeof(bmp);
        bmp.cx = rcl.xRight - rcl.xLeft;
        bmp.cy = rcl.yTop - rcl.yBottom;
        bmp.cPlanes = alData[0];
        bmp.cBitCount = alData[1];
        bmp.ulCompression = BCA_UNCOMP;
        bmp.cbImage = (((bmp.cx *
                       (1 << bmp.cPlanes) * (1 << bmp.cBitCount)) + 31) /
                        32) * bmp.cy;
        bmp.usUnits = BRU_METRIC;
        bmp.usRecording = BRA_BOTTOMUP;
        bmp.usRendering = BRH_NOTHALFTONED;
        bmp.ulColorEncoding = BCE_RGB;

        if(!DosAllocMem((PPVOID)&pbmi,
                        sizeof(BITMAPINFOHEADER2) +
                        (sizeof(RGB2) * (1 << bmp.cPlanes) *
                         (1 << bmp.cBitCount)),
                        PAG_COMMIT | PAG_READ | PAG_WRITE)) {
          pbmi->cbFix = bmp.cbFix;
          pbmi->cx = bmp.cx;
          pbmi->cy = bmp.cy;
          pbmi->cPlanes = bmp.cPlanes;
          pbmi->cBitCount = bmp.cBitCount;
          pbmi->ulCompression = BCA_UNCOMP;
          pbmi->cbImage = ((bmp.cx + 31) / 32) * bmp.cy;
          pbmi->cxResolution = 70;
          pbmi->cyResolution = 70;
          pbmi->cclrUsed = 2;
          pbmi->cclrImportant = 0;
          pbmi->usUnits = BRU_METRIC;
          pbmi->usReserved = 0;
          pbmi->usRecording = BRA_BOTTOMUP;
          pbmi->usRendering = BRH_NOTHALFTONED;
          pbmi->cSize1 = 0;
          pbmi->cSize2 = 0;
          pbmi->ulColorEncoding = BCE_RGB;
          pbmi->ulIdentifier = 0;
          /* Create a bit map that is compatible with the display. */
          hbm = GpiCreateBitmap(hpsMem,
                                &bmp,
                                FALSE,
                                NULL,
                                pbmi);

          // NOTE!!!! Memory freed here!!!
          DosFreeMem(pbmi);

          if(hbm != GPI_ERROR) {
            /* Associate the bit map and the memory presentation space. */
            if(GpiSetBitmap(hpsMem, hbm) != HBM_ERROR) {
              /* Copy the screen to the bit map.                            */
              aptl[0].x = 0;           /* Lower-left corner of dest rect    */
              aptl[0].y = 0;           /* Lower-left corner of dest rect    */
              aptl[1].x = bmp.cx;      /* Upper-right corner of dest rect   */
              aptl[1].y = bmp.cy;      /* Upper-right corner of dest rect   */
              aptl[2].x = rcl.xLeft;   /* Lower-left corner of source rect  */
              aptl[2].y = rcl.yBottom; /* Lower-left corner of source rect  */
              aptl[3].x = rcl.xRight;  /* Upper-right corner of source rect */
              aptl[3].y = rcl.yTop;    /* Upper-right corner of source rect */
              if(!hwndCap ||
                 hwndCap == HWND_DESKTOP)
                hps = WinGetScreenPS(HWND_DESKTOP);
              else
                hps = WinGetPS(hwndCap);
              if(hps) {
                if(GpiBitBlt(hpsMem,
                             hps,
                             sizeof(aptl) / sizeof(POINTL), /* # points in aptl */
                             aptl,
                             ROP_SRCCOPY,
                             BBO_IGNORE) != GPI_ERROR) {
                  success = TRUE;
                }
                WinReleasePS(hps);
              }
              GpiSetBitmap(hpsMem,(HBITMAP)0);
            }
          }
        }
      }
      GpiAssociate(hpsMem,NULLHANDLE);
      GpiDestroyPS(hpsMem);
    }
    DevCloseDC(hdcMem);
  }

  if(success) {
    if(saveit) {
      DosError(FERR_DISABLEHARDERR);
      DosCreateDir(".\\SCRNSHTS",NULL);
      SaveName(name,".BMP");
      success = SaveBitmap(hab,
                           hbm,
                           name,
                           scrnbitcount);
      if(success)
        DosBeep(1000,100);
      else
        DosBeep(50,100);
    }
    else
      return hbm;
  }

  if(hbm)
    GpiDeleteBitmap(hbm);

  if(success) {

    PROGDETAILS pgd;
    char       *env,s[CCHMAXPATH + 80];

    env = getenv("COMSPEC");
    if(!env)
      env = getenv("OS2_SHELL");
    if(!env)
      env = "CMD.EXE";
    sprintf(s,
            "/C %sAFTRSCRN.CMD %s",
            mydir,
            name);
    memset(&pgd,
           0,
           sizeof(pgd));
    pgd.Length = sizeof(pgd);
    pgd.swpInitial.fl = SWP_MINIMIZE | SWP_ZORDER;
    pgd.swpInitial.hwndInsertBehind = HWND_BOTTOM;
    pgd.progt.fbVisible = SHE_VISIBLE;
    pgd.progt.progc = PROG_WINDOWABLEVIO;
    pgd.pszExecutable = env;
    WinStartApp((HWND)0,
                &pgd,
                s,
                NULL,
                SAF_BACKGROUND);
  }
  return (HBITMAP)success;
}

