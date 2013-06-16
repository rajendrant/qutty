/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include <QPainter>
#include <QDrag>
#include <QMimeData>
#include "GuiDrag.h"
#include "GuiMainWindow.h"
#include "GuiTerminalWindow.h"
#include "GuiSplitter.h"
#include "GuiTabWidget.h"
#include "GuiTabBar.h"

void GuiMainWindow::inittializeDragDropWidget()
{
    dragDropSite.hide();
}

GuiDragDropSite::GuiDragDropSite(QWidget *parent)
    : QWidget(parent),
      layout(this),
      drop_loc(GuiDragDropSite::DRAG_DROP_NONE),
      tabind(-1),
      drop_mode(GuiBase::TYPE_NONE)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    btn[GuiBase::TYPE_UP].setText(QString::fromUtf8("\xe2\x96\xb2"));
    layout.addWidget(&btn[GuiBase::TYPE_UP], 1, 2);
    btn[GuiBase::TYPE_LEFT].setText(QString::fromUtf8("\xe2\x97\x80"));
    layout.addWidget(&btn[GuiBase::TYPE_LEFT], 2, 1);
    btn[GuiBase::TYPE_RIGHT].setText(QString::fromUtf8("\xe2\x96\xb6"));
    layout.addWidget(&btn[GuiBase::TYPE_RIGHT], 2, 3);
    btn[GuiBase::TYPE_DOWN].setText(QString::fromUtf8("\xe2\x96\xbc"));
    layout.addWidget(&btn[GuiBase::TYPE_DOWN], 3, 2);
    btn[4].setText(QString::fromUtf8("\xe2\x96\xbc"));
    layout.addWidget(&btn[4], 2, 2);
    btn[GuiBase::TYPE_UP].setProperty("qutty_GuiDropSite_btn_drop_type", GuiBase::TYPE_UP);
    btn[GuiBase::TYPE_LEFT].setProperty("qutty_GuiDropSite_btn_drop_type", GuiBase::TYPE_LEFT);
    btn[GuiBase::TYPE_RIGHT].setProperty("qutty_GuiDropSite_btn_drop_type", GuiBase::TYPE_RIGHT);
    btn[GuiBase::TYPE_DOWN].setProperty("qutty_GuiDropSite_btn_drop_type", GuiBase::TYPE_DOWN);
    btn[4].setProperty("qutty_GuiDropSite_btn_drop_type", GuiBase::TYPE_LEAF);

    layout.setColumnStretch(0, 1);
    layout.setColumnStretch(4, 1);
    layout.setRowStretch(0, 1);
    layout.setRowStretch(4, 1);
    layout.setAlignment(Qt::AlignCenter);
    layout.setContentsMargins(QMargins(0,0,0,0));
}

void GuiDragDropSite::paintEvent(QPaintEvent *e)
{
    if (drop_loc == GuiDragDropSite::DRAG_DROP_ON_TABBAR) {
        btn[GuiBase::TYPE_UP].setVisible(false);
        btn[GuiBase::TYPE_DOWN].setVisible(false);
        btn[4].setVisible(true);
    } else {
        btn[GuiBase::TYPE_UP].setVisible(true);
        btn[GuiBase::TYPE_DOWN].setVisible(true);
        btn[4].setVisible(false);
    }

    QWidget::paintEvent(e);

    QPainter painter(this);
    painter.setOpacity(0.5);
    switch (drop_mode) {
    case GuiBase::TYPE_LEFT:
        painter.fillRect(0, 0, width()/2, height(), Qt::blue);
        break;
    case GuiBase::TYPE_RIGHT:
        painter.fillRect(width()/2, 0, width(), height(), Qt::blue);
        break;
    case GuiBase::TYPE_UP:
        painter.fillRect(0, 0, width(), height()/2, Qt::blue);
        break;
    case GuiBase::TYPE_DOWN:
        painter.fillRect(0, height()/2, width(), height(), Qt::blue);
        break;
    default:
        ;
    }
}

GuiBase::SplitType GuiDragDropSite::updateDropMode(const QPoint &pos)
{
    QWidget *w;
    if (!geometry().contains(pos))
        goto cu0;

    w = childAt(mapFromParent(pos));
    if (w) {
        QVariant v = w->property("qutty_GuiDropSite_btn_drop_type");
        GuiBase::SplitType split;
        if (v.type() == QVariant::Int &&
            (split=(GuiBase::SplitType)(v.toInt())) != drop_mode) {
            drop_mode = split;
            repaint();
        }
        return drop_mode;
    }


cu0:
    if (drop_mode != GuiBase::TYPE_NONE) {
        drop_mode = GuiBase::TYPE_NONE;
        repaint();
    }
    return drop_mode;
}

int GuiDragDropSite::updateDropOnTabBar(const QPoint &pos, GuiTabBar *tab)
{
    QWidget *w;
    int newind = tab->tabAt(pos);
    if (newind == -1 || !tab)
        return -1;

    if (newind != tabind) {
        drop_loc = GuiDragDropSite::DRAG_DROP_ON_TABBAR;
        tabind = newind;
        setParent(tab);
        show();
        move(tab->tabRect(tabind).topLeft());
        resize(tab->tabRect(tabind).size());
    }

    if (!geometry().contains(pos)) {
        clearDropMode();
        return -1;
    }

    w = childAt(mapFromParent(pos));
    if (w) {
        QVariant v = w->property("qutty_GuiDropSite_btn_drop_type");
        GuiBase::SplitType split;
        if (v.type() == QVariant::Int &&
            (split=(GuiBase::SplitType)(v.toInt())) != drop_mode) {
            drop_mode = split;
            if (drop_mode == GuiBase::TYPE_LEAF)
                tab->setCurrentIndex(tabind);
            repaint();
        }
    }

    return 0;
}

void GuiDragDropSite::clearDropMode()
{
    drop_loc = GuiDragDropSite::DRAG_DROP_NONE;
    tabind = -1;
    drop_mode = GuiBase::TYPE_NONE;
    this->setParent(NULL);
    this->hide();
}

void GuiTerminalWindow::dragStartEvent (QMouseEvent *)
{
    QPixmap pixmap = QPixmap(":/images/drag.png");

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("qutty-terminal-drag-drop-action", "application/tab-detach");

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(0,0));

    viewport()->setCursor(Qt::DragMoveCursor);
    drag->exec(Qt::MoveAction);
    viewport()->setCursor(Qt::IBeamCursor);
}

void GuiTerminalWindow::dragEnterEvent (QDragEnterEvent *e)
{
    if (e->source() == this ||
        !e->mimeData()->hasFormat("qutty-terminal-drag-drop-action")) {
        e->ignore();
        return;
    }
    mainWindow->dragDropSite.clearDropMode();
    mainWindow->dragDropSite.setParent(this);
    mainWindow->dragDropSite.show();
    mainWindow->dragDropSite.move(0, 0);
    mainWindow->dragDropSite.resize(viewport()->size());
    e->acceptProposedAction();
}


void GuiTerminalWindow::dragLeaveEvent (QDragLeaveEvent *)
{
    mainWindow->dragDropSite.clearDropMode();
}


void GuiTerminalWindow::dragMoveEvent (QDragMoveEvent *e)
{
    if (e->source() == this ||
        !e->mimeData()->hasFormat("qutty-terminal-drag-drop-action")) {
        e->ignore();
        return;
    }

    if (mainWindow->dragDropSite.updateDropMode(e->pos()) == GuiBase::TYPE_NONE) {
        e->ignore();
        return;
    }
    e->acceptProposedAction();
}


void GuiTerminalWindow::dropEvent (QDropEvent *e)
{
    if (e->source() == this ||
        !e->mimeData()->hasFormat("qutty-terminal-drag-drop-action")) {
        e->ignore();
        return;
    }
    GuiBase::SplitType split = mainWindow->dragDropSite.drop_mode;
    GuiTerminalWindow *dst, *dropped;
    dst = dynamic_cast<GuiTerminalWindow*>(this);
    dropped = dynamic_cast<GuiTerminalWindow*>(e->source());
    if (!dropped || !dst)
        goto cu0;
    if (dropped->getMainWindow() != dst->getMainWindow()) {
        // window-to-window drag-drop
        // TODO
        goto cu0;
    }
    qDebug()<<__FUNCTION__<<dropped->cfg.host<<dst->cfg.host<<split;

    if (dropped->parentSplit) {
        dropped->parentSplit->removeSplitLayout(dropped);
    }
    dst->createSplitLayout(split, dropped);

    // set the focus to the dropped terminal
    dropped->setFocus();

cu0:
    mainWindow->dragDropSite.clearDropMode();
}


void GuiTabBar::dragEnterEvent (QDragEnterEvent *e)
{
    if (e->source() == this ||
        !e->mimeData()->hasFormat("qutty-terminal-drag-drop-action")) {
        e->ignore();
        return;
    }
    mainWindow->dragDropSite.clearDropMode();

    e->acceptProposedAction();
}


void GuiTabBar::dragLeaveEvent (QDragLeaveEvent *)
{
    mainWindow->dragDropSite.clearDropMode();
}


void GuiTabBar::dragMoveEvent (QDragMoveEvent *e)
{
    if (e->source() == this ||
        !e->mimeData()->hasFormat("qutty-terminal-drag-drop-action")) {
        e->ignore();
        return;
    }

    if (mainWindow->dragDropSite.updateDropOnTabBar(e->pos(), this) == -1) {
        e->ignore();
        return;
    }
    e->acceptProposedAction();
}


void GuiTabBar::dropEvent (QDropEvent *e)
{
    if (e->source() == this ||
        !e->mimeData()->hasFormat("qutty-terminal-drag-drop-action")) {
        e->ignore();
        return;
    }
    GuiBase::SplitType split = mainWindow->dragDropSite.drop_mode;
    int tabind = mainWindow->dragDropSite.get_tabind();

    if ( mainWindow->dragDropSite.get_drop_loc() != GuiDragDropSite::DRAG_DROP_ON_TABBAR ||
         (split != GuiBase::TYPE_LEFT && split != GuiBase::TYPE_RIGHT) ||
         tabind == -1)
        goto cu0;

    GuiTerminalWindow *dropped;
    dropped = dynamic_cast<GuiTerminalWindow*>(e->source());
    if (!dropped)
        goto cu0;

    if (dropped->getMainWindow() != this->mainWindow) {
        // window-to-window drag-drop
        // TODO
        goto cu0;
    }

    qDebug()<<__FUNCTION__<<dropped->cfg.host<<tabind<<split;

    if (dropped->parentSplit) {
        dropped->parentSplit->removeSplitLayout(dropped);
    }
    mainWindow->setupLayout(dropped, GuiBase::TYPE_LEAF,
                            split == GuiBase::TYPE_LEFT ? tabind : tabind+1);

cu0:
    mainWindow->dragDropSite.clearDropMode();
}
