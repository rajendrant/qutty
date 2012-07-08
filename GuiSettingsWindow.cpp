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
#include "QtConfig.h"
extern "C" {
#include "putty.h"
}

QtConfig qutty_config;

GuiSettingsWindow::GuiSettingsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GuiSettingsWindow)
{
    memset(&this->cfg, 0, sizeof(Config));

    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    ui->treeWidget->topLevelItem(0)->setData(0, Qt::UserRole, GUI_PAGE_SESSION);
    ui->treeWidget->topLevelItem(0)->child(0)->setData(0, Qt::UserRole, GUI_PAGE_LOGGING);
    ui->treeWidget->topLevelItem(1)->setData(0, Qt::UserRole, GUI_PAGE_TERMINAL);
    ui->treeWidget->topLevelItem(1)->child(0)->setData(0, Qt::UserRole, GUI_PAGE_KEYBOARD);
    ui->treeWidget->topLevelItem(1)->child(1)->setData(0, Qt::UserRole, GUI_PAGE_BELL);
    ui->treeWidget->topLevelItem(1)->child(2)->setData(0, Qt::UserRole, GUI_PAGE_FEATURES);
    ui->treeWidget->topLevelItem(2)->setData(0, Qt::UserRole, GUI_PAGE_WINDOW);
    ui->treeWidget->topLevelItem(2)->child(0)->setData(0, Qt::UserRole, GUI_PAGE_APPEARANCE);
    ui->treeWidget->topLevelItem(2)->child(1)->setData(0, Qt::UserRole, GUI_PAGE_BEHAVIOUR);
    ui->treeWidget->topLevelItem(2)->child(2)->setData(0, Qt::UserRole, GUI_PAGE_TRANSLATION);
    ui->treeWidget->topLevelItem(2)->child(3)->setData(0, Qt::UserRole, GUI_PAGE_SELECTION);
    ui->treeWidget->topLevelItem(2)->child(4)->setData(0, Qt::UserRole, GUI_PAGE_COLOURS);
    ui->treeWidget->topLevelItem(3)->setData(0, Qt::UserRole, GUI_PAGE_CONNECTION);
    ui->treeWidget->topLevelItem(3)->child(0)->setData(0, Qt::UserRole, GUI_PAGE_DATA);
    ui->treeWidget->topLevelItem(3)->child(1)->setData(0, Qt::UserRole, GUI_PAGE_PROXY);
    ui->treeWidget->topLevelItem(3)->child(2)->setData(0, Qt::UserRole, GUI_PAGE_TELNET);
    ui->treeWidget->topLevelItem(3)->child(3)->setData(0, Qt::UserRole, GUI_PAGE_RLOGIN);
    ui->treeWidget->topLevelItem(3)->child(4)->setData(0, Qt::UserRole, GUI_PAGE_SSH);
    ui->treeWidget->topLevelItem(3)->child(4)->child(0)->setData(0, Qt::UserRole, GUI_PAGE_KEX);
    ui->treeWidget->topLevelItem(3)->child(4)->child(1)->setData(0, Qt::UserRole, GUI_PAGE_AUTH);
    ui->treeWidget->topLevelItem(3)->child(4)->child(1)->child(0)->setData(0, Qt::UserRole, GUI_PAGE_GSSAPI);
    ui->treeWidget->topLevelItem(3)->child(4)->child(2)->setData(0, Qt::UserRole, GUI_PAGE_TTY);
    ui->treeWidget->topLevelItem(3)->child(4)->child(3)->setData(0, Qt::UserRole, GUI_PAGE_X11);
    ui->treeWidget->topLevelItem(3)->child(4)->child(4)->setData(0, Qt::UserRole, GUI_PAGE_TUNNELS);
    ui->treeWidget->topLevelItem(3)->child(4)->child(5)->setData(0, Qt::UserRole, GUI_PAGE_BUGS);
    ui->treeWidget->topLevelItem(3)->child(5)->setData(0, Qt::UserRole, GUI_PAGE_SERIAL);

    /* Options controlling session logging */

    ui->gp_seslog->setId(ui->rb_sessionlog_none, LGTYP_NONE);
    ui->gp_seslog->setId(ui->rb_sessionlog_printout, LGTYP_ASCII);
    ui->gp_seslog->setId(ui->rb_sessionlog_alloutput, LGTYP_DEBUG);
    ui->gp_seslog->setId(ui->rb_sessionlog_sshpacket, LGTYP_PACKETS);
    ui->gp_seslog->setId(ui->rb_sessionlog_rawdata, LGTYP_SSHRAW);

    ui->gp_logfile->setId(ui->rb_sessionlog_overwrite, LGXF_OVR);
    ui->gp_logfile->setId(ui->rb_sessionlog_append, LGXF_APN);
    ui->gp_logfile->setId(ui->rb_sessionlog_askuser, LGXF_ASK);

    ui->gp_termopt_echo->setId(ui->rb_termopt_echoauto, AUTO);
    ui->gp_termopt_echo->setId(ui->rb_termopt_echoon, FORCE_ON);
    ui->gp_termopt_echo->setId(ui->rb_termopt_echooff, FORCE_OFF);

    ui->gp_termopt_edit->setId(ui->rb_termopt_editauto,  AUTO);
    ui->gp_termopt_edit->setId(ui->rb_termopt_editon, FORCE_ON);
    ui->gp_termopt_edit->setId(ui->rb_termopt_editoff, FORCE_OFF);

    this->getConfig();
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

void GuiSettingsWindow::on_buttonBox_accepted()
{
    int rc;
    GuiTerminalWindow *newWnd = mainWindow->newTerminal();
    newWnd->cfg = *this->getConfig();

    if ((rc=newWnd->initTerminal())) {
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


void GuiSettingsWindow::setConfig(Config *_cfg)
{
    this->cfg = *_cfg;

    // update the ui with the given settings
    ui->le_hostname->setText(cfg.host);
    ui->le_port->setText(QString::number(cfg.port));
    (cfg.protocol==PROT_SSH ? ui->rb_contype_ssh : ui->rb_contype_telnet)->click();
    ui->le_saved_sess->setText(cfg.config_name);
    QList<QListWidgetItem*> sel_saved_sess = ui->l_saved_sess->findItems(cfg.config_name, Qt::MatchExactly);
    if (sel_saved_sess.size() > 0)
        ui->l_saved_sess->setCurrentItem(sel_saved_sess[0]);

    /* Options controlling session logging */
    ui->gp_seslog->button(cfg.logtype)->click();
    ui->le_sessionlog_filename->setText(cfg.logfilename.path);
    ui->gp_logfile->button(cfg.logxfovr)->click();
    ui->chb_sessionlog_flush->setChecked(cfg.logflush);
    ui->chb_sessionlog_omitpasswd->setChecked(cfg.logomitpass);
    ui->chb_sessionlog_omitdata->setChecked(cfg.logomitdata);

    /* Options controlling the terminal emulation */
    ui->chb_terminaloption_autowrap->setChecked(cfg.wrap_mode);
    ui->chb_terminaloption_decorigin->setChecked(cfg.dec_om);
    ui->chb_terminaloption_lf->setChecked(cfg.lfhascr);
    ui->chb_terminaloption_bgcolor->setChecked(cfg.bce);
    ui->chb_terminaloption_blinktext->setChecked(cfg.blinktext);
    ui->le_termopt_ansback->setText(cfg.answerback);
    ui->gp_termopt_echo->button(cfg.localecho)->click();
    ui->gp_termopt_edit->button(cfg.localedit)->click();
}

Config *GuiSettingsWindow::getConfig()
{
    Config *cfg = &this->cfg;

    // update the config with current ui selection and return it
    qstring_to_char(cfg->host, ui->le_hostname->text(), sizeof(cfg->host));
    cfg->port = ui->le_port->text().toInt();
    cfg->protocol = ui->gp_contype->checkedButton()==ui->rb_contype_ssh ? PROT_SSH :
                                                                        PROT_TELNET ;
    if(ui->le_saved_sess->text() != "")
        qstring_to_char(cfg->config_name, ui->le_saved_sess->text(), sizeof(cfg->config_name));
    else if (ui->l_saved_sess->currentItem())
        qstring_to_char(cfg->config_name, ui->l_saved_sess->currentItem()->text(), sizeof(cfg->config_name));

    /* Options controlling session logging */
    cfg->logtype = ui->gp_seslog->checkedId();
    qstring_to_char(cfg->logfilename.path, ui->le_sessionlog_filename->text(), sizeof(cfg->logfilename.path));
    cfg->logxfovr = ui->gp_logfile->checkedId();
    cfg->logflush = ui->chb_sessionlog_flush->isChecked() ? 1 : 0 ;
    cfg->logomitpass = ui->chb_sessionlog_omitpasswd->isChecked() ? 1 : 0 ;
    cfg->logomitdata = ui->chb_sessionlog_omitdata->isChecked() ? 1 : 0 ;

    /* Options controlling the terminal emulation */
    cfg->wrap_mode = ui->chb_terminaloption_autowrap->isChecked() ? 1 : 0;
    cfg->dec_om = ui->chb_terminaloption_decorigin->isChecked() ? 1 : 0;
    cfg->lfhascr = ui->chb_terminaloption_lf->isChecked() ? 1 : 0;
    cfg->bce = ui->chb_terminaloption_bgcolor->isChecked() ? 1 : 0;
    cfg->blinktext = ui->chb_terminaloption_blinktext->isChecked() ? 1 : 0;
    qstring_to_char(cfg->answerback, ui->le_termopt_ansback->text(), sizeof(cfg->answerback));
    cfg->localecho = ui->gp_termopt_echo->checkedId();
    cfg->localedit = ui->gp_termopt_edit->checkedId();

    return cfg;
}

void GuiSettingsWindow::loadSessionNames()
{
    ui->l_saved_sess->clear();
    for(std::map<string, Config>::iterator it = qutty_config.config_list.begin();
        it != qutty_config.config_list.end(); it++) {
        ui->l_saved_sess->addItem(new QListWidgetItem(it->first.c_str()));
    }
}

void GuiSettingsWindow::on_b_load_sess_clicked()
{
    char config_name[100];
    if (!ui->l_saved_sess->currentItem())
        return;
    qstring_to_char(config_name, ui->l_saved_sess->currentItem()->text(), sizeof(config_name));
    if (qutty_config.config_list.find(config_name) == qutty_config.config_list.end())
        return;
    setConfig(&qutty_config.config_list[config_name]);
}

void GuiSettingsWindow::on_b_save_sess_clicked()
{
    Config *cfg = this->getConfig();
    qutty_config.config_list[cfg->config_name] = *cfg;
    qutty_config.saveConfig();
    qutty_config.restoreConfig();
    loadSessionNames();
}

void GuiSettingsWindow::loadDefaultSettings()
{
    if (qutty_config.config_list.find(QUTTY_DEFAULT_CONFIG_SETTINGS)
            != qutty_config.config_list.end()) {
        setConfig(&qutty_config.config_list[QUTTY_DEFAULT_CONFIG_SETTINGS]);
        ui->le_saved_sess->setText(QUTTY_DEFAULT_CONFIG_SETTINGS);
    }
}

void GuiSettingsWindow::on_b_delete_sess_clicked()
{
    char config_name[100];
    if (!ui->l_saved_sess->currentItem())
        return;
    qstring_to_char(config_name, ui->l_saved_sess->currentItem()->text(), sizeof(config_name));
    if (ui->l_saved_sess->currentItem()->text() == QUTTY_DEFAULT_CONFIG_SETTINGS)
        return;
    if (qutty_config.config_list.find(config_name) == qutty_config.config_list.end())
        return;
    qutty_config.config_list.erase(config_name);
    qutty_config.saveConfig();
    qutty_config.restoreConfig();
    loadSessionNames();
}
