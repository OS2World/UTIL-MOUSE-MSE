/*
 * MSE deinstallation program
 *
 * Run with /Y argument to avoid being asked if you're sure.
 */
'@Echo off'
'cls'
/* tell the nice people who we are */
say ''
say ' 旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커'
say ' �            MSE Mouse and System Enhancer deinstallation program            �'
say ' 읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸'
say ''
/* get current directory */
curdir = directory()
/* see if we have what we need to kill off */
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
  when dummy = '/Y'
    then dummy = 'Y'
otherwise
do
  /* be sure they meant it */
  call charout ,"Remove MSE from your system? [Y/N] "
  /* get input */
  dummy = ''
  do forever
    dummy = SysGetKey('NOECHO')
    parse upper var dummy dummy
    if dummy = '1b'x then dummy = 'N'
    if dummy = '0d'x then dummy = 'Y'
    if dummy = 'N' then leave
    if dummy = 'Y' then leave
  end
  call charout ,dummy
  say ''
end
end

/* well, did they really mean it? */
if dummy = 'N' then exit
/* disembowel ourself */
'mse /k'
call SysSleep 2
call SysDestroyObject "<MSE_START>"
call SysDestroyObject "<MSE_DESKTOP>"
'del mse.ini 1>NUL 2>NUL'
'del install.cmd 1>NUL 2>NUL'
'del fexclude.lst 1>NUL 2>NUL'
'del exclude.lst 1>NUL 2>NUL'
'del read.me 1>NUL 2>NUL'
'del msemenu?.dat 1>NUL 2>NUL'
'del mse.inf 1>NUL 2>NUL'
'del aftrscrn.cmd 1>NUL 2>NUL'
'del file_id.diz 1>NUL 2>NUL'
'del register.txt 1>NUL 2>NUL'
'del purchase.exe 1>NUL 2>NUL'
'del purchase.app 1>NUL 2>NUL'
'del SCRNSHTS\*.bmp 1>NUL 2>NUL'
'rmdir SCRNSHTS 1>NUL 2>NUL'
'del CLIPS\clipbrd.* 1>NUL 2>NUL'
'rmdir CLIPS 1>NUL 2>NUL'
'del files.lst 1>NUL 2>NUL'
'del dirs.lst 1>NUL 2>NUL'
'del mse.exe 1>NUL 2>NUL'
'del msehook.dll 1>NUL 2>NUL'
'del runmse.cmd 1>NUL 2>NUL'
cd..
/* tell the nice people we're done */
say 'MSE deinstallation is complete.  Remove MSE directory '
say curdir' if desired with RD or DELTREE.'
/* now blow our brains out all over the wall */
'del 'curdir'\uninstal.cmd'
