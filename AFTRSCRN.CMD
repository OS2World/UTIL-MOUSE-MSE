rem
rem This script is run (minimized) after a screen capture.
rem Input is the name of the newly created bitmap.
rem Modify to taste -- this is only a sample.
rem

rem
rem Iconify the image using ICONIFY.EXE from the FM/2 Utilities:
rem Note it may be necessary to use "View->Refresh now" in a folder
rem to make the new icon show up.  Weird and multiform are the ways
rem of OS/2.
rem

ICONIFY.EXE /h %1
