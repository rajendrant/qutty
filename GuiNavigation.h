/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#ifndef GUINAVIGATION_H
#define GUINAVIGATION_H

#include <stdint.h>
#include <QListWidget>

class GuiMainWindow;
class GuiTerminalWindow;

class GuiTabNavigation : public QListWidget
{
    GuiMainWindow *mainWindow;

public:
    GuiTabNavigation(GuiMainWindow *p);

    void navigateToTabNext();
    void navigateToTabPrev();
    void acceptNavigation();

protected:
    void focusOutEvent ( QFocusEvent * e );
    void keyReleaseEvent ( QKeyEvent * e );
    bool event( QEvent * e );
};

class GuiPaneNavigation : public QWidget
{
    GuiMainWindow *mainWindow;

public:
    GuiPaneNavigation(GuiMainWindow *p);

    void navigateToPaneNext();
    void navigateToPanePrev();
    void acceptNavigation();

protected:
    void focusOutEvent ( QFocusEvent * e );
    void keyReleaseEvent ( QKeyEvent * e );
    //bool event( QEvent * e );
};

#endif // GUINAVIGATION_H
