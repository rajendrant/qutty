/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#ifndef GUIMENU_H
#define GUIMENU_H

#include <QString>
#include <QKeySequence>
#include <QToolButton>
#include <QToolBar>

class GuiMainWindow;
class GuiTerminalWindow;

enum qutty_menu_id_t {
    MENU_NEW_SESSION,
    MENU_RESTART_SESSION,
    MENU_DUPLICATE_SESSION,
    MENU_CHANGE_SETTINGS,
    MENU_PASTE,
    MENU_NEW_TAB,
    MENU_NEW_WINDOW,
    MENU_CLOSE_SESSION,
    MENU_SPLIT_HORIZONTAL,
    MENU_SPLIT_VERTICAL,
    MENU_SWITCH_LEFT_TAB,
    MENU_SWITCH_RIGHT_TAB,
    MENU_IMPORT_FILE,
    MENU_IMPORT_REGISTRY,
    MENU_EXPORT_FILE,
    MENU_EXIT,
    MENU_MENUBAR,
    MENU_FULLSCREEN,
    MENU_ALWAYSONTOP,
    MENU_PREFERENCES,
    MENU_TERM_TOP_CLOSE_PANE,
    MENU_TERM_TOP_DRAG_START,

    MENU_SEPARATOR,

    MENU_FILE,
    MENU_EDIT,
    MENU_VIEW,
    MENU_EXPORT_IMPORT,
    MENU_SAVED_SESSIONS,
    MENU_SPLIT_SESSION,
    MENU_TERM_WINDOW,
    MENU_TAB_BAR,
    MENU_ID_LAST
};

#define MENU_MAX_ACTION     (MENU_SEPARATOR)
#define MENU_MAX_MENU       (MENU_ID_LAST - MENU_SEPARATOR - 1)

struct qutty_menu_actions_t {
    const char *name;
    const char *key;
    const char *slot;
    const char *tooltip;
    int opt;
};

struct qutty_menu_links_t {
    const char *name;
    int len;
    qutty_menu_id_t links[20];
};

extern qutty_menu_actions_t qutty_menu_actions[];
extern qutty_menu_links_t qutty_menu_links[];


class GuiToolbarTerminalTop : QToolBar {
    QToolButton btns[3];
    bool menuVisible;
    bool initSizes;
    int totalWidth;
    int totalHeight;
public:
    GuiToolbarTerminalTop(GuiMainWindow *p);

    void processMouseMoveTerminalTop(GuiTerminalWindow *term, QMouseEvent *e);
};

#endif // GUIMENU_H
