/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include <QDebug>
#include <QKeyEvent>
#include <QToolButton>
#include <QTabWidget>
#include <QMessageBox>
#include "GuiMainWindow.h"
#include "GuiTerminalWindow.h"
#include "GuiSettingsWindow.h"
//#include "windows.h"
extern "C" {
#include "putty.h"
#include "ssh.h"
}

int initConfigDefaults(Config *cfg);

GuiMainWindow::GuiMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    /*
    mdiArea = new TerminalMdiArea;
    mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setViewMode(QMdiArea::TabbedView);
    mdiArea->setTabsClosable(true);
    mdiArea->setTabsMovable(true);
*/
    tabArea = new QTabWidget;
    tabArea->setTabsClosable(true);
    tabArea->setMovable(true);

    // this removes the frame border of QTabWidget
    tabArea->setDocumentMode(true);

    connect(tabArea, SIGNAL(tabCloseRequested(int)), SLOT(closeTerminal(int)));
    connect(tabArea, SIGNAL(currentChanged(int)), SLOT(currentChanged(int)));

    QToolButton* closeTabButton = new QToolButton();
    closeTabButton->setText(tr("+"));
    //connect(closeTabButton, SIGNAL(clicked()), this, SLOT(newTelnetTerminal()));
    connect(closeTabButton, SIGNAL(clicked()), this, SLOT(openSettingsWindow()));
    tabArea->setCornerWidget( closeTabButton, Qt::TopRightCorner );

    setWindowTitle(tr("QuTTY"));

    this->setCentralWidget(tabArea);
    //setWindowFlags(Qt::CustomizeWindowHint);
    //showMaximized();
    //setStyle(QStyle::);
}

GuiMainWindow::~GuiMainWindow()
{
    delete tabArea;
}


GuiTerminalWindow *GuiMainWindow::newTerminal()
{
    GuiTerminalWindow *termWnd = new GuiTerminalWindow(tabArea);
    tabArea->addTab(termWnd, tr("Qutty"));
    terminalList.append(termWnd);
    termWnd->setWindowState(termWnd->windowState() | Qt::WindowMaximized);
    return termWnd;
}

void GuiMainWindow::closeTerminal(int index)
{
    GuiTerminalWindow *termWnd = (GuiTerminalWindow*)tabArea->widget(index);
    if (termWnd) {
        if (termWnd->cfg.warn_on_close &&
            termWnd->as->qtsock->state() == QAbstractSocket::ConnectedState &&
            QMessageBox::No == QMessageBox::question(this, "Exit Confirmation?",
                                      "Are you sure you want to close this session?",
                                      QMessageBox::Yes|QMessageBox::No))
            return;
        terminalList.removeAll(termWnd);
    }
    tabArea->removeTab(index);
}

void GuiMainWindow::closeTerminal(GuiTerminalWindow *termWnd)
{
    if (termWnd->cfg.warn_on_close &&
        termWnd->as->qtsock->state() == QAbstractSocket::ConnectedState &&
        QMessageBox::No == QMessageBox::question(this, "Exit Confirmation?",
                                  "Are you sure you want to close this session?",
                                  QMessageBox::Yes|QMessageBox::No))
        return;
    tabArea->removeTab(tabArea->indexOf(termWnd));
    terminalList.removeAll(termWnd);
}

void GuiMainWindow::closeEvent ( QCloseEvent * event )
{
    event->ignore();
    if (QMessageBox::Yes == QMessageBox::question(this, "Exit Confirmation?",
                                  "Are you sure you want to close all the sessions?",
                                  QMessageBox::Yes|QMessageBox::No))
    {
        event->accept();
    }
}

void GuiMainWindow::openSettingsWindow()
{
    GuiSettingsWindow *ss = new GuiSettingsWindow(mainWindow);
    ss->loadDefaultSettings();
    ss->show();
}

extern "C" Socket get_ssh_socket(void *handle);
extern "C" Socket get_telnet_socket(void *handle);

void GuiMainWindow::timerHandler()
{
    long next;
    qDebug() << "Timer fired";
    if (run_timers(timing_next_time, &next)) {
        timer_change_notify(next);
    }
}

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
    strcpy(cfg->font.name, "Monospace");
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
