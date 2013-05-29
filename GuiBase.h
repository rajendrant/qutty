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
        TYPE_NONE = -1,
        TYPE_LEAF,
        TYPE_HORIZONTAL,
        TYPE_VERTICAL
    };
    GuiSplitter *parentSplit;

    GuiBase();
};

#include "GuiSplitter.h"

#endif // GUILAYOUT_H
