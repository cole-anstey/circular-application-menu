# Circular-Application-Menu

A circular-application-menu (C-A-M) representation for the GNOME desktop.

Circular-Application-Menu (C-A-M) by Cole Anstey

This is a circular-application-menu (C-A-M) prototype mirroring the same structure as the existing application menu for the GNOME desktop. This was originally just a paper prototype which I did back in March 2007, and decided to code over Christmas 2007-2008.

## Other Implementations

http://en.wikipedia.org/wiki/Pie_menu

## YouTube Videos of C-A-M In Use

http://www.youtube.com/watch?v=nXh1Tm24kTE
http://www.youtube.com/watch?v=mNyLu6JoUX8
http://www.youtube.com/watch?v=5yvLfUFHuCc
http://www.youtube.com/watch?v=kzV44KmLJrA

## Updated

This project is only maintained periodically and no further contributions will be accepted.

29th March 2020 - Upgraded to the latest GTK+ 3 version.

3rd December 2011 - Steffen Rumberger partially migrated the codebase to GTK+ 3 and Debian “testing/Wheezy”.

28th November 2011 - Steffen Rumberger (aka. Inne) provided updates to make a Debian “Squeeze” package.

18th May 2009 - Fixed an error where .svg icons were not being used by default over any .xpm alternatives (http://code.google.com/p/circular-application-menu/issues/detail?id=15#c9).

14th May 2009 - Fixed an error where programs were launching and ignoring any arguments (http://code.google.com/p/circular-application-menu/issues/detail?id=15#c5). A command such as the following should now hopefully work 'nautilus /home/collie/code/circular-application-menu/'.

4th September 2008 - Added an request to display only one menu at a time (http://code.google.com/p/circular-application-menu/issues/detail?id=7). The shown menu in the hierachy can be changed via the Page-Up/PageDown/Home and End keys. Thanks to bitseven.

3rd September 2008 - Fixed an error than some people where experiencing during construction (http://code.google.com/p/circular-application-menu/issues/detail?id=6&can=1). Thanks to everyone who provided a stack trace.

27th August 2008 - Added a reflection effect by default. This stresses the processor somewhat; but it can be easily turned off (see below). Also added a highlight around the edge incase it's displayed over a black background.

25th August 2008 - Made any 'overlapped' portions of a menu by a sub menu translucent by 75%. This helps to stop the previous visual ambiguity with overlapped menus items merging with the current menu items. Also changed the small triangle which denoted a sub menu into a glyph to make it easier to differentiate between applications and menu types.

24th August 2008 - Added the ability to specify the emblem to use for the 'root' menu. By default it's the GNOME logo; but it can now be changed to allow for a specific distro logo. An Ubuntu one has been provided as an example. See the 'Application Options' section on how to specify your own.

23rd August 2008 - Changed the right mouse button click functionality slightly so it now aligns the currently shown menu to the click coordinate. Also fixed a bug so it warps the mouse to the centre of the screen when the root menu is initially shown.

28th July 2008 - The menu now uses the blur plugin by default; to blur underneath the menus. This can be turned off with an optional parameter (see below).

25th July 2008 - Fixed an error that sometimes occurred if the angle for a submenu was over 360.0 i.e. 360.6. This was a bugger to hunt down as it didn't ever happen for me, though I got reports that assertions were occasionally happening for other people in this scenario.

8th March 2008 - Initial revision.

## Requirements
* GNOME
* A composited environment (i.e compiz, etc.)
* gtk+ 3.0
* compiz-devel

## Installation

~~~bash
git clone https://github.com/cole-anstey/circular-application-menu.git

cd circular-application-menu

make
make install
~~~

## Usage:

    circular-main-menu [OPTION...] - circular-application-menu.

Help Options:

    -?, --help Show help options

Application Options:

    -h, --hide-preview

        Hides the menu preview displayed when the mouse is over a menu.

    -w, --warp-mouse-off

        Stops the mouse from warping to the centre of the screen whenever a menu is shown.

    -g, --glyph-size=S

        The size of the glyphs [S: 1=small 2=medium 3=large (default)].

    -b, --blur-off

        Stops the blur from underneath the menu.

    -e, --emblem=E

        Specifies the emblems (colon separated) to use for the root menu [E: ./pixmaps/ubuntu-emblem-normal.png:./pixmaps/ubuntu-emblem-prelight.png].

    -t, --render-tabbed-only

        Only shows the currently tabbed menu.

## Adding as a panel launcher

Right-click on any vacant panel space and choose 'Add to Panel' from the displayed context menu.
When the 'Add to Panel' dialog opens, enter these details (ignore the quotes).
~~~bash
    Name: 'Circular Applications Menu'
    Generic name: 'Circular Application Menu'
    Comment: 'A Circular Application Menu Representation.'
    Command: 'circular-application-menu'
    Type: 'Application'
    Icon: < Pick one you like, I haven't designed one yet! >
    Run In Terminal: < Choose this option so the command that does not create a window in which to run. >
~~~

When you click on a menu segment the segment disappears and its displayed as the tab of the child menu. You can also use Page-Up, Page-Down, Home and End keys to navigate.

Making C-A-M the default menu for Alt+F1 (Metacity)

Change the existing 'Main Menu' to Alt+F3:

    gconftool-2 --set --type string /apps/metacity/global_keybindings/panel_main_menu "


## Assign C-A-M as being Alt+F1

    gconftool-2 --set --type string /apps/metacity/keybinding_commands/command_1 circular-main-menu

    gconftool-2 --set --type string /apps/metacity/global_keybindings/run_command_1 "

## Anything Else

    Page-Up/PageDown - Quick jump between menus in the hierachy.

    Home/End - Jumps to the root menu or last sub menu.

    Right-Click - Moves the currently opened menu to the mouse position.

## Miscellaneous Information

C-A-M receives a mention in the September 2008 issue of the 'LINUX Magazine'.
Someone (not me) has dugg C-A-M !!! http://digg.com/linux_unix/Awesome_circular_application_menu_Google_Code
My other project which I used to work on quite a lot is available at http://curlyankles.sourceforge.net/.

## Project Information
License: GNU GPL v2
   
## Labels:  
gnome cairo applicationmenu menu gtk compiz circular cole CAM C-A-M anstey curlyankles ansley debian
