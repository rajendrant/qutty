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

QT       += core gui network widgets webkitwidgets

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
    QtSessionTreeModel.cpp \
    QtCompleterWithAdvancedCompletion.cpp \
    puttysrc/pgssapi.c \
    puttysrc/int64.c \
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
    puttysrc/be_all_s.c \
    tmux/TmuxBackend.cpp \
    tmux/TmuxGateway.cpp \
    tmux/TmuxWindowPane.cpp \
    tmux/TmuxLayout.cpp \
    serialize/QtMRUSessionList.cpp \
    plugin/GuiWebPlugin.cpp \
    serialize/QtWebPluginMap.cpp \
    puttysrc/settings.c \
    puttysrc/unix/gtkwin.c

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
    QtSessionTreeModel.h \
    QtSessionTreeItem.h \
    QtComboBoxWithTreeView.h \
    QtCompleterWithAdvancedCompletion.h \
    puttysrc/ssh.h \
    puttysrc/putty.h \
    puttysrc/missing.h \
    puttysrc/QtStuff.h \
    puttysrc/puttyps.h \
    puttysrc/network.h \
    puttysrc/misc.h \
    puttysrc/puttymem.h \
    puttysrc/terminal.h \
    puttysrc/tree234.h \
    puttysrc/sshgss.h \
    puttysrc/pgssapi.h \
    puttysrc/sshgssc.h \
    puttysrc/proxy.h \
    puttysrc/int64.h \
    puttysrc/ldisc.h \
    tmux/tmux.h \
    tmux/TmuxGateway.h \
    tmux/TmuxWindowPane.h \
    tmux/TmuxLayout.h \
    serialize/QtMRUSessionList.h \
    plugin/GuiWebPlugin.h \
    plugin/guiwebterminal.h \
    serialize/QtWebPluginMap.h


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

win32-* {
SOURCES +=  \
    puttysrc/windows/winnoise.c \
    puttysrc/windows/winstore.c \
    puttysrc/windows/windefs.c \
    puttysrc/windows/settings.c

HEADERS +=  \
    puttysrc/windows/STORAGE.H
}

win32-msvc* {
    LIBS += user32.lib advapi32.lib
    QMAKE_CFLAGS    += -D_CRT_SECURE_NO_WARNINGS
    QMAKE_CXXFLAGS  += -D_CRT_SECURE_NO_WARNINGS
}

win32-g++ {
    QMAKE_CXXFLAGS  += -std=gnu++0x -Wreorder -Wunused -fpermissive -Wdelete-non-virtual-dtor
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

linux-g++ {
QMAKE_CXXFLAGS += -std=c++11
SOURCES +=  \
    puttysrc/unix/uxnoise.c \
    puttysrc/unix/uxstore.c
HEADERS +=  \
    puttysrc/unix/storage.h \
    puttysrc/unix/unix.h
}
