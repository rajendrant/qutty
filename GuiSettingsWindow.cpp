/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include "GuiSettingsWindow.h"
#include "GuiMainWindow.h"
#include "GuiTerminalWindow.h"
#include "ui_GuiSettingsWindow.h"
#include <QDebug>
#include <QVariant>
#include <QAbstractButton>
#include <QRadioButton>
#include <QString>
#include <QMessageBox>
#include <QFontDialog>
#include <QColorDialog>
#include "QtCommon.h"
#include "QtConfig.h"
extern "C" {
#include "putty.h"
}

QtConfig qutty_config;

static char sessname_split = '/';

static int QUTTY_ROLE_FULL_SESSNAME = Qt::UserRole + 3;

void adjust_sessname_hierarchy(QTreeWidgetItem *item);

vector<string> qutty_string_split(string &str, char delim)
{
    stringstream ss(str);
    string tmp;
    vector<string> ret;
    while (std::getline(ss, tmp, delim)) {
        ret.push_back(tmp);
    }
    return ret;
}

GuiSettingsWindow::GuiSettingsWindow(QWidget *parent, GuiBase::SplitType openmode) :
    QDialog(parent),
    openMode(openmode),
    ui(new Ui::GuiSettingsWindow),
    pending_session_changes(false)
{
    memset(&this->cfg, 0, sizeof(Config));
    isChangeSettingsMode = false;

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

    // expand all 1st level items
    ui->treeWidget->expandToDepth(0);

    ui->rb_contype_raw->setVisible(false);
    ui->rb_contype_rlogin->setVisible(false);
    ui->rb_contype_serial->setVisible(false);

    ui->gp_exit_close->setId(ui->rb_exit_always, FORCE_ON);
    ui->gp_exit_close->setId(ui->rb_exit_never, FORCE_OFF);
    ui->gp_exit_close->setId(ui->rb_exit_clean, AUTO);

    /* Options controlling session logging */

    ui->gp_seslog->setId(ui->rb_sessionlog_none, LGTYP_NONE);
    ui->gp_seslog->setId(ui->rb_sessionlog_printout, LGTYP_ASCII);
    ui->gp_seslog->setId(ui->rb_sessionlog_alloutput, LGTYP_DEBUG);
    ui->gp_seslog->setId(ui->rb_sessionlog_sshpacket, LGTYP_PACKETS);
    ui->gp_seslog->setId(ui->rb_sessionlog_rawdata, LGTYP_SSHRAW);

    ui->gp_logfile->setId(ui->rb_sessionlog_overwrite, LGXF_OVR);
    ui->gp_logfile->setId(ui->rb_sessionlog_append, LGXF_APN);
    ui->gp_logfile->setId(ui->rb_sessionlog_askuser, LGXF_ASK__);

    ui->gp_termopt_echo->setId(ui->rb_termopt_echoauto, AUTO);
    ui->gp_termopt_echo->setId(ui->rb_termopt_echoon, FORCE_ON);
    ui->gp_termopt_echo->setId(ui->rb_termopt_echooff, FORCE_OFF);

    ui->gp_termopt_edit->setId(ui->rb_termopt_editauto,  AUTO);
    ui->gp_termopt_edit->setId(ui->rb_termopt_editon, FORCE_ON);
    ui->gp_termopt_edit->setId(ui->rb_termopt_editoff, FORCE_OFF);

    ui->gp_fnkeys->setId(ui->rb_fnkeys_esc, FUNKY_TILDE);
    ui->gp_fnkeys->setId(ui->rb_fnkeys_linux, FUNKY_LINUX);
    ui->gp_fnkeys->setId(ui->rb_fnkeys_xtermr6, FUNKY_XTERM);
    ui->gp_fnkeys->setId(ui->rb_fnkeys_vt400, FUNKY_VT400);
    ui->gp_fnkeys->setId(ui->rb_fnkeys_vt100, FUNKY_VT100P);
    ui->gp_fnkeys->setId(ui->rb_fnkeys_sco, FUNKY_SCO);

    ui->gp_remote_qtitle_action->setId(ui->rb_featqtitle_none, TITLE_NONE);
    ui->gp_remote_qtitle_action->setId(ui->rb_featqtitle_empstring, TITLE_EMPTY);
    ui->gp_remote_qtitle_action->setId(ui->rb_featqtitle_wndtitle, TITLE_REAL);

    ui->gp_resize_action->setId(ui->rb_wndresz_rowcolno, RESIZE_TERM);
    ui->gp_resize_action->setId(ui->rb_wndresz_fontsize, RESIZE_FONT);
    ui->gp_resize_action->setId(ui->rb_wndresz_onlywhenmax, RESIZE_EITHER);
    ui->gp_resize_action->setId(ui->rb_wndresz_forbid, RESIZE_DISABLED);

    ui->gp_addressfamily->setId(ui->rb_connectprotocol_auto, ADDRTYPE_UNSPEC);
    ui->gp_addressfamily->setId(ui->rb_connectprotocol_ipv4, ADDRTYPE_IPV4);
    ui->gp_addressfamily->setId(ui->rb_connectprotocol_ipv6, ADDRTYPE_IPV6);

    ui->gp_curappear->setId(ui->rb_curappear_block, 0);
    ui->gp_curappear->setId(ui->rb_curappear_underline, 1);
    ui->gp_curappear->setId(ui->rb_curappear_vertline, 2);

    ui->gp_fontquality->setId(ui->rb_fontappear_antialiase, FQ_ANTIALIASED);
    ui->gp_fontquality->setId(ui->rb_fontappear_nonantialiase, FQ_NONANTIALIASED);
    ui->gp_fontquality->setId(ui->rb_fontappear_default, FQ_DEFAULT);
    ui->gp_fontquality->setId(ui->rb_fontappear_clear, FQ_CLEARTYPE);

    this->loadSessionNames();

    // resize to minimum needed dimension
    this->resize(0, 0);

    this->connect(ui->l_saved_sess, SIGNAL(sig_hierarchyChanged(QTreeWidgetItem*)),
                  SLOT(slot_sessname_hierarchy_changed(QTreeWidgetItem*)));
    this->connect(this, SIGNAL(rejected()), SLOT(slot_GuiSettingsWindow_rejected()));

    // set focus to hostname/ipaddress
    this->ui->le_hostname->setFocus();
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

void GuiSettingsWindow::on_buttonBox_accepted()
{
    if (isChangeSettingsMode) {
        emit signal_session_change(*getConfig(), termWnd);
        goto cu0;
    }

    saveConfigChanges();

    if (ui->le_hostname->text() == "" &&
        ui->l_saved_sess->currentItem()->text(0) == QUTTY_DEFAULT_CONFIG_SETTINGS) {
        return;
    } else if (ui->le_hostname->text() == "") {
        char config_name[100];
        qstring_to_char(config_name, ui->l_saved_sess->currentItem()->text(0), sizeof(config_name));
        if (qutty_config.config_list.find(config_name) == qutty_config.config_list.end())
            return;
        setConfig(&qutty_config.config_list[config_name]);
    }
    // check for NOT_YET_SUPPORTED configs
    chkUnsupportedConfigs(*getConfig());

    emit signal_session_open(*getConfig(), openMode);

cu0:
    this->close();
    this->deleteLater();
}

void GuiSettingsWindow::on_buttonBox_rejected()
{
    this->close();
}

void GuiSettingsWindow::slot_GuiSettingsWindow_rejected()
{
    saveConfigChanges();
    emit signal_session_close();
    this->close();
    this->deleteLater();
}

void GuiSettingsWindow::saveConfigChanges()
{
    if (!pending_session_changes)
        return;
    qutty_config.saveConfig();
}

void GuiSettingsWindow::setConfig(Config *_cfg)
{
    int ind;

    this->cfg = *_cfg;

    // update the ui with the given settings
    if (cfg.host[0] != '\0')
        ui->le_hostname->setText(cfg.host);

    (cfg.protocol==PROT_SSH ? ui->rb_contype_ssh : ui->rb_contype_telnet)->click();
    ui->le_port->setText(QString::number(cfg.port));

    auto cfg_name_split = qutty_string_split(string(cfg.config_name), sessname_split);
    ui->le_saved_sess->setText(QString::fromStdString(cfg_name_split.back()));

    QList<QTreeWidgetItem*> sel_saved_sess = ui->l_saved_sess->findItems(cfg.config_name, Qt::MatchExactly);
    if (sel_saved_sess.size() > 0)
        ui->l_saved_sess->setCurrentItem(sel_saved_sess[0]);

    ui->gp_exit_close->button(cfg.close_on_exit)->click();

    /* Options controlling session logging */
    ui->gp_seslog->button(cfg.logtype)->click();
    ui->le_sessionlog_filename->setText(cfg.logfilename.path);
    if (cfg.logxfovr == LGXF_ASK)   // handle -ve value
        ui->gp_logfile->button(LGXF_ASK__)->click();
    else
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

    /* keyboard options */
    (cfg.bksp_is_delete ? ui->rb_backspacekey_ctrlh : ui->rb_backspace_ctrl127)
            ->setChecked(true);
    (cfg.rxvt_homeend ? ui->rb_homeendkeys_rxvt : ui->rb_homeendkeys_std)
            ->setChecked(true);
    ui->gp_fnkeys->button(cfg.funky_type)->click();
    (cfg.app_cursor ? ui->rb_inicursorkeys_app : ui->rb_inicursorkeys_normal)
            ->setChecked(true);
    (cfg.nethack_keypad ? ui->rb_ininumerickeys_nethack :
        cfg.app_keypad ? ui->rb_ininumkeys_app :
                         ui->rb_ininumkeys_normal)
            ->setChecked(true);
    ui->chb_altgrkey->setChecked(cfg.compose_key);
    ui->chb_ctrl_alt->setChecked(cfg.ctrlaltkeys);

    /* terminal features */
    ui->chb_no_applic_c->setChecked(cfg.no_applic_c);
    ui->chb_no_applic_k->setChecked(cfg.no_applic_k);
    ui->chb_no_mouse_rep->setChecked(cfg.no_mouse_rep);
    ui->chb_no_remote_resize->setChecked(cfg.no_remote_resize);
    ui->chb_no_alt_screen->setChecked(cfg.no_alt_screen);
    ui->chb_no_remote_wintitle->setChecked(cfg.no_remote_wintitle);
    ui->chb_no_dbackspace->setChecked(cfg.no_dbackspace);
    ui->chb_no_remote_charset->setChecked(cfg.no_remote_charset);
    ui->chb_no_arabic->setChecked(cfg.arabicshaping);
    ui->chb_no_bidi->setChecked(cfg.bidi);
    ui->gp_remote_qtitle_action->button(cfg.remote_qtitle_action)->click();

    /* window options */
    ui->le_window_column->setText(QString::number(cfg.width));
    ui->le_window_row->setText(QString::number(cfg.height));
    ui->le_wndscroll_lines->setText(QString::number(cfg.savelines));
    ui->chb_wndscroll_display->setChecked(cfg.scrollbar);
    ui->chb_wndscroll_fullscreen->setChecked(cfg.scrollbar_in_fullscreen);
    ui->chb_wndscroll_resetdisply->setChecked(cfg.scroll_on_disp);
    ui->chb_wndscroll_resetkeypress->setChecked(cfg.scroll_on_key);
    ui->chb_wndscroll_pusherasedtext->setChecked(cfg.erase_to_scrollback);
    ui->gp_resize_action->button(cfg.resize_action)->click();
    ui->gp_curappear->button(cfg.cursor_type)->click();
    ui->chb_curblink->setChecked(cfg.blink_cur);
    ui->gp_fontquality->button(cfg.font_quality)->click();
    ui->lbl_fontsel->setText(QString("%1, %2%3-point")
                             .arg(cfg.font.name,
                                  cfg.font.isbold ? "Bold, " : "",
                                  QString::number(cfg.font.height)));
    ui->chb_behaviour_warn->setChecked(cfg.warn_on_close);
    ind = ui->cb_codepage->findText(cfg.line_codepage);
    if (ind == -1) ind = 0;
    ui->cb_codepage->setCurrentIndex(ind);

    /* connection options */
    ui->le_ping_interval->setText(QString::number(cfg.ping_interval));
    ui->chb_tcp_keepalive->setChecked(cfg.tcp_keepalives);
    ui->chb_tcp_nodelay->setChecked(cfg.tcp_nodelay);
    ui->gp_addressfamily->button(cfg.addressfamily)->click();
    ui->le_loghost->setText(cfg.loghost);

    /* connection data options */
    ui->le_datausername->setText(cfg.username);
    (cfg.username_from_env ? ui->rb_datausername_systemsuse : ui->rb_datausername_prompt)
            ->setChecked(true);
    ui->le_termtype->setText(cfg.termtype);
    ui->le_termspeed->setText(cfg.termspeed);

    /* ssh options */
    ui->le_remote_cmd->setText(cfg.remote_cmd);

    /* ssh auth options */
    ui->chb_ssh_no_userauth->setChecked(cfg.ssh_no_userauth);
    ui->chb_ssh_show_banner->setChecked(cfg.ssh_show_banner);
    ui->chb_ssh_tryagent->setChecked(cfg.tryagent);
    ui->chb_ssh_try_tis_auth->setChecked(cfg.try_tis_auth);
    ui->chb_ssh_try_ki_auth->setChecked(cfg.try_ki_auth);
    ui->chb_ssh_agentfwd->setChecked(cfg.agentfwd);
    ui->chb_ssh_change_username->setChecked(cfg.change_username);
    ui->le_ssh_auth_keyfile->setText(cfg.keyfile.path);
}

Config *GuiSettingsWindow::getConfig()
{
    Config *cfg = &this->cfg;

    // update the config with current ui selection and return it
    qstring_to_char(cfg->host, ui->le_hostname->text(), sizeof(cfg->host));
    cfg->port = ui->le_port->text().toInt();
    cfg->protocol = ui->gp_contype->checkedButton()==ui->rb_contype_ssh ? PROT_SSH :
                                                                        PROT_TELNET ;
    if (ui->l_saved_sess->currentItem())
        qstring_to_char(cfg->config_name,
                        ui->l_saved_sess->currentItem()
                            ->data(0, Qt::UserRole+3).toString(),
                        sizeof(cfg->config_name));

    cfg->close_on_exit = ui->gp_exit_close->checkedId();

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

    /* keyboard options */
    cfg->bksp_is_delete = ui->rb_backspacekey_ctrlh->isChecked() ? 1 : 0;
    cfg->rxvt_homeend = ui->rb_homeendkeys_rxvt->isChecked() ? 1 : 0;
    cfg->funky_type = ui->gp_fnkeys->checkedId();
    cfg->app_cursor = ui->rb_inicursorkeys_app->isChecked();
    cfg->nethack_keypad = ui->rb_ininumerickeys_nethack->isChecked();
    cfg->app_keypad = ui->rb_ininumkeys_app->isChecked();
    cfg->compose_key = ui->chb_altgrkey->isChecked();
    cfg->ctrlaltkeys = ui->chb_ctrl_alt->isChecked();

    /* terminal features */
    cfg->no_applic_c = ui->chb_no_applic_c->isChecked();
    cfg->no_applic_k = ui->chb_no_applic_k->isChecked();
    cfg->no_mouse_rep = ui->chb_no_mouse_rep->isChecked();
    cfg->no_remote_resize = ui->chb_no_remote_resize->isChecked();
    cfg->no_alt_screen = ui->chb_no_alt_screen->isChecked();
    cfg->no_remote_wintitle = ui->chb_no_remote_wintitle->isChecked();
    cfg->no_dbackspace = ui->chb_no_dbackspace->isChecked();
    cfg->no_remote_charset = ui->chb_no_remote_charset->isChecked();
    cfg->arabicshaping = ui->chb_no_arabic->isChecked();
    cfg->bidi = ui->chb_no_bidi->isChecked();
    cfg->remote_qtitle_action = ui->gp_remote_qtitle_action->checkedId();

    /* window options */
    cfg->width = ui->le_window_column->text().toInt();
    cfg->height = ui->le_window_row->text().toInt();
    cfg->savelines = ui->le_wndscroll_lines->text().toInt();
    cfg->scrollbar = ui->chb_wndscroll_display->isChecked();
    cfg->scrollbar_in_fullscreen = ui->chb_wndscroll_fullscreen->isChecked();
    cfg->scroll_on_disp = ui->chb_wndscroll_resetdisply->isChecked();
    cfg->scroll_on_key = ui->chb_wndscroll_resetkeypress->isChecked();
    cfg->erase_to_scrollback = ui->chb_wndscroll_pusherasedtext->isChecked();
    cfg->resize_action = ui->gp_resize_action->checkedId();
    cfg->cursor_type = ui->gp_curappear->checkedId();
    cfg->blink_cur = ui->chb_curblink->isChecked();
    cfg->font_quality = ui->gp_fontquality->checkedId();
    cfg->warn_on_close = ui->chb_behaviour_warn->isChecked();
    qstring_to_char(cfg->line_codepage, ui->cb_codepage->currentText(), sizeof(cfg->line_codepage));

    /* connection options */
    cfg->ping_interval = ui->le_ping_interval->text().toInt();
    cfg->tcp_keepalives = ui->chb_tcp_keepalive->isChecked();
    cfg->tcp_nodelay = ui->chb_tcp_nodelay->isChecked();
    cfg->addressfamily = ui->gp_addressfamily->checkedId();
    qstring_to_char(cfg->loghost, ui->le_loghost->text(), sizeof(cfg->loghost));

    /* connection data options */
    qstring_to_char(cfg->username, ui->le_datausername->text(), sizeof(cfg->username));
    cfg->username_from_env = ui->rb_datausername_systemsuse->isChecked();
    qstring_to_char(cfg->termtype, ui->le_termtype->text(), sizeof(cfg->termtype));
    qstring_to_char(cfg->termspeed, ui->le_termspeed->text(), sizeof(cfg->termspeed));

    /* ssh options */
    qstring_to_char(cfg->remote_cmd, ui->le_remote_cmd->text(), sizeof(cfg->remote_cmd));

    /* ssh auth options */
    cfg->ssh_no_userauth = ui->chb_ssh_no_userauth->isChecked();
    cfg->ssh_show_banner = ui->chb_ssh_show_banner->isChecked();
    cfg->tryagent = ui->chb_ssh_tryagent->isChecked();
    cfg->try_tis_auth = ui->chb_ssh_try_tis_auth->isChecked();
    cfg->try_ki_auth = ui->chb_ssh_try_ki_auth->isChecked();
    cfg->agentfwd = ui->chb_ssh_agentfwd->isChecked();
    cfg->change_username = ui->chb_ssh_change_username->isChecked();
    qstring_to_char(cfg->keyfile.path, ui->le_ssh_auth_keyfile->text(),
                    sizeof(cfg->keyfile.path));

    return cfg;
}

void GuiSettingsWindow::loadSessionNames()
{
    map<string, QTreeWidgetItem*> folders;
    folders[""] = ui->l_saved_sess->invisibleRootItem();
    ui->l_saved_sess->clear();
    for(std::map<string, Config>::iterator it = qutty_config.config_list.begin();
        it != qutty_config.config_list.end(); it++) {
        string fullsessname = it->first;
        if (folders.find(fullsessname) != folders.end())
            continue;
        if (fullsessname.back() == sessname_split)
            fullsessname.pop_back();
        vector<string> split = qutty_string_split(fullsessname, sessname_split);
        string sessname = split.back();
        string dirname = fullsessname.substr(0, fullsessname.find_last_of(sessname_split));
        if (dirname == fullsessname)
            dirname = "";
        if (folders.find(dirname) == folders.end()) {
            QTreeWidgetItem *par = ui->l_saved_sess->invisibleRootItem();
            string tmpdir = "";
            for (int i=0; i<split.size()-1; i++) {
                tmpdir += split[i];
                if (folders.find(tmpdir) == folders.end()) {
                    QTreeWidgetItem *newitem = new QTreeWidgetItem(par);
                    newitem->setText(0, QString::fromStdString(split[i]));
                    newitem->setData(0, Qt::UserRole+3, QString::fromStdString(tmpdir));
                    newitem->setData(0, GuiTreeWidget::DragEnabledProperty, true);
                    newitem->setData(0, GuiTreeWidget::DropEnabledProperty, true);
                    folders[tmpdir] = newitem;
                    par = newitem;
                } else
                    par = folders[tmpdir];
                tmpdir += sessname_split;
            }
        }
        QTreeWidgetItem *item = new QTreeWidgetItem(folders[dirname]);
        item->setText(0, QString::fromStdString(sessname));
        item->setData(0, Qt::UserRole+3, QString::fromStdString(fullsessname));
        item->setData(0, GuiTreeWidget::DragEnabledProperty, true);
        item->setData(0, GuiTreeWidget::DropEnabledProperty, true);
        if (fullsessname == QUTTY_DEFAULT_CONFIG_SETTINGS) {
            item->setData(0, GuiTreeWidget::DragEnabledProperty, false);
            item->setData(0, GuiTreeWidget::DropEnabledProperty, false);
        }
        folders[fullsessname] = item;
    }
}

void GuiSettingsWindow::on_l_saved_sess_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    if (isChangeSettingsMode)
        return;
    if (!current)
        return;
    string config_name;
    config_name = current->data(0, QUTTY_ROLE_FULL_SESSNAME).toString().toStdString();
    if (qutty_config.config_list.find(config_name) == qutty_config.config_list.end())
        return;
    setConfig(&qutty_config.config_list[config_name]);
    ui->le_saved_sess->setText(current->text(0));
}

void GuiSettingsWindow::on_b_sess_newfolder_clicked()
{
    QTreeWidgetItem *item = ui->l_saved_sess->currentItem();
    if (!item)
        return;
    QString fullname;
    QTreeWidgetItem *parent = item->parent();
    if (!parent)
        parent = ui->l_saved_sess->invisibleRootItem();
    else
        fullname = parent->data(0, QUTTY_ROLE_FULL_SESSNAME).toString() + sessname_split;
    QTreeWidgetItem *newitem = new QTreeWidgetItem(0);
    QString foldername = tr("New Session");
    for (int i=1; ; i++) {
        bool isunique = true;
        for (int j=0; j<parent->childCount(); j++) {
            if (parent->child(j)->text(0) == foldername) {
                isunique = false;
                foldername = tr("New Session (") + QString::number(i) + tr(")");
                break;
            }
        }
        if (isunique)
            break;
    }
    fullname += foldername;
    newitem->setText(0, foldername);
    newitem->setData(0, QUTTY_ROLE_FULL_SESSNAME, fullname);
    newitem->setData(0, GuiTreeWidget::DragEnabledProperty, true);
    newitem->setData(0, GuiTreeWidget::DropEnabledProperty, true);
    parent->insertChild(parent->indexOfChild(item)+1, newitem);
    Config cfg = qutty_config.config_list[QUTTY_DEFAULT_CONFIG_SETTINGS];
    qstring_to_char(cfg.config_name, fullname, sizeof(cfg.config_name));
    qutty_config.config_list[fullname.toStdString()] = cfg;

    pending_session_changes = true;
}

void GuiSettingsWindow::on_b_save_sess_clicked()
{
    QTreeWidgetItem *item = ui->l_saved_sess->currentItem();
    string oldfullname, fullname, oldname, name;
    if (!item || item->text(0) == QUTTY_DEFAULT_CONFIG_SETTINGS)
        return;
    name = ui->le_saved_sess->text().toStdString();
    if (!name.length())
        return;
    oldname = item->text(0).toStdString();
    oldfullname = item->data(0, QUTTY_ROLE_FULL_SESSNAME).toString().toStdString();
    if (item->parent())
        fullname = item->parent()->data(0, QUTTY_ROLE_FULL_SESSNAME)
                .toString().toStdString() + sessname_split;
    fullname += name;
    qutty_config.config_list.erase(oldfullname);
    Config *cfg = this->getConfig();
    qutty_config.config_list[fullname] = *cfg;

    item->setText(0, QString::fromStdString(name));
    item->setData(0, QUTTY_ROLE_FULL_SESSNAME, QString::fromStdString(fullname));

    for(int i=0; i<item->childCount(); i++)
        adjust_sessname_hierarchy(item->child(i));

    pending_session_changes = true;
}

void GuiSettingsWindow::loadDefaultSettings()
{
    if (qutty_config.config_list.find(QUTTY_DEFAULT_CONFIG_SETTINGS)
            != qutty_config.config_list.end()) {
        setConfig(&qutty_config.config_list[QUTTY_DEFAULT_CONFIG_SETTINGS]);
        ui->le_saved_sess->setText(QUTTY_DEFAULT_CONFIG_SETTINGS);
    }
}

void GuiSettingsWindow::enableModeChangeSettings(Config *cfg, GuiTerminalWindow *termWnd)
{
    isChangeSettingsMode = true;
    this->termWnd = termWnd;
    setConfig(cfg);

    ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    ui->gp_connection->setEnabled(false);
    ui->b_delete_sess->setEnabled(false);
    ui->b_sess_newfolder->setEnabled(false);
    ui->l_saved_sess->setEnabled(false);
}

void GuiSettingsWindow::on_b_delete_sess_clicked()
{
    QTreeWidgetItem *delitem = ui->l_saved_sess->currentItem();
    string config_name;
    if (!delitem || delitem->text(0) == QUTTY_DEFAULT_CONFIG_SETTINGS)
        return;
    config_name = delitem->data(0, QUTTY_ROLE_FULL_SESSNAME).toString().toStdString();
    qutty_config.config_list.erase(config_name);
    delete delitem;

    pending_session_changes = true;
}

void GuiSettingsWindow::on_l_saved_sess_doubleClicked(const QModelIndex &)
{
    if (isChangeSettingsMode)
        return;

    if (!ui->le_hostname->text().length())
        return;
    QTreeWidgetItem *item = ui->l_saved_sess->currentItem();
    if (!item)
        return;
    if (item->childCount()) // allow double-click to expand
        return;
    on_buttonBox_accepted();
}

void adjust_sessname_hierarchy(QTreeWidgetItem *item)
{
    if (!item)
        return;
    QTreeWidgetItem *par = item->parent();
    QString fullname, oldfullname;
    if (par)
        fullname = par->data(0, QUTTY_ROLE_FULL_SESSNAME).toString() + sessname_split;
    fullname += item->text(0);
    oldfullname = item->data(0, QUTTY_ROLE_FULL_SESSNAME).toString();
    if (fullname == oldfullname)
        return; // no change
    item->setData(0, QUTTY_ROLE_FULL_SESSNAME, fullname);
    Config cfg = qutty_config.config_list[oldfullname.toStdString()];
    qstring_to_char(cfg.config_name, fullname, sizeof(cfg.config_name));
    qutty_config.config_list.erase(oldfullname.toStdString());
    qutty_config.config_list[fullname.toStdString()] = cfg;
    for(int i=0; i<item->childCount(); i++)
        adjust_sessname_hierarchy(item->child(i));
}

void GuiSettingsWindow::slot_sessname_hierarchy_changed(QTreeWidgetItem *item)
{
    // current item & its child have moved to different place
    adjust_sessname_hierarchy(item);
    pending_session_changes = true;
}

void GuiSettingsWindow::on_btn_ssh_auth_browse_keyfile_clicked()
{
    ui->le_ssh_auth_keyfile->setText(QFileDialog::getOpenFileName(
                                         this, tr("Select private key file"),
                                         ui->le_ssh_auth_keyfile->text(), tr("*.ppk")));
}

void GuiSettingsWindow::chkUnsupportedConfigs(Config &cfg)
{
    QString opt_unsupp = "";

    if (cfg.try_gssapi_auth) {
        cfg.try_gssapi_auth = 0;
    }
    if (cfg.portfwd[0] != '\0') {
        cfg.portfwd[0] = '\0';
        opt_unsupp += " * SSH Tunnels/port forwarding\n";
    }
    if (opt_unsupp.length() > 0)
        QMessageBox::warning(NULL, QObject::tr("Qutty Configuration"),
                         QObject::tr("Following options are not yet supported in QuTTY.\n\n%1")
                         .arg(opt_unsupp));
}

void GuiSettingsWindow::on_btn_fontsel_clicked()
{
    QFont oldfont = QFont(cfg.font.name, cfg.font.height);
    oldfont.setBold(cfg.font.isbold);
    QFont selfont = QFontDialog::getFont(NULL, oldfont);

    qstring_to_char(cfg.font.name, selfont.family(), sizeof(cfg.font.name));
    cfg.font.height = selfont.pointSize();
    cfg.font.isbold = selfont.bold();
    ui->lbl_fontsel->setText(QString("%1, %2%3-point")
                             .arg(cfg.font.name,
                                  cfg.font.isbold ? "Bold, " : "",
                                  QString::number(cfg.font.height)));
    ui->lbl_fontsel_varpitch->setText(
                selfont.fixedPitch() ?
                    "The selected font has variable-pitch. Doesn't have fixed-pitch" : "");
}

void GuiSettingsWindow::on_treeWidget_itemSelectionChanged()
{
    ui->stackedWidget->setCurrentIndex(ui->treeWidget->selectedItems()[0]->data(0,  Qt::UserRole).toInt());
}

void GuiSettingsWindow::on_btn_about_clicked()
{
    QMessageBox::about(this,
                       "About " APPNAME,
                       APPNAME "\nRelease " QUTTY_RELEASE_VERSION "\n\nhttp://code.google.com/p/qutty/"
                       );
}

void GuiSettingsWindow::on_btn_colour_modify_clicked()
{
    QColor oldcol = QColor(ui->le_colour_r->text().toInt(),
                           ui->le_colour_g->text().toInt(),
                           ui->le_colour_b->text().toInt());
    QColor newcol = QColorDialog::getColor(oldcol);
    if (newcol.isValid()) {
        ui->le_colour_r->setText(QString::number(newcol.red()));
        ui->le_colour_g->setText(QString::number(newcol.green()));
        ui->le_colour_b->setText(QString::number(newcol.blue()));
        int currind = ui->l_colour->currentIndex().row();
        if (currind >= 0 && currind < NCFGCOLOURS) {
            cfg.colours[currind][0] = newcol.red();
            cfg.colours[currind][1] = newcol.green();
            cfg.colours[currind][2] = newcol.blue();
        }
    }
}

void GuiSettingsWindow::on_l_colour_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    int prev = -1, curr = -1;
    if (previous) {
        prev = ui->l_colour->row(previous);
    }
    if (current) {
        curr = ui->l_colour->row(current);
    }
    qDebug() << prev << curr;
    if (prev >= 0 && prev < NCFGCOLOURS) {
        cfg.colours[prev][0] = ui->le_colour_r->text().toInt();
        cfg.colours[prev][1] = ui->le_colour_g->text().toInt();
        cfg.colours[prev][2] = ui->le_colour_b->text().toInt();
    }
    if (curr >= 0 && curr < NCFGCOLOURS) {
        ui->le_colour_r->setText(QString::number(cfg.colours[curr][0]));
        ui->le_colour_g->setText(QString::number(cfg.colours[curr][1]));
        ui->le_colour_b->setText(QString::number(cfg.colours[curr][2]));
    }
}
