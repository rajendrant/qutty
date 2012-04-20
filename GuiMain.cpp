/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include <QtGui/QApplication>
#include <QShortcut>
#include "GuiMainWindow.h"
#include "GuiTerminalWindow.h"
#include "GuiSettingsWindow.h"

GuiMainWindow *mainWindow;

void initKeyboardShortcuts()
{
    QShortcut *shortcut = new QShortcut(QKeySequence("Shift+Right"), mainWindow);
    shortcut->setContext(Qt::ApplicationShortcut);
    QObject::connect(shortcut, SIGNAL(activated()), mainWindow, SLOT(tabNext()));
    shortcut = new QShortcut(QKeySequence("Shift+Left"), mainWindow);
    shortcut->setContext(Qt::ApplicationShortcut);
    QObject::connect(shortcut, SIGNAL(activated()), mainWindow, SLOT(tabPrev()));
    shortcut = new QShortcut(QKeySequence("Ctrl+Shift+t"), mainWindow);
    shortcut->setContext(Qt::ApplicationShortcut);
    QObject::connect(shortcut, SIGNAL(activated()), mainWindow, SLOT(openSettingsWindow()));
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    mainWindow = new GuiMainWindow();
    QObject::connect(&a, SIGNAL(focusChanged(QWidget *, QWidget *)), mainWindow, SLOT(focusChanged(QWidget*,QWidget*)));
    mainWindow->show();
    initKeyboardShortcuts();
    GuiSettingsWindow *ss = new GuiSettingsWindow(mainWindow);
    ss->show();
    return a.exec();
}
