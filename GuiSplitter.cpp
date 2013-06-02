/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include "GuiSplitter.h"
#include "GuiTerminalWindow.h"

GuiSplitter::GuiSplitter(Qt::Orientation split, GuiSplitter *parentsplit, int ind) :
    QSplitter(split, parentsplit),
    child()
{
    parentSplit = parentsplit;
    if (parentSplit) {
        parentSplit->child.push_back(this);
        parentSplit->insertWidget(ind, this);
    }
    setHandleWidth(1);
}

void GuiSplitter::reqCloseTerminal(bool userRequest)
{
    for(vector<GuiBase*>::iterator it = child.begin() ; it != child.end(); ++it)
        (*it)->reqCloseTerminal(userRequest);
    child.clear();
    this->close();
    this->deleteLater();
}

void GuiTerminalWindow::createSplitLayout(GuiBase::SplitType split, GuiTerminalWindow *newTerm)
{
    Qt::Orientation orient = (split==GuiBase::TYPE_HORIZONTAL || split==GuiBase::TYPE_UP)
                                ? Qt::Vertical : Qt::Horizontal;

    if (!parentSplit) {
        int tabind = mainWindow->tabArea->indexOf(this);
        int initsize = orient==Qt::Vertical ? viewport()->width() :
                                              viewport()->height();
        assert(tabind != -1);

        mainWindow->tabArea->removeTab(tabind);
        GuiSplitter *splitter = new GuiSplitter(orient);
        if (split==GuiBase::TYPE_HORIZONTAL || split==GuiBase::TYPE_VERTICAL) {
            splitter->addBaseWidget(0, this);
            splitter->addBaseWidget(1, newTerm);
        } else {
            splitter->addBaseWidget(0, newTerm);
            splitter->addBaseWidget(1, this);
        }

        mainWindow->tabArea->insertTab(tabind, splitter, tr(APPNAME));
        mainWindow->tabArea->setCurrentIndex(tabind);

        QList<int> listsizes = splitter->sizes();
        listsizes[0] = initsize - initsize/2;
        listsizes[1] = initsize/2;
        splitter->setSizes(listsizes);
        this->show();
    } else {
        parentSplit->createSplitLayout(orient, split, this, newTerm);
    }

    newTerm->show();
}

void GuiSplitter::createSplitLayout(Qt::Orientation orient, GuiBase::SplitType split,
                                    GuiTerminalWindow *oldTerm, GuiTerminalWindow *newTerm)
{
    QList<int> listsizes = sizes();
    int ind = indexOf(oldTerm);
    int initsize = orient==Qt::Vertical ? oldTerm->viewport()->width() :
                                          oldTerm->viewport()->height();

    assert(ind != -1);

    if (orient != orientation()) {
        GuiSplitter *splitter = new GuiSplitter(orient, this, ind);
        this->removeBaseWidget(oldTerm);
        if (split==GuiBase::TYPE_HORIZONTAL || split==GuiBase::TYPE_VERTICAL) {
            splitter->addBaseWidget(0, oldTerm);
            splitter->addBaseWidget(1, newTerm);
        } else {
            splitter->addBaseWidget(0, newTerm);
            splitter->addBaseWidget(1, oldTerm);
        }

        // restore the sizes
        this->setSizes(listsizes);
        listsizes = splitter->sizes();
        listsizes[0] = initsize - initsize/2;
        listsizes[1] = initsize/2;
        splitter->setSizes(listsizes);
        this->show();
    } else {
        if (split==GuiBase::TYPE_HORIZONTAL || split==GuiBase::TYPE_VERTICAL)
            this->addBaseWidget(ind+1, newTerm);
        else
            this->addBaseWidget(ind, newTerm);
        listsizes.insert(ind+1, listsizes[ind]/2);
        listsizes[ind] -= listsizes[ind]/2;
        this->setSizes(listsizes);
    }
}

void GuiSplitter::addBaseWidget(int ind, GuiBase *base) {
    child.push_back(base);
    base->parentSplit = this;
    insertWidget(ind, base->getWidget());
}

void GuiSplitter::removeBaseWidget(GuiBase *base) {
    child.erase(std::remove(child.begin(), child.end(), base));
    base->parentSplit = NULL;
    //removeWidget(base);
    base->getWidget()->setParent(NULL);
}

void GuiSplitter::removeSplitLayout(GuiTerminalWindow *term)
{
    removeBaseWidget(term);

    if (child.size() != 1)
        return;

    GuiBase *b = child[0];
    removeBaseWidget(b);
    if (parentSplit) {
        parentSplit->child.erase(std::remove(parentSplit->child.begin(),
                                             parentSplit->child.end(),
                                             this));
        parentSplit->addBaseWidget(parentSplit->indexOf(this), b);
        parentSplit = NULL;
    } else {
        int currtab = term->getMainWindow()->tabArea->currentIndex();
        int tabind = term->getMainWindow()->tabArea->indexOf(this);
        term->getMainWindow()->tabArea->removeTab(tabind);
        term->getMainWindow()->tabArea->insertTab(tabind, b->getWidget(), tr(APPNAME));
        if (currtab == tabind)
            term->getMainWindow()->tabArea->setCurrentIndex(tabind);
    }
    this->close();
    this->deleteLater();
}
