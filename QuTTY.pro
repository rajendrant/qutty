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

QT       += core gui network widgets

TARGET = QuTTY
TEMPLATE = app


SOURCES +=  \
    GuiMain.cpp \
    GuiMainWindow.cpp \
    GuiTerminalWindow.cpp \
    GuiSettingsWindow.cpp \
    GuiPreferencesWindow.cpp \
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
    GuiTreeWidget.cpp \
    GuiImportExportFile.cpp \
    GuiCompactSettingsWindow.cpp \
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
    GuiPreferencesWindow.h \
    GuiTreeWidget.h \
    GuiImportExportFile.h \
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
    GuiCompactSettingsWindow.h \
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
    GuiSettingsWindow.ui \
    GuiPreferencesWindow.ui

OTHER_FILES += \
    qutty.rc \
    qutty.ico

RC_FILE = qutty.rc

RESOURCES += \
    QuTTY.qrc


win32-msvc* {
    LIBS += user32.lib advapi32.lib
    QMAKE_CFLAGS    += -D_CRT_SECURE_NO_WARNINGS
    QMAKE_CXXFLAGS  += -D_CRT_SECURE_NO_WARNINGS
}

win32-g++ {
    QMAKE_CXXFLAGS  += -std=gnu++0x
}

Release: {
win32-msvc* {
# enable google-breakpad support for release mode in MSVC

INCLUDEPATH += third-party/google-breakpad/

SOURCES +=  \
    third-party/google-breakpad/client/windows/handler/exception_handler.cc \
    third-party/google-breakpad/client/windows/crash_generation/crash_generation_client.cc \
    third-party/google-breakpad/common/windows/guid_string.cc \

HEADERS +=  \
    third-party/google-breakpad/client/windows/common/ipc_protocol.h \
    third-party/google-breakpad/client/windows/crash_generation/crash_generation_client.h \
    third-party/google-breakpad/client/windows/handler/exception_handler.h \
    third-party/google-breakpad/common/scoped_ptr.h \
    third-party/google-breakpad/common/windows/guid_string.h \
    third-party/google-breakpad/common/windows/string_utils-inl.h \
    third-party/google-breakpad/google_breakpad/common/breakpad_types.h \
    third-party/google-breakpad/google_breakpad/common/minidump_cpu_amd64.h \
    third-party/google-breakpad/google_breakpad/common/minidump_cpu_arm.h \
    third-party/google-breakpad/google_breakpad/common/minidump_cpu_ppc.h \
    third-party/google-breakpad/google_breakpad/common/minidump_cpu_ppc64.h \
    third-party/google-breakpad/google_breakpad/common/minidump_cpu_sparc.h \
    third-party/google-breakpad/google_breakpad/common/minidump_cpu_x86.h \
    third-party/google-breakpad/google_breakpad/common/minidump_exception_win32.h \
    third-party/google-breakpad/google_breakpad/common/minidump_exception_linux.h \
    third-party/google-breakpad/google_breakpad/common/minidump_exception_mac.h \
    third-party/google-breakpad/google_breakpad/common/minidump_exception_ps3.h \
    third-party/google-breakpad/google_breakpad/common/minidump_exception_solaris.h \
    third-party/google-breakpad/google_breakpad/common/minidump_format.h \

    QMAKE_LFLAGS_RELEASE +=/MAP
    QMAKE_LFLAGS_RELEASE +=/debug /opt:ref

    DEFINES += QUTTY_ENABLE_BREAKPAD_SUPPORT="1"
}
}
