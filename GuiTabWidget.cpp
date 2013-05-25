#include <QTabBar>
#include <QMenu>
#include "GuiTabWidget.h"
#include "GuiTerminalWindow.h"

GuiTabWidget::GuiTabWidget(GuiMainWindow * parent) :
    QTabWidget(parent),
    menu(this),
    mainWindow(parent)
{
    QMenu *tmp;
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(showContextMenu(QPoint)));

    menu.addAction(tr("Paste"));
    menu.addSeparator();
    menu.addAction(tr("New Session"), mainWindow, SLOT(openSettingsWindow()), QKeySequence("Ctrl+Shift+t"));
    menu.addAction(tr(""));
    tmp = menu.addMenu(tr("Saved Sessions"));
    tmp = menu.addMenu(tr("Split Session"));
    tmp->addAction(tr("Horizontally"));
    tmp->addAction(tr("Vertcally"));
    menu.addAction(tr("Change Settings"));
    menu.addSeparator();
    menu.addAction(tr("Close"));
}

void GuiTabWidget::showContextMenu(const QPoint &point)
{
    // handling only right-click on tabbar
    if (point.isNull())
        return;

    int tabIndex;
    QWidget *widget;
    if ( ((tabIndex = tabBar()->tabAt(point)) == -1) ||
         (!(widget = this->widget(tabIndex))))
        return;

    GuiTerminalWindow *termWindow = static_cast<GuiTerminalWindow*>(widget);
    if (termWindow && !termWindow->isSockDisconnected)
        menu.actions()[3]->setText(tr("Duplicate Session"));
    else
        menu.actions()[3]->setText(tr("Restart Session"));

    menu.exec(this->mapToGlobal(point));
}
