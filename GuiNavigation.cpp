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
    std::map<int,uint32_t> mrumap;
    auto termlist = mainWindow->getTerminalList();
    for(auto term=termlist->begin(); term != termlist->end(); term++) {
        GuiSplitter *split = (*term)->parentSplit;
        int tabid;
        if (split) {
            while(split->parentSplit) split = split->parentSplit;
            tabid = mainWindow->tabArea->indexOf(split);
        } else
            tabid = mainWindow->tabArea->indexOf(*term);
        assert(tabid != -1);
        if (mrumap.find(tabid) == mrumap.end())
            mrumap.insert(std::pair<int,uint32_t>(tabid, (*term)->mru_count));
        else
            mrumap[tabid] = std::max(mrumap[tabid], (*term)->mru_count);
    }
    std::multimap<uint32_t,int> mrumaptab;
    for(auto it = mrumap.begin(); it != mrumap.end(); ++it)
        mrumaptab.insert(std::pair<uint32_t,int>(it->second, it->first));
    for(auto it = mrumaptab.rbegin(); it != mrumaptab.rend(); ++it) {
        QListWidgetItem *w = new QListWidgetItem(mainWindow->tabArea->tabText(it->second), this);
        w->setData(Qt::UserRole, it->second);
    }
    adjustSize();
    move((mainWindow->width()-width())/2,
              (mainWindow->height()-height())/2);
    show();
    raise();
    setCurrentRow(0);
    setFocus();
}

void GuiTabNavigation::acceptNavigation()
{
    int sel;
    if ((sel = currentRow()) != -1) {
        mainWindow->tabArea->setCurrentIndex(currentItem()->data(Qt::UserRole).toInt());
    }
}

void GuiTabNavigation::navigateToTabNext()
{
    if (currentRow() == count()-1)
        setCurrentRow(0);
    else
        setCurrentRow(currentRow() + 1);
}

void GuiTabNavigation::navigateToTabPrev()
{
    if (currentRow() == 0)
        setCurrentRow(count()-1);
    else
        setCurrentRow(currentRow() - 1);
}

void GuiTabNavigation::focusOutEvent ( QFocusEvent * e )
{
    mainWindow->tabNavigate = NULL;
    mainWindow->currentChanged(mainWindow->tabArea->currentIndex());
    this->deleteLater();
}

void GuiTabNavigation::keyReleaseEvent ( QKeyEvent * e )
{
    if (e->key() == Qt::Key_Control) {
        this->acceptNavigation();
        this->close();
    }
    QListWidget::keyReleaseEvent(e);
}

bool GuiTabNavigation::event ( QEvent * e )
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
        if (keyEvent->key() == Qt::Key_Up) {
            navigateToTabPrev();
        } else if (keyEvent->key() == Qt::Key_Down) {
            navigateToTabNext();
        }
        return true;
    }
    return QListWidget::event(e);
}

GuiPaneNavigation::GuiPaneNavigation(GuiMainWindow *p)
    : QWidget(p),
      mainWindow(p)
{
    GuiBase *base;
    vector<GuiTerminalWindow*> list;
    if(!(base=qobject_cast<GuiTerminalWindow*>(mainWindow->tabArea->currentWidget())))
        return;
    base->populateAllTerminals(&list);

    move(0, 0);
    resize(100, 100);
    show();
    raise();
    setFocus();
    setAutoFillBackground(true);
}

void GuiPaneNavigation::acceptNavigation()
{
}

void GuiPaneNavigation::navigateToPaneNext()
{
}

void GuiPaneNavigation::navigateToPanePrev()
{
}

void GuiPaneNavigation::focusOutEvent(QFocusEvent *e)
{
    mainWindow->paneNavigate = NULL;
    mainWindow->currentChanged(mainWindow->tabArea->currentIndex());
    this->deleteLater();
}

void GuiPaneNavigation::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Control) {
        this->acceptNavigation();
        this->close();
    }
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

void GuiMainWindow::contextMenuMRUTab()
{
    if (!tabNavigate)
        tabNavigate = new GuiTabNavigation(this);
    tabNavigate->navigateToTabNext();
}

void GuiMainWindow::contextMenuLRUTab()
{
    if (!tabNavigate)
        tabNavigate = new GuiTabNavigation(this);
    tabNavigate->navigateToTabPrev();
}

void GuiMainWindow::contextMenuMRUPane()
{
    if (!paneNavigate)
        paneNavigate = new GuiPaneNavigation(this);
    paneNavigate->navigateToPaneNext();
}

void GuiMainWindow::contextMenuLRUPane()
{
    if (!paneNavigate)
        paneNavigate = new GuiPaneNavigation(this);
    paneNavigate->navigateToPanePrev();
}
