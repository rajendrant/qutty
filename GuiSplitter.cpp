/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include "GuiSplitter.h"

GuiSplitter::GuiSplitter(Qt::Orientation split, QWidget *parent, GuiSplitter *parentsplit) :
    QSplitter(split, parent),
    child()
{
    GuiBase::parentSplit = parentsplit;
    if (parentsplit)
        parentsplit->child.push_back(this);
    QSplitter::setHandleWidth(1);
}

void GuiSplitter::focusInEvent ( QFocusEvent * e )
{
    //this->focusWidget()->setFocus();
    //focusNextChild();
}

void GuiSplitter::focusOutEvent ( QFocusEvent * e )
{
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
    this->insertWidget(this->indexOf(after)+1, a);
    this->child.push_back(a);

    oldsizes.insert(ind+1, 0);
    oldsizes[ind + 1] = oldsizes[ind] - oldsizes[ind]/2;
    oldsizes[ind] = oldsizes[ind]/2;
    this->setSizes(oldsizes);
}
