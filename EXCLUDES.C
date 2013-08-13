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


void FreeExcludes (char ***ex,int *nex) {

  int x;

  if(ex && *ex) {
    for(x = 0;(*ex)[x];x++)
      free((*ex)[x]);
    free(*ex);
    _heapmin();
  }
  if(nex)
    *nex = 0;
  if(ex)
    *ex = NULL;
}


void LoadExcludes (char ***ex,int *nex,char *filename) {

  FILE       *fp;
  int         x;
  char      **test;
  static char s[CCHMAXPATHCOMP + 4];
  BOOL        temp = fSuspend;

  if(!ex ||
     !nex ||
     !filename)
    return;
  fSuspend = TRUE;
  if(*ex)
    FreeExcludes(ex,
                 nex);
  x = 0;
  sprintf(s,
          "%s%s",
          mydir,
          filename);
  fp = fopen(s,"r");
  if(fp) {
    while(!feof(fp)) {
      if(!fgets(s,
                sizeof(s),
                fp))
        break;
      s[sizeof(s) - 1] = 0;
      stripcr(s);
      lstrip(rstrip(s));
      if(*s) {
        if(x >= *nex - 1) {
          test = realloc(*ex,
                         sizeof(char *) * (*nex + 2));
          if(test)
            *ex = test;
          else
            break;
        }
        (*ex)[*nex] = strdup(s);
        if((*ex)[*nex]) {
          (*nex)++;
          (*ex)[*nex] = NULL;
        }
        else
          break;
      }
    }
    fclose(fp);
  }
  fSuspend = temp;
}


BOOL IsExcluded (char *titletext) {

  int x;

  if(excludes) {
    for(x = 0;excludes[x];x++) {
      if(!strnicmp(excludes[x],
         titletext,
         strlen(excludes[x])))
        return TRUE;
    }
  }
  return FALSE;
}

