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

class GuiSplitter : public QSplitter, public GuiBase
{
    Q_OBJECT

public:
    vector<GuiBase*> child;
    GuiSplitter(Qt::Orientation split, GuiSplitter *parentsplit=NULL, int ind=-1);
    virtual ~GuiSplitter() { };

    QWidget *getWidget() { return this; }

    void addBaseWidget(int ind, GuiBase *base);
    void removeBaseWidget(GuiBase *base);

    void createSplitLayout(Qt::Orientation orient, SplitType split, GuiTerminalWindow *oldTerm, GuiTerminalWindow *newTerm);
    void reqCloseTerminal(bool userRequest);
    void removeSplitLayout(GuiTerminalWindow *term);

    void populateAllTerminals(vector<GuiTerminalWindow*> *list) {
        for(auto it=child.begin(); it != child.end(); it++)
            (*it)->populateAllTerminals(list);
    }

    GuiTerminalWindow* navigatePane(Qt::Key key, GuiTerminalWindow *tofind, int splitind=-1);
};

#endif // GUISPLITTER_H
