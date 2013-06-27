/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include <QProxyStyle>
#include <QMouseEvent>
#include <QString>
#include <QAction>
#include <QMenuBar>
#include <QInputDialog>
#include "GuiMainWindow.h"
#include "GuiTerminalWindow.h"
#include "GuiSplitter.h"
#include "GuiTabWidget.h"
#include "GuiPreferencesWindow.h"

qutty_menu_actions_t qutty_menu_actions[MENU_STATIC_ACTION_MAX] = {
    { "Restart Session",        "",              SLOT( contextMenuRestartSessionTriggered() ),
      "Restart the currently active session/pane"},
    { "Duplicate Session",      "",              SLOT( contextMenuDuplicateSessionTriggered() ),
      "Duplicate the currently active session/pane to new Tab"},
    { "Change Settings",        "",              SLOT( contextMenuChangeSettingsTriggered() ),
      "Change settings of the currently active session"},
    { "Paste",                  "",              SLOT( contextMenuPaste() ),
      "Paste the clipboard into the currently active terminal"},
    { "New Tab",                "Ctrl+Shift+T",  SLOT( on_openNewTab() ),
      "Bring up settings window to open a session in new tab"},
    { "New Window",             "",              SLOT( on_openNewWindow() ),
      "Bring up settings window to open a session in new window"},
    { "Close",                  "",              SLOT( contextMenuCloseSessionTriggered() ),
      "Close currently active session/pane"},
    { "Horizontally",           "Ctrl+Shift+H",  SLOT( on_openNewSplitHorizontal() ),
      "Bring up settings window to open a session in Horizontal split"},
    { "Vertically",             "Ctrl+Shift+V",  SLOT( on_openNewSplitVertical() ),
      "Bring up settings window to open a session in Vertical split"},
    { "Duplicate to new HSplit", "",             SLOT( contextMenuOpenDuplicateHSplit() ),
      "Duplicate the currently active session/pane to Horizontal split-pane"},
    { "Duplicate to new VSplit", "",             SLOT( contextMenuOpenDuplicateVSplit() ),
      "Duplicate the currently active session/pane to Vertical split-pane"},
    { "Switch to Left Tab",     "Shift+Left",    SLOT( tabPrev() ),
      "Switch to the tab in the left"},
    { "Switch to Right Tab",    "Shift+Right",   SLOT( tabNext() ),
      "Switch to the tab in the right"},
    { "Switch to Top Pane",     "Ctrl+Shift+Up", SLOT( contextMenuPaneUp() ),
      "Switch to the split-pane that is above the current pane"},
    { "Switch to Bottom Pane",  "Ctrl+Shift+Down",SLOT( contextMenuPaneDown() ),
      "Switch to the split-pane that is below the current pane"},
    { "Switch to Left Pane",    "Ctrl+Shift+Left",SLOT( contextMenuPaneLeft() ),
      "Switch to the split-pane that is on the left of the current pane"},
    { "Switch to Right Pane",   "Ctrl+Shift+Right",SLOT( contextMenuPaneRight() ),
      "Switch to the split-pane that is above on the right of the current pane"},
    { "Switch to MRU Tab",      "Ctrl+Tab",      SLOT( contextMenuMRUTab() ),
      "Switch to the Tab that is most recently used"},
    { "Switch to LRU Tab",      "Ctrl+Shift+tab",  SLOT( contextMenuLRUTab() ),
      "Switch to the Tab that is least recently used"},
    { "Switch to MRU Pane",     "Ctrl+Shift+[",  SLOT( contextMenuMRUPane() ),
      "Switch to the split-pane that is most recently used"},
    { "Switch to LRU Pane",     "Ctrl+Shift+]",  SLOT( contextMenuLRUPane() ),
      "Switch to the split-pane that is least recently used"},
    { "Import from File",       "",              "",                                             ""},
    { "Import PuTTY sessions",  "",              "",                                             ""},
    { "Export from File",       "",              "",                                             ""},
    { "Exit",                   "",              SLOT( contextMenuCloseWindowTriggered() ),
      "Close the window"},
    { "Show Menubar",           "",              SLOT( contextMenuMenuBar() ),
      "Toggle viewing of menu bar"},
    { "Fullscreen",             "",              SLOT( contextMenuFullScreen() ),
      "Toggle fullscreen mode"},
    { "Always on top",          "",              SLOT( contextMenuAlwaysOnTop() ),
      "Toggle keeping QuTTY window on top"},
    { "Preferences",            "",              SLOT( contextMenuPreferences() ),
      "Bring up the 'Preferences' Dialog to configure options"},
    { "Rename Session",         "",              SLOT( contextMenuRenameTab() ),
      "Rename the currently active session/pane"},
    { "Find",                   "Ctrl+Shift+F",  SLOT( contextMenuFind() ),
      "Find text in the terminal"},
    { "Find Next",              "F3",            SLOT( contextMenuFindNext() ),
      ""},
    { "Find Previous",          "Shift+F3",      SLOT( contextMenuFindPrevious() ),
      ""},
    { "Case sensitive",         "",              "",                                             ""},
    { "Match regular expression","",             "",                                             ""},
    { "Highlight all matches",  "",              "",                                             ""},
};

qutty_menu_links_t qutty_menu_links[MENU_MAX_MENU] = {
{ "File", 10, {MENU_NEW_TAB, MENU_NEW_WINDOW, MENU_SEPARATOR, MENU_SAVED_SESSIONS, MENU_SEPARATOR,
               MENU_SPLIT_SESSION, MENU_SEPARATOR, MENU_EXPORT_IMPORT, MENU_SEPARATOR, MENU_EXIT} },
{ "Edit", 9,  {MENU_PASTE, MENU_SEPARATOR, MENU_RENAME_TAB, MENU_SEPARATOR,
               MENU_FIND, MENU_FIND_NEXT, MENU_FIND_PREVIOUS,
               MENU_SEPARATOR, MENU_PREFERENCES} },
{ "View", 9,  { MENU_SWITCH_LEFT_TAB, MENU_SWITCH_RIGHT_TAB, MENU_SEPARATOR,
                MENU_SWITCH_UP_PANE, MENU_SWITCH_BOTTOM_PANE, MENU_SEPARATOR,
                MENU_MENUBAR, MENU_ALWAYSONTOP, MENU_FULLSCREEN } },
{ "Export/Import Settings",     3,  { MENU_IMPORT_FILE, MENU_IMPORT_REGISTRY, MENU_EXPORT_FILE} },
{ "Saved Sessions",             0,  {} },
{ "Split Session",              2,  { MENU_SPLIT_HORIZONTAL, MENU_SPLIT_VERTICAL } },
{ "Menu Term Window",           10, { MENU_PASTE, MENU_SEPARATOR,
                                      MENU_NEW_TAB, MENU_RESTART_SESSION, MENU_DUPLICATE_SESSION,
                                      MENU_SAVED_SESSIONS, MENU_SPLIT_SESSION, MENU_CHANGE_SETTINGS,
                                      MENU_SEPARATOR, MENU_RENAME_TAB, MENU_SEPARATOR, MENU_CLOSE_SESSION} },
{ "Menu Tabbar",                12, { MENU_NEW_TAB, MENU_NEW_WINDOW, MENU_SEPARATOR,
                                      MENU_SAVED_SESSIONS, MENU_SEPARATOR, MENU_SPLIT_SESSION, MENU_SEPARATOR,
                                      MENU_EXPORT_IMPORT, MENU_SEPARATOR, MENU_VIEW, MENU_SEPARATOR,
                                      MENU_EDIT, MENU_SEPARATOR, MENU_EXIT } },
{ "Find Options",               3,  { MENU_FIND_CASE_INSENSITIVE, MENU_FIND_REGEX, MENU_FIND_HIGHLIGHT } },
};

class MyStyle : public QProxyStyle
{
    GuiMainWindow *mainWindow;
public:
    MyStyle(GuiMainWindow *wnd)
    {
        mainWindow = wnd;
    }

    QRect subElementRect ( SubElement element, const QStyleOption * option, const QWidget * widget = 0 ) const
    {
        QRect rc = QProxyStyle::subElementRect(element, option, widget);
        if (element == QStyle::SE_TabWidgetRightCorner) {
            // This is a very bad hack we are doing
            // Reference: http://www.qtcentre.org/threads/12539-QTabWidget-corner-widget-is-not-shown
            QWidget *wid = mainWindow->tabArea->cornerWidget();
            if (wid)
                rc.setSize(wid->size());
        }
        return rc;
    }
};

void GuiMainWindow::initializeMenuSystem()
{
    for(int i=0; i<MENU_STATIC_ACTION_MAX; i++) {
        if (!qutty_menu_actions[i].name)
            continue;
        QKeySequence keyseq(qutty_menu_actions[i].key);
        auto it = qutty_config.menu_action_list.find(i);
        if (it != qutty_config.menu_action_list.end())
            keyseq = it->second.shortcut;

        QAction *act = new QAction(qutty_menu_actions[i].name, this);
        act->setToolTip(qutty_menu_actions[i].tooltip);
        act->setShortcutContext(Qt::WidgetShortcut);
        connect(act, SIGNAL(triggered()), this, qutty_menu_actions[i].slot);

        QShortcut *shortcut = NULL;
        if (qutty_menu_actions[i].slot && qutty_menu_actions[i].slot[0] &&
            !keyseq.isEmpty()) {
            act->setShortcut(keyseq);
            shortcut = new QShortcut(QKeySequence(keyseq), this);
            shortcut->setContext(Qt::ApplicationShortcut);
            connect(shortcut, SIGNAL(activated()), this, qutty_menu_actions[i].slot);
        }
        // note that menuCommonShortcuts should be in sorted order
        menuCommonShortcuts.push_back(make_tuple(i, shortcut, act));
    }
    for(int i=0; i<MENU_MAX_MENU; i++) {
        menuCommonMenus[i].setTitle(qutty_menu_links[i].name);
        populateMenu(menuCommonMenus[i], qutty_menu_links[i].links, qutty_menu_links[i].len);
    }

    menuGetActionById(MENU_MENUBAR)->setCheckable(true);
    menuGetActionById(MENU_FULLSCREEN)->setCheckable(true);
    menuGetActionById(MENU_ALWAYSONTOP)->setCheckable(true);

    // setup main menubar
    menuBar()->addMenu(&menuCommonMenus[MENU_FILE-MENU_SEPARATOR-1]);
    menuBar()->addMenu(&menuCommonMenus[MENU_EDIT-MENU_SEPARATOR-1]);
    menuBar()->addMenu(&menuCommonMenus[MENU_VIEW-MENU_SEPARATOR-1]);
    menuGetActionById(MENU_MENUBAR)->setChecked(true);

    newTabToolButton.setArrowType(Qt::DownArrow);
    newTabToolButton.setMenu(menuGetMenuById(MENU_TAB_BAR));
    newTabToolButton.setPopupMode(QToolButton::InstantPopup);

    tabArea->setCornerWidget(&newTabToolButton, Qt::TopRightCorner);
    tabArea->setStyle(new MyStyle(this));   // TODO MEMLEAK

    connect(&qutty_config, SIGNAL(savedSessionsChanged()), this, SLOT(contextMenuSavedSessionsChanged()));
    this->contextMenuSavedSessionsChanged();

    // find-next, find-previous are disabled by default
    menuGetActionById(MENU_FIND_NEXT)->setEnabled(false);
    menuGetActionById(MENU_FIND_PREVIOUS)->setEnabled(false);
    menuGetActionById(MENU_FIND_CASE_INSENSITIVE)->setCheckable(true);
    menuGetActionById(MENU_FIND_HIGHLIGHT)->setCheckable(true);
    menuGetActionById(MENU_FIND_REGEX)->setCheckable(true);

    menuCustomSavedSessionSigMapper = new QSignalMapper(this);
    connect(menuCustomSavedSessionSigMapper, SIGNAL(mapped(int)), SLOT(contextMenuCustomSavedSession(int)));
    initializeCustomSavedSessionShortcuts();
}


QAction * GuiMainWindow::menuGetActionById(qutty_menu_id_t id)
{
    auto it = std::lower_bound(menuCommonShortcuts.begin(),
                               menuCommonShortcuts.end(),
                               std::make_tuple(id, (QShortcut*)NULL, (QAction*)NULL));
    if (it != menuCommonShortcuts.end() && std::get<0>(*it) == id)
        return std::get<2>(*it);
    return NULL;
}

void GuiMainWindow::menuSetShortcutById(qutty_menu_id_t id, QKeySequence key)
{
    auto it = std::lower_bound(menuCommonShortcuts.begin(),
                               menuCommonShortcuts.end(),
                               std::make_tuple(id, (QShortcut*)NULL, (QAction*)NULL));
    if (it != menuCommonShortcuts.end() && std::get<0>(*it) == id) {
        if (!std::get<1>(*it)) {
            auto shortcut = new QShortcut(QKeySequence(key), this);
            shortcut->setContext(Qt::ApplicationShortcut);
            connect(shortcut, SIGNAL(activated()), this, qutty_menu_actions[id].slot);
            std::get<1>(*it) = shortcut;
        }
        std::get<1>(*it)->setKey(key);
        std::get<2>(*it)->setShortcut(key);
    }
}

QKeySequence GuiMainWindow::menuGetShortcutById(qutty_menu_id_t id)
{
    auto it = std::lower_bound(menuCommonShortcuts.begin(),
                               menuCommonShortcuts.end(),
                               std::make_tuple(id, (QShortcut*)NULL, (QAction*)NULL));
    if (   it != menuCommonShortcuts.end() && std::get<0>(*it) == id
        && std::get<1>(*it))
            return std::get<1>(*it)->key();
    return QKeySequence();
}

void GuiMainWindow::initializeCustomSavedSessionShortcuts()
{
    for(auto it = menuCommonShortcuts.begin();
        it != menuCommonShortcuts.end(); ) {
        if (std::get<0>(*it) >= MENU_CUSTOM_OPEN_SAVED_SESSION &&
            std::get<0>(*it) < MENU_CUSTOM_OPEN_SAVED_SESSION_END ) {
            if (std::get<1>(*it))
                delete std::get<1>(*it);
            if (std::get<2>(*it))
                delete std::get<2>(*it);
            it = menuCommonShortcuts.erase(it);
        } else
            ++it;
    }
    auto it_begin = qutty_config.menu_action_list.lower_bound(MENU_CUSTOM_OPEN_SAVED_SESSION);
    auto it_end = qutty_config.menu_action_list.upper_bound(MENU_CUSTOM_OPEN_SAVED_SESSION_END);
    for(auto it = it_begin; it != it_end; ++it) {
        QShortcut *shortcut = new QShortcut(it->second.shortcut, this);
        shortcut->setContext(Qt::ApplicationShortcut);
        connect(shortcut, SIGNAL(activated()), menuCustomSavedSessionSigMapper, SLOT(map()));
        menuCustomSavedSessionSigMapper->setMapping(shortcut, it->first);
        // menuCommonShortcuts should be in sorted order
        menuCommonShortcuts.push_back(make_tuple((int32_t)it->first, shortcut, (QAction*)NULL));
    }
    // expect sort to be O(n) in already (partially) sorted vector
    std::sort(menuCommonShortcuts.begin(), menuCommonShortcuts.end());
}

void GuiMainWindow::populateMenu(QMenu &menu, qutty_menu_id_t menu_list[], int len)
{
    for (int i=0; i < len; i++) {
        if (menu_list[i] < MENU_SEPARATOR) {
            menu.addAction(menuGetActionById(menu_list[i]));
        } else if (menu_list[i] > MENU_SEPARATOR) {
            menu.addMenu(&menuCommonMenus[menu_list[i] - MENU_SEPARATOR - 1]);
        } else {
            menu.addSeparator();
        }
    }
}

void GuiTerminalWindow::showContextMenu(QMouseEvent *e)
{
    // handling ctrl + right-click on terminal
    if (e->pos().isNull())
        return;
    qutty_menu_id_t id;
    if (this->isSockDisconnected) {
        id = MENU_DUPLICATE_SESSION;
    } else {
        id = MENU_RESTART_SESSION;
    }
    this->mainWindow->menuCookieTermWnd = this;
    mainWindow->menuGetActionById(id)->setVisible(false);
    this->mainWindow->menuGetMenuById(MENU_TERM_WINDOW)->exec(e->globalPos());
    mainWindow->menuGetActionById(id)->setVisible(true);
    this->mainWindow->menuCookieTermWnd = NULL;
}

void GuiMainWindow::contextMenuSavedSessionsChanged()
{
    QMenu *menuSavedSessions = &menuCommonMenus[MENU_SAVED_SESSIONS - MENU_SEPARATOR - 1];
    if (!menuSavedSessions)
        return;
    menuSavedSessions->clear();
    for(auto it = qutty_config.config_list.begin();
        it != qutty_config.config_list.end(); it++) {
        if (it->first == QUTTY_DEFAULT_CONFIG_SETTINGS)
            continue;
        menuSavedSessions->addAction(it->first.c_str(), this, SLOT(contextMenuSavedSessionTriggered()));
    }
}

void GuiMainWindow::contextMenuSavedSessionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    char sessname[100];
    qstring_to_char(sessname, action->text(), sizeof(sessname));
    if (qutty_config.config_list.find(sessname) == qutty_config.config_list.end())
        return;
    this->createNewTab(&qutty_config.config_list[sessname]);
}

void GuiMainWindow::contextMenuPaste()
{
    GuiTerminalWindow *term = menuCookieTermWnd;
    if ((term || (term = this->getCurrentTerminal())) &&
        terminalList.indexOf(term)==-1)
        return;
    term->requestPaste();
}

void GuiMainWindow::contextMenuDuplicateSessionTriggered()
{
    GuiTerminalWindow *term = menuCookieTermWnd;
    if ((term || (term = this->getCurrentTerminal())) &&
        terminalList.indexOf(term)==-1)
        return;
    this->createNewTab(&term->cfg);
}

void GuiMainWindow::contextMenuOpenDuplicateHSplit()
{
    GuiTerminalWindow *term = menuCookieTermWnd;
    if ((term || (term = this->getCurrentTerminal())) &&
        terminalList.indexOf(term)==-1)
        return;
    this->on_createNewSession(term->cfg, GuiBase::TYPE_HORIZONTAL);
}

void GuiMainWindow::contextMenuOpenDuplicateVSplit()
{
    GuiTerminalWindow *term = menuCookieTermWnd;
    if ((term || (term = this->getCurrentTerminal())) &&
        terminalList.indexOf(term)==-1)
        return;
    this->on_createNewSession(term->cfg, GuiBase::TYPE_VERTICAL);
}

void GuiMainWindow::contextMenuRestartSessionTriggered()
{
    GuiTerminalWindow *term = menuCookieTermWnd;
    if ((term || (term = this->getCurrentTerminal())) &&
        terminalList.indexOf(term)==-1)
        return;
    term->restartTerminal();
}

void GuiMainWindow::contextMenuChangeSettingsTriggered()
{
    GuiTerminalWindow *term = menuCookieTermWnd;
    if ((term || (term = this->getCurrentTerminal())) &&
        terminalList.indexOf(term)==-1)
        return;
    on_changeSettingsTab(term);
}

void GuiMainWindow::contextMenuTermTopCloseTriggered()
{
    GuiTerminalWindow *term = menuCookieTermWnd;
    if ((term || (term = this->getCurrentTerminal())) &&
        terminalList.indexOf(term)==-1)
        return;
    toolBarTerminalTop.hideMe();
    term->reqCloseTerminal(false);
}

void GuiMainWindow::contextMenuCloseSessionTriggered()
{
    GuiTerminalWindow *term = menuCookieTermWnd;
    if ((term || (term = this->getCurrentTerminal())) &&
        terminalList.indexOf(term)==-1)
        return;
    term->reqCloseTerminal(false);
}

void GuiMainWindow::contextMenuCloseWindowTriggered()
{
    this->close();
}

void GuiMainWindow::contextMenuMenuBar()
{
    if (qobject_cast<QShortcut*>(sender()))  // if invoked from keyboard-shortcut
        menuGetActionById(MENU_MENUBAR)->toggle();
    if(menuGetActionById(MENU_MENUBAR)->isChecked()) {
        menuBar()->show();
    } else {
        menuBar()->hide();
    }
}

void GuiMainWindow::contextMenuFullScreen()
{
    if (qobject_cast<QShortcut*>(sender()))  // if invoked from keyboard-shortcut
        menuGetActionById(MENU_FULLSCREEN)->toggle();
    if(menuGetActionById(MENU_FULLSCREEN)->isChecked()) {
        setWindowState(windowState() | Qt::WindowFullScreen);
    } else {
        setWindowState(windowState() ^ Qt::WindowFullScreen);
    }
}

void GuiMainWindow::contextMenuAlwaysOnTop()
{
    if (qobject_cast<QShortcut*>(sender()))  // if invoked from keyboard-shortcut
        menuGetActionById(MENU_ALWAYSONTOP)->toggle();
    if(menuGetActionById(MENU_ALWAYSONTOP)->isChecked()) {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    } else {
        setWindowFlags(windowFlags() ^ Qt::WindowStaysOnTopHint);
    }
    this->show();
}

void GuiMainWindow::contextMenuTermTopDragPaneTriggered()
{
    if (!menuCookieTermWnd)
        return;
    if (terminalList.indexOf(menuCookieTermWnd) == -1)
        return;
    // the toolbutton stays in up/clicked state if we start dragging
    // trick to perform a complete click, so that the image goes to normal
    GuiTerminalWindow *term = menuCookieTermWnd;
    menuCookieTermWnd = NULL;
    toolBarTerminalTop.btns[1].animateClick(0);

    toolBarTerminalTop.hideMe();
    term->dragStartEvent(NULL);
}

GuiToolbarTerminalTop::GuiToolbarTerminalTop(GuiMainWindow *p)
    : QToolBar(p),
      menuVisible(false),
      initSizes(false)
{
}

void GuiToolbarTerminalTop::initializeToolbarTerminalTop(GuiMainWindow *p)
{
    btns[MENU_TERMTOP_MENU].setIcon(QIcon(":/images/cog_alt_16x16.png"));
    btns[MENU_TERMTOP_MOVE].setIcon(QIcon(":/images/move_16x16.png"));
    btns[MENU_TERMTOP_CLOSE].setIcon(QIcon(":/images/x_14x14.png"));
    btns[MENU_TERMTOP_MENU].setMenu(p->menuGetMenuById(MENU_TERM_WINDOW));
    btns[MENU_TERMTOP_MENU].setPopupMode(QToolButton::InstantPopup);
    btns[MENU_TERMTOP_MENU].setToolTip(tr("Menu"));
    btns[MENU_TERMTOP_MOVE].setToolTip(tr("Click and start dragging this pane to some other pane"));
    btns[MENU_TERMTOP_CLOSE].setToolTip(tr("Close this pane"));

    addWidget(&btns[MENU_TERMTOP_MENU]);
    addWidget(&btns[MENU_TERMTOP_MOVE]);
    addWidget(&btns[MENU_TERMTOP_CLOSE]);

    connect(&btns[MENU_TERMTOP_MOVE], SIGNAL(pressed()), p, SLOT(contextMenuTermTopDragPaneTriggered()));
    connect(&btns[MENU_TERMTOP_CLOSE], SIGNAL(clicked()), p, SLOT(contextMenuTermTopCloseTriggered()));

    setParent(NULL);
    setIconSize(QSize(16, 16));
    setAutoFillBackground(true);
    totalWidth = INT_MAX;
    totalHeight = INT_MAX;
}

void GuiToolbarTerminalTop::hideMe()
{
    setParent(NULL);
    hide();
    menuVisible = false;
}

void GuiToolbarTerminalTop::processMouseMoveTerminalTop(GuiTerminalWindow *term, QMouseEvent *e)
{
    if ( (!term->parentSplit || (e->y() > totalHeight ||
          (e->x() < term->viewport()->width() - totalWidth))) &&
         !menuVisible)
        return;     // fast return

    if (!initSizes) {
        // need to update width/height
        setParent(term);
        show();
        move(0, 0);
        totalWidth = width();
        totalHeight = height();
        qDebug() << size() << minimumSize() << minimumSizeHint();
        initSizes = true;
        // be sure to remove parent from terminal
        hideMe();
    }
    bool toShow = (e->x() >= term->viewport()->width() - totalWidth) &&
                  (e->y() <= totalHeight);
    if (toShow && !menuVisible) {
        term->getMainWindow()->menuCookieTermWnd = term;
        setParent(term);
        show();
        move(term->viewport()->width() - totalWidth, 0);
        totalWidth = width();
        totalHeight = height();
        menuVisible = true;
    } else if (!toShow && menuVisible) {
        term->getMainWindow()->menuCookieTermWnd = NULL;
        hideMe();
    }
}

void GuiMainWindow::contextMenuPreferences()
{
    GuiPreferencesWindow *pref = new GuiPreferencesWindow(this);
    pref->show();
}

void GuiMainWindow::contextMenuRenameTab()
{
    GuiTerminalWindow *term = menuCookieTermWnd;
    if ((term || (term = this->getCurrentTerminal())) &&
        terminalList.indexOf(term)==-1)
        return;
    bool ok;
    QString text = QInputDialog::getText(this, tr("Rename Session"),
                                         tr("Session name:"), QLineEdit::Normal,
                                         term->getCustomSessionTitle(),
                                         &ok);
    if (ok && !text.isEmpty())
        term->setCustomSessionTitle(text);
}

void GuiMainWindow::contextMenuCustomSavedSession(int ind)
{
    auto it = qutty_config.menu_action_list.find(ind);
    if (it == qutty_config.menu_action_list.end())
        return;
    char sessname[100];
    qstring_to_char(sessname, it->second.str_data, sizeof(sessname));
    auto it_cfg = qutty_config.config_list.find(sessname);
    if (it_cfg == qutty_config.config_list.end())
        return;
    this->on_createNewSession(it_cfg->second, GuiBase::SplitType(it->second.int_data));
}
