# Prerequisites #

  * Qt 4.8.0 or Qt 4.8.4 (Qt 5.0 is not yet supported)
  * Visual Studio Express 2010 or 2012 or MingW gcc
  * git to download code

# Download code #

  * `git clone https://code.google.com/p/qutty/`

# Compiling #

Visual Studio Express or Mingw gcc compilers are supported in Qt. You can either use Qt Creator GUI or use command prompt to compile.

## Using Qt Creator ##

Compiling using Qt Creator is straight forward. Just open the 'QuTTY.pro' file in Qt Creator. You should be able to build & run the project from 'Build' menu.
Qt Creator should be already configured with one of the supported compiler.

## Using command prompt ##

Lets assume Qutty has been downloaded to `c:\tmp\qutty\` and Qt has been installed in `c:\Qt\qt-4.8.4\`. Note that the installed Qt should support the compiler you are planning to compile qutty with.

### Visual Studio command prompt ###
Open the Visual Studio command prompt from `Start Menu -> All Programs -> Microsoft Visual Studio -> Visual Studio Tools -> Native Tools command prompt`
  * `cd c:\tmp\qutty`
  * `c:\Qt\qt-4.8.4\bin\qmake.exe -spec win32-msvc2012`
  * `nmake`
Note that you should use `-spec win32-msvc2010` if compiling using Visual Studio Express 2010.

### gcc command prompt ###
Open the command prompt. Make sure mingw bin path is included in your `PATH` environment variable.
  * `cd c:\tmp\qutty`
  * `c:\Qt\qt-4.8.4\bin\qmake.exe -spec win32-g++`
  * `mingw32-make`

The qutty binary will be created in `bin\QuTTY.exe`