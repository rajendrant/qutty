/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include "GuiSplitter.h"

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

void GuiSplitter::addTerminalConsecutive(GuiTerminalWindow *a, GuiTerminalWindow *b)
{
    a->parentSplit = this;
    b->parentSplit = this;
    this->addWidget(a);
    this->insertWidget(this->indexOf(a)+1, b);
    this->child.push_back(a);
    this->child.push_back(b);
}

void GuiSplitter::addTerminalAfter(GuiTerminalWindow *a, GuiTerminalWindow *after)
{
    int ind = this->indexOf(after);
    if (ind == -1)
        return;

    QList<int> oldsizes = this->sizes();

    a->parentSplit = this;
    this->insertWidget(ind+1, a);
    this->child.push_back(a);
    a->setParent(this);

    oldsizes.insert(ind+1, oldsizes[ind]/2);
    oldsizes[ind] -= oldsizes[ind]/2;
    this->setSizes(oldsizes);
}

void GuiSplitter::reqCloseTerminal(bool userRequest)
{
    for(vector<GuiBase*>::iterator it = child.begin() ; it != child.end(); ++it)
        (*it)->reqCloseTerminal(userRequest);
    child.clear();
}

void GuiTerminalWindow::createSplitLayout(GuiBase::SplitType split, GuiTerminalWindow *newTerm)
{
    Qt::Orientation orient = split==GuiBase::TYPE_HORIZONTAL ? Qt::Vertical :
                                                               Qt::Horizontal;
    this->_disableResize = newTerm->_disableResize = true;

    if (!parentSplit) {
        int tabind = mainWindow->tabArea->indexOf(this);
        int initsize = orient==Qt::Vertical ? viewport()->width() :
                                              viewport()->height();
        assert(tabind != -1);

        mainWindow->tabArea->removeTab(tabind);
        GuiSplitter *splitter = new GuiSplitter(orient);
        splitter->addTerminal(0, this);
        splitter->addTerminal(1, newTerm);

        mainWindow->tabArea->insertTab(tabind, splitter, tr(APPNAME));
        mainWindow->tabArea->setCurrentIndex(tabind);

        QList<int> listsizes = splitter->sizes();
        listsizes[0] = initsize - initsize/2;
        listsizes[1] = initsize/2;
        splitter->setSizes(listsizes);
        this->show();
    } else {
        parentSplit->createSplitLayout(orient, this, newTerm);
    }

    newTerm->show();
    this->_disableResize = newTerm->_disableResize = false;
    newTerm->resizeEvent(NULL);
    this->resizeEvent(NULL);
}

void GuiSplitter::createSplitLayout(Qt::Orientation orient, GuiTerminalWindow *oldTerm, GuiTerminalWindow *newTerm)
{
    QList<int> listsizes = sizes();
    int ind = indexOf(oldTerm);
    int initsize = orient==Qt::Vertical ? oldTerm->viewport()->width() :
                                          oldTerm->viewport()->height();

    assert(ind != -1);

    if (orient != orientation()) {
        GuiSplitter *splitter = new GuiSplitter(orient, this, ind);
        this->removeTerminal(oldTerm);
        splitter->addTerminal(0, oldTerm);
        splitter->addTerminal(1, newTerm);

        // restore the sizes
        this->setSizes(listsizes);
        listsizes = splitter->sizes();
        listsizes[0] = initsize - initsize/2;
        listsizes[1] = initsize/2;
        splitter->setSizes(listsizes);
        this->show();
    } else {
        this->addTerminal(ind+1, newTerm);
        listsizes.insert(ind+1, listsizes[ind]/2);
        listsizes[ind] -= listsizes[ind]/2;
        this->setSizes(listsizes);
    }
}

void GuiSplitter::addTerminal(int ind, GuiTerminalWindow *term) {
    child.push_back(term);
    term->parentSplit = this;
    insertWidget(ind, term);
}

void GuiSplitter::removeTerminal(GuiTerminalWindow *term) {
    child.erase(std::remove(child.begin(), child.end(), term));
    term->parentSplit = NULL;
    //removeWidget(term);
    term->setParent(NULL);
}
