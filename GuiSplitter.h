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
#include "GuiTerminalWindow.h"

class GuiSplitter : public GuiBase, public QSplitter
{
public:
    vector<GuiBase*> child;
    GuiSplitter(Qt::Orientation split, GuiSplitter *parentsplit=NULL, int ind=-1);

    void addTerminalConsecutive(GuiTerminalWindow *a, GuiTerminalWindow *b);
    void addTerminalAfter(GuiTerminalWindow *a, GuiTerminalWindow *after);
    void createSplitLayout(Qt::Orientation orient, GuiTerminalWindow *oldTerm, GuiTerminalWindow *newTerm);
    void reqCloseTerminal(bool userRequest);

    void addTerminal(int ind, GuiTerminalWindow *term);
    void removeTerminal(GuiTerminalWindow *term);
};

#endif // GUISPLITTER_H