## QuTTY = PuTTY + Qt ##

QuTTY is a multi-tabbed, multi-paned SSH/Telnet client with a terminal emulator. The goal is to support advanced features similar to iterm2.

Just download and try it. If you see any issues you can try the following. If you still face issues report it to us.
  * install one of the Visual Studio runtimes [VC++ 2012](http://www.microsoft.com/en-us/download/details.aspx?id=30679) or [VC++ 2010](http://www.microsoft.com/en-us/download/details.aspx?id=5555).
  * Configure qutty executable to be run in compatibility mode for Windows 7/Vista/Xp to see if it helps.
  * Look for any coredumps in %USERPROFILE%\qutty\dumps\ and email them if any.

Report any issues or feature-requests in [issue-tracker](http://code.google.com/p/qutty/issues/list). We need help from volunteers to find and fix any blocking issues, so that a stable release can be made soon.

**Links to some documentation:**

  * [Features](Features.md)

  * [TabsInTitlebar](TabsInTitlebar.md)

  * [HierarchicalSessions](HierarchicalSessions.md)

  * [KeyboardShortcuts](KeyboardShortcuts.md)

  * [TmuxIntegration](TmuxIntegration.md)

  * [HowToBuild](HowToBuild.md)

### 0.8-beta release (08/21/2013) ###
Eighth beta release is made today with a few bugfixes.
Thanks to Dragan, Nick Bisby for reporting some important issues.
Thanks to Suriyapriya Veluchamy for fixing them as well.

### 0.7-beta release (07/29/2013) ###
Seventh beta release is made today with these features.
  * Major feature supported is tabs in titlebar similar to Firefox/Chrome. See [TabsInTitlebar](TabsInTitlebar.md) for more information.
  * Suriya priya Veluchamy has implemented support for logging sessions similar to Putty.
  * Few other minor features & bugfixes were in as well.

### 0.6-beta release (07/15/2013) ###
Sixth beta release is made today. Thanks to Suriya priya Veluchamy for implementing/testing these much-needed features. Thanks to Nick Bisby for suggesting these features as well.
  * Sessions can be organized in a hierarchical session tree.
  * A new compact settings window is introduced instead of the classical detailed PuTTY style window.
  * Auto completion of hostname allows selecting a session/profile easily.
  * Import/Export sessions is supported. This allows settings to be imported from XML file/PuTTY registry and to be exported as XML file.
  * Last but not least, QuTTY icon has been updated by Suriya priya (created from scratch using gimp 2.8.4 opensource tool, instead of freeware/shareware tools used for older icon. gimp project files are available as well, so that future modifications will be easy).

Also see [HierarchicalSessions](HierarchicalSessions.md) for more documentation & tips.

### 0.5-beta release (06/23/2013) ###
Fifth beta release is made today.
  * Main feature added is configurable keyboard shortcuts.
  * Navigation among Tabs/Split-panes is fully supported. See [KeyboardShortcuts](KeyboardShortcuts.md)

### 0.4-beta release (06/10/2013) ###
Fourth beta release is made today.
  * Main feature implemented is split-pane support.
  * Use ctrl+shift+h and ctrl+shift+v to create horizontal/vertical split-panes.
  * Drag-drop of split-panes is supported as well. To start dragging ctrl + click in terminal or use drag button in terminal top-right menu. Once dragged it can be dropped in other terminals.
  * ctrl+shift+Arrow keys to navigate between split-panes. Navigation is still preliminary.

### 0.3-beta release (05/28/2013) ###
Third beta release is made today. Many features have been added. Menus (Main menu, Tabbar right-click menu, ctrl-right-click on terminal) are supported. Multiple minor features are in as well. Some features shown in menu (Split session, Import/Export sessions) are still TODO. They will be implemented in next release, which will be soon in couple of weeks.

### 0.2-beta release (05/17/2013) ###
Second beta release is made today. I have been using this for a week now, and it is quite stable. One feature of note is automatic loading of saved PuTTY sessions when QuTTY is run first-time. Multiple minor features/fixes were in as well. Visual Studio 2012 Express edition along with Qt 4.8.4 is used for development, and the binary size is down to 2.7MB (Visual Studio Express 2010 with Qt 4.8.1 produced 3.8MB binary).

### 0.1-beta release (07/08/2012) ###
First beta release is made today. It works very well as a multi-tabbed SSH/Telnet client. Pretty decent configuration ui similar to PuTTY is supported. Keyboard shortcuts help navigate between tabs. Tmux command mode is integrated as well.

### update 05/05/2012 ###
Suriyapriya Veluchamy, a new developer has joined us. Stay tuned for Qutty's first release soon.

### update 04/10/2012 ###
QuTTY is still in development. For now telnet/ssh connectivity works with some basic features. Still some basic features/configuration options are missing. First release will be made soon once QuTTY supports this.
Then we will work on the advanced features similar to iterm2.