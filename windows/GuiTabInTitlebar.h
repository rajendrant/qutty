#ifndef GUITABINTITLEBAR_H
#define GUITABINTITLEBAR_H

#include "windows/dwm/GuiDwmApi.h"
#include <QMainWindow>
#include <QTabWidget>
#include <QTabBar>

class GuiTabInTitlebar
{
    QMainWindow *mainWindow;
    QTabWidget *tabArea;
    QTabBar *tabBar;

public:
    GuiTabInTitlebar(QMainWindow *mainwindow, QTabWidget *tabarea, QTabBar *tabbar);
    bool handleWinEvent(MSG *msg, long *result);
    void setTabAreaCornerWidget(QWidget *w);
};

#endif // GUITABINTITLEBAR_H
