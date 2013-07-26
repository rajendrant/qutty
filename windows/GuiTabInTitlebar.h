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

    int tabbar_height;
    int titlebar_frame_width;
    int window_frame_width;

    bool hitTestNCA(MSG *msg, long *result);

public:
    GuiTabInTitlebar(QMainWindow *mainwindow, QTabWidget *tabarea, QTabBar *tabbar, bool enable=true);
    bool handleWinEvent(MSG *msg, long *result);
    void handleWindowStateChangeEvent(Qt::WindowStates);
    void setTabAreaCornerWidget(QWidget *w);
};

#endif // GUITABINTITLEBAR_H
