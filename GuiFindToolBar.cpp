/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include "GuiMainWindow.h"
#include "GuiTerminalWindow.h"
#include "GuiFindToolBar.h"

GuiFindToolBar::GuiFindToolBar(GuiMainWindow *p)
    : QToolBar(p),
      mainWnd(p)
{
    QToolButton *b;

    addWidget(new QLineEdit);

    b = new QToolButton(this);
    b->setText("Next");
    connect(b, SIGNAL(clicked()), this, SLOT(on_findNext()));
    addWidget(b);

    b = new QToolButton(this);
    b->setText("Previous");
    connect(b, SIGNAL(clicked()), this, SLOT(on_findPrevious()));
    addWidget(b);

    b = new QToolButton(this);
    b->setIcon(QIcon(":/images/cog_alt_16x16.png"));
    b->setMenu(p->getMenuById(MENU_FIND_OPTIONS));
    b->setPopupMode(QToolButton::InstantPopup);
    addWidget(b);

    b = new QToolButton(this);
    b->setIcon(QIcon(":/images/x_14x14.png"));
    connect(b, SIGNAL(clicked()), this, SLOT(on_findClose()));
    addWidget(b);

    setIconSize(QSize(16, 16));
    setMovable(false);
    setAutoFillBackground(true);
    adjustSize();
}

void GuiMainWindow::contextMenuFind()
{
    if (!this->getCurrentTerminal())
        return;

    if (!findToolBar) {
        findToolBar = new GuiFindToolBar(this);
        findToolBar->show();

        GuiTerminalWindow *t = getCurrentTerminal();
        findToolBar->move(t->viewport()->width() - findToolBar->width(),
                          t->mapTo(this, QPoint(0,0)).y());

        findToolBar->setFocus();
        menuCommonActions[MENU_FIND_NEXT]->setEnabled(true);
        menuCommonActions[MENU_FIND_PREVIOUS]->setEnabled(true);
    } else {
        findToolBar->on_findClose();
    }
}

void GuiMainWindow::contextMenuFindNext()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    if (!findToolBar)
        return;
    findToolBar->on_findNext();
}

void GuiMainWindow::contextMenuFindPrevious()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    if (!findToolBar)
        return;
    findToolBar->on_findPrevious();
}

void GuiFindToolBar::on_findNext()
{

}

void GuiFindToolBar::on_findPrevious()
{

}

void GuiFindToolBar::on_findClose()
{
    mainWnd->menuCommonActions[MENU_FIND_NEXT]->setEnabled(false);
    mainWnd->menuCommonActions[MENU_FIND_PREVIOUS]->setEnabled(false);
    mainWnd->findToolBar = NULL;
    this->deleteLater();
}
