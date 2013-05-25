#include <QTabBar>
#include <QMenu>
#include "GuiTabWidget.h"
#include "GuiTerminalWindow.h"
#include "QtConfig.h"

GuiTabWidget::GuiTabWidget(GuiMainWindow * parent) :
    QTabWidget(parent),
    menu(this),
    mainWindow(parent),
    menuTabIndex(-1),
    menuSavedSessions(tr("Saved Sessions"))
{
    QMenu *tmp;
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(showContextMenu(QPoint)));

    menu.addAction(tr("Paste"));
    menu.addSeparator();
    menu.addAction(tr("New Session"), mainWindow, SLOT(openSettingsWindow()), QKeySequence("Ctrl+Shift+t"));
    menu.addAction(tr(""));
    menu.addMenu(&menuSavedSessions);
    tmp = menu.addMenu(tr("Split Session"));
    tmp->addAction(tr("Horizontally"));
    tmp->addAction(tr("Vertcally"));
    menu.addAction(tr("Change Settings"), this, SLOT(contextMenuChangeSettingsTriggered()));
    menu.addSeparator();
    menu.addAction(tr("Close"), this, SLOT(contextMenuCloseSessionTriggered()));

    connect(&qutty_config, SIGNAL(savedSessionsChanged()), this, SLOT(savedSessionsChanged()));
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
    if (termWindow && !termWindow->isSockDisconnected) {
        menu.actions()[3]->setText(tr("Duplicate Session"));
        menu.actions()[3]->disconnect();
        connect(menu.actions()[3], SIGNAL(triggered()), this, SLOT(contextMenuDuplicateSessionTriggered()));
    } else {
        menu.actions()[3]->setText(tr("Restart Session"));
        menu.actions()[3]->disconnect();
        connect(menu.actions()[3], SIGNAL(triggered()), this, SLOT(contextMenuRestartSessionTriggered()));
    }

    menu.exec(this->mapToGlobal(point));
}

void GuiTabWidget::savedSessionsChanged()
{
    menuSavedSessions.clear();
    for(std::map<string, Config>::iterator it = qutty_config.config_list.begin();
        it != qutty_config.config_list.end(); it++) {
        if (it->first == QUTTY_DEFAULT_CONFIG_SETTINGS)
            continue;
        menuSavedSessions.addAction(it->first.c_str(), this, SLOT(contextMenuSavedSessionTriggered()));
    }
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

void GuiTabWidget::contextMenuSavedSessionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action ||
        qutty_config.config_list.find(action->text().toStdString()) == qutty_config.config_list.end())
        return;
    mainWindow->createNewTab(&qutty_config.config_list[action->text().toStdString()]);
}

void GuiTabWidget::contextMenuChangeSettingsTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
}

void GuiTabWidget::contextMenuCloseSessionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    emit tabCloseRequested(menuTabIndex);
}
