#ifndef GUITABWIDGET_H
#define GUITABWIDGET_H

#include <QTabWidget>
#include <QMenu>
#include "GuiMainWindow.h"

class GuiTabWidget : public QTabWidget {
    Q_OBJECT

    GuiMainWindow *mainWindow;
    // context menu to show when right-click on Tabbar
    QMenu menu;

public:
    GuiTabWidget(GuiMainWindow *parent);
public slots:
    void showContextMenu(const QPoint &point);
};


#endif // GUITABWIDGET_H
