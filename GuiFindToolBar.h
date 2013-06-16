/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#ifndef GUIFINDTOOLBAR_H
#define GUIFINDTOOLBAR_H

#include <QToolBar>
#include <QLineEdit>

class GuiMainWindow;

class GuiFindToolBar : public QToolBar
{
    Q_OBJECT

    GuiMainWindow *mainWnd;

public:
    GuiFindToolBar(GuiMainWindow *p);
    virtual ~GuiFindToolBar() { }
public slots:
    void on_findNext();
    void on_findPrevious();
    void on_findClose();
};

#endif // GUIFINDTOOLBAR_H
