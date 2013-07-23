#ifndef GUITABINTITLEBAR_H
#define GUITABINTITLEBAR_H

#include "windows/dwm/GuiDwmApi.h"
#include <QMainWindow>
#include <QTabWidget>
#include <QTabBar>

class GuiTabInTitlebar
{
    GuiDwmApi dwmApi;
    QMainWindow *mainWindow;
    QTabWidget *tabArea;
    QTabBar *tabBar;
    QWidget *tabAreaCornerWidget;
    bool isCompositionEnabled;

    bool hitTestNCA(MSG *msg, long *result);

public:
    GuiTabInTitlebar(QMainWindow *mainwindow, QTabWidget *tabarea, QTabBar *tabbar);
    bool handleWinEvent(MSG *msg, long *result);
    void setTabAreaCornerWidget(QWidget *w);
};

#endif // GUITABINTITLEBAR_H
