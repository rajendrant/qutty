/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include <QProxyStyle>
#include <QMouseEvent>
#include <QString>
#include <QAction>
#include <QMenuBar>
#include "GuiMainWindow.h"
#include "GuiTerminalWindow.h"

qutty_menu_actions_t qutty_menu_actions[MENU_MAX_ACTION] = {
    { "New Session",            "Ctrl+Shift+T",  SLOT( on_openNewTab() ) },
    { "Restart Session",        "",              SLOT( contextMenuRestartSessionTriggered() ) },
    { "Duplicate Session",      "",              SLOT( contextMenuDuplicateSessionTriggered() ) },
    { "Change Settings",        "",              SLOT( contextMenuChangeSettingsTriggered() ) },
    { "Paste",                  "",              SLOT( contextMenuPaste() ) },
    { "New Tab",                "Ctrl+Shift+T",  SLOT( on_openNewTab() ) },
    { "New Window",             "",              SLOT( on_openNewWindow() ) },
    { "Close",                  "",              SLOT( contextMenuCloseSessionTriggered() ) },
    { "Horizontally",           "Ctrl+Shift+H",  SLOT( on_openNewSplitHorizontal() ) },
    { "Vertically",             "Ctrl+Shift+V",  SLOT( on_openNewSplitVertical() ) },
    { "Switch to Left Tab",     "Shift+Left",    SLOT( tabPrev() ) },
    { "Switch to Right Tab",    "Shift+Right",   SLOT( tabNext() ) },
    { "Import from File",       "",              "" },
    { "Import PuTTY sessions",  "",              "" },
    { "Export from File",       "",              "" },
    { "Exit",                   "",              SLOT( contextMenuCloseWindowTriggered() ) },
    { "Show Menubar",           "",              SLOT( contextMenuMenuBar() ) },
    { "Fullscreen",             "",              SLOT( contextMenuFullScreen() ) },
    { "Always on top",          "",              SLOT( contextMenuAlwaysOnTop() ) },
    { "Preferences",            "",              "" }
};

qutty_menu_links_t qutty_menu_links[MENU_MAX_MENU] = {
{ "File", 10, {MENU_NEW_TAB, MENU_NEW_WINDOW, MENU_SEPARATOR, MENU_SAVED_SESSIONS, MENU_SEPARATOR, MENU_SPLIT_SESSION, MENU_SEPARATOR, MENU_EXPORT_IMPORT, MENU_SEPARATOR, MENU_EXIT} },
{ "Edit", 0,  {} },
{ "View", 6,  { MENU_SWITCH_LEFT_TAB, MENU_SWITCH_RIGHT_TAB, MENU_SEPARATOR, MENU_MENUBAR, MENU_ALWAYSONTOP, MENU_FULLSCREEN } },
{ "Export/Import Settings",     3,  { MENU_IMPORT_FILE, MENU_IMPORT_REGISTRY, MENU_EXPORT_FILE} },
{ "Saved Sessions",             0,  {} },
{ "Split Session",              2,  { MENU_SPLIT_HORIZONTAL, MENU_SPLIT_VERTICAL } },
{ "Menu Term Window",           10, { MENU_PASTE, MENU_SEPARATOR,
                                      MENU_NEW_SESSION, MENU_RESTART_SESSION, MENU_DUPLICATE_SESSION,
                                      MENU_SAVED_SESSIONS, MENU_SPLIT_SESSION, MENU_CHANGE_SETTINGS,
                                      MENU_SEPARATOR, MENU_CLOSE_SESSION} },
{ "Menu Tabbar",                10, { MENU_NEW_TAB, MENU_NEW_WINDOW, MENU_SEPARATOR,
                                      MENU_SAVED_SESSIONS, MENU_SEPARATOR, MENU_SPLIT_SESSION, MENU_SEPARATOR,
                                      MENU_EXPORT_IMPORT, MENU_SEPARATOR, MENU_VIEW, MENU_SEPARATOR, MENU_EXIT } },
};

class MyStyle : public QProxyStyle
{
    GuiMainWindow *mainWindow;
public:
    MyStyle(GuiMainWindow *wnd)
    {
        mainWindow = wnd;
    }

    QRect subElementRect ( SubElement element, const QStyleOption * option, const QWidget * widget = 0 ) const
    {
        QRect rc = QProxyStyle::subElementRect(element, option, widget);
        if (element == QStyle::SE_TabWidgetRightCorner) {
            // This is a very bad hack we are doing
            // Reference: http://www.qtcentre.org/threads/12539-QTabWidget-corner-widget-is-not-shown
            QWidget *wid = mainWindow->tabArea->cornerWidget();
            if (wid)
                rc.setSize(wid->size());
        }
        return rc;
    }
};

void GuiMainWindow::initializeMenuSystem()
{
    for(int i=0; i<MENU_MAX_ACTION; i++) {
        menuCommonActions[i] = new QAction(qutty_menu_actions[i].name, this);
        menuCommonActions[i]->setShortcut(QKeySequence(qutty_menu_actions[i].key));
        connect(menuCommonActions[i], SIGNAL(triggered()), this, qutty_menu_actions[i].slot);
    }
    for(int i=0; i<MENU_MAX_MENU; i++) {
        menuCommonMenus[i].setTitle(qutty_menu_links[i].name);
        populateMenu(menuCommonMenus[i], qutty_menu_links[i].links, qutty_menu_links[i].len);
    }

    menuCommonActions[MENU_MENUBAR]->setCheckable(true);
    menuCommonActions[MENU_FULLSCREEN]->setCheckable(true);
    menuCommonActions[MENU_ALWAYSONTOP]->setCheckable(true);

    // setup main menubar
    menuBar()->addMenu(&menuCommonMenus[MENU_FILE-MENU_SEPARATOR-1]);
    menuBar()->addMenu(&menuCommonMenus[MENU_EDIT-MENU_SEPARATOR-1]);
    menuBar()->addMenu(&menuCommonMenus[MENU_VIEW-MENU_SEPARATOR-1]);
    menuCommonActions[MENU_MENUBAR]->setChecked(true);

    newTabToolButton.setArrowType(Qt::DownArrow);
    newTabToolButton.setMenu(getMenuById(MENU_TAB_BAR));
    newTabToolButton.setPopupMode(QToolButton::InstantPopup);

    tabArea->setCornerWidget(&newTabToolButton, Qt::TopRightCorner);
    tabArea->setStyle(new MyStyle(this));   // TODO MEMLEAK

    connect(&qutty_config, SIGNAL(savedSessionsChanged()), this, SLOT(contextMenuSavedSessionsChanged()));
    this->contextMenuSavedSessionsChanged();
}

void GuiMainWindow::populateMenu(QMenu &menu, qutty_menu_id_t menu_list[], int len)
{
    for (int i=0; i < len; i++) {
        if (menu_list[i] < MENU_SEPARATOR) {
            menu.addAction(menuCommonActions[menu_list[i]]);
        } else if (menu_list[i] > MENU_SEPARATOR) {
            menu.addMenu(&menuCommonMenus[menu_list[i] - MENU_SEPARATOR - 1]);
        } else {
            menu.addSeparator();
        }
    }
}

void GuiTerminalWindow::showContextMenu(QMouseEvent *e)
{
    // handling ctrl + right-click on terminal
    if (e->pos().isNull())
        return;
    qutty_menu_id_t id;
    if (this->isSockDisconnected) {
        id = MENU_DUPLICATE_SESSION;
    } else {
        id = MENU_RESTART_SESSION;
    }
    this->mainWindow->menuCookieTermWnd = this;
    mainWindow->menuCommonActions[id]->setVisible(false);
    this->mainWindow->getMenuById(MENU_TERM_WINDOW)->exec(e->globalPos());
    mainWindow->menuCommonActions[id]->setVisible(true);
    this->mainWindow->menuCookieTermWnd = NULL;
}

void GuiMainWindow::contextMenuSavedSessionsChanged()
{
    QMenu *menuSavedSessions = &menuCommonMenus[MENU_SAVED_SESSIONS - MENU_SEPARATOR - 1];
    if (!menuSavedSessions)
        return;
    menuSavedSessions->clear();
    for(std::map<string, Config>::iterator it = qutty_config.config_list.begin();
        it != qutty_config.config_list.end(); it++) {
        if (it->first == QUTTY_DEFAULT_CONFIG_SETTINGS)
            continue;
        menuSavedSessions->addAction(it->first.c_str(), this, SLOT(contextMenuSavedSessionTriggered()));
    }
}

void GuiMainWindow::contextMenuSavedSessionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    char sessname[100];
    qstring_to_char(sessname, action->text(), sizeof(sessname));
    if (qutty_config.config_list.find(sessname) == qutty_config.config_list.end())
        return;
    this->createNewTab(&qutty_config.config_list[sessname]);
}

void GuiMainWindow::contextMenuPaste()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    if (!menuCookieTermWnd)
        return;
    menuCookieTermWnd->requestPaste();
}

void GuiMainWindow::contextMenuDuplicateSessionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    if (!menuCookieTermWnd)
        return;
    this->createNewTab(&menuCookieTermWnd->cfg);
}

void GuiMainWindow::contextMenuRestartSessionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    if (!menuCookieTermWnd)
        return;
    menuCookieTermWnd->restartTerminal();
}

void GuiMainWindow::contextMenuChangeSettingsTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    if (!menuCookieTermWnd)
        return;
    on_changeSettingsTab(menuCookieTermWnd);
}

void GuiMainWindow::contextMenuCloseSessionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    if (!menuCookieTermWnd)
        return;
    menuCookieTermWnd->reqCloseTerminal(false);
}

void GuiMainWindow::contextMenuCloseWindowTriggered()
{
    this->close();
}

void GuiMainWindow::contextMenuMenuBar()
{
    if(menuCommonActions[MENU_MENUBAR]->isChecked()) {
        menuBar()->show();
    } else {
        menuBar()->hide();
    }
}

void GuiMainWindow::contextMenuFullScreen()
{
    if(menuCommonActions[MENU_FULLSCREEN]->isChecked()) {
        setWindowState(windowState() | Qt::WindowFullScreen);
    } else {
        setWindowState(windowState() ^ Qt::WindowFullScreen);
    }
}

void GuiMainWindow::contextMenuAlwaysOnTop()
{
    if(menuCommonActions[MENU_ALWAYSONTOP]->isChecked()) {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    } else {
        setWindowFlags(windowFlags() ^ Qt::WindowStaysOnTopHint);
    }
    this->show();
}
