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
    QMenu menuSavedSessions;
    int menuTabIndex;

public:
    GuiTabWidget(GuiMainWindow *parent);
public slots:
    void showContextMenu(const QPoint &point);
    void savedSessionsChanged();
    void contextMenuDuplicateSessionTriggered();
    void contextMenuRestartSessionTriggered();
    void contextMenuSavedSessionTriggered();
    void contextMenuChangeSettingsTriggered();
    void contextMenuCloseSessionTriggered();
};


#endif // GUITABWIDGET_H
