/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include <QApplication>
#include "GuiMainWindow.h"
#include "GuiTerminalWindow.h"
#include "GuiSettingsWindow.h"

#ifdef QUTTY_ENABLE_BREAKPAD_SUPPORT
#include "client/windows/handler/exception_handler.h"
#endif

int main(int argc, char *argv[])
{
#ifdef QUTTY_ENABLE_BREAKPAD_SUPPORT
    new google_breakpad::ExceptionHandler(
                std::wstring(L"C:/dumps/"),
                NULL,
                NULL,
                NULL,
                google_breakpad::ExceptionHandler::HANDLER_ALL);
#endif

    QApplication app(argc, argv);

    // restore all settings from qutty.xml
    qutty_config.restoreConfig();

    GuiMainWindow *mainWindow = new GuiMainWindow();

    mainWindow->on_openNewTab();
    mainWindow->show();

    return app.exec();
}
