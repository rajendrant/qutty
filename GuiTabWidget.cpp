#include <QTabBar>
#include <QMenu>
#include "GuiTabWidget.h"
#include "GuiTerminalWindow.h"
#include "QtConfig.h"
#include "GuiSettingsWindow.h"
#include "GuiMenu.h"

GuiTabWidget::GuiTabWidget(GuiMainWindow * parent) :
    QTabWidget(parent),
    menu(this),
    mainWindow(parent),
    menuTabIndex(-1)
{
    QMenu *tmp;
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(showContextMenu(QPoint)));

    QUTTY_MENU_ADD_ACTION(&menu, MENU_PASTE, this, contextMenuPaste());
    menu.addSeparator();
    QUTTY_MENU_ADD_ACTION(&menu, MENU_NEW_SESSION, mainWindow, on_openNewTab());
    menu.addAction("");  // Restart/Duplicate
    menu.addMenu(&mainWindow->menuSavedSessions);
    tmp = menu.addMenu(qutty_menu_common_name[MENU_SPLIT_SESSION]);
    tmp->addAction(qutty_menu_common_name[MENU_SPLIT_HORIZONTAL]);
    tmp->addAction(qutty_menu_common_name[MENU_SPLIT_VERTICAL]);
    QUTTY_MENU_ADD_ACTION(&menu, MENU_CHANGE_SETTINGS, this, contextMenuChangeSettingsTriggered());
    menu.addSeparator();
    QUTTY_MENU_ADD_ACTION(&menu, MENU_CLOSE_SESSION, this, contextMenuCloseSessionTriggered());
}

void GuiTabWidget::showContextMenu(const QPoint &point)
{
    // handling only right-click on tabbar
    if (point.isNull())
        return;

    QWidget *widget;
    if ( ((menuTabIndex = tabBar()->tabAt(point)) == -1) ||
         (!(widget = this->widget(menuTabIndex))))
        return;

    GuiTerminalWindow *termWindow = static_cast<GuiTerminalWindow*>(widget);
    QAction *act = menu.actions()[3];
    act->disconnect();
    if (termWindow && !termWindow->isSockDisconnected) {
        act->setText(qutty_menu_common_name[MENU_DUPLICATE_SESSION]);
        connect(act, SIGNAL(triggered()), this, SLOT(contextMenuDuplicateSessionTriggered()));
    } else {
        act->setText(qutty_menu_common_name[MENU_RESTART_SESSION]);
        connect(act, SIGNAL(triggered()), this, SLOT(contextMenuRestartSessionTriggered()));
    }

    menu.exec(this->mapToGlobal(point));
}


void GuiTabWidget::contextMenuPaste()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    GuiTerminalWindow *termWnd = (GuiTerminalWindow*)this->widget(menuTabIndex);
    assert(termWnd);
    termWnd->requestPaste();
}

void GuiTabWidget::contextMenuDuplicateSessionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    GuiTerminalWindow *termWnd = (GuiTerminalWindow*)this->widget(menuTabIndex);
    assert(termWnd);
    mainWindow->createNewTab(&termWnd->cfg);
}

void GuiTabWidget::contextMenuRestartSessionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    GuiTerminalWindow *termWnd = (GuiTerminalWindow*)this->widget(menuTabIndex);
    assert(termWnd);
    termWnd->restartTerminal();
}

void GuiTabWidget::contextMenuChangeSettingsTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    emit sig_tabChangeSettings(menuTabIndex);
}

void GuiTabWidget::contextMenuCloseSessionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    emit tabCloseRequested(menuTabIndex);
}
