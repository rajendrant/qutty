/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include <QtGui/QApplication>
#include "GuiMainWindow.h"
#include "GuiTerminalWindow.h"
#include "GuiSettingsWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // restore all settings from qutty.xml
    qutty_config.restoreConfig();

    GuiMainWindow *mainWindow = new GuiMainWindow();

    mainWindow->on_openNewTab();
    mainWindow->show();

    return app.exec();
}
