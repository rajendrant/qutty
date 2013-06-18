/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <map>
#include <QMainWindow>
#include <QMdiArea>
#include <QMenu>
#include <QToolButton>
#include <QShortcut>
#include "QtCommon.h"
#include "GuiSettingsWindow.h"
#include "GuiMenu.h"
#include "GuiDrag.h"
#include "GuiNavigation.h"

class GuiSettingsWindow;
class GuiFindToolBar;
class GuiTabWidget;

class GuiMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    QAction *menuCommonActions[MENU_MAX_ACTION];
    QShortcut *menuCommonShortcuts[MENU_MAX_ACTION];
    QMenu menuCommonMenus[MENU_MAX_MENU];
    GuiTerminalWindow *menuCookieTermWnd;
    QMenu menuSavedSessions;
    GuiToolbarTerminalTop toolBarTerminalTop;    // top-right of terminal in split-mode

    // members for drag-drop support
    GuiDragDropSite dragDropSite;

    // find window
    GuiFindToolBar *findToolBar;

    // tab order-of-usage navigation window
    uint32_t mru_count_last;
    GuiTabNavigation *tabNavigate;
    GuiPaneNavigation *paneNavigate;

    GuiTabWidget *tabArea;

private:
    GuiSettingsWindow *settingsWindow;
    QList<GuiTerminalWindow *> terminalList;
    QToolButton newTabToolButton;   // shown in top right corner of tabbar

    // containers for fast-retrieval
    std::map<const QWidget*,int> tabIndexMap;
    std::vector<std::pair<GuiSplitter*,GuiTerminalWindow*> > widgetAtIndex;

public:
    GuiMainWindow(QWidget *parent = 0);
    ~GuiMainWindow();
    void createNewTab(Config *cfg, GuiBase::SplitType splittype=GuiBase::TYPE_LEAF);
    bool winEvent ( MSG * msg, long * result );
    void closeEvent ( QCloseEvent * event );
    GuiTerminalWindow *getCurrentTerminal();
    GuiTerminalWindow *getCurrentTerminalInTab(int tabIndex);

    QMenu *getMenuById(qutty_menu_id_t id) {
        assert(id > MENU_SEPARATOR && id <= MENU_SEPARATOR + MENU_MAX_MENU);
        return &menuCommonMenus[id - MENU_SEPARATOR - 1];
    }

    void tabInsert(int tabind, QWidget *w, const QString &title);
    void tabRemove(int tabind);
    int setupLayout(GuiTerminalWindow *newTerm, GuiBase::SplitType split, int tabind = -1);

    int getTerminalTabInd(const QWidget *term);

    const QList<GuiTerminalWindow*> *getTerminalList() { return &terminalList; }
private:
    void initializeMenuSystem();
    void inittializeDragDropWidget();
    void populateMenu(QMenu &menu, qutty_menu_id_t menu_list[], int len);

    void readSettings();
    void writeSettings();

public slots:
    void on_openNewWindow();
    void on_openNewSession(GuiBase::SplitType splittype);
    void on_openNewTab() { on_openNewSession(GuiBase::TYPE_LEAF); }
    void on_openNewSplitHorizontal() { on_openNewSession(GuiBase::TYPE_HORIZONTAL); }
    void on_openNewSplitVertical() { on_openNewSession(GuiBase::TYPE_VERTICAL); }
    void on_createNewSession(Config cfg, GuiBase::SplitType splittype);
    void on_settingsWindowClose();
    void on_changeSettingsTab(GuiTerminalWindow *termWnd);
    void on_changeSettingsTabComplete(Config cfg, GuiTerminalWindow *termWnd);
    void closeTerminal(GuiTerminalWindow *termWnd);
    void tabCloseRequested (int index);
    void currentChanged(int index);
    void on_tabLayoutChanged();

    void tabNext();
    void tabPrev();

    void contextMenuPaste();
    void contextMenuSavedSessionsChanged();
    void contextMenuSavedSessionTriggered();
    void contextMenuDuplicateSessionTriggered();
    void contextMenuOpenDuplicateHSplit();
    void contextMenuOpenDuplicateVSplit();
    void contextMenuRestartSessionTriggered();
    void contextMenuChangeSettingsTriggered();
    void contextMenuCloseSessionTriggered();
    void contextMenuCloseWindowTriggered();
    void contextMenuMenuBar();
    void contextMenuFullScreen();
    void contextMenuAlwaysOnTop();
    void contextMenuTermTopDragPaneTriggered();
    void contextMenuTermTopCloseTriggered();
    void contextMenuPreferences();
    void contextMenuRenameTab();
    void contextMenuFind();
    void contextMenuFindNext();
    void contextMenuFindPrevious();
    void contextMenuPaneUp();
    void contextMenuPaneDown();
    void contextMenuPaneLeft();
    void contextMenuPaneRight();
    void contextMenuMRUTab();
    void contextMenuLRUTab();
    void contextMenuMRUPane();
    void contextMenuLRUPane();
};

#endif // MAINWINDOW_H
