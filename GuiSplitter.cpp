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
        splitter->addBaseWidget(0, this);
        splitter->addBaseWidget(1, newTerm);

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
        this->removeBaseWidget(oldTerm);
        splitter->addBaseWidget(0, oldTerm);
        splitter->addBaseWidget(1, newTerm);

        // restore the sizes
        this->setSizes(listsizes);
        listsizes = splitter->sizes();
        listsizes[0] = initsize - initsize/2;
        listsizes[1] = initsize/2;
        splitter->setSizes(listsizes);
        this->show();
    } else {
        this->addBaseWidget(ind+1, newTerm);
        listsizes.insert(ind+1, listsizes[ind]/2);
        listsizes[ind] -= listsizes[ind]/2;
        this->setSizes(listsizes);
    }
}

void GuiSplitter::addBaseWidget(int ind, GuiBase *base) {
    QWidget *w;
    child.push_back(base);
    base->parentSplit = this;
    if ((w = dynamic_cast<QWidget*>(base)))
        insertWidget(ind, w);
}

void GuiSplitter::removeBaseWidget(GuiBase *base) {
    QWidget *w;
    child.erase(std::remove(child.begin(), child.end(), base));
    base->parentSplit = NULL;
    //removeWidget(base);
    if ((w = dynamic_cast<QWidget*>(base)))
        w->setParent(NULL);
}

void GuiSplitter::removeSplitLayout(GuiTerminalWindow *term)
{
    removeBaseWidget(term);

    if (child.size() != 1)
        return;

    GuiBase *b = child[0];
    QWidget *w;
    removeBaseWidget(b);
    if (parentSplit) {
        parentSplit->child.erase(std::remove(parentSplit->child.begin(),
                                             parentSplit->child.end(),
                                             this));
        parentSplit->addBaseWidget(parentSplit->indexOf(this), b);
        parentSplit = NULL;
    } else if ((w = dynamic_cast<QWidget*>(b))) {
        int tabind = term->getMainWindow()->tabArea->indexOf(this);
        term->getMainWindow()->tabArea->removeTab(tabind);
        term->getMainWindow()->tabArea->insertTab(tabind, w, tr(APPNAME));
    }
    this->close();
    this->deleteLater();
}
