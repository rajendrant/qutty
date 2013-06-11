/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#ifndef GUINAVIGATION_H
#define GUINAVIGATION_H

#include <QListWidget>

class GuiMainWindow;
class GuiTerminalWindow;

class GuiTabNavigation : public QListWidget
{
    GuiMainWindow *mainWindow;
    QList<int> tablist;
    bool is_active;
    bool accept_sel;

public:
    GuiTabNavigation(GuiMainWindow *p);
    void terminalFocusIn(GuiTerminalWindow *term);

    void activateTabNavigateGUI();
    void navigateToTabNext();
    void navigateToTabPrev();
    void deactivateTabNavigateGUI();

protected:
    void focusOutEvent ( QFocusEvent * e );
    void keyPressEvent ( QKeyEvent * e );
    void keyReleaseEvent ( QKeyEvent * e );
    bool event( QEvent * e );
};

#endif // GUINAVIGATION_H
