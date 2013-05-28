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

qutty_menu_links_t qutty_menu_links[MENU_MAX_MENU] = {
{ "File", 7, {MENU_NEW_TAB, MENU_NEW_WINDOW, MENU_SEPARATOR, MENU_SAVED_SESSIONS, MENU_EXPORT_IMPORT, MENU_SEPARATOR, MENU_EXIT} },
{ "Edit", 0, {} },
{ "View", 6, { MENU_SWITCH_LEFT_TAB, MENU_SWITCH_RIGHT_TAB, MENU_SEPARATOR, MENU_MENUBAR, MENU_ALWAYSONTOP, MENU_FULLSCREEN } },
{ "Export/Import Settings",     3, { MENU_IMPORT_FILE, MENU_IMPORT_REGISTRY, MENU_EXPORT_FILE} },
{ "Saved Sessions",             0, {} },
{ "Split Session",              2, { MENU_SPLIT_HORIZONTAL, MENU_SPLIT_VERTICAL } },
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
#define QUTTY_ENTRY(id, str, key, slot) \
        menuCommonActions[id] = new QAction(str, this); \
        menuCommonActions[id]->setShortcut(QKeySequence(key)); \
        connect(menuCommonActions[id], SIGNAL(triggered()), SLOT(slot()));
        QUTTY_MENU_ACTIONS
#undef QUTTY_ENTRY

    for(int i=0; i<MENU_MAX_MENU; i++) {
        menuCommonMenus[i] = new QMenu(qutty_menu_links[i].name);
    }
    for(int i=0; i<MENU_MAX_MENU; i++) {
        populateMenu(*menuCommonMenus[i], qutty_menu_links[i].links, qutty_menu_links[i].len);
    }

    menuCommonActions[MENU_MENUBAR]->setCheckable(true);
    menuCommonActions[MENU_FULLSCREEN]->setCheckable(true);
    menuCommonActions[MENU_ALWAYSONTOP]->setCheckable(true);

    // setup main menubar
    menuBar()->addMenu(menuCommonMenus[MENU_FILE-MENU_SEPARATOR-1]);
    menuBar()->addMenu(menuCommonMenus[MENU_EDIT-MENU_SEPARATOR-1]);
    menuBar()->addMenu(menuCommonMenus[MENU_VIEW-MENU_SEPARATOR-1]);

    qutty_menu_id_t menus_term_wnd[] = {
        MENU_PASTE,
        MENU_SEPARATOR,
        MENU_NEW_SESSION, MENU_RESTART_SESSION, MENU_DUPLICATE_SESSION,
        MENU_SAVED_SESSIONS, MENU_SPLIT_SESSION, MENU_CHANGE_SETTINGS,
        MENU_SEPARATOR,
        MENU_CLOSE_SESSION
    };

    qutty_menu_id_t menus_toolbar[] = {
        MENU_NEW_TAB, MENU_NEW_WINDOW,
        MENU_SEPARATOR,
        MENU_SAVED_SESSIONS,
        MENU_SEPARATOR,
        MENU_EXPORT_IMPORT,
        MENU_SEPARATOR,
        MENU_VIEW,
        MENU_SEPARATOR,
        MENU_EXIT
    };

    // populate terminal context menu
    populateMenu(menuTermWnd, menus_term_wnd, sizeof(menus_term_wnd)/sizeof(menus_term_wnd[0]));

    // populate terminal context menu
    populateMenu(menuTabBar, menus_toolbar, sizeof(menus_toolbar)/sizeof(menus_toolbar[0]));

    newTabToolButton.setText(tr("+"));
    newTabToolButton.setMenu(&menuTabBar);
    newTabToolButton.setPopupMode(QToolButton::MenuButtonPopup);

    connect(&newTabToolButton, SIGNAL(clicked()), SLOT(on_openNewTab()));
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
            menu.addMenu(menuCommonMenus[menu_list[i] - MENU_SEPARATOR - 1]);
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
    this->mainWindow->menuCookieTabIndex = -1;
    mainWindow->menuCommonActions[id]->setVisible(false);
    this->mainWindow->menuTermWnd.exec(e->globalPos());
    mainWindow->menuCommonActions[id]->setVisible(true);
    this->mainWindow->menuCookieTermWnd = NULL;
    this->mainWindow->menuCookieTabIndex = -1;
}

void GuiMainWindow::contextMenuSavedSessionsChanged()
{
    QMenu *menuSavedSessions = menuCommonMenus[MENU_SAVED_SESSIONS - MENU_SEPARATOR - 1];
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
    if (!menuCookieTermWnd && menuCookieTabIndex == -1)
        return;
    if (!menuCookieTermWnd)
        menuCookieTermWnd = (GuiTerminalWindow*)tabArea->widget(menuCookieTabIndex);
    menuCookieTermWnd->requestPaste();
}

void GuiMainWindow::contextMenuDuplicateSessionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    if (!menuCookieTermWnd && menuCookieTabIndex == -1)
        return;
    if (!menuCookieTermWnd)
        menuCookieTermWnd = (GuiTerminalWindow*)tabArea->widget(menuCookieTabIndex);
    this->createNewTab(&menuCookieTermWnd->cfg);
}

void GuiMainWindow::contextMenuRestartSessionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    if (!menuCookieTermWnd && menuCookieTabIndex == -1)
        return;
    if (!menuCookieTermWnd)
        menuCookieTermWnd = (GuiTerminalWindow*)tabArea->widget(menuCookieTabIndex);
    menuCookieTermWnd->restartTerminal();
}

void GuiMainWindow::contextMenuChangeSettingsTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    if (!menuCookieTermWnd && menuCookieTabIndex == -1)
        return;
    int index = (menuCookieTabIndex == -1) ?
                    tabArea->indexOf(menuCookieTermWnd) : menuCookieTabIndex;
    if (index != -1)
        on_changeSettingsTab(index);
}

void GuiMainWindow::contextMenuCloseSessionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    if (!menuCookieTermWnd && menuCookieTabIndex == -1)
        return;
    int index = (menuCookieTabIndex == -1) ?
                    tabArea->indexOf(menuCookieTermWnd) : menuCookieTabIndex;
    if (index != -1)
        tabCloseRequested(index);
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
