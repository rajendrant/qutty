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
#include <QLabel>
#include <QGraphicsBlurEffect>
#include <QBitmap>
#include "GuiMainWindow.h"
#include "GuiTerminalWindow.h"
#include "GuiSettingsWindow.h"
#include "GuiTabWidget.h"
#include "GuiTabBar.h"
#include "GuiSplitter.h"
#include "serialize/QtMRUSessionList.h"
//#include "windows.h"
extern "C" {
#include "putty.h"
#include "ssh.h"
}

GuiMainWindow::GuiMainWindow(QWidget *parent)
    : QMainWindow(parent),
      menuCookieTermWnd(NULL),
      toolBarTerminalTop(this),
      dragDropSite(),
      findToolBar(NULL),
      mru_count_last(0),
      tabNavigate(NULL),
      paneNavigate(NULL),
      tabArea(new GuiTabWidget(this)),
      textFilterWnd(NULL),
      compactSettingsWindow(NULL),
      settingsWindow(NULL),
      newTabToolButton() {
  setWindowTitle(APPNAME);

  tabArea->setTabsClosable(true);
  tabArea->setMovable(true);

  // this removes the frame border of QTabWidget
  tabArea->setDocumentMode(true);

  connect(tabArea, SIGNAL(tabCloseRequested(int)), SLOT(tabCloseRequested(int)));
  connect(tabArea, SIGNAL(currentChanged(int)), SLOT(currentChanged(int)));
  connect(tabArea->getGuiTabBar(), SIGNAL(sig_tabInserted()), SLOT(on_tabLayoutChanged()));
  connect(tabArea->getGuiTabBar(), SIGNAL(sig_tabRemoved()), SLOT(on_tabLayoutChanged()));
  connect(tabArea->getGuiTabBar(), SIGNAL(tabMoved(int, int)), SLOT(on_tabLayoutChanged()));

  initializeMenuSystem();
  inittializeDragDropWidget();
  toolBarTerminalTop.initializeToolbarTerminalTop(this);

  tabArea->setCornerWidget(&newTabToolButton, Qt::TopLeftCorner);

  this->setCentralWidget(tabArea);

  resize(800, 600);  // initial size
  // read & restore the settings
  readWindowSettings();
}

GuiMainWindow::~GuiMainWindow() {
  delete tabArea;
  for (auto it = menuCommonShortcuts.begin(); it != menuCommonShortcuts.end(); ++it) {
    if (std::get<1>(*it)) delete std::get<1>(*it);
    if (std::get<2>(*it)) delete std::get<2>(*it);
  }
}

void GuiMainWindow::on_createNewSession(Config cfg, GuiBase::SplitType splittype) {
  // User has selected a session
  this->createNewTab(&cfg, splittype);
}

void GuiMainWindow::createNewTab(Config *cfg, GuiBase::SplitType splittype) {
  int rc;
  GuiTerminalWindow *newWnd = new GuiTerminalWindow(tabArea, this);
  newWnd->cfg = *cfg;
  auto config_name = QString(cfg->config_name);
  auto hostname = QString(cfg->host);

  if ((rc = newWnd->initTerminal())) goto err_exit;

  if (this->setupLayout(newWnd, splittype)) goto err_exit;

  // To set the current session to MRU list
  qutty_mru_sesslist.insertSession(config_name, hostname);

  return;

err_exit:
  delete newWnd;
}

void GuiMainWindow::tabInsert(int tabind, QWidget *w, const QString &title) {
  tabArea->insertTab(tabind, w, title);
}

void GuiMainWindow::tabRemove(int tabind) { tabArea->removeTab(tabind); }

void GuiMainWindow::closeTerminal(GuiTerminalWindow *termWnd) {
  assert(termWnd);
  int ind = tabArea->indexOf(termWnd);
  terminalList.removeAll(termWnd);
  if (ind != -1) tabRemove(ind);
  on_tabLayoutChanged();
}

void GuiMainWindow::hideTerminal(GuiTerminalWindow *termWnd) {
  assert(termWnd);
  int ind = tabArea->indexOf(termWnd);
  if (ind != -1) {
    tabRemove(ind);
    on_tabLayoutChanged();
  }
}

void GuiMainWindow::closeEvent(QCloseEvent *event) {
  event->ignore();
  if (tabArea->count() == 0 ||
      QMessageBox::Yes == QMessageBox::question(this, "Exit Confirmation?",
                                                "Are you sure you want to close all the sessions?",
                                                QMessageBox::Yes | QMessageBox::No)) {
    // at least close the open sessions
    QList<GuiTerminalWindow *> child(terminalList);
    for (auto it = child.begin(); it != child.end(); it++) {
      (*it)->reqCloseTerminal(true);
    }
    terminalList.clear();
    writeWindowSettings();
    event->accept();
  }
}

void GuiMainWindow::tabCloseRequested(int index) {
  // user cloing the tab
  auto it = widgetAtIndex[index];
  if (it.second) {
    // single terminal to close
    it.second->reqCloseTerminal(false);
  } else if (it.first) {
    // multiple terminals to close
    if (QMessageBox::No ==
        QMessageBox::question(this, "Exit Confirmation?",
                              "Are you sure you want to close all session panes?",
                              QMessageBox::Yes | QMessageBox::No))
      return;
    it.first->reqCloseTerminal(true);
  }
}

void GuiMainWindow::on_openNewSession(Config cfg, GuiBase::SplitType splittype) {
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
  connect(settingsWindow, SIGNAL(signal_session_open(Config, GuiBase::SplitType)),
          SLOT(on_createNewSession(Config, GuiBase::SplitType)));
  connect(settingsWindow, SIGNAL(signal_session_close()), SLOT(on_settingsWindowClose()));
  settingsWindow->loadInitialSettings(cfg);
  settingsWindow->show();
}

void GuiMainWindow::on_openNewCompactSession(GuiBase::SplitType splittype) {
  /*
   * 1. Context menu -> New Tab
   * 2. Main Menu -> New tab
   * 3. Keyboard shortcut
   * 4. Split sessions
   */
  if (compactSettingsWindow) {
    QMessageBox::information(this, tr("Cannot open"), tr("Close the existing settings window"));
    return;
  }
  compactSettingsWindow = new GuiCompactSettingsWindow(this, splittype);
  connect(compactSettingsWindow, SIGNAL(signal_on_open(Config, GuiBase::SplitType)),
          SLOT(on_createNewSession(Config, GuiBase::SplitType)));
  connect(compactSettingsWindow, SIGNAL(signal_on_close()), SLOT(on_settingsWindowClose()));
  connect(compactSettingsWindow, SIGNAL(signal_on_detail(Config, GuiBase::SplitType)),
          SLOT(on_openNewSession(Config, GuiBase::SplitType)));
  compactSettingsWindow->show();
}

void GuiMainWindow::on_settingsWindowClose() {
  settingsWindow = NULL;
  compactSettingsWindow = NULL;
}

void GuiMainWindow::on_openNewWindow() {
  GuiMainWindow *mainWindow = new GuiMainWindow;
  mainWindow->on_openNewTab();
  mainWindow->show();
}

void GuiMainWindow::on_changeSettingsTab(GuiTerminalWindow *termWnd) {
  if (settingsWindow) {
    QMessageBox::information(this, tr("Cannot open"), tr("Close the existing settings window"));
    return;
  }
  assert(terminalList.indexOf(termWnd) != -1);
  settingsWindow = new GuiSettingsWindow(this);
  settingsWindow->enableModeChangeSettings(&termWnd->cfg, termWnd);
  connect(settingsWindow, SIGNAL(signal_session_change(Config, GuiTerminalWindow *)),
          SLOT(on_changeSettingsTabComplete(Config, GuiTerminalWindow *)));
  connect(settingsWindow, SIGNAL(signal_session_close()), SLOT(on_settingsWindowClose()));
  settingsWindow->show();
}

void GuiMainWindow::on_changeSettingsTabComplete(Config cfg, GuiTerminalWindow *termWnd) {
  settingsWindow = NULL;
  assert(terminalList.indexOf(termWnd) != -1);
  termWnd->reconfigureTerminal(&cfg);
}

extern "C" Socket get_ssh_socket(void *handle);
extern "C" Socket get_telnet_socket(void *handle);

void GuiMainWindow::currentChanged(int index) {
  if (index < 0) return;
  if (index < (signed)widgetAtIndex.size()) {
    auto it = widgetAtIndex[index];
    if (it.first) {
      if (it.first->focusWidget()) it.first->focusWidget()->setFocus();
    } else if (it.second)
      it.second->setFocus();
    return;
  }

  // slow_mode: if tab is inserted just now, widgetAtIndex may not
  // be up-to-date.
  if (tabArea->widget(index)) {
    QWidget *currWgt = tabArea->widget(index);
    if (qobject_cast<GuiSplitter *>(currWgt)) {
      if (currWgt->focusWidget()) currWgt->focusWidget()->setFocus();
    } else if (qobject_cast<GuiTerminalWindow *>(currWgt))
      currWgt->setFocus();
  }
}

int initConfigDefaults(Config *cfg) {
  memset(cfg, 0, sizeof(Config));
  cfg->protocol = PROT_SSH;
  cfg->port = 22;
  cfg->width = 80;
  cfg->height = 30;
  // cfg->savelines = 1000;
  cfg->passive_telnet = 0;
  strcpy(cfg->termtype, "xterm");
  strcpy(cfg->termspeed, "38400,38400");
  // strcpy(cfg->username, "user");
  strcpy(cfg->environmt, "");
  // strcpy(cfg->line_codepage, "ISO-8859-1:1998 (Latin-1, West Europe)");
  strcpy(cfg->line_codepage, "ISO 8859-1");
  cfg->vtmode = VT_UNICODE;
  // char *ip_addr = /*"192.168.230.129";*/ "192.168.1.103";

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
      "187,187,187", "255,255,255", "0,0,0",       "85,85,85",   "0,0,0",     "0,255,0",
      "0,0,0",       "85,85,85",    "187,0,0",     "255,85,85",  "0,187,0",   "85,255,85",
      "187,187,0",   "255,255,85",  "0,0,187",     "85,85,255",  "187,0,187", "255,85,255",
      "0,187,187",   "85,255,255",  "187,187,187", "255,255,255"};
  for (uint i = 0; i < lenof(cfg->colours); i++) {
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

  // strcpy(cfg->ttymodes, "INTR", 6);

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
  cfg->try_gssapi_auth = 0;  // TODO dont enable
  cfg->sshbug_ignore1 = 2;
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

  static const int cfg_wordness_defaults[] = {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 1, 1, 1, 1, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2};
  for (uint i = 0; i < sizeof(cfg->wordness) / sizeof(cfg->wordness[0]); i++)
    cfg->wordness[i] = cfg_wordness_defaults[i];

  return 0;
}

GuiTerminalWindow *GuiMainWindow::getCurrentTerminal() {
  GuiTerminalWindow *termWindow;
  QWidget *widget = tabArea->currentWidget();
  if (!widget) return NULL;
  termWindow = dynamic_cast<GuiTerminalWindow *>(widget->focusWidget());
  if (!termWindow || terminalList.indexOf(termWindow) == -1) return NULL;
  return termWindow;
}

GuiTerminalWindow *GuiMainWindow::getCurrentTerminalInTab(int tabIndex) {
  auto it = widgetAtIndex[tabIndex];
  if (it.second || !it.first) return it.second;
  return qobject_cast<GuiTerminalWindow *>(it.first->focusWidget());
}

void GuiMainWindow::readWindowSettings() {
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPNAME, APPNAME);

  settings.beginGroup("GuiMainWindow");
  qutty_config.mainwindow.size = settings.value("Size", size()).toSize();
  qutty_config.mainwindow.pos = settings.value("Position", pos()).toPoint();
  qutty_config.mainwindow.state = settings.value("WindowState", (int)windowState()).toInt();
  qutty_config.mainwindow.flag = settings.value("WindowFlags", (int)windowFlags()).toInt();
  qutty_config.mainwindow.menubar_visible = settings.value("ShowMenuBar", false).toBool();
  qutty_config.mainwindow.titlebar_tabs = settings.value("ShowTabsInTitlebar", true).toBool();
  settings.endGroup();

  if (qutty_config.mainwindow.titlebar_tabs && qutty_config.mainwindow.menubar_visible)
    qutty_config.mainwindow.menubar_visible = false;

  resize(qutty_config.mainwindow.size);
  move(qutty_config.mainwindow.pos);
  setWindowState((Qt::WindowState)qutty_config.mainwindow.state);
  setWindowFlags((Qt::WindowFlags)qutty_config.mainwindow.flag);

  menuGetActionById(MENU_FULLSCREEN)->setChecked((windowState() & Qt::WindowFullScreen));
  menuGetActionById(MENU_ALWAYSONTOP)->setChecked((windowFlags() & Qt::WindowStaysOnTopHint));
  menuGetActionById(MENU_MENUBAR)->setChecked(qutty_config.mainwindow.menubar_visible);
  menuGetActionById(MENU_TAB_IN_TITLE_BAR)->setChecked(qutty_config.mainwindow.titlebar_tabs);

  if (qutty_config.mainwindow.menubar_visible) {
    // setup main menubar
    menuBar()->show();
    menuBar()->addMenu(&menuCommonMenus[MENU_FILE - MENU_SEPARATOR - 1]);
    menuBar()->addMenu(&menuCommonMenus[MENU_EDIT - MENU_SEPARATOR - 1]);
    menuBar()->addMenu(&menuCommonMenus[MENU_VIEW - MENU_SEPARATOR - 1]);
  }
}

void GuiMainWindow::writeWindowSettings() {
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPNAME, APPNAME);

  settings.beginGroup("GuiMainWindow");
  settings.setValue("WindowState", (int)windowState());
  settings.setValue("WindowFlags", (int)windowFlags());
  settings.setValue("ShowMenuBar", qutty_config.mainwindow.menubar_visible);
  settings.setValue("ShowTabsInTitlebar", qutty_config.mainwindow.titlebar_tabs);
  if (!isMaximized()) {
    settings.setValue("Size", size());
    settings.setValue("Position", pos());
  }
  settings.endGroup();
}

int GuiMainWindow::setupLayout(GuiTerminalWindow *newTerm, GuiSplitter *splitter) {
  if (!splitter) {
    setupLayout(newTerm, GuiBase::TYPE_LEAF, -1);
  } else {
    splitter->addBaseWidget(-1, newTerm);
    newTerm->setFocus();
    terminalList.append(newTerm);
    on_tabLayoutChanged();
  }
  return 0;
}

int GuiMainWindow::setupLayout(GuiTerminalWindow *newTerm, GuiBase::SplitType split, int tabind) {
  // fallback to create tab
  if (tabArea->count() == 0) split = GuiBase::TYPE_LEAF;

  switch (split) {
    case GuiBase::TYPE_LEAF:
      newTerm->setParent(tabArea);
      this->tabInsert(tabind, newTerm, "");
      terminalList.append(newTerm);
      tabArea->setCurrentWidget(newTerm);
      newTerm->setWindowState(newTerm->windowState() | Qt::WindowMaximized);

      setupTerminalSize(newTerm);
      on_tabLayoutChanged();
      break;
    case GuiBase::TYPE_HORIZONTAL:
    case GuiBase::TYPE_VERTICAL: {
      GuiTerminalWindow *currTerm;
      if (!(currTerm = getCurrentTerminal())) goto err_exit;

      currTerm->createSplitLayout(split, newTerm);
      newTerm->setFocus();
      terminalList.append(newTerm);
      on_tabLayoutChanged();
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

void GuiMainWindow::setupTerminalSize(GuiTerminalWindow *newTerm) {
  // resize according to config if window is smaller
  if (!(windowState() & Qt::WindowMaximized) && (tabArea->count() == 1) /* only for 1st window */ &&
      (newTerm->viewport()->width() < newTerm->cfg.width * newTerm->getFontWidth() ||
       newTerm->viewport()->height() < newTerm->cfg.height * newTerm->getFontHeight())) {
    this->resize(
        newTerm->cfg.width * newTerm->getFontWidth() + width() - newTerm->viewport()->width(),
        newTerm->cfg.height * newTerm->getFontHeight() + height() - newTerm->viewport()->height());
    term_size(newTerm->term, newTerm->cfg.height, newTerm->cfg.width, newTerm->cfg.savelines);
  }
}

int GuiMainWindow::getTerminalTabInd(const QWidget *term) {
  auto it = tabIndexMap.find(term);
  if (it != tabIndexMap.end()) return it->second;
  return -1;
}

/*
 * Called whenever tab/pane layout changes
 */
void GuiMainWindow::on_tabLayoutChanged() {
  QWidget *w;
  GuiTerminalWindow *term;
  GuiSplitter *split;

  tabIndexMap.clear();
  widgetAtIndex.resize(tabArea->count());
  for (int i = 0; i < tabArea->count(); i++) {
    split = NULL;
    term = NULL;
    w = tabArea->widget(i);
    tabIndexMap[w] = i;
    if ((term = qobject_cast<GuiTerminalWindow *>(w))) {
      term->on_sessionTitleChange(true);
    } else if ((split = qobject_cast<GuiSplitter *>(w))) {
      vector<GuiTerminalWindow *> list;
      split->populateAllTerminals(&list);
      for (auto it = list.begin(); it - list.end(); ++it) {
        tabIndexMap[*it] = i;
        (*it)->on_sessionTitleChange(true);
      }
    }
    widgetAtIndex[i] = std::pair<GuiSplitter *, GuiTerminalWindow *>(split, term);
  }
}

void GuiToolButton::paintEvent(QPaintEvent *e) {
  QStyleOptionToolButton option;
  option.initFrom(this);
  QPainter painter(this);
  if (option.state & QStyle::State_MouseOver)
    painter.drawPixmap(e->rect(), QPixmap(":/images/menu-hover.png"));
  else
    painter.drawPixmap(e->rect(), QPixmap(":/images/drag.png"));
}

QSize GuiToolButton::sizeHint() const {
  QSize s1(QToolButton::sizeHint());
  QSize s2(QPixmap(":/images/drag.png").size());
  float f = float(s2.width()) / s2.height();
  return QSize(f * s1.width(), s1.height());
}
