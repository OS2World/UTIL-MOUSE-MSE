
MSE.EXE version 1.38                           		02 Apr 2003
====================

                MSE -- An OS/2 PM Mouse and System Enhancer
                       Free Software from Mark Kimes

MSE is designed to make mousing about the old desktop a bit easier.  It 
allows you to assign various commands to your mouse buttons with or without 
keyboard modifiers.  It also provides optional clipboard management and 
extension, swapfile monitor, desktop clock, virtual desktops, screen capture, 
file dialog enhancement, titlebar enhancement and some other mouse-related 
options.  Sorry, kitchen sink not included. MSE requires OS/2 Warp (3.x+) or 
better.


Installation
============

Unzip the contents of the installation zip file to the directory where you 
want mse installed.  Run the provided INSTALL.CMD program to set up the 
desktop objects and build the sample control files.

INSTALL.CMD can be run with one of several options:

  /D	Create object on desktop
  /S	Create object in startup folder
  /B	Create object in both places
  /N	Create no objects

All of these options create the sample control files.

If you wish, VIEW MSE.INF and run the installer from there.  You can even 
read this file from there.  Try it -- type VIEW MSE.INF at a command line, 
or open MSE.INF's WPS object.


Uninstalling
============

The UNINSTAL.CMD can remove MSE from your system automagically for you,
if you decide to do so later.


There's more info in the MSE.INF file in the archive (type "VIEW MSE").
Please DO browse through it!  The program has convenient links to
sections of this file, accessible by clicking the various [?] buttons.


Using MSE
=========

Just enter 'MSE' from an OS/2 command prompt or start it from a program object.

The program will open the main dialog.  Configure the settings to match 
your preferences and MSE is ready to use.  See MSE.INF for usage and 
configuration details.


Settings
========

MSE saves it's settings in MSE.INI under the Application name MSE.
If you suspect these settings may corrupted, use an INI file editor to delete
the suspect keys.  They will be rebuilt the next time you restart MSE.


Known problems/shortcomings
=========================== 

These problems have been reported but have not been duplicated.

 - File Open Dialog Hotkeys do not function correctly.

 - Mouse Motion reaction delay sporadic

 - Mouse Pointer does not always track the Current Focus Window

 - Titlebar Configuration Slider Bars do not work (Warp3?)

 - Dialog Window too large at 640x480 (Warp3?)


About MSE
=========

MSE was originally written by:

  Mark Kimes
  <hectorplasmic@worldnet.att.net>

He has kindly allowed me to take over maintenance and support of MSE and to 
release the program under the GNU GPL license.  I'm sure he would appreciate 
a Thank You note for his generosity.


Support
=======

Please address support questions and enhancement requests to:

  Steven H. Levine
  steve53@earthlink.net

I also monitor the comp.os.os2.apps newsgroup and others in the comp.os.os2.* 
hierarchy.

Thanks and enjoy.

$TLIB$: $ &(#) %n - Ver %v, %f $
TLIB: $ $
