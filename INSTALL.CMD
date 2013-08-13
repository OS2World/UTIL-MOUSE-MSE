/*
 * MSE installation program
 *
 * Note:  To run unattended, use /B, /D, /S or /N on the command line
 * (for example, "INSTALL /B") to answer the one question that
 * MSE asks.  /N means don't make any objects.
 */
'@Echo off'
'cls'
/* tell the nice people who we are */
say ''
say ' 旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커'
say ' �             MSE Mouse and System Enhancer installation program             �'
say ' 읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸'
say ''
/* get current directory */
curdir = directory()
/* see if we have what we need to run */
rc = stream('msehook.dll','c','query exists')
if rc = '' then
do
  say 'Sorry, MSEHOOK.DLL not found.  Must not be right directory.  Terminating.'
  exit
end
rc = stream('mse.exe','c','query exists')
if rc = '' then
do
  say 'Sorry, MSE.EXE not found.  Must not be right directory.  Terminating.'
  exit
end
/* load system utility functions */
call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs
/* skip input? */
dummy = ''
parse upper arg dummy
select
  when dummy = '/B'
    then dummy = 'B'
  when dummy = '/S'
    then dummy = 'S'
  when dummy = '/D'
    then dummy = 'D'
  when dummy = '/N'
    then dummy = 'N'
otherwise
do
  /* ask where they want the program object */
  call charout ,"Create MSE object on [D]esktop, in [S]tartup folder, or [B]oth? [D/S/B/N] "
  /* get input */
  dummy = ''
  do forever
    dummy = SysGetKey('NOECHO')
    parse upper var dummy dummy
    if dummy = 'D' then leave
    if dummy = 'S' then leave
    if dummy = 'B' then leave
    if dummy = 'N' then leave
  end
  /* got input; echo it */
  call charout ,dummy
  say ''
end
end
/* now do it */
if dummy = 'D' then
do
  location = '<WP_DESKTOP>'
  id = '<MSE_DESKTOP>'
  parm = '%'
end
if dummy \= 'D' then
do
  location = '<WP_START>'
  id = '<MSE_START>'
  parm = '/h %'
end
if dummy \= 'N' then
do
  title = 'MSE'
  classname = 'WPProgram'
  setup = 'OBJECTID='id';EXENAME='curdir'\MSE.EXE;STARTUPDIR='curdir';PARAMETERS='parm
  rc = SysCreateObject(classname,title,location,setup,u)
  if dummy = 'B' then
  do
    location = '<WP_DESKTOP>'
    id = '<MSE_DESKTOP>'
    setup = 'OBJECTID='id';EXENAME='curdir'\MSE.EXE;STARTUPDIR='curdir';PARAMETERS=%'
    rc = SysCreateObject(classname,title,location,setup,u)
  end
end
/* create samples if they don't already exist; clean up temp files */
rc = stream('MSEMENU1.DAT','c','query exists')
if rc = '' then
do
  rc = stream('MSEMENU1.TMP','c','query exists')
  if rc \= '' then
  do
    say 'Creating a sample MSEMENU1.DAT file for you.'
    'REN MSEMENU1.TMP MSEMENU1.DAT 1>NUL 2>NUL'
  end
end
'DEL MSEMENU1.TMP 1>NUL 2>NUL'
rc = stream('AFTRSCRN.CMD','c','query exists')
if rc = '' then
do
  rc = stream('AFTRSCRN.TMP','c','query exists')
  if rc \= '' then
  do
    say 'Creating a sample AFTRSCRN.CMD file for you.'
    'REN AFTRSCRN.TMP AFTRSCRN.CMD 1>NUL 2>NUL'
  end
end
'DEL AFTRSCRN.TMP 1>NUL 2>NUL'
rc = stream('EXCLUDE.LST','c','query exists')
if rc = '' then
do
  rc = stream('EXCLUDE.TMP','c','query exists')
  if rc \= '' then
  do
    say 'Creating a sample EXCLUDE.LST file for you.'
    'REN EXCLUDE.TMP EXCLUDE.LST 1>NUL 2>NUL'
  end
end
'DEL EXCLUDE.TMP 1>NUL 2>NUL'
rc = stream('TTLBARS.DAT','c','query exists')
if rc = '' then
do
  rc = stream('TTLBARS.TMP','c','query exists')
  if rc \= '' then
  do
    say 'Creating a sample TTLBARS.TMP file for you.'
    'REN TTLBARS.TMP TTLBARS.DAT 1>NUL 2>NUL'
  end
end
'DEL TTLBARS.TMP 1>NUL 2>NUL'
/* delete useless BBS junk file */
'del file_id.diz 1>NUL 2>NUL'
/* tell the nice people that we're done, run it, terminate. */
say 'MSE installation is complete.'
'start mse.exe'
exit
