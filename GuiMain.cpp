/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include <QtGui/QApplication>
#include "GuiMainWindow.h"
#include "GuiTerminalWindow.h"
#include "GuiSettingsWindow.h"

GuiMainWindow *mainWindow;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    mainWindow = new GuiMainWindow();
    QObject::connect(&a, SIGNAL(focusChanged(QWidget *, QWidget *)), mainWindow, SLOT(focusChanged(QWidget*,QWidget*)));
    //mainWindow->newTelnetTerminal();
    mainWindow->show();
    GuiSettingsWindow *ss = new GuiSettingsWindow(mainWindow);
    ss->show();
    return a.exec();
}
