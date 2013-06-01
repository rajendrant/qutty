/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#ifndef GUISPLITTER_H
#define GUISPLITTER_H

class GuiBase;
class GuiTerminalWindow;
#include "GuiBase.h"

class GuiSplitter : public GuiBase, public QSplitter
{
public:
    vector<GuiBase*> child;
    GuiSplitter(Qt::Orientation split, GuiSplitter *parentsplit=NULL, int ind=-1);

    QWidget *getWidget() { return this; }

    void addBaseWidget(int ind, GuiBase *base);
    void removeBaseWidget(GuiBase *base);

    void createSplitLayout(Qt::Orientation orient, SplitType split, GuiTerminalWindow *oldTerm, GuiTerminalWindow *newTerm);
    void reqCloseTerminal(bool userRequest);
    void removeSplitLayout(GuiTerminalWindow *term);
};

#endif // GUISPLITTER_H
