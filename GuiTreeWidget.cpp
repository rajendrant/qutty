#include "GuiTreeWidget.h"
#include <QDebug>
#include <QDragMoveEvent>

GuiTreeWidget::GuiTreeWidget(QWidget *parent) : QTreeWidget(parent) {}

void GuiTreeWidget::dropEvent(QDropEvent *e) {
  QTreeWidgetItem *cur = this->currentItem();
  QTreeWidget::dropEvent(e);
  emit sig_hierarchyChanged(cur);
}
