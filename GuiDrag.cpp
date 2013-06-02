/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include <QPainter>
#include "GuiDrag.h"
#include "GuiMainWindow.h"
#include "GuiTerminalWindow.h"
#include "GuiSplitter.h"

void GuiMainWindow::inittializeDragDropWidget()
{
    dragDropSite.hide();
}

GuiDragDropSite::GuiDragDropSite(QWidget *parent)
    : QWidget(parent),
      layout(this),
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

    layout.setColumnStretch(0, 1);
    layout.setColumnStretch(4, 1);
    layout.setRowStretch(0, 1);
    layout.setRowStretch(4, 1);
    layout.setAlignment(Qt::AlignCenter);
}

void GuiDragDropSite::paintEvent(QPaintEvent *e)
{
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
    }
}

GuiBase::SplitType GuiDragDropSite::updateDropMode(const QPoint &pos)
{
    QToolButton *b;
    if (!geometry().contains(pos))
        goto cu0;

    b = dynamic_cast<QToolButton*>(childAt(mapFromParent(pos)));
    for (GuiBase::SplitType i=GuiBase::TYPE_UP;
         i <= GuiBase::TYPE_RIGHT;
         i = (GuiBase::SplitType) (((int)i)+1)) {
        if (b == btn+i) {
            if (i != drop_mode) {
                drop_mode = i;
                repaint();
            }
            return drop_mode;
        }
    }

cu0:
    if (drop_mode != GuiBase::TYPE_NONE) {
        drop_mode = GuiBase::TYPE_NONE;
        repaint();
    }
    return drop_mode;
}

void GuiDragDropSite::clearDropMode()
{
    drop_mode = GuiBase::TYPE_NONE;
    this->setParent(NULL);
    this->hide();
}

void GuiTerminalWindow::dragStartEvent (QMouseEvent *e)
{
    QPixmap pixmap = QPixmap("qutty.ico");

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


void GuiTerminalWindow::dragLeaveEvent (QDragLeaveEvent *e)
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

cu0:
    mainWindow->dragDropSite.clearDropMode();
}
