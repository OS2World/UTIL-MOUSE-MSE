:userdoc.
:title.MSE Help file
:docprof toc=123456.
:body.


:h1 res=1.MSE installation, upgrading, startup, deinstallation
:i1.MSE installation, upgrading, startup, deinstallation

.br
:artwork name='mse.bmp' runin.
Please choose from the following topics&colon.
:p.
:link reftype=hd res=2.Installation:elink.
.br
:link reftype=hd res=3.Upgrading:elink.
.br
:link reftype=hd res=4.Command line (startup):elink.
.br
:link reftype=hd res=5.Deinstallation:elink.
.br
:link reftype=hd res=6.Troubleshooting:elink.
:p.
For your convenience, installation and deinstallation may be accomplished
from this help file.  I suggest you browse the help file first.

:h2 res=2.MSE installation
:i1.MSE installation

:p.
:link reftype=launch object='CMD.EXE' data='/C UNZIP.EXE MSE.ZIP'.Unzip
MSE.ZIP:elink. into an empty directory (using this link requires UNZIP.EXE
to be in your PATH).
:p.
Review the :link reftype=launch object='E.EXE'
data='.\READ.ME'.READ.ME:elink. file.
:p.
Run :link reftype=launch object='CMD.EXE' data='/C
.\INSTALL.CMD'.INSTALL.CMD:elink. to create program
objects and start MSE.

:h2 res=3.Upgrading MSE
:i1.Upgrading MSE

:p.
:link reftype=launch object='CMD.EXE' data='/C RUNMSE.CMD /k'.Close
MSE:elink. if it is running.
:p.
:link reftype=launch object='CMD.EXE' data='/C UNZIP.EXE MSE.ZIP'.Unzip
the new MSE.ZIP:elink. into an empty directory (using this link requires
UNZIP.EXE to be in your PATH, and this should be the new help file
you're reading, or the old one can't be overwritten) into the MSE
directory, being especially sure to overwrite MSE.EXE and MSEHOOK.DLL.
You can click the :hp1.About:ehp1. button to check to see if the compile
time and date changed to be sure you did actually overwrite the files.
If MSE gives you a version mismatch error and refuses to run, you missed
one of them.
:p.
Review the new :link reftype=launch object='E.EXE'
data='.\READ.ME'.READ.ME:elink. file.
:p.
Rerun :link reftype=launch object='CMD.EXE' data='/C .\INSTALL.CMD
/N'.INSTALL.CMD:elink. to clean up some temporary files and restart MSE.
:p.
.br
If you used the :hp1.Pick click:ehp1. :link reftype=hd res=115.button
command:elink., you may have to adjust that button's command on the
:link reftype=hd res=110.Buttons page:elink..
:p.
In some circumstances, MSE's DLL (MSEHOOK.DLL) may be "stuck," and you
can't overwrite it even after exiting MSE.  In this situation, remove
the MSE program object from your Startup folder, if one exists (to
prevent MSE from starting automatically), reboot, unpack MSE, then run
INSTALL.CMD to rebuild the MSE program object in the Startup folder.
:p.
.br
The latest version of MSE can be found at
http&colon.//www.bmtmicro.com/catalog/mse.
:p.
The :link reftype=launch object='E.EXE'
data='.\HISTORY.TXT'.HISTORY.TXT:elink. file will show you what's new in
this version of MSE.


:h2 res=4.MSE command line arguments
:i1.MSE command line arguments

:p.
MSE's :link reftype=hd res=2.installation program:elink. builds program
objects to launch MSE for you.  However, you may want to make others
for specialized purposes.  MSE has some command line arguments you can
use if you have the need or desire.  It's not required that you use any
of these, or even know what they are.  Feel free to skip this section.
:p.
The following command line arguments can be used with MSE&colon.
:p.
/c    Show MSE clipboard manager of any running instance of MSE.
.br
/d#   # sets the delay in seconds before MSE starts up (1-99).
.br
/h    Hide MSE after startup.
.br
/k    Kill any running instance of MSE.
.br
/n#   # sets the number of clipboard entries retained by MSE (5-99).
.br
/u    Resets any running instance of MSE (excludes, menus and schemes are reloaded).
.br
/3    Make the notebook an old-style notebook (if run under OS/2 v4) or new-style (if run under OS/2 v3).
:p.
MSE may be :link reftype=launch object='CMD.EXE' data='/C
RUNMSE.CMD'.rerun to surface a running instance:elink..  Only one
instance will actually be running on the system.
:p.
Note that command line arguments go in the "Optional Parameters" field
of a program object's Properties notebook (what we used to call the
Settings notebook in ye olde days).  Or you can just type 'em on a
command line, you dinosaur, you. &colon.-)

:h2 res=5.MSE deinstallation
:i1.MSE deinstallation

:p.
:link reftype=launch object='CMD.EXE' data='/C RUNMSE.CMD /k'.Close
MSE:elink. if it's running, then run :link reftype=launch
object='CMD.EXE' data='/C UNINSTAL.CMD'.UNINSTAL.CMD:elink..  Close this
help file before answering the confirmation question that UNINSTAL asks,
so it can be deleted, too.


:h2 res=6.Troubleshooting
:i1.Troubleshooting MSE problems

:p.
Hopefully, you'll never see a problem running MSE.  However, there might
be a few things that might make you scratch your head now and then, so
I'll try to address them here.  I'll add more as I notice them or
someone has a problem or tricky question.
:p.
If a program traps while using the enhanced file dialog, MSE might not
really end if you close it (though all its windows will close).  The
reason for this is that MSE never receives a WM_DESTROY message for the
file dialog window, so it doesn't know the window's dead, and so hangs
about in memory so the enhanced file dialog code will still be available
until the application closes the file dialog -- only the rogue
application's now dead, and the window died (without notification) along
with it!  You can safely and confidently use the FM/2 Kill Process
program to kill MSE in this case.


:h1 res=100.Cover page
:i1.Cover page

:artwork name='mse.bmp' runin.
The cover page contains two groups of buttons and one group of
checkboxes.  The buttons are conveniences (note&colon.  the
screenshot and clipboard folders are created the first time
MSE has something to write into them).
:p.
Here's a breakdown of the checkboxes&colon.
:p.
The :hp1.Minimize memory usage:ehp1. checkbox can be used to cause MSE
to free all the memory used by the various notebook pages when MSE is
minimized.  All dialogs are closed and all pages are removed from the
notebook.  When MSE is restored, the pages are reloaded.  This is a
compromise, as reloading the pages takes a little time.
:p.
If :hp1.Confirm MSE exit:ehp1. is checked, you'll get a dialog box
asking you if you're sure you really want to close MSE when you attempt
to close the window.  This might be useful to you, since the Close
button is right by the Minimize button.  Your choice.
:p.
If you check :hp1.Disable MSE:ehp1., most of MSE's features will become
dormant (you can get the same effect by pressing Scroll Lock so that the
Scroll Lock light comes on).  You can't have this and the :hp1.Remove
MSE from window list:ehp1. checkbox on at the same time, as it might
make it hard to call up MSE once it's minimized (you might have to
resort to a :link reftype=hd res=4.command line:elink. -- the horror!).


:h1 res=110.Buttons page
:i1 id=buttons.Buttons page
:i2 refid=buttons.Mouse button redefinitions page

:artwork name='mseicon.bmp' runin.
This page allows you to assign :link reftype=hd res=115.button
commands:elink. to mouse button/keyboard modifier combinations. Command
names prefaced with an asterisk (for example, "*Roll up/down") work only
when the mouse button is clicked on a titlebar window or system menu
window (warning:  some dialogs may not unroll if you use "*Roll up/down"
instead of "Roll up/down" because their titlebars get covered by the controls
in the window -- they weren't meant to be resized).  Other commands
should work anywhere the button is clicked. Just select the command for
a given mouse button/keyboard modifier from the dropdown listboxes.
:p.
A word about "keyboard modifiers" (called "Key mods" on the page)&colon.
When you click a mouse button, you can also hold down a Shift, Ctrl or
Alt key, or some combination thereof.  This is known as "modifying" the
click.  With three mouse buttons and three modifer keys, you get a total
of twenty-four different "modified" clicks (including "plain" or
"unmodified" clicks where no key is pressed).
:p.
The :hp1.Clear all button commands:ehp1. pushbutton can be used to set
all the combo boxes to "OS/2 default."  This disables all mouse button
redefinitions.  You'll have to confirm this one.
:p.
In case you aren't familiar with meese and their buttons, here's an
illustration&colon.
:artwork name='butdescr.bmp' align=left.


:h2 res=115.Button commands
:i1.Button commands

Here's a breakdown of the button commands available&colon.
:p.
.br
:hp1.OS/2 default:ehp1.
.br
No command -- just do what OS/2 would do.
:p.
:hp1.Show MSE:ehp1.
.br
Show the MSE window.
:p.
:hp1.*Close window/Close window:ehp1.
.br
Well, close the window you clicked, of course.
:p.
:hp1.*Window to back/Window to back:ehp1.
.br
Send the window you clicked to the bottom of the ZOrder (window pile).
:p.
:hp1.*Window on screen/Window on screen:ehp1.
.br
Move the the window you clicked so that it is entirely on the screen.
:p.
:hp1.*Roll up/down/Roll up/down:ehp1.
.br
"Roll up" a window so that only its titlebar shows.  If the window's
already rolled up, roll it back down.  Note that not all windows take
kindly to such treatment, but most don't mind.
:p.
:hp1.*Move window/Move window:ehp1.
.br
:hp1.*Size window/Size window:ehp1.
.br
:hp1.*Maximize window/Maximize window:ehp1.
.br
:hp1.*Minimize window/Minimize window:ehp1.
.br
:hp1.Window list:ehp1.
.br
The same as if you'd selected these commands from the window's system menu.
:p.
:hp1.Switch list:ehp1.
.br
Calls up MSE's switch list (AKA MSE tasklist), from which you can select
a window, close it, hide it, or move it to the current desktop (if
you're using virtual desktops).  The switch list shows which desktop
each application is running in, if you're using virtual desktops.  When
you elect to close (AKA kill) a window in MSE, it does so as gently as
possible, but will go so far as to kill the process at the base operating
system level if required (if it doesn't respond appropriately to window
messages asking it to close itself).
:p.
:hp1.Next window:ehp1.
.br
Calls up the next window in the ZOrder (window pile).  Like pressing
Alt+TAB.
:p.
:hp1.B1 2-click:ehp1.
.br
Mimics a mouse button one double-click.  Note&colon.  This one works
better if there are no modifiers involved (i.e. assigned to mouse button
three with no key mods).  You can still use the chord shortcuts with
this command, but you must use them with the mouse pointer in a position
on the screen where a button one double-click won't evoke a reaction
(for example, on an empty spot of the desktop).  Tech note&colon.  MSE
waits for a quarter of the time you've allotted for a double-click
before sending out the second click (to make it sorta "smell" like a
human did it to any ill-mannered applications that actually take timing
into consideration).  You can adjust the timing using the Mouse WPS
object (there's a button for it on MSE's Cover page).
:p.
:hp1.B1+2 Chord:ehp1.
.br
Mimics a mouse button one + mouse button two chord (a chord is when you
press two or more mouse buttons simultaneously).
:p.
:hp1.Win text->clip:ehp1.
.br
Copies the clicked window's window text to the clipboard.
:p.
:hp1.Win text->clip append:ehp1.
.br
Appends the clicked window's window text to the clipboard.  Note that OS/2
seems to have a 64k limit to clipboard text...
:p.
:hp1.Clip->Win text:ehp1.
.br
Sets the clicked window's window text to the clipboard's text.
:p.
:hp1.Start FM/2:ehp1.
.br
Starts File Manager/2, best file manager around.  What, you don't have a
copy of the program that won a 1995 OS/2 Magazine Editor's Choice award,
a 1996 OS/2 e-Zine! Reader's Choice award, and was nominated for SIA's
best shareware utility of 1997?!?  Get one!
http&colon.//www.bmtmicro.com/catalog/fm2
:p.
:hp1.Open object:ehp1.
.br
Brings up a dialog that lets you open any file, folder or named WPS object.
:p.
:hp1.Minimize all:ehp1.
.br
Minimizes all windows.
:p.
:hp1.Menu #1-#7:ehp1.
.br
Brings up a user-programmable menu.  See the :link reftype=launch
object='E.EXE' data='.\MSEMENU1.DAT'.MSEMENU1.DAT:elink. that came with
MSE for an example.
:p.
:hp1.Virtual desktops:ehp1.
.br
Displays the virtual desktop pick window.
:p.
:hp1.Clipboard manager:ehp1.
.br
Displays the :link reftype=hd res=155.clipboard manager:elink. window.
:p.
:hp1.Calculator:ehp1.
.br
Displays a simple calculator.
:p.
:hp1.OS/2 command line:ehp1.
.br
Starts an OS/2 command line session.
:p.
:hp1.Scrollbar:ehp1.
.br
Hides the cursor and puts the mouse in control of the scrollbar that was
clicked upon.  Hold down the Shift key to scroll a "page" at a time
rather than a "line" at a time.  Click any button to return mouse
control to normal.  This works particularly well with trackballs.
:p.
:hp1.Pick click:ehp1.
.br
Brings up a menu from which you can pick an MSE command to apply to the
clicked window.  This allows you to get to any MSE command without
having to use the keyboard, for those who have physical problems with
the keyboard modifiers.
:p.
Commands prefaced with an asterisk (*) only work if you click a
titlebar.  Otherwise, the OS/2 default click is used.
:p.
.br
:hp3.Hint&colon.:ehp3.  You should probably :hp3.not:ehp3. assign a
command to mouse button one or mouse button two with no modifiers or
with the Alt modifier -- these are used extensively by the WPS and PM.
 :hp2.Ignore this advice at your peril.:ehp2.  If you try it anyway and
get yourself into a jam, remember that the Scroll Lock key will
temporarily disable MSE so you can fix it.


:h1 res=117.Buttons page 2
:i1 id=buttons2.Buttons page 2
:i2 refid=buttons2.Miscellaneous button command settings page

:artwork name='mseicon.bmp' runin.
Settings on this page affect the way button commands work.
:p.
:hp1.Show MSE in MSE tasklist:ehp1. controls whether the MSE tasklist
:link reftype=hd res=115.button command:elink. will show MSE as a
selectable item.
If you :hp1.Enable chord shortcuts:ehp1., MSE will attempt to activate
the following chord shortcuts:  Hold down mouse button three, then press
mouse button two to display the MSE notebook.  Hold down mouse button
three, then press mouse button one to show the :link reftype=hd
res=145.virtual desktop window:elink.. Hold down mouse button three,
then press mouse button two while holding down either the Shift, Alt or
Ctrl keys to display the :link reftype=hd res=155.clipboard
manager:elink. window.
:p.
The :hp1.Max scrollbar runahead:ehp1. field fine-tunes MSE's Scrollbar
:link reftype=hd res=115.button command:elink..  The faster you move the
mouse, the more MSE will scroll the scrollbar in "line scroll" mode,
(not in "page scroll" mode when the Shift key's down) by sending
multiple "line down" commands to the scrollbar.  This setting determines
how "far ahead" you can get by a fast mouse motion.  Adjust to taste --
the default is 4.  A setting of 1 will disable runahead.
:p.
By the way, this page is page two of the :link reftype=hd
res=110.Buttons page:elink. -- go to the Buttons page, then click the +
symbol on the dog-eared corner of the page.


:h1 res=120.Mouse page
:i1 id=mouse.Mouse page
:i2 refid=mouse.Miscellaneous mouse settings page

:artwork name='mseicon.bmp' runin.
Settings on this page affect various ways that the mouse can be made to
behave under MSE.
:p.
The :hp1.Sliding focus:ehp1. checkbox causes the window under the mouse
to receive the focus without clicking.  The :hp1.No ZOrder change:ehp1.
checkbox prevents the window from being brought to the top.
:p.
:hp1.Center mouse on selected windows:ehp1. causes MSE to place the
mouse pointer over the center of windows selected from the MSE switch
list or virtual desktop selection window.  :hp1.Mouse to default
button:ehp1. causes MSE to place the mouse pointer over the default
button of a dialog when it pops up.  The :hp1.Center mouse on dialogs
without default buttons:ehp1. checkbox causes MSE to center the mouse
over dialogs that don't have a default button.  Test the last one -- it
might sound good, but you may not like it much in practice.
:p.
The :hp1.Wrap at screen edges:ehp1. checkbox causes the mouse pointer to
wrap to the opposite side of the screen instead of stopping at an edge.
Note that when this setting is checked, you'll probably want "Shift
required to bump desktops" on the :link reftype=hd res=140.Virtual
page:elink. checked as well.
:p.
The :hp1.Mouse motion reaction delay:ehp1. field tells MSE how many
milliseconds (1/1000ths of a second) to delay reacting when the mouse
moves.  This delay applies to sliding focus, desktop bumping, mouse
wrapping and corner command activation, and can provide a feeling of
"resistance" which may give you useful feedback.  The default is 0 (no
delay).  Try starting with low (~50s) figures.
:p.
(By the way, this is a good time to mention that MSE updates the
internal values behind text entry fields when you move the focus off the
text entry field.  You can do this by hitting the Tab key or selecting
another control, page or application).
:p.
The :hp1.Seconds until pointer hides:ehp1. field controls how many
seconds MSE waits while the mouse pointer doesn't move before making the
mouse pointer invisible (move the mouse to bring it back).  Enter 0,
which is the default, to disable this feature.  Note that this is only
accurate to about ten second intervals.  There is no interrelationship
between this timer and the Mouse motion reaction delay timer above.
:p.
If the :hp1.Autodrop combo boxes:ehp1. checkbox is set, the listbox part
of drop-down combo boxes will be activated when you click the entry
field (or deactivated if already open).
:p.
You can use Corner commands to have MSE invoke a command when you move
the mouse pointer into one of the four corners of the desktop. The
listbox contains possible commands (a subset of the usual :link
reftype=hd res=115.button commands:elink.).  There is also a
representation of the desktop with radio buttons in the four corners.
Select a radio button and the listbox shows the command associated with
that corner. Select another command from the listbox by double-clicking
it, and that command becomes the command for that corner. The
:hp1.Nothing:ehp1. command can be used to make the corner behave
normally -- nothing happens when you move the mouse there.


:h1 res=130.FileDlg page
:i1 id=filedlg.FileDlg page
:i2 refid=filedlg.Enhanced file dialog settings page

:artwork name='fdlgicon.bmp' runin.
You can :hp1.Enable enhanced file dialog:ehp1. to have MSE provide an
:link reftype=hd res=135.improved standard file dialog:elink..
Applications that use custom file dialogs will not be "enhanced," but
most don't.
:p.
There are some toggles you can use to turn on or off some of the
enhancements offered.
:p.
The :hp1.Confirm directory delete:ehp1. and :hp1.Confirm file
delete:ehp1. checkboxes determine whether you are asked to confirm
deletion of those two types of file system objects.
:p.
The :hp1.Remember directories in which files were selected:ehp1. and
:hp1.Remember files that were selected:ehp1. checkboxes determine
whether a "ring" of the last 25 directories and files is kept for
you in the "User-defined directories" and "User-defined files"
dropdown boxes in the enhanced file dialog.
:p.
The :hp1.Be aggressive in enhancing:ehp1. checkbox tells MSE to be a
little more reckless when taking over the file dialog.  Use this with
caution; it can cause problems with some programs (for example, MR/2 Ice
crashes if this is checked and the file dialog comes up).  If this
toggle causes problems for you with your program suite, you'll either
have to discontinue its use, or exclude those programs which have
problems (see below).
:p.
The :hp1.Editor:ehp1. entry field can be used to enter the name of your
favorite text editing program.  E.EXE is used as the default if you have
nothing entered here.  This is used to edit the user-defined directory
and file lists, as well as various exclude files.
:p.
The :hp1.Viewer:ehp1. entry field can be used to enter the name of
your favorite file viewing program.  The AV/2 module from FM/2 is used
as the default if you have nothing entered here.
:p.
You can selectively exclude programs from this by placing
the name of the program in a list file named :hp2.FEXCLUDE.LST:ehp2. in
the MSE directory.  The :hp1.Exclusions:ehp1. button will allow
you to edit this file.


:h2 res=135.Enhanced file dialog
:i1.Enhanced file dialog

If you're tired of that dinky little file dialog that OS/2 uses -- you
know, the one that makes you scroll around a lot in tiny listboxes
looking for files and directories -- try enabling this enhancement on
the :link reftype=hd res=130.FileDlg page:elink..
:p.
MSE alters the standard file dialog to be full screen height to cut down
on scrolling, adds user-definable directory and file pulldowns,
at-a-glance help, and even allows you to open or delete files and
directories from the dialog.
:p.
Wave your mouse pointer around over the various fields in the dialog,
and a quick description of the basic function of the fields will be
shown at the top of the window as the pointer passes over them.
:p.
Not every program uses the standard file dialog box.  MSE tries to be
intelligent about when it steps in and when it doesn't, but if you run
into conflicts, you can exclude some programs from using this
enhancement.  Click the "Exclusions" button on the FileDlg page to do
this.
:p.
:hp3.Hints&colon.:ehp3.  The :hp1.File&colon.:ehp1. and
:hp1.Directory&colon.:ehp1. text fields are clickable -- they open the
WPS object for the first highlighted file or current directory,
respectively.  The text above the filename entry field, when clicked,
will view whatever file is entered in that field.  Clicking the
Directory text field with mouse button two (usually the right button)
will let you create a directory.


:h1 res=140.Virtual page
:i1 id=virtual.Virtual page
:i2 refid=virtual.Virtual desktops settings page

:artwork name='virticon.bmp' runin.
Check the :hp1.Enable virtual desktops:ehp1. checkbox to turn on :link
reftype=hd res=145.virtual desktops:elink., which expand your available
desktop space nine times.  You can elect to have the window's title text
shown in the window representation or not using :hp1.Show text in
virtual window:ehp1..
:p.
The :hp1.Leave WPS folders:ehp1. toggle determines whether opened WPS
folders are present on all desktops, or only in one (or, put another
way, whether they move from desktop to desktop as you do, or stay put).
:p.
You can :hp1.Mouse-bump desktops:ehp1., which means you'll switch
virtual desktops when the mouse bumps up against the edge of the screen.
Note that this is a 3-state checkbox -- when checked, mouse bumping is
on; when "greyed" (third state -- the funny looking one), mouse bumping
is on and the mouse will be positioned to the opposite side of the
screen after bumping.  You can opt to have :hp1.Shift required to bump
desktops:ehp1. which can help eliminate accidentally bumping to another
virtual desktop during normal mouse motion.
:p.
MSE usually "normalizes" the desktops when it exits (in other words,
places all running programs back on the real desktop), but you can check
the :hp1.Don't normalize on exit:ehp1. button to prevent this.  You can
still use the :hp1.Reset desktops:ehp1. command, which places you on the
real desktop, AKA the center virtual desktop (#5), and moves all windows
to that virtual desktop.
:p.
The :hp1.Show desktops:ehp1. button calls up the virtual desktop
selection window.
:p.
You may want to exclude some windows from the virtual desktop array (or
think of it as having some windows available on all virtual desktops).
Simply place the titles of the windows in a text file named
:hp2.EXCLUDE.LST:ehp2. in the MSE directory.  The :hp1.Exclusions:ehp1.
button will allow you to edit this file.  Partial titles are sufficient,
one per line.


:h2 res=145.About virtual desktops
:i1.About virtual desktops

:artwork name='virtual.bmp' align=left.
This is what virtual desktops might look like if you could see them all
at once.  The desktop you start in is numbered "5" in the illustration
-- it has a couple of programs running in it.  Note that the WPS Desktop
is present in all virtual desktops.
:p.
:artwork name='bigdlg.bmp' align=left.
The virtual desktop selection window displays a graphical representation
of all the desktops combined, including open windows.  As you move the
mouse pointer over the grid, it shows the name of the top window, if
any, under the pointer.  When you click on a grid square, that desktop
is made the active desktop, and if any open window was under the
pointer, that window is made the active window, as well.
:p.
:hp3.Hints&colon.:ehp3.  Minimized and hidden windows don't "move" when
you change desktops -- in other words, they restore to whatever the
current desktop happens to be.  You can see which virtual desktop
programs are running in by using the MSE switch list :link reftype=hd
res=115.button command:elink..  The default action when you select a
program from that switch list is to change to the desktop that holds it,
but you can hold down the Shift key while clicking to bring that program
to the current desktop.  Hold down Shift while clicking, and the window
under the mouse pointer (if any) is brought to the current desktop (the
current desktop doesn't change).  Requesting a context menu brings up
the MSE switch list.  You can drag fonts onto the window (both the main
window and the tip window).  Need smaller fonts in the main window?  You
can enter a smaller point size than is shown in the dropdown listbox for
some fonts (try 6-point Helv, for example) in the font dialog.


:h1 res=150.Clipbrd page
:i1 id=clipbrd.Clipbrd page
:i2 refid=clipbrd.Clipboard manager settings page

:artwork name='clip.bmp' runin.
If you check :hp1.Enable clipboard manager:ehp1., MSE will turn on a
special :link reftype=hd res=155.clipboard manager:elink. window that
can save text clipboards, allow you to switch between them, view
standard clipboard formats, and many other functions.
:p.
:hp1.Autosave clipboards:ehp1. causes MSE to save text clipboards to
disk when MSE closes.  You can also manually save clipboards using the
context menu you get on the clipboard manager's tip window.  Saved
clipboards are always automatically loaded when the clipboard manager is
activated (which may be when you start MSE, if you last closed it with
the clipboard manager active).
:p.
You can elect to :hp1.Append clipboard:ehp1., which causes MSE to add
the text just placed into the clipboard to the end of the current
MSE clipboard entry (:hp2.not:ehp2. the "real" clipboard).  In order
to avoid runaway problems, MSE only appends the text if it's not already
contained in the text in that clipboard entry.
:p.
Click the :hp1.Show clipboard manager:ehp1. button to display the
clipboard manager (you can also set up a :link reftype=hd res=115.button
command:elink. to display the clipboard manager window).
:p.
The :hp1.Show clipboard folder:ehp1. button opens the WPS folder where
clipboards are saved.


:h2 res=155.Clipboard manager
:i1.Clipboard manager

:artwork name='clip.bmp' runin.
The MSE clipboard manager includes a clipboard viewer that displays
text, bitmaps and metafiles.  Context menus (also known as popup menus
or button two menus) are available on all windows to let you save files,
load files into the clipboard, adjust settings, etc.  You can also drag
files to the various windows to import them into the clipboard.
:p.
The clipboard manager automatically saves the last twenty-five text
entries in the clipboard for quick recall via context menu.  The two
arrow buttons to the right of the tip window let you cycle through
those entries.  The :link reftype=hd res=3.command line argument:elink.
:hp1./n#:ehp1. can be used to increase or decrease the number of saved
entries available.  This argument is only valid when starting MSE --
you can't change it on-the-fly.
:p.
The clipboard manager can be assigned to an :link reftype=hd res=115.MSE
button command:elink..
:p.
Note&colon.  To import pictures to the clipboard, they need to be in
OS/2 bitmap format (.BMP).  You can convert pictures in other formats to
bitmap format using the OS/2 image viewer (ib.exe) or IMAGE.EXE from the
FM/2 Utilities package, if desired.  Also, don't forget that doing an
Edit->Copy in most image manipulation packages places a bitmap in the
clipboard.
:p.
If you're already running a clipboard viewer (for example, CLIPOS2.EXE),
MSE's clipboard manager won't install itself.  Close the other clipboard
viewer first.  If you run the OS/2 clipboard viewer, MSE's clipboard
manager will get knocked out of the loop (the OS/2 clipboard viewer is
busted).  If this happens, use the Settings dialog to turn the clipboard
off and then back on.
:p.
:hp3.Hints&colon.:ehp3.  To save the clipboard manager's size and
position, close (not minimize, but close -- F3 is the accelerator key)
the window.  This also saves any clipboard entries to disk if you have
Autosave clipboards turned on.  Don't overlook the context menu on the
tip window itself.  If you want to copy text to the clipboard and
:hp1.not:ehp1. have the clipboard manager save it, turn on the Scroll
Lock light before copying (don't forget to turn it back off after).
:p.
Here's a picture of the clipboard manager window with all the windows
turned on and labelled.  You can turn off the windows whose descriptions
are prefaced with '*'.
:artwork name='clipmgr.bmp' align=left.


:h1 res=160.Screen page
:i1 id=screen.Screen page
:i2 refid=screen.Screen capture settings page

:artwork name='scrnicon.bmp' runin.
The :hp1.Enable screen capture:ehp1. checkbox allows MSE to save bitmaps
of the screen when you press the PrintScrn key.  Bitmaps are saved to
the SCRNSHTS directory off of the MSE directory and are named
SCRN###.BMP, where ### is an incrementing number to allow multiple
captures.  Note that VIO windows eat the PrintScrn keystroke when they
have the focus -- change the focus to some other window, or the desktop,
in that case.  If someone knows the trick to work around this
programmatically, let me know.
:p.
You can choose to have MSE :hp1.Capture only active window:ehp1. if you
like.
:p.
The :hp1.Show screenshot folder:ehp1. button beside this checkbox opens
up the WPS folder for the SCRNSHTS directory so you can do maintenance
there.
:p.
A command file, :hp2.AFTRSCRN.CMD:ehp2., is run after screen shots are
captured so that you can further process the resultant bitmap file if
desired.  A sample came with MSE which you can edit to taste; clicking
the :hp1.Edit AFTRSCRN.CMD:ehp1. button will allow you to do this.
:p.
Finally, you can set the number of :hp1.Color bits:ehp1. to use when
capturing the screen.  A value of 1 gives a black-and-white bitmap, a
value of 4 gives a 16-color bitmap, a value of 8 gives a 256-color
bitmap, and a value of 24 gives a "true color" bitmap.  The higher this
value, the more diskspace and (very temporarily) memory the bitmap
requires.


:h1 res=170.Monitors page
:i1 id=monitors.Monitors page
:i2 refid=monitors.Monitors settings page

:artwork name='monicon.bmp' runin.
Settings on this page allow you to turn on and control MSE's built-in
monitors (small windows that track some part of your system).
:p.
The :hp1.Enable swapfile monitor:ehp1. checkbox causes MSE to display a
small swapfile counter on the screen. The size of the swapfile and the
amount of free space on the drive holding the swapfile are shown.  If
the free space falls below 32k, the background of this monitor window
turns red.  The swapfile monitor has some minor smarts -- it normally
checks the swapfile about once every two minutes, but when changes are
noticed or freespace is below 32k, it checks more often.  You can
:hp1.Show swap drive free space:ehp1. (the room the swapfile has to
grow) in the monitor as well, if you like.
:p.
If you :hp1.Enable clock:ehp1., MSE displays a small clock on the
desktop.  You can :hp1.Include date:ehp1. in the clock.  If you prefer a
12-hour clock with am and pm designations, check :hp1.12-hour (am/pm)
clock:ehp1..  The clock is updated about once every minute.
:p.
The :hp1.Enable harddrive monitor:ehp1. checkbox causes a monitor to
appear that displays the freespace on a hard drive.  Clicking the
monitor changes the hard drive being monitored (increments it by one, so
if C: is being monitored, D: will be monitored).  You can cause MSE to
:hp1.Show free space in menus:ehp1. called up on this monitor.
Note&colon. some drives take a bit of time to respond to the request
to find out how much space is available on them.
:p.
:hp1.Enable process monitor:ehp1. turns on a monitor that keeps track of
how many processes and threads are running on the system.
:p.
Check :hp1.Enable CPU monitor:ehp1. to have MSE display a monitor that
sorta keeps track of CPU usage.  It ain't any more scientific than Pulse,
but it ain't any worse.  You can adjust the milliseconds between samples
using the :hp1.CPU sample milliseconds:ehp1. entry field.  The default's
1000 (once a second).  You can't go lower than 100 milliseconds.  You can
:hp1.Show average CPU usage:ehp1. in the monitor if you like.  I don't
recommend running a CPU monitor all the time, by the way.  I didn't include
a pretty graph because Pulse, which comes with OS/2, already does that.
:p.
You can :hp1.Enable clipboard monitor:ehp1. to have a way to keep an eye
on what kind of data is in the clipboard in a very small window, and to
keep a way to quickly get to the clipboard manager window lying about.
The display shows what type of data is in the clipboard between two
brackets [].  T, B and M are shown for text, bitmap and metafile data.
Lowercase letters are shown for the corresponding alternate display
formats.  A P is shown for a palette format.  If one of the types is
empty, a dash - is displayed.  Additionally, if :link reftype=hd
res=150.clipboard append:elink. is on, a plus sign + appears at the
right side of the monitor.
:p.
You can :hp1.Enable virtual memory monitor:ehp1. to have MSE display the
free including virtual memory on the system in a monitor. You can also
elect to :hp1.Show 'physical memory':ehp1. to have MSE show the amount
of free RAM in the system (note&colon. the free physical memory number
doesn't really mean all that much under a virtual memory operating
system like OS/2...).
:p.
If you'd like the monitors to "float" to the top of the window stack
every once in awhile, check the :hp1.Float monitors to top:ehp1.
checkbox.  If you'd like monitors to respond to being clicked by mouse
button 1 simply by coming to the top of the window stack (Zorder)
without activating any commands, check :hp1.Disable monitor click:ehp1.
(you can still double-click to activate the commands).  MSE usually
rounds numbers in monitors to whole numbers, but you can have it show
up to two numbers to the right of a decimal point for more accuracy
if you check :hp1.Show fractions:ehp1..
:p.
:hp3.Hints&colon.:ehp3.  Click the clock to call up the OS/2 System
Clock's Settings so you can change the time and/or date (hold down Shift
while clicking to get the clock itself).  Click the swapfile monitor to
call up the OS/2 Launchpad.  Click the CPU monitor to call up OS/2's
Pulse program (the MSE CPU monitor goes idle until you close Pulse).
Click the clipboard monitor to call up MSE's clipboard manager.  Click
the memory monitor to open a command line. Click the process monitor to
start FM/2's Kill Process program. Drag any monitor to any position you
like on the screen with mouse button two (the regular drag-an-object
button). Change the colors and font of monitors by dropping colors and
fonts onto them from the system palettes (remember, use Ctrl while
dropping a color to change the text color). Click and hold mouse button
one on either while "dragging" the mouse to show MSE.  Click mouse
button two on any monitor to force an immediate window update and get a
context menu.


:h1 res=180.Titlebars page
:i1 id=titlebars.Titlebars page
:i2 refid=titlebars.Enhanced titlebars settings page

:artwork name='titleba2.bmp' runin.
If you check the :hp1.Enable titlebar enhancements:ehp1. checkbox, MSE
will draw your titlebars using gradient fills that you can customize, or
bitmaps.  You can even have different titlebars for different programs.
:p.
You can elect to have titlebars popup an abbreviated system menu
by checking the :hp1.Enable titlebar menus:ehp1. checkbox.  You can
add some MSE-specific commands to this menu by checking :hp1.Add MSE
specifics to menus:ehp1..
:p.
You can have MSE display a clock in the titlebar of the active window
if you check :hp1.Put clock in titlebars:ehp1..  The clock displayed
there uses the settings of the :link reftype=hd res=170.clock
monitor:elink., except, of course, for the font.
:p.
The :hp1.:link reftype=hd res=185.Titlebar enhancement
parameters:elink.:ehp1. button calls up the dialog where you can
customize your titlebars.
:p.
Note&colon.  Parameters not configured by the titlebar enhancements
parameters dialog are still set the old-fashioned way -- by
drag-and-drop from the WPS Color and Font palettes, and/or the Scheme
palette.


:h2 res=185.Titlebar enhancement parameters
:i1.Titlebar enhancement parameters

:artwork name='titlebar.bmp' align=left.
MSE's titlebar enhancements allow you to give your titlebars a more
polished look.  As with all options in MSE, this code's not loaded
unless you use it.
:p.
The Titlebar enhancement parameters dialog allows you to set two colors
for your titlebars -- start and end.  MSE then uses these two colors to
draw a pleasing gradient background.  You can control whether the
gradient is drawn horizontally or vertically, and whether it starts at
one side or in the center.  You can also determine what kind of border
your titlebars have -- none, one that seems to "bulge" out, or one that
seems to "recess" inward.  Visual feedback is given on a "fake titlebar"
included in the dialog as you make changes.
:p.
The :hp1.Steps:ehp1. spin button allows you to control how many steps
are used in drawing the gradient, to compromise between speed and
smoothness.  The optimal setting for this will depend on the speed
of your computer and the color depth of your graphics.
:p.
For convenience, the :hp1.Invert gradient:ehp1. button will reverse
the settings of the starting and ending colors.
:p.
For a different look, try checking the :hp1.Texture bars:ehp1. button.
MSE will draw texture bars on the unused portion of the titlebar
(where text doesn't appear).
:p.
You can set a bitmap file to be displayed in a titlebar -- enter the
name of the bitmap file (.BMP extension included) into the entry field
next to the :hp1.Use bitmap>:ehp1. checkbox and make sure the checkbox
is checked.  You can use the :hp1.Find bitmap:ehp1. button to locate the
bitmap file.  Notes&colon.  MSE updates the bitmap name to the dialog
when you move the cursor from the entry field.  Small bitmaps that can
be tiled will probably work best (and load the fastest, and use the
least memory). OS/2 comes with a few of these, like PLASTER.BMP and
GMARBLE.BMP -- you can find them in the \OS2\BITMAP directory -- and
there are a lot of tileable backgrounds on the web.  By the way, if you
click the fake titlebar, MSE will tell you how tall titlebars are on
your system.  A tech note&colon.  MSE automatically trims in-memory
copies of titlebar bitmaps to be no taller than your titlebar, to
conserve as much memory as possible.  Finally, if you use IMAGE.EXE from
the FM/2 Utilities package, you can save bitmaps from other formats so
they can be used in titlebars -- save as an "IBM OS/2 1.3 PM Bitmap
Image" in as few colors as possible and still retain the desired
quality.  IMAGE also allows you to crop bitmaps.
:p.
The :hp1.Apply:ehp1. button updates the titlebar parameters without
dismissing the dialog (:hp1.Okay:ehp1. does the same thing, but
dismissed the dialog, too).  The :hp1.Undo:ehp1. button restores the
parameters to what they were before the dialog appeared (or you clicked
the Apply button).  :hp1.Cancel:ehp1. will dismiss the dialog without
updating titlebar parameters.
:p.
At the top of the dialog is a listbox where any "schemes" you've saved
with the :hp1.Add Scheme>:ehp1. button are shown.  Double-clicking
any scheme name places its settings into the dialog.  You can delete
any scheme by highlighting it in the listbox and clicking the :hp1.Del
Scheme^:ehp1. button.
:p.
To add a scheme, set up the titlebar parameters as desired, then enter a
name into the entry field beside the Add Scheme button. Finally, click
the Add Scheme button.  If you checked :hp1.Scheme is program
name:ehp1., MSE will apply this scheme to that program whenever it's
run.  For example, if you save a scheme named "FM3.EXE" with Scheme is
program name checked, the next time you run FM/2 that scheme will be
applied to FM/2's titlebars (note&colon. an instance already running
won't be affected).  Alternatively, you can make the scheme name
(partial) titlebar text, and check :hp1.Scheme is title text:ehp1..
Or, if you want to exclude the window from having its titlebar
enhanced, check the :hp1.Exclude this one:ehp1. button.  You can
rearrange the order of schemes using the up and down arrow button
above the listbox.
:p.
Remember, if you've been playing with schemes and don't want to update
the global titlebar parameters, click Cancel to exit the dialog, or
click Undo before clicking Okay.
:p.
Schemes are saved in a binary file named TTLBARS.DAT which is not
human-editable.
:p.
At this time, WPS windows lose the added " - Icon View" (or whatever
view it is) text when MSE is enhancing their titlebars -- I don't know
how to find this text!  If anyone does, let me know.  It still shows up
in the Window List.
:p.
:hp3.Hints&colon.:ehp3.  You can update the titlebars of running
instances of programs for which you've changed or added schemes by
turning off titlebar enhancements, then turning them back on.  You can
edit an existing scheme by double-clicking it, deleting it, changing it,
then adding it again.  You can get away with much lower Steps settings
for vertical gradients than for horizontal gradients.  Gradients look a
lot better on 16 or 24-bit color displays, but are passable on 8-bit. On
4-bit (16 color, like standard VGA mode) or less, you're just wasting
your time.  To set a scheme for WPS windows, use PMSHELL.EXE as the
scheme name and check Scheme is a program name.  For .INF files, use
VIEWDOC.EXE.  For DOS programs, you'll probably have to key on title
text.


:index.
:euserdoc.
