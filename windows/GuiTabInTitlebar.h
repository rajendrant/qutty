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
    QWidget *tabAreaLCornerWidget;
    bool isCompositionEnabled;

    int tabbar_height;
    int titlebar_frame_width;
    int titlebar_captionbtn_width;
    int window_frame_width;

    bool hitTestNCA(MSG *msg, long *result);

public:
    GuiTabInTitlebar(QMainWindow *mainwindow, QTabWidget *tabarea, QTabBar *tabbar);
    void initialize(bool enable);
    void handleWindowResize();
    bool handleWinEvent(MSG *msg, long *result);
    void handleWindowStateChangeEvent(Qt::WindowStates);
    void setTabAreaCornerWidget(QWidget *w);
    bool getCompositionStatus() {return isCompositionEnabled; }
};

#endif // GUITABINTITLEBAR_H
