/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#ifndef GUISETTINGSWINDOW_H
#define GUISETTINGSWINDOW_H

#include <QDialog>
#include<QTreeWidgetItem>
#include<QButtonGroup>

namespace Ui {
class GuiSettingsWindow;
}

class GuiSettingsWindow : public QDialog
{
    Q_OBJECT
    enum
    {
       SESSION, LOGGING, TERMINAL, KEYBOARD, BELL, FEATURES, WINDOW, APPEARANCE, BEHAVIOUR, TRANSLATION, SELECTION, COLOURS, CONNECTION,
        DATA, PROXY, TELNET, RLOGIN, SSH, KEX, AUTH, GSSAPI, TTY, X11, TUNNELS, BUGS, SERIAL
    }category;
    enum
    {
        NONE, PRINT_OUT, ALL_SES_OUT, SSH_PACKET, SSH_RAWDATA
    }logging;

public:
    explicit GuiSettingsWindow(QWidget *parent = 0);
    ~GuiSettingsWindow();

private slots:
    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_rb_contype_telnet_clicked();

    void on_rb_contype_ssh_clicked();

private:
    Ui::GuiSettingsWindow *ui;
};

#endif // GUISETTINGSWINDOW_H
