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
#include "GuiSplitter.h"
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
      menuCookieTermWnd(NULL),
      dragDropSite(),
      toolBarTerminalTop(this),
      findToolBar(NULL)
{
    memset(menuCommonActions, 0, sizeof(menuCommonActions));

    setWindowTitle(APPNAME);

    tabArea = new GuiTabWidget(this);
    tabArea->setTabsClosable(true);
    tabArea->setMovable(true);

    // this removes the frame border of QTabWidget
    tabArea->setDocumentMode(true);

    connect(tabArea, SIGNAL(tabCloseRequested(int)), SLOT(tabCloseRequested(int)));
    connect(tabArea, SIGNAL(currentChanged(int)), SLOT(currentChanged(int)));
    connect(tabArea, SIGNAL(sig_tabChangeSettings(GuiTerminalWindow*)), SLOT(on_changeSettingsTab(GuiTerminalWindow*)));

    initializeMenuSystem();
    inittializeDragDropWidget();
    toolBarTerminalTop.initializeToolbarTerminalTop(this);

    this->setCentralWidget(tabArea);

    resize(800, 600);   // initial size
    // read & restore the settings
    readSettings();
}

GuiMainWindow::~GuiMainWindow()
{
    delete tabArea;
    for (int i=0; i < MENU_MAX_ACTION; i++)
        delete menuCommonActions[i];
}

void GuiMainWindow::on_createNewSession(Config cfg, GuiBase::SplitType splittype)
{
    // User has selected a session
    this->createNewTab(&cfg, splittype);
}

void GuiMainWindow::createNewTab(Config *cfg, GuiBase::SplitType splittype)
{
    int rc;
    GuiTerminalWindow *newWnd = new GuiTerminalWindow(tabArea, this);
    newWnd->cfg = *cfg;

    if ((rc=newWnd->initTerminal()))
        goto err_exit;

    if (this->setupLayout(newWnd, splittype))
        goto err_exit;

    return;

err_exit:
    delete newWnd;
}

void GuiMainWindow::closeTab(GuiTerminalWindow *termWnd)
{
    assert(termWnd);
    tabArea->removeTab(tabArea->indexOf(termWnd));
    terminalList.removeAll(termWnd);
    //termWnd->deleteLater();
}

void GuiMainWindow::closeEvent ( QCloseEvent * event )
{
    event->ignore();
    if (tabArea->count() == 0 ||
        QMessageBox::Yes == QMessageBox::question(this, "Exit Confirmation?",
                                  "Are you sure you want to close all the sessions?",
                                  QMessageBox::Yes|QMessageBox::No))
    {
        // at least close the open sessions
        GuiTerminalWindow *t;
        QList<GuiTerminalWindow*> child(terminalList);
        for (QList<GuiTerminalWindow*>::const_iterator it = child.begin();
             it != child.end(); it++) {
            (*it)->reqCloseTerminal(true);
        }
        terminalList.clear();
        writeSettings();
        event->accept();
    }
}

void GuiMainWindow::tabCloseRequested (int index)
{
    // user cloing the tab
    GuiBase *base = dynamic_cast<GuiBase*>(tabArea->widget(index));
    GuiTerminalWindow *termWnd = dynamic_cast<GuiTerminalWindow*>(base);
    assert(base);
    if (termWnd) {
        // single terminal to close
        termWnd->reqCloseTerminal(false);
    } else if (base) {
        // multiple terminals to close
        if (QMessageBox::No == QMessageBox::question(this, "Exit Confirmation?",
                                "Are you sure you want to close all session panes?",
                                QMessageBox::Yes|QMessageBox::No))
            return;
        base->reqCloseTerminal(true);
    }
}

void GuiMainWindow::on_openNewSession(GuiBase::SplitType splittype)
{
    /*
     * 1. Context menu -> New Tab
     * 2. Main Menu -> New tab
     * 3. Keyboard shortcut
     * 4. Split sessions
     */
    if (settingsWindow) {
        QMessageBox::information(this, tr("Cannot open"), tr("Close the existing settings window"));
        return;
    }
    settingsWindow = new GuiSettingsWindow(this, splittype);
    connect(settingsWindow, SIGNAL(signal_session_open(Config, GuiBase::SplitType)), SLOT(on_createNewSession(Config, GuiBase::SplitType)));
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

void GuiMainWindow::on_changeSettingsTab(GuiTerminalWindow *termWnd)
{
    if (settingsWindow) {
        QMessageBox::information(this, tr("Cannot open"), tr("Close the existing settings window"));
        return;
    }
    assert(terminalList.indexOf(termWnd) != -1);
    settingsWindow = new GuiSettingsWindow(this);
    settingsWindow->enableModeChangeSettings(&termWnd->cfg, termWnd);
    connect(settingsWindow, SIGNAL(signal_session_change(Config, GuiTerminalWindow*)), SLOT(on_changeSettingsTabComplete(Config, GuiTerminalWindow*)));
    connect(settingsWindow, SIGNAL(signal_session_close()), SLOT(on_settingsWindowClose()));
    settingsWindow->show();
}

void GuiMainWindow::on_changeSettingsTabComplete(Config cfg, GuiTerminalWindow *termWnd)
{
    settingsWindow = NULL;
    assert(terminalList.indexOf(termWnd) != -1);
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
    if (index!=-1 && tabArea->widget(index)) {
        QWidget *currWgt = tabArea->widget(index);
        if (qobject_cast<GuiSplitter*>(currWgt)) {
            if (currWgt->focusWidget())
                currWgt->focusWidget()->setFocus();
        } else if (qobject_cast<GuiTerminalWindow*>(currWgt))
            currWgt->setFocus();
    }
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
    GuiTerminalWindow *termWindow;
    QWidget *widget = tabArea->currentWidget();
    if (!widget)
        return NULL;
    termWindow = dynamic_cast<GuiTerminalWindow*>(widget->focusWidget());
    if (!termWindow || terminalList.indexOf(termWindow) == -1)
        return NULL;
    return termWindow;
}

GuiTerminalWindow *GuiMainWindow::getCurrentTerminalInTab(int tabIndex)
{
    GuiTerminalWindow *termWindow;
    QWidget *widget = tabArea->widget(tabIndex);
    if (!widget)
        return NULL;
    if ((termWindow = dynamic_cast<GuiTerminalWindow*>(widget)))
        return termWindow;

    termWindow = dynamic_cast<GuiTerminalWindow*>(widget->focusWidget());
    if (!termWindow || terminalList.indexOf(termWindow) == -1)
        return NULL;
    return termWindow;
}


void GuiMainWindow::readSettings()
{
    bool menuBarVisible;
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPNAME, APPNAME);

    settings.beginGroup("GuiMainWindow");
    resize(settings.value("Size", size()).toSize());
    move(settings.value("Position", pos()).toPoint());
    setWindowState((Qt::WindowState)settings.value("WindowState", (int)windowState()).toInt());
    setWindowFlags((Qt::WindowFlags)settings.value("WindowFlags", (int)windowFlags()).toInt());
    menuBarVisible = settings.value("ShowMenuBar", true).toBool();
    settings.endGroup();

    menuCommonActions[MENU_FULLSCREEN]->setChecked((windowState() & Qt::WindowFullScreen));
    menuCommonActions[MENU_ALWAYSONTOP]->setChecked((windowFlags() & Qt::WindowStaysOnTopHint));
    menuCommonActions[MENU_MENUBAR]->setChecked(menuBarVisible);
    menuBarVisible ? menuBar()->show() : menuBar()->hide();

    this->show();
}

void GuiMainWindow::writeSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPNAME, APPNAME);

    settings.beginGroup("GuiMainWindow");
    settings.setValue("WindowState", (int)windowState());
    settings.setValue("WindowFlags", (int)windowFlags());
    settings.setValue("ShowMenuBar", menuBar()->isVisible());
    if (!isMaximized()) {
        settings.setValue("Size", size());
        settings.setValue("Position", pos());
    }
    settings.endGroup();
}

int GuiMainWindow::setupLayout(GuiTerminalWindow *newTerm, GuiBase::SplitType split, int tabind)
{
    // fallback to create tab
    if (tabArea->count() == 0)
        split = GuiBase::TYPE_LEAF;

    switch (split) {
    case GuiBase::TYPE_LEAF:
        newTerm->setParent(tabArea);
        tabArea->insertTab(tabind, newTerm, "");
        terminalList.append(newTerm);
        tabArea->setCurrentWidget(newTerm);
        set_title(newTerm, APPNAME);
        newTerm->setWindowState(newTerm->windowState() | Qt::WindowMaximized);

        // resize according to config if window is smaller
        if ( !(windowState() & Qt::WindowMaximized) &&
              (tabArea->count()==1) /* only for 1st window */ &&
             ( newTerm->viewport()->width() < newTerm->cfg.width*newTerm->getFontWidth() ||
                newTerm->viewport()->height() < newTerm->cfg.height*newTerm->getFontHeight())) {
            this->resize(newTerm->cfg.width*newTerm->getFontWidth() + width() - newTerm->viewport()->width(),
                         newTerm->cfg.height*newTerm->getFontHeight() + height() - newTerm->viewport()->height());
            term_size(newTerm->term, newTerm->cfg.height, newTerm->cfg.width, newTerm->cfg.savelines);
        }
        break;
    case GuiBase::TYPE_HORIZONTAL:
    case GuiBase::TYPE_VERTICAL:
    {
        GuiTerminalWindow *currTerm;
        if (!(currTerm = getCurrentTerminal()))
            goto err_exit;

        currTerm->createSplitLayout(split, newTerm);
        newTerm->setFocus();
        terminalList.append(newTerm);
        break;
    }
    default:
        assert(0);
        return -1;
    }
    return 0;

err_exit:
    return -1;
}
