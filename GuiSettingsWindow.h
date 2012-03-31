/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#ifndef GUISETTINGSWINDOW_H
#define GUISETTINGSWINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QButtonGroup>

class GuiSettingsWindow : public QDialog
{
    Q_OBJECT
public:
    QLineEdit *txtHostName;
    QLineEdit *txtPort;
    QButtonGroup *btnConnType;
    explicit GuiSettingsWindow(QWidget *parent = 0);

public slots:
    void newTerminal();
    void btnConnTypeClicked(int id);
};

#endif // GUISETTINGSWINDOW_H
