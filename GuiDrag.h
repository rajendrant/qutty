/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#ifndef GUIDRAG_H
#define GUIDRAG_H

#include <QGridLayout>
#include <QToolButton>
#include "GuiBase.h"

class GuiBase;

class GuiDragDropSite : public QWidget {
public:
    GuiBase::SplitType drop_mode;

    GuiDragDropSite(QWidget *parent = NULL);
    GuiBase::SplitType updateDropMode(const QPoint &pos);
    void clearDropMode();
    void paintEvent(QPaintEvent *e);

private:
    QGridLayout layout;
    QToolButton btn[4];
};

#endif // GUIDRAG_H
