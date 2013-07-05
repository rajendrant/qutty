#include "GuiTreeWidget.h"
#include <QDebug>
#include <QDragMoveEvent>


GuiTreeWidget::GuiTreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
}

void GuiTreeWidget::dragMoveEvent (QDragMoveEvent *e)
{
    QTreeWidgetItem *cur = this->currentItem();
    if (!cur || !cur->data(0, GuiTreeWidget::DragEnabledProperty).toBool()) {
        e->ignore();
        return;
    }

    QTreeWidgetItem *ch = this->itemAt(e->pos());
    if (!ch || !ch->data(0, GuiTreeWidget::DropEnabledProperty).toBool()) {
        e->ignore();
        return;
    }
    QTreeWidget::dragMoveEvent(e);
}

void GuiTreeWidget::dropEvent (QDropEvent *e)
{
    QTreeWidgetItem *cur = this->currentItem();
    if (!cur || !cur->data(0, GuiTreeWidget::DragEnabledProperty).toBool()) {
        e->ignore();
        return;
    }

    QTreeWidgetItem *ch = this->itemAt(e->pos());
    if (!ch || !ch->data(0, GuiTreeWidget::DropEnabledProperty).toBool()) {
        e->ignore();
        return;
    }
    QTreeWidget::dropEvent(e);
    emit sig_hierarchyChanged(cur);
}
