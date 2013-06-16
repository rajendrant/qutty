/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#ifndef GUITABBAR_H
#define GUITABBAR_H

#include <QTabBar>

class GuiTabWidget;
class GuiMainWindow;

class GuiTabBar : public QTabBar {
    Q_OBJECT

    GuiMainWindow *mainWindow;

public:
    GuiTabBar(GuiTabWidget *t, GuiMainWindow *main);
    virtual ~GuiTabBar() { }

    // Needed functions for drag-drop support
    void dragEnterEvent (QDragEnterEvent *e);
    void dragLeaveEvent (QDragLeaveEvent *e);
    void dragMoveEvent (QDragMoveEvent *e);
    void dropEvent (QDropEvent *e);

protected:
    void tabInserted(int index) { (void)index; emit sig_tabInserted(); }
    void tabRemoved(int index) { (void)index; emit sig_tabRemoved(); }

signals:
    void sig_tabInserted();
    void sig_tabRemoved();
};

#endif // GUITABBAR_H
