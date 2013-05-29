/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#ifndef GUISPLITTER_H
#define GUISPLITTER_H

class GuiBase;
#include "GuiBase.h"
#include "GuiTerminalWindow.h"

class GuiSplitter : public GuiBase, public QSplitter
{
public:
    vector<GuiBase*> child;
    GuiSplitter(Qt::Orientation split, QWidget *parent=NULL, GuiSplitter *parentsplit=NULL);

    void focusInEvent ( QFocusEvent * e );
    void focusOutEvent ( QFocusEvent * e );
    void addTerminalConsecutive(GuiTerminalWindow *a, GuiTerminalWindow *b);
    void addTerminalAfter(GuiTerminalWindow *a, GuiTerminalWindow *after);
};

#endif // GUISPLITTER_H
