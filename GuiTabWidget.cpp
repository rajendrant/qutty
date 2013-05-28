#include <QTabBar>
#include <QMenu>
#include "GuiTabWidget.h"
#include "GuiTerminalWindow.h"
#include "QtConfig.h"
#include "GuiSettingsWindow.h"
#include "GuiMenu.h"

GuiTabWidget::GuiTabWidget(GuiMainWindow * parent) :
    QTabWidget(parent),
    mainWindow(parent)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(showContextMenu(QPoint)));
}

void GuiTabWidget::showContextMenu(const QPoint &point)
{
    // handling only right-click on tabbar
    if (point.isNull())
        return;

    QWidget *widget;
    int menuTabIndex;
    if ( ((menuTabIndex = tabBar()->tabAt(point)) == -1) ||
         (!(widget = this->widget(menuTabIndex))))
        return;

    GuiTerminalWindow *termWindow = static_cast<GuiTerminalWindow*>(widget);
    qutty_menu_id_t id;
    if (termWindow && !termWindow->isSockDisconnected) {
        id = MENU_RESTART_SESSION;
    } else {
        id = MENU_DUPLICATE_SESSION;
    }

    this->mainWindow->menuCookieTermWnd = NULL;
    this->mainWindow->menuCookieTabIndex = menuTabIndex;
    mainWindow->menuCommonActions[id]->setVisible(false);
    this->mainWindow->menuTermWnd.exec(this->mapToGlobal(point));
    mainWindow->menuCommonActions[id]->setVisible(true);
    this->mainWindow->menuCookieTermWnd = NULL;
    this->mainWindow->menuCookieTabIndex = -1;
}
