#define INCL_WIN
#define INCL_DOS
#define INCL_GPI

#define DATAS 1

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "msehook.h"
#include "mse.h"


void MonitorIdleLoopThread (void *args) {

  IdleCounter = 0;
  DosSetPriority(PRTYS_THREAD,
                 PRTYC_IDLETIME,
                 PRTYD_MINIMUM,
                 0);
  while(!amclosing &&
        !closethreads)
    IdleCounter++;
  IdleTID = 0;
}

