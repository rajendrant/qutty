/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#ifndef GUITABWIDGET_H
#define GUITABWIDGET_H

#include <QTabWidget>
#include <QMenu>
#include "GuiMainWindow.h"

class GuiTabWidget : public QTabWidget {
    Q_OBJECT

    GuiMainWindow *mainWindow;

public:
    GuiTabWidget(GuiMainWindow *parent);

public slots:
    void showContextMenu(const QPoint &point);
};

#endif // GUITABWIDGET_H
