#--------------------------------------------------------
# Copyright (C) 2012 Rajendran Thirupugalsamy
# See LICENSE for full copyright and license information.
# See COPYING for distribution information.
#--------------------------------------------------------
 
#-------------------------------------------------
#
# Project created by QtCreator 2012-01-02T17:45:25
#
#-------------------------------------------------

QT       += core gui network

TARGET = QuTTY
TEMPLATE = app


SOURCES +=  \
    GuiMain.cpp \
    GuiMainWindow.cpp \
    GuiTerminalWindow.cpp \
    GuiSettingsWindow.cpp \
    terminalmisc.cpp \
    QtCommon.cpp \
    QtNet.cpp \
    QtUnicode.cpp \
    QtPageant.cpp \
    QtDlg.cpp \
    QtGss.cpp \
    QtX11.cpp \
    QtConfig.cpp \
    GuiTabWidget.cpp \
    GuiMenu.cpp \
    GuiBase.cpp \
    GuiSplitter.cpp \
    GuiDrag.cpp \
    GuiFindToolBar.cpp \
    GuiNavigation.cpp \
    puttysrc/WINDOWS/winnoise.c \
    puttysrc/WINDOWS/winstore.c \
    puttysrc/WINDOWS/windefs.c \
    puttysrc/WINDOWS/settings.c \
    puttysrc/PGSSAPI.C \
    puttysrc/INT64.C \
    puttysrc/ssh.c \
    puttysrc/proxy.c \
    puttysrc/portfwd.c \
    puttysrc/misc.c \
    puttysrc/terminal.c \
    puttysrc/ldisc.c \
    puttysrc/ldiscucs.c \
    puttysrc/timing.c \
    puttysrc/sshbn.c \
    puttysrc/sshpubk.c \
    puttysrc/sshrand.c \
    puttysrc/sshrsa.c \
    puttysrc/sshsh256.c \
    puttysrc/sshsh512.c \
    puttysrc/sshsha.c \
    puttysrc/x11fwd.c \
    puttysrc/tree234.c \
    puttysrc/telnet.c \
    puttysrc/pinger.c \
    puttysrc/sshmd5.c \
    puttysrc/sshaes.c \
    puttysrc/sshdes.c \
    puttysrc/sshzlib.c \
    puttysrc/sshdh.c \
    puttysrc/sshblowf.c \
    puttysrc/ssharcf.c \
    puttysrc/sshcrcda.c \
    puttysrc/sshdss.c \
    puttysrc/cproxy.c \
    puttysrc/logging.c \
    puttysrc/minibidi.c \
    puttysrc/sshgssc.c \
    puttysrc/wcwidth.c \
    puttysrc/sshcrc.c \
    puttysrc/wildcard.c \
    puttysrc/BE_ALL_S.C \
    tmux/TmuxBackend.cpp \
    tmux/TmuxGateway.cpp \
    tmux/TmuxWindowPane.cpp \
    tmux/TmuxLayout.cpp

HEADERS +=  \
    GuiMainWindow.h \
    GuiTerminalWindow.h \
    GuiSettingsWindow.h \
    QtCommon.h \
    QtLogDbg.h \
    QtConfig.h \
    QtConfigTag.h \
    QtTimer.h \
    GuiTabWidget.h \
    GuiMenu.h \
    GuiBase.h \
    GuiSplitter.h \
    GuiDrag.h \
    GuiFindToolBar.h \
    GuiTabBar.h \
    GuiNavigation.h \
    puttysrc/WINDOWS/STORAGE.H \
    puttysrc/TREE234.H \
    puttysrc/TERMINAL.H \
    puttysrc/SSHGSSC.H \
    puttysrc/SSHGSS.H \
    puttysrc/SSH.H \
    puttysrc/PUTTYPS.H \
    puttysrc/PUTTYMEM.H \
    puttysrc/PUTTY.H \
    puttysrc/PROXY.H \
    puttysrc/PGSSAPI.H \
    puttysrc/NETWORK.H \
    puttysrc/missing.h \
    puttysrc/LDISC.H \
    puttysrc/INT64.H \
    puttysrc/QtStuff.h \
    puttysrc/MISC.H \
    tmux/tmux.h \
    tmux/TmuxGateway.h \
    tmux/TmuxWindowPane.h \
    tmux/TmuxLayout.h


INCLUDEPATH += ./ puttysrc/

FORMS += \
    GuiSettingsWindow.ui

OTHER_FILES += \
    qutty.rc \
    qutty.ico

RC_FILE = qutty.rc

RESOURCES += \
    QuTTY.qrc


win32-msvc*:contains(QMAKE_TARGET.arch, x86_64) : {
    LIBS += user32.lib advapi32.lib
}

win32-msvc* {
    QMAKE_CFLAGS    += -D_CRT_SECURE_NO_WARNINGS
    QMAKE_CXXFLAGS  += -D_CRT_SECURE_NO_WARNINGS
}
win32-g++ {
    QMAKE_CXXFLAGS  += -std=gnu++0x
}
