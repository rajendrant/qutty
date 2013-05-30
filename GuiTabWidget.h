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

signals:
    void sig_tabChangeSettings(GuiTerminalWindow *termWnd);
};


#endif // GUITABWIDGET_H
