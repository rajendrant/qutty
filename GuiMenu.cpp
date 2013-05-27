/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include <QProxyStyle>
#include <QMouseEvent>
#include <QString>
#include <QAction>
#include "GuiMainWindow.h"
#include "GuiTerminalWindow.h"


QString qutty_menu_common_name[MENU_MAX_ENTRY+1] = {
#define QUTTY_MENU_ENTRY(id, str, key) QString(str),
    QUTTY_MENU_COMMON_LIST
#undef QUTTY_MENU_ENTRY
};

QKeySequence qutty_menu_common_shortcut[MENU_MAX_ENTRY+1] = {
#define QUTTY_MENU_ENTRY(id, str, key) QKeySequence(key),
    QUTTY_MENU_COMMON_LIST
#undef QUTTY_MENU_ENTRY
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
    newTabToolButton.setText(tr("+"));

    QMenu *submenu;
    QUTTY_MENU_ADD_ACTION(&menuTabBar, MENU_NEW_TAB, this, on_openNewTab());
    QUTTY_MENU_ADD_ACTION(&menuTabBar, MENU_NEW_WINDOW, this, on_openNewWindow());

    submenu = new QMenu("View");
    QUTTY_MENU_ADD_ACTION(submenu, MENU_SWITCH_LEFT_TAB, this, tabNext());
    QUTTY_MENU_ADD_ACTION(submenu, MENU_SWITCH_RIGHT_TAB, this, tabPrev());
    menuTabBar.addMenu(submenu);

    newTabToolButton.setMenu(&menuTabBar);
    newTabToolButton.setPopupMode(QToolButton::MenuButtonPopup);

    connect(&newTabToolButton, SIGNAL(clicked()), SLOT(on_openNewTab()));
    tabArea->setCornerWidget(&newTabToolButton, Qt::TopRightCorner);
    tabArea->setStyle(new MyStyle(this));   // TODO MEMLEAK

    // populate terminal context menu
    QUTTY_MENU_ADD_ACTION(&menuTermWnd, MENU_PASTE, this, contextMenuPaste());
    menuTermWnd.addSeparator();
    QUTTY_MENU_ADD_ACTION(&menuTermWnd, MENU_NEW_SESSION, this, on_openNewTab());
    menuTermWnd.addAction("");  // Restart/Duplicate
    menuTermWnd.addMenu(&menuSavedSessions);
    submenu = menuTermWnd.addMenu(qutty_menu_common_name[MENU_SPLIT_SESSION]);
    submenu->addAction(qutty_menu_common_name[MENU_SPLIT_HORIZONTAL]);
    submenu->addAction(qutty_menu_common_name[MENU_SPLIT_VERTICAL]);
    QUTTY_MENU_ADD_ACTION(&menuTermWnd, MENU_CHANGE_SETTINGS, this, contextMenuChangeSettingsTriggered());
    menuTermWnd.addSeparator();
    QUTTY_MENU_ADD_ACTION(&menuTermWnd, MENU_CLOSE_SESSION, this, contextMenuCloseSessionTriggered());

    menuSavedSessions.setTitle(qutty_menu_common_name[MENU_SAVED_SESSIONS]);
    connect(&qutty_config, SIGNAL(savedSessionsChanged()), this, SLOT(contextMenuSavedSessionsChanged()));
}

void GuiTerminalWindow::showContextMenu(QMouseEvent *e)
{
    // handling ctrl + right-click on terminal
    if (e->pos().isNull())
        return;
    QAction *act = this->mainWindow->menuTermWnd.actions()[3];
    act->disconnect();
    if (!this->isSockDisconnected) {
        act->setText(qutty_menu_common_name[MENU_DUPLICATE_SESSION]);
        connect(act, SIGNAL(triggered()), mainWindow, SLOT(contextMenuDuplicateSessionTriggered()));
    } else {
        act->setText(qutty_menu_common_name[MENU_RESTART_SESSION]);
        connect(act, SIGNAL(triggered()), mainWindow, SLOT(contextMenuRestartSessionTriggered()));
    }
    this->mainWindow->menuTermWndCurr = this;
    this->mainWindow->menuTermWnd.exec(e->globalPos());
}

void GuiMainWindow::contextMenuSavedSessionsChanged()
{
    menuSavedSessions.clear();
    for(std::map<string, Config>::iterator it = qutty_config.config_list.begin();
        it != qutty_config.config_list.end(); it++) {
        if (it->first == QUTTY_DEFAULT_CONFIG_SETTINGS)
            continue;
        menuSavedSessions.addAction(it->first.c_str(), this, SLOT(contextMenuSavedSessionTriggered()));
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
    menuTermWndCurr->requestPaste();
}

void GuiMainWindow::contextMenuDuplicateSessionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    this->createNewTab(&menuTermWndCurr->cfg);
}

void GuiMainWindow::contextMenuRestartSessionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    menuTermWndCurr->restartTerminal();
}

void GuiMainWindow::contextMenuChangeSettingsTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    int index = tabArea->indexOf(menuTermWndCurr);
    if (index != -1)
        on_changeSettingsTab(index);
}

void GuiMainWindow::contextMenuCloseSessionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    int index = tabArea->indexOf(menuTermWndCurr);
    if (index != -1)
        tabCloseRequested(index);
}
