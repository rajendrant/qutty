#ifndef GUITREEWIDGET_H
#define GUITREEWIDGET_H

#include <QTreeWidget>

class GuiTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit GuiTreeWidget(QWidget *parent = 0);

    // Needed functions for drag-drop support
    void dropEvent (QDropEvent *e);

signals:

public slots:

signals:
    void sig_hierarchyChanged(QTreeWidgetItem *item);
    
};

#endif // GUITREEWIDGET_H
