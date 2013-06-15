/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include <QGraphicsColorizeEffect>
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
        int tabid = mainWindow->getTerminalTabInd(*term);
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

void GuiTabNavigation::keyPressEvent ( QKeyEvent * e )
{
    if (e->key() == Qt::Key_Up) {
        navigateToTabPrev();
    } else if (e->key() == Qt::Key_Down) {
        navigateToTabNext();
    }
    QListWidget::keyPressEvent(e);
}

void GuiTabNavigation::keyReleaseEvent ( QKeyEvent * e )
{
    if (e->key() == Qt::Key_Control) {
        this->acceptNavigation();
        this->close();
    }
    QListWidget::keyReleaseEvent(e);
}

GuiPaneNavigation::GuiPaneNavigation(GuiMainWindow *p, bool is_direction_mode)
    : QWidget(p),
      mainWindow(p),
      curr_sel(-1)
{
    GuiSplitter *base;
    vector<GuiTerminalWindow*> list;
    if(!(base=qobject_cast<GuiSplitter*>(mainWindow->tabArea->currentWidget())))
        goto cu0;
    base->populateAllTerminals(&list);
    for(auto it=list.begin(); it != list.end(); ++it)
        mrupanemap.insert(std::pair<uint32_t, GuiTerminalWindow*>((*it)->mru_count, (*it)));

    curr_sel = mrupanemap.rbegin()->first;
    if (is_direction_mode) {
        auto term = mainWindow->getCurrentTerminal();
        if (term) {
            for(auto it = mrupanemap.begin(); it != mrupanemap.end(); ++it)
                if(it->second == term)
                    curr_sel = it->first;
        }
    }

cu0:
    move(0, 0);
    resize(0, 0);
    show();
    raise();
    setFocus();
}

void GuiPaneNavigation::acceptNavigation()
{
    if (mrupanemap.size() <= 1)
        return;
    auto it = mrupanemap.find(curr_sel);
    if (it == mrupanemap.end())
        return;
    it->second->setFocus();
}

void GuiPaneNavigation::navigateToMRUPane()
{
    if (mrupanemap.size() <= 1)
        return;
    auto it = mrupanemap.find(curr_sel);
    if (it == mrupanemap.end())
        return;
    it->second->viewport()->setGraphicsEffect(NULL);
    ++it;
    if (it == mrupanemap.end())
        it = mrupanemap.begin();
    curr_sel = it->first;
    it->second->viewport()->setGraphicsEffect(new QGraphicsColorizeEffect);
}

void GuiPaneNavigation::navigateToLRUPane()
{
    if (mrupanemap.size() <= 1)
        return;
    auto it = mrupanemap.find(curr_sel);
    if (it == mrupanemap.end())
        return;
    it->second->viewport()->setGraphicsEffect(NULL);
    if (it == mrupanemap.begin())
        it = mrupanemap.end();
    --it;
    curr_sel = it->first;
    it->second->viewport()->setGraphicsEffect(new QGraphicsColorizeEffect);
}

void GuiPaneNavigation::focusOutEvent(QFocusEvent *e)
{
    mainWindow->paneNavigate = NULL;
    mainWindow->currentChanged(mainWindow->tabArea->currentIndex());

    auto it = mrupanemap.find(curr_sel);
    if (mrupanemap.size() <= 1)
        goto cu0;
    if (it == mrupanemap.end())
        goto cu0;
    it->second->viewport()->setGraphicsEffect(NULL);

cu0:
    this->deleteLater();
}

void GuiPaneNavigation::navigateToDirectionPane(Qt::Key key)
{
    if (mrupanemap.size() <= 1)
        return;
    auto it = mrupanemap.find(curr_sel);
    if (it == mrupanemap.end())
        return;
    auto term = it->second;
    if (!term || !term->parentSplit)
        return;
    term = term->parentSplit->navigatePane(key, term);
    if (term) {
        it->second->viewport()->setGraphicsEffect(NULL);
        for(auto it = mrupanemap.begin(); it != mrupanemap.end(); ++it)
            if (it->second == term)
                curr_sel = it->first;
        term->viewport()->setGraphicsEffect(new QGraphicsColorizeEffect);
    }
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
    paneNavigate->navigateToMRUPane();
}

void GuiMainWindow::contextMenuLRUPane()
{
    if (!paneNavigate)
        paneNavigate = new GuiPaneNavigation(this);
    paneNavigate->navigateToLRUPane();
}

void GuiMainWindow::contextMenuPaneUp()
{
    if (!paneNavigate)
        paneNavigate = new GuiPaneNavigation(this, true);
    paneNavigate->navigateToDirectionPane(Qt::Key_Up);
}

void GuiMainWindow::contextMenuPaneDown()
{
    if (!paneNavigate)
        paneNavigate = new GuiPaneNavigation(this, true);
    paneNavigate->navigateToDirectionPane(Qt::Key_Down);
}

void GuiMainWindow::contextMenuPaneLeft()
{
    if (!paneNavigate)
        paneNavigate = new GuiPaneNavigation(this, true);
    paneNavigate->navigateToDirectionPane(Qt::Key_Left);
}

void GuiMainWindow::contextMenuPaneRight()
{
    if (!paneNavigate)
        paneNavigate = new GuiPaneNavigation(this, true);
    paneNavigate->navigateToDirectionPane(Qt::Key_Right);
}
