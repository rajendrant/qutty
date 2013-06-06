/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#ifndef GUILAYOUT_H
#define GUILAYOUT_H
#include <QSplitter>
#include <vector>

using namespace std;

class GuiTerminalWindow;
class GuiSplitter;

class GuiBase
{

public:
    enum SplitType {
        TYPE_NONE       = -1,
        TYPE_UP         = 0,
        TYPE_LEFT       = 1,
        TYPE_DOWN       = 2,
        TYPE_RIGHT      = 3,
        TYPE_LEAF       = 4,
        TYPE_HORIZONTAL = TYPE_DOWN,
        TYPE_VERTICAL   = TYPE_RIGHT
    };
    GuiSplitter *parentSplit;

    GuiBase();
    virtual ~GuiBase() { };
    virtual void reqCloseTerminal(bool userRequest) = 0;
    virtual QWidget *getWidget() = 0;
};

#endif // GUILAYOUT_H
