/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QMdiArea>
#include <QMenu>
#include <QToolButton>
#include "GuiMainWindow.h"
#include "QtCommon.h"
#include "GuiSettingsWindow.h"
#include "GuiMenu.h"

class GuiSettingsWindow;

class GuiMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    QAction *menuCommonActions[MENU_MAX_ACTION];
    QMenu *menuCommonMenus[MENU_MAX_MENU];
    QMenu menuTermWnd;              // shown in terminal ctrl-right-click
    GuiTerminalWindow *menuCookieTermWnd;
    int menuCookieTabIndex;
    QMenu menuSavedSessions;

    GuiMainWindow(QWidget *parent = 0);
    ~GuiMainWindow();
    GuiTerminalWindow *newTerminal();
    void createNewTab(Config *cfg);
    bool winEvent ( MSG * msg, long * result );
    void closeEvent ( QCloseEvent * event );
    GuiTerminalWindow *getCurrentTerminal();

    QTabWidget *tabArea;
private:
    GuiSettingsWindow *settingsWindow;
    QList<GuiTerminalWindow *> terminalList;
    QToolButton newTabToolButton;   // shown in top right corner of tabbar
    QMenu menuTabBar;               // shown in top right corner of tabbar

    void initializeMenuSystem();
    void populateMenu(QMenu &menu, qutty_menu_id_t menu_list[], int len);

    void readSettings();
    void writeSettings();

public slots:
    void on_openNewWindow();
    void on_openNewTab();
    void on_createNewTab(Config cfg);
    void on_settingsWindowClose();
    void on_changeSettingsTab(int tabIndex);
    void on_changeSettingsTabComplete(Config cfg, int tabIndex);
    void closeTerminal(int index);
    void closeTerminal(GuiTerminalWindow *termWnd);
    void tabCloseRequested (int index);
    void currentChanged(int index);
    void focusChanged ( QWidget * old, QWidget * now );

    void tabNext();
    void tabPrev();

    void contextMenuPaste();
    void contextMenuSavedSessionsChanged();
    void contextMenuSavedSessionTriggered();
    void contextMenuDuplicateSessionTriggered();
    void contextMenuRestartSessionTriggered();
    void contextMenuChangeSettingsTriggered();
    void contextMenuCloseSessionTriggered();
    void contextMenuCloseWindowTriggered();
    void contextMenuMenuBar();
    void contextMenuFullScreen();
    void contextMenuAlwaysOnTop();
};

#endif // MAINWINDOW_H
