/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include <QDebug>
#include <QKeyEvent>
#include <QToolButton>
#include <QTabWidget>
#include "GuiMainWindow.h"
#include "GuiTerminalWindow.h"
#include "GuiSettingsWindow.h"
//#include "windows.h"
extern "C" {
#include "putty.h"
#include "ssh.h"
}

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

    connect(tabArea, SIGNAL(tabCloseRequested(int)), SLOT(closeTerminal(int)));
    connect(tabArea, SIGNAL(currentChanged(int)), SLOT(currentChanged(int)));

    QToolButton* closeTabButton = new QToolButton();
    closeTabButton->setText(tr("+"));
    //connect(closeTabButton, SIGNAL(clicked()), this, SLOT(newTelnetTerminal()));
    connect(closeTabButton, SIGNAL(clicked()), this, SLOT(newTerminal()));
    tabArea->setCornerWidget( closeTabButton, Qt::TopRightCorner );

    setWindowTitle(tr("QuTTY"));

    this->setCentralWidget(tabArea);
    //setWindowFlags(Qt::CustomizeWindowHint);
    //showMaximized();
    //setStyle(QStyle::);
    setMinimumSize(960,660);

}

GuiMainWindow::~GuiMainWindow()
{
    delete tabArea;
}

GuiTerminalWindow *GuiMainWindow::newTerminal()
{
    GuiSettingsWindow *ss = new GuiSettingsWindow(mainWindow);
    ss->show();
    return NULL;
}

extern "C" Socket get_ssh_socket(void *handle);
extern "C" Socket get_telnet_socket(void *handle);

GuiTerminalWindow *GuiMainWindow::newTelnetTerminal(const char *ip_addr, const char *port, int protocol)
{
    void *ldisc;
    Terminal *term;
    Backend *back;
    void *backhandle;
    GuiTerminalWindow *termWnd = new GuiTerminalWindow(tabArea);
    tabArea->addTab(termWnd, tr("Qutty"));
    //subWindow->setFixedSize(termWnd->size());
    //qDebug()<<"subWindow"<<subWindow->geometry();
    terminalList.append(termWnd);
    termWnd->setWindowState(termWnd->windowState() | Qt::WindowMaximized);

    char *realhost = NULL;
    Config *cfg = &termWnd->cfg;
    memset(cfg, 0, sizeof(Config));
    cfg->protocol = protocol;
    cfg->port = atoi(port);
    cfg->width = 80;
    cfg->height = 30;
    //cfg->savelines = 1000;
    cfg->passive_telnet = 0;
    strcpy(cfg->termtype, "xterm");
    strcpy(cfg->termspeed,"38400,38400");
    strcpy(cfg->username, "user");
    strcpy(cfg->environmt, "");
    //strcpy(cfg->line_codepage, "ISO-8859-1:1998 (Latin-1, West Europe)");
    strcpy(cfg->line_codepage, "UTF-8");
    cfg->vtmode = VT_UNICODE;
    //char *ip_addr = /*"192.168.230.129";*/ "192.168.1.103";

    // font
    strcpy(cfg->font.name, "Courier New");
    strcpy(cfg->font.name, "Monospace");
    cfg->font.height = 11;
    cfg->font.isbold = 0;
    cfg->font.charset = 0;
    termWnd->setTermFont(&cfg->font);

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
    for(int i=0; i<lenof(cfg->colours); i++) {
        int c0, c1, c2;
        if (sscanf(default_colors[i], "%d,%d,%d", &c0, &c1, &c2) == 3) {
            cfg->colours[i][0] = c0;
            cfg->colours[i][1] = c1;
            cfg->colours[i][2] = c2;
        }
    }
    termWnd->cfgtopalette(cfg);

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


    memset(&termWnd->ucsdata, 0, sizeof(struct unicode_data));
    init_ucs(cfg, &termWnd->ucsdata);

    term = term_init(cfg, &termWnd->ucsdata, termWnd);
    term_size(term, cfg->height, cfg->width, cfg->savelines);

    termWnd->backend = back = backend_from_proto(cfg->protocol);
    termWnd->backend->init(termWnd, &backhandle, cfg, (char*)ip_addr, cfg->port, &realhost, 1, 0);
    switch(cfg->protocol) {
    case PROT_TELNET:
        termWnd->as = (Actual_Socket)get_telnet_socket(backhandle);
        break;
    case PROT_SSH:
        termWnd->as = (Actual_Socket)get_ssh_socket(backhandle);
        break;
    default:
        assert(0);
    }
    termWnd->qtsock = termWnd->as->qtsock;
    QObject::connect(termWnd->as->qtsock, SIGNAL(readyRead()), termWnd, SLOT(readyRead()));

    /*
     * Connect the terminal to the backend for resize purposes.
     */
    term_provide_resize_fn(term, back->size, backhandle);

    /*
     * Set up a line discipline.
     */
    ldisc = ldisc_create(cfg, term, back, backhandle, termWnd);

    termWnd->ldisc = ldisc;
    termWnd->term = term;
    termWnd->backhandle = backhandle;

    return termWnd;
}

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

void GuiMainWindow::closeTerminal(int index)
{
    tabArea->removeTab(index);
}

void GuiMainWindow::currentChanged(int index)
{
    tabArea->widget(index)->setFocus();
    //term_set_focus(tabArea->currentWidget()->term, TRUE);
    //term_update(tabArea->currentWidget()->term);
}

void GuiMainWindow::focusChanged ( QWidget * old, QWidget * now )
{
    //qDebug()<<__FUNCTION__<<old<<now;
}
