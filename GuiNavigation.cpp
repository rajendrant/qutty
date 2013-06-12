/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include "GuiNavigation.h"
#include "GuiBase.h"
#include "GuiTerminalWindow.h"
#include "GuiSplitter.h"

GuiTabNavigation::GuiTabNavigation(GuiMainWindow *p)
    : QListWidget(p),
      mainWindow(p)
{
    hide();
    setFixedHeight(200);
    is_active = false;
    accept_sel = false;
}

void GuiTabNavigation::terminalFocusIn(GuiTerminalWindow *term)
{
    int ind = mainWindow->tabArea->currentIndex();
    if (!tablist.isEmpty() && tablist[0] == ind)
        return;
    int oldind = tablist.indexOf(ind);
    if (oldind != -1) {
        tablist.removeOne(ind);
    }
    tablist.insert(0, ind);
}

void GuiTabNavigation::tabClosing(int tabid)
{
    int ind = tablist.indexOf(tabid);
    if (ind != -1) {
        tablist.removeAt(ind);
    }
    for (tabid++; tabid <= tablist.size(); tabid++)
        tablist[tablist.indexOf(tabid)] = tabid - 1;
}

void GuiTabNavigation::activateTabNavigateGUI()
{
    if (is_active)
        return;
    QStringList strlist;
    strlist.reserve(tablist.size());
    for(int i=0; i< tablist.size(); i++) {
        strlist.append(mainWindow->tabArea->tabText(tablist[i]));
    }
    this->addItems(strlist);
    move((mainWindow->width()-width())/2,
              (mainWindow->height()-height())/2);
    show();
    raise();
    setCurrentRow(0);
    setFocus();
    is_active = true;
    accept_sel = false;
}

void GuiTabNavigation::deactivateTabNavigateGUI()
{
    if (!is_active)
        return;
    int sel;
    if ((sel = currentRow()) != -1) {
        mainWindow->tabArea->setCurrentIndex(tablist[sel]);
    }
    mainWindow->currentChanged(mainWindow->tabArea->currentIndex());
    this->clear();
    hide();
    is_active = false;
    accept_sel = false;
}

void GuiTabNavigation::navigateToTabNext()
{
    activateTabNavigateGUI();
    if (currentRow() == count()-1)
        setCurrentRow(0);
    else
        setCurrentRow(currentRow() + 1);
}

void GuiTabNavigation::navigateToTabPrev()
{
    activateTabNavigateGUI();
    if (currentRow() == 0)
        setCurrentRow(count()-1);
    else
        setCurrentRow(currentRow() - 1);
}

void GuiTabNavigation::focusOutEvent ( QFocusEvent * e )
{
    this->deactivateTabNavigateGUI();
}

void GuiTabNavigation::keyPressEvent ( QKeyEvent * e )
{
    if (e->key() == Qt::Key_Tab && e->modifiers() & Qt::ControlModifier) {
        if (e->modifiers() & Qt::ShiftModifier)
            navigateToTabPrev();
        else
            navigateToTabNext();
        return;
    }
    QListWidget::keyReleaseEvent(e);
}

void GuiTabNavigation::keyReleaseEvent ( QKeyEvent * e )
{
    if (e->key() == Qt::Key_Control) {
        accept_sel = true;
        this->deactivateTabNavigateGUI();
    }
    QListWidget::keyReleaseEvent(e);
}

bool GuiTabNavigation::event ( QEvent * e )
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
        if (keyEvent->key() == Qt::Key_Tab) {
            // ctrl + tab
            if (keyEvent->modifiers() & Qt::ControlModifier)
                navigateToTabNext();
        } else if (keyEvent->key() == Qt::Key_Backtab) {
            // ctrl + shift + tab
            if (keyEvent->modifiers() & Qt::ControlModifier)
                navigateToTabPrev();
        } else if (keyEvent->key() == Qt::Key_Up) {
            navigateToTabPrev();
        } else if (keyEvent->key() == Qt::Key_Down) {
            navigateToTabNext();
        }
        return true;
    }
    return QListWidget::event(e);
}

void GuiMainWindow::tabNext ()
{
    if (tabArea->currentIndex() != tabArea->count()-1)
        tabArea->setCurrentIndex(tabArea->currentIndex()+1);
    else
        tabArea->setCurrentIndex(0);
}

void GuiMainWindow::tabPrev ()
{
    if (tabArea->currentIndex() != 0)
        tabArea->setCurrentIndex(tabArea->currentIndex()-1);
    else
        tabArea->setCurrentIndex(tabArea->count()-1);
}

void GuiMainWindow::contextMenuMRUPane()
{
    tabNavigate.navigateToTabNext();
}

void GuiMainWindow::contextMenuLRUPane()
{
    tabNavigate.navigateToTabPrev();
}
