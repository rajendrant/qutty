/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include <QApplication>
#include "GuiMainWindow.h"
#include "GuiTerminalWindow.h"
#include "GuiSettingsWindow.h"
#include "QtTimer.h"

QtTimer *globalTimer;

int main(int argc, char *argv[])
{
    globalTimer = new QtTimer;

    QApplication a(argc, argv);
    GuiMainWindow *mainWindow = new GuiMainWindow();

    qutty_config.restoreConfig();

    mainWindow->on_openNewTab();
    mainWindow->show();

    return a.exec();
}
