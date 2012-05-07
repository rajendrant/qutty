/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include "GuiSettingsWindow.h"
#include "GuiMainWindow.h"
#include "GuiTerminalWindow.h"
#include "ui_GuiSettingsWindow.h"
#include<QDebug>
#include<QVariant>
#include <QAbstractButton>
#include<QRadioButton>
#include<QString>
#include "QtCommon.h"
extern "C" {
#include "putty.h"
}

GuiSettingsWindow::GuiSettingsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GuiSettingsWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    ui->treeWidget->topLevelItem(0)->setData(0, Qt::UserRole, SESSION);
    ui->treeWidget->topLevelItem(0)->child(0)->setData(0, Qt::UserRole, LOGGING);
    ui->treeWidget->topLevelItem(1)->setData(0, Qt::UserRole, TERMINAL);
    ui->treeWidget->topLevelItem(1)->child(0)->setData(0, Qt::UserRole, KEYBOARD);
    ui->treeWidget->topLevelItem(1)->child(1)->setData(0, Qt::UserRole, BELL);
    ui->treeWidget->topLevelItem(1)->child(2)->setData(0, Qt::UserRole, FEATURES);
    ui->treeWidget->topLevelItem(2)->setData(0, Qt::UserRole, WINDOW);
    ui->treeWidget->topLevelItem(2)->child(0)->setData(0, Qt::UserRole, APPEARANCE);
    ui->treeWidget->topLevelItem(2)->child(1)->setData(0, Qt::UserRole, BEHAVIOUR);
    ui->treeWidget->topLevelItem(2)->child(2)->setData(0, Qt::UserRole, TRANSLATION);
    ui->treeWidget->topLevelItem(2)->child(3)->setData(0, Qt::UserRole, SELECTION);
    ui->treeWidget->topLevelItem(2)->child(4)->setData(0, Qt::UserRole, COLOURS);
    ui->treeWidget->topLevelItem(3)->setData(0, Qt::UserRole, CONNECTION);
    ui->treeWidget->topLevelItem(3)->child(0)->setData(0, Qt::UserRole, DATA);
    ui->treeWidget->topLevelItem(3)->child(1)->setData(0, Qt::UserRole, PROXY);
    ui->treeWidget->topLevelItem(3)->child(2)->setData(0, Qt::UserRole, TELNET);
    ui->treeWidget->topLevelItem(3)->child(3)->setData(0, Qt::UserRole, RLOGIN);
    ui->treeWidget->topLevelItem(3)->child(4)->setData(0, Qt::UserRole, SSH);
    ui->treeWidget->topLevelItem(3)->child(4)->child(0)->setData(0, Qt::UserRole, KEX);
    ui->treeWidget->topLevelItem(3)->child(4)->child(1)->setData(0, Qt::UserRole, AUTH);
    ui->treeWidget->topLevelItem(3)->child(4)->child(1)->child(0)->setData(0, Qt::UserRole, GSSAPI);
    ui->treeWidget->topLevelItem(3)->child(4)->child(2)->setData(0, Qt::UserRole, TTY);
    ui->treeWidget->topLevelItem(3)->child(4)->child(3)->setData(0, Qt::UserRole, X11);
    ui->treeWidget->topLevelItem(3)->child(4)->child(4)->setData(0, Qt::UserRole, TUNNELS);
    ui->treeWidget->topLevelItem(3)->child(4)->child(5)->setData(0, Qt::UserRole, BUGS);
    ui->treeWidget->topLevelItem(3)->child(5)->setData(0, Qt::UserRole, SERIAL);

    ui->le_hostname->setText("192.168.1.103");
}

GuiSettingsWindow::~GuiSettingsWindow()
{
    delete ui;
}

void GuiSettingsWindow::on_rb_contype_telnet_clicked()
{
    ui->le_port->setText("23");
}

void GuiSettingsWindow::on_rb_contype_ssh_clicked()
{
   ui->le_port->setText("22");
}

void GuiSettingsWindow::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    ui->stackedWidget->setCurrentIndex(item->data(column,  Qt::UserRole).toInt());
}

void qstring_to_char(char *dst, QString src, int dstlen)
{
    QByteArray name = src.toUtf8();
    strncpy(dst, name.constData(), dstlen);
}

int initConfigDefaults(Config *cfg);
void GuiSettingsWindow::on_buttonBox_accepted()
{
    int rc;
    GuiTerminalWindow *newWnd = mainWindow->newTerminal();
    Config *c=&newWnd->cfg;
    initConfigDefaults(c);

    qstring_to_char(c->host, ui->le_hostname->text(), sizeof(c->host));
    c->port = ui->le_port->text().toInt();
    c->protocol = ui->gp_contype->checkedButton()==ui->rb_contype_ssh ? PROT_SSH :
                                                                        PROT_TELNET ;

    /* Options controlling session logging */

    c->logtype = ui->gp_seslog->checkedButton()==ui->rb_sessionlog_none? LGTYP_NONE :
                          ui->gp_seslog->checkedButton()==ui->rb_sessionlog_printout? LGTYP_ASCII :
                          ui->gp_seslog->checkedButton()==ui->rb_sessionlog_alloutput? LGTYP_DEBUG :
                          ui->gp_seslog->checkedButton()==ui->rb_sessionlog_sshpacket? LGTYP_PACKETS :
                                                                                       LGTYP_SSHRAW ;

    qstring_to_char(c->logfilename.path, ui->le_sessionlog_filename->text(), sizeof(c->logfilename.path));


    c->logxfovr = ui->gp_logfile->checkedButton()==ui->rb_sessionlog_overwrite? LGXF_OVR :
                  ui->gp_logfile->checkedButton()==ui->rb_sessionlog_append? LGXF_APN :
                                                                             LGXF_ASK ;

    c->logflush = ui->chb_sessionlog_flush->isChecked() ? 1 : 0 ;
    c->logomitpass = ui->chb_sessionlog_omitpasswd->isChecked() ? 1 : 0 ;
    c->logomitdata = ui->chb_sessionlog_omitdata->isChecked() ? 1 : 0 ;

    /* Options controlling the terminal emulation */

    c->wrap_mode = ui->chb_terminaloption_autowrap->isChecked() ? 1 : 0;
    c->dec_om = ui->chb_terminaloption_decorigin->isChecked() ? 1 : 0;
    c->lfhascr = ui->chb_terminaloption_lf->isChecked() ? 1 : 0;
    c->bce = ui->chb_terminaloption_bgcolor->isChecked() ? 1 : 0;
    c->blinktext = ui->chb_terminaloption_blinktext->isChecked() ? 1 : 0;

    qstring_to_char(c->answerback, ui->le_termopt_ansback->text(), sizeof(c->answerback));

    c->localecho = ui->gp_termopt_echo->checkedButton()==ui->rb_termopt_echoauto ? AUTO :
                   ui->gp_termopt_echo->checkedButton()==ui->rb_termopt_echoon ? FORCE_ON :
                                                                                 FORCE_OFF ;

    c->localedit = ui->gp_termopt_edit->checkedButton()==ui->rb_termopt_editauto ? AUTO :
                   ui->gp_termopt_edit->checkedButton()==ui->rb_termopt_editon ? FORCE_ON :
                                                                                 FORCE_OFF ;

    if (rc=newWnd->initTerminal()) {
        delete newWnd;
    } else {    // success
        mainWindow->tabArea->setCurrentWidget(newWnd);
    }

    this->close();
}

void GuiSettingsWindow::on_buttonBox_rejected()
{
    this->close();
}
