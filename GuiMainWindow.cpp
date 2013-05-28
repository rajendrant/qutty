/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include <QDebug>
#include <QKeyEvent>
#include <QTabWidget>
#include <QMessageBox>
#include <QTabBar>
#include <QSettings>
#include <QMenuBar>
#include "GuiMainWindow.h"
#include "GuiTerminalWindow.h"
#include "GuiSettingsWindow.h"
#include "GuiTabWidget.h"
//#include "windows.h"
extern "C" {
#include "putty.h"
#include "ssh.h"
}

int initConfigDefaults(Config *cfg);

GuiMainWindow::GuiMainWindow(QWidget *parent)
    : QMainWindow(parent),
      settingsWindow(NULL),
      newTabToolButton(),
      menuTabBar(&newTabToolButton),
      menuTermWnd(this),
      menuCookieTermWnd(NULL),
      menuCookieTabIndex(-1)
{
    memset(menuCommonActions, 0, sizeof(menuCommonActions));
    memset(menuCommonMenus, 0, sizeof(menuCommonMenus));

    setWindowTitle(APPNAME);

    tabArea = new GuiTabWidget(this);
    tabArea->setTabsClosable(true);
    tabArea->setMovable(true);

    // this removes the frame border of QTabWidget
    tabArea->setDocumentMode(true);

    connect(tabArea, SIGNAL(tabCloseRequested(int)), SLOT(tabCloseRequested(int)));
    connect(tabArea, SIGNAL(currentChanged(int)), SLOT(currentChanged(int)));
    connect(tabArea, SIGNAL(sig_tabChangeSettings(int)), SLOT(on_changeSettingsTab(int)));

    initializeMenuSystem();

    this->setCentralWidget(tabArea);

    // read & restore the settings
    readSettings();
}

GuiMainWindow::~GuiMainWindow()
{
    delete tabArea;
}

void GuiMainWindow::on_createNewTab(Config cfg)
{
    // User has selected a session
    this->createNewTab(&cfg);
}

void GuiMainWindow::createNewTab(Config *cfg)
{
    int rc;
    GuiTerminalWindow *newWnd = this->newTerminal();
    newWnd->cfg = *cfg;

    if ((rc=newWnd->initTerminal())) {
        delete newWnd;
    } else {    // success
        tabArea->setCurrentWidget(newWnd);
    }
}

GuiTerminalWindow *GuiMainWindow::newTerminal()
{
    GuiTerminalWindow *termWnd = new GuiTerminalWindow(tabArea, this);
    tabArea->addTab(termWnd, "");
    terminalList.append(termWnd);
    termWnd->setWindowState(termWnd->windowState() | Qt::WindowMaximized);
    set_title(termWnd, APPNAME);
    return termWnd;
}

void GuiMainWindow::closeTerminal(int index)
{
    GuiTerminalWindow *termWnd = (GuiTerminalWindow*)tabArea->widget(index);
    closeTerminal(termWnd);
}

void GuiMainWindow::closeTerminal(GuiTerminalWindow *termWnd)
{
    assert(termWnd);
    tabArea->removeTab(tabArea->indexOf(termWnd));
    terminalList.removeAll(termWnd);
    termWnd->deleteLater();
}

void GuiMainWindow::closeEvent ( QCloseEvent * event )
{
    event->ignore();
    if (tabArea->count() == 0 ||
        QMessageBox::Yes == QMessageBox::question(this, "Exit Confirmation?",
                                  "Are you sure you want to close all the sessions?",
                                  QMessageBox::Yes|QMessageBox::No))
    {
        writeSettings();
        event->accept();
    }
}

void GuiMainWindow::tabCloseRequested (int index)
{
    // user cloing the tab
    GuiTerminalWindow *termWnd = (GuiTerminalWindow*)tabArea->widget(index);
    assert(termWnd);
    termWnd->userClosingTab = true;
    if (termWnd->cfg.warn_on_close &&
        !termWnd->isSockDisconnected &&
        QMessageBox::No == QMessageBox::question(this, "Exit Confirmation?",
                                  "Are you sure you want to close this session?",
                                  QMessageBox::Yes|QMessageBox::No))
        return;
    closeTerminal(index);
}

void GuiMainWindow::on_openNewTab()
{
    /*
     * 1. Context menu -> New Tab
     * 2. Main Menu -> New tab
     * 3. Keyboard shortcut
     */
    if (settingsWindow) {
        QMessageBox::information(this, tr("Cannot open"), tr("Close the existing settings window"));
        return;
    }
    settingsWindow = new GuiSettingsWindow(this);
    connect(settingsWindow, SIGNAL(signal_session_open(Config)), SLOT(on_createNewTab(Config)));
    connect(settingsWindow, SIGNAL(signal_session_close()), SLOT(on_settingsWindowClose()));
    settingsWindow->loadDefaultSettings();
    settingsWindow->show();
}

void GuiMainWindow::on_settingsWindowClose()
{
    settingsWindow = NULL;
}

void GuiMainWindow::on_openNewWindow()
{
    GuiMainWindow *mainWindow = new GuiMainWindow;
    mainWindow->on_openNewTab();
    mainWindow->show();
}

void GuiMainWindow::on_changeSettingsTab(int tabIndex)
{
    if (settingsWindow) {
        QMessageBox::information(this, tr("Cannot open"), tr("Close the existing settings window"));
        return;
    }
    GuiTerminalWindow *termWnd = (GuiTerminalWindow*)tabArea->widget(tabIndex);
    assert(termWnd);
    settingsWindow = new GuiSettingsWindow(this);
    settingsWindow->enableModeChangeSettings(&termWnd->cfg, tabIndex);
    connect(settingsWindow, SIGNAL(signal_session_change(Config, int)), SLOT(on_changeSettingsTabComplete(Config, int)));
    connect(settingsWindow, SIGNAL(signal_session_close()), SLOT(on_settingsWindowClose()));
    settingsWindow->show();
}

void GuiMainWindow::on_changeSettingsTabComplete(Config cfg, int tabIndex)
{
    settingsWindow = NULL;
    GuiTerminalWindow *termWnd = (GuiTerminalWindow*)tabArea->widget(tabIndex);
    assert(termWnd);
    termWnd->reconfigureTerminal(&cfg);
}

extern "C" Socket get_ssh_socket(void *handle);
extern "C" Socket get_telnet_socket(void *handle);

bool GuiMainWindow::winEvent ( MSG * msg, long * result )
{
    int ret;
    HDC hdc;
    //RECT r1, r2;

    /*
    switch(msg->message) {
    case WM_NCCALCSIZE:
        qDebug()<<"got WM_NCCALCSIZE "<<hex<<msg->wParam<<" "<<msg->lParam<<endl;
        if (msg->wParam) {
                NCCALCSIZE_PARAMS* param = (NCCALCSIZE_PARAMS*)msg->lParam;
                memcpy(&r1, param->rgrc, sizeof(RECT));
                qDebug()<<dec<<param->lppos<<" "<<param->rgrc->top<<" "<<param->rgrc->left<<" "<<param->rgrc->bottom<<" "<<param->rgrc->right<<endl;
        } else {
                RECT* param = (RECT*)msg->lParam;
                qDebug()<<dec<<param->top<<" "<<param->left<<" "<<param->bottom<<" "<<param->right<<endl;
        }
        ret = DefWindowProc((HWND) this->winId(), msg->message, msg->wParam, msg->lParam);
        if (msg->wParam) {
                NCCALCSIZE_PARAMS* param = (NCCALCSIZE_PARAMS*)msg->lParam;
                qDebug()<<hex<<"ret "<<ret<<dec<<param->lppos<<" "<<param->rgrc->top<<" "<<param->rgrc->left<<" "<<param->rgrc->bottom<<" "<<param->rgrc->right<<endl;
                memcpy(&r2, param->rgrc, sizeof(RECT));
                memcpy(param->rgrc, &r1, sizeof(RECT));
                param->rgrc->top += r1.bottom - r2.bottom + 1;
                //if(mainWindow->windowState()&Qt::WindowMaximized)
                //    param->rgrc->top = r1.top + 4;
        } else {
                RECT* param = (RECT*)msg->lParam;
                qDebug()<<hex<<"ret "<<ret<<dec<<param->top<<" "<<param->left<<" "<<param->bottom<<" "<<param->right<<endl;
                //param->top -= 40;
        }
        *result = ret;
        return true;
    case WM_NCACTIVATE:
        qDebug() << "got wm_ncactivate "<<msg->message<<" "<<msg->lParam<<" "<<msg->wParam<<"\n";
    case WM_NCPAINT:
        hdc = GetWindowDC((HWND) this->winId());
        if ((int)hdc != 0)
        {
            //ret = DefWindowProc((HWND) this->winId(), msg->message, msg->wParam, msg->lParam);
            /*TextOut(hdc, 0, 0, L"Hello, Windows!", 15);
            RECT rect;
            rect.top = 0; rect.left=0; rect.bottom=10; rect.right=20;
            DrawEdge(hdc, &rect, EDGE_RAISED, BF_RECT | BF_ADJUST);
            FillRect(hdc, &rect, GetSysColorBrush(COLOR_BTNFACE));
            ReleaseDC((HWND) this->winId(), hdc);
            qDebug()<<"painted WM_NCPAINT\n";* /
        } else
            qDebug()<<"failed painting WM_NCPAINT\n";
        *result = 0;
        return true;
    }*/

    return false;
}

void GuiMainWindow::currentChanged(int index)
{
    if (index!=-1 && tabArea->widget(index))
        tabArea->widget(index)->setFocus();
    //term_set_focus(tabArea->currentWidget()->term, TRUE);
    //term_update(tabArea->currentWidget()->term);
}

void GuiMainWindow::focusChanged ( QWidget * old, QWidget * now )
{
    //qDebug()<<__FUNCTION__<<old<<now;
}

int initConfigDefaults(Config *cfg)
{
    memset(cfg, 0, sizeof(Config));
    cfg->protocol = PROT_SSH;
    cfg->port = 23;
    cfg->width = 80;
    cfg->height = 30;
    //cfg->savelines = 1000;
    cfg->passive_telnet = 0;
    strcpy(cfg->termtype, "xterm");
    strcpy(cfg->termspeed,"38400,38400");
    //strcpy(cfg->username, "user");
    strcpy(cfg->environmt, "");
    //strcpy(cfg->line_codepage, "ISO-8859-1:1998 (Latin-1, West Europe)");
    strcpy(cfg->line_codepage, "ISO 8859-1");
    cfg->vtmode = VT_UNICODE;
    //char *ip_addr = /*"192.168.230.129";*/ "192.168.1.103";

    // font
    strcpy(cfg->font.name, "Courier New");
    cfg->font.height = 11;
    cfg->font.isbold = 0;
    cfg->font.charset = 0;

    // colors
    cfg->ansi_colour = 1;
    cfg->xterm_256_colour = 1;
    cfg->bold_colour = 1;
    cfg->try_palette = 0;
    cfg->system_colour = 0;
    static const char *const default_colors[] = {
        "187,187,187", "255,255,255", "0,0,0", "85,85,85", "0,0,0",
        "0,255,0", "0,0,0", "85,85,85", "187,0,0", "255,85,85",
        "0,187,0", "85,255,85", "187,187,0", "255,255,85", "0,0,187",
        "85,85,255", "187,0,187", "255,85,255", "0,187,187",
        "85,255,255", "187,187,187", "255,255,255"
    };
    for(uint i=0; i<lenof(cfg->colours); i++) {
        int c0, c1, c2;
        if (sscanf(default_colors[i], "%d,%d,%d", &c0, &c1, &c2) == 3) {
            cfg->colours[i][0] = c0;
            cfg->colours[i][1] = c1;
            cfg->colours[i][2] = c2;
        }
    }

    // blink cursor
    cfg->blink_cur = 0;

    cfg->funky_type = FUNKY_TILDE;
    cfg->ctrlaltkeys = 1;
    cfg->compose_key = 0;
    cfg->no_applic_k = 0;
    cfg->nethack_keypad = 0;
    cfg->bksp_is_delete = 1;
    cfg->rxvt_homeend = 0;
    cfg->localedit = AUTO;
    cfg->localecho = AUTO;
    cfg->bidi = 0;
    cfg->arabicshaping = 0;
    cfg->ansi_colour = 1;
    cfg->xterm_256_colour = 1;

    // all cfg settings
    cfg->warn_on_close = 1;
    cfg->close_on_exit = 1;
    cfg->tcp_nodelay = 1;
    cfg->proxy_dns = 2;

    //strcpy(cfg->ttymodes, "INTR", 6);

    cfg->remote_qtitle_action = 1;
    cfg->telnet_newline = 1;
    cfg->alt_f4 = 1;
    cfg->scroll_on_disp = 1;
    cfg->erase_to_scrollback = 1;
    cfg->savelines = 20000;
    cfg->wrap_mode = 1;
    cfg->scrollbar = 1;
    cfg->bce = 1;
    cfg->window_border = 1;
    strcpy(cfg->answerback, "PuTTY");
    cfg->mouse_is_xterm = 0;
    cfg->mouse_override = 1;
    cfg->utf8_override = 1;
    cfg->x11_forward = 1;
    cfg->x11_auth = 1;

    // ssh options
    cfg->ssh_cipherlist[0] = 3;
    cfg->ssh_cipherlist[1] = 2;
    cfg->ssh_cipherlist[2] = 1;
    cfg->ssh_cipherlist[3] = 0;
    cfg->ssh_cipherlist[4] = 5;
    cfg->ssh_cipherlist[5] = 4;
    cfg->ssh_kexlist[0] = 3;
    cfg->ssh_kexlist[1] = 2;
    cfg->ssh_kexlist[2] = 1;
    cfg->ssh_kexlist[3] = 4;
    cfg->ssh_kexlist[4] = 0;
    cfg->ssh_rekey_time = 60;
    strcpy(cfg->ssh_rekey_data, "1G");
    cfg->sshprot = 2;
    cfg->ssh_show_banner = 1;
    cfg->try_ki_auth = 1;
    cfg->try_gssapi_auth = 0; // TODO dont enable
    cfg->sshbug_ignore1	= 2;
    cfg->sshbug_plainpw1 = 2;
    cfg->sshbug_rsa1 = 2;
    cfg->sshbug_hmac2 = 2;
    cfg->sshbug_derivekey2 = 2;
    cfg->sshbug_rsapad2 = 2;
    cfg->sshbug_pksessid2 = 2;
    cfg->sshbug_rekey2 = 2;
    cfg->sshbug_maxpkt2 = 2;
    cfg->sshbug_ignore2 = 2;
    cfg->ssh_simple = 0;

    static const int cfg_wordness_defaults[] =
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,1,2,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,
        1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,2,
        1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2
    };
    for(uint i=0; i<sizeof(cfg->wordness)/sizeof(cfg->wordness[0]); i++)
        cfg->wordness[i] = cfg_wordness_defaults[i];

    return 0;
}

void GuiMainWindow::tabNext ()
{
    if (tabArea->currentIndex() != tabArea->count()-1)
        tabArea->setCurrentIndex(tabArea->currentIndex()+1);
    else
        tabArea->setCurrentIndex(0);
}

void GuiMainWindow::tabPrev ()
{
    if (tabArea->currentIndex() != 0)
        tabArea->setCurrentIndex(tabArea->currentIndex()-1);
    else
        tabArea->setCurrentIndex(tabArea->count()-1);
}

GuiTerminalWindow * GuiMainWindow::getCurrentTerminal()
{
    QWidget *widget = tabArea->currentWidget();
    if (!widget)
        return NULL;
    GuiTerminalWindow *termWindow = static_cast<GuiTerminalWindow*>(widget);
    if (terminalList.indexOf(termWindow) != -1)
        return termWindow;
    return NULL;
}

void GuiMainWindow::readSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPNAME, APPNAME);

    settings.beginGroup("GuiMainWindow");
    resize(settings.value("Size", QSize(400, 400)).toSize());
    move(settings.value("Position", QPoint(200, 200)).toPoint());
    setWindowState((Qt::WindowState)settings.value("WindowState", (int)windowState()).toInt());
    setWindowFlags((Qt::WindowFlags)settings.value("WindowFlags", (int)windowFlags()).toInt());
    menuBar()->setVisible(settings.value("ShowMenuBar", true).toBool());
    settings.endGroup();

    menuCommonActions[MENU_FULLSCREEN]->setChecked((windowState() & Qt::WindowFullScreen));
    menuCommonActions[MENU_ALWAYSONTOP]->setChecked((windowFlags() & Qt::WindowStaysOnTopHint));
    menuCommonActions[MENU_MENUBAR]->setChecked(menuBar()->isVisible());

    this->show();
}

void GuiMainWindow::writeSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPNAME, APPNAME);

    settings.beginGroup("GuiMainWindow");
    settings.setValue("Size", size());
    settings.setValue("Position", pos());
    settings.setValue("WindowState", (int)windowState());
    settings.setValue("WindowFlags", (int)windowFlags());
    settings.setValue("ShowMenuBar", menuBar()->isVisible());
    settings.endGroup();
}
