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
    /*
     * These indexes will be saved in xml config file
     * Don't change the order of the entries
     * Insert new items only at the end
     */
    MENU_RESTART_SESSION,
    MENU_DUPLICATE_SESSION,
    MENU_CHANGE_SETTINGS,
    MENU_PASTE,
    MENU_NEW_TAB,
    MENU_NEW_WINDOW,
    MENU_CLOSE_SESSION,
    MENU_SPLIT_HORIZONTAL,
    MENU_SPLIT_VERTICAL,
    MENU_DUPLICATE_HSPLIT,
    MENU_DUPLICATE_VSPLIT,
    MENU_SWITCH_LEFT_TAB,
    MENU_SWITCH_RIGHT_TAB,
    MENU_SWITCH_UP_PANE,
    MENU_SWITCH_BOTTOM_PANE,
    MENU_SWITCH_LEFT_PANE,
    MENU_SWITCH_RIGHT_PANE,
    MENU_SWITCH_MRU_TAB,
    MENU_SWITCH_LRU_TAB,
    MENU_SWITCH_MRU_PANE,
    MENU_SWITCH_LRU_PANE,
    MENU_IMPORT_FILE,
    MENU_IMPORT_REGISTRY,
    MENU_EXPORT_FILE,
    MENU_EXIT,
    MENU_MENUBAR,
    MENU_FULLSCREEN,
    MENU_ALWAYSONTOP,
    MENU_PREFERENCES,
    MENU_RENAME_TAB,
    MENU_FIND,
    MENU_FIND_NEXT,
    MENU_FIND_PREVIOUS,
    MENU_FIND_CASE_INSENSITIVE,
    MENU_FIND_REGEX,
    MENU_FIND_HIGHLIGHT,
    MENU_TAB_IN_TITLE_BAR,

    /*
     * Insert any new actions before this comment
     */

    MENU_STATIC_ACTION_MAX,

    MENU_CUSTOM_OPEN_SAVED_SESSION      = 100000,
    MENU_CUSTOM_OPEN_SAVED_SESSION_END  = 200000,
    MENU_CUSTOM_ACTION_MAX,
    MENU_MAX_ACTION = MENU_CUSTOM_ACTION_MAX,

    MENU_SEPARATOR                      = 100000000,


    /*
     * These indexes will be saved in xml config file
     * Don't change the order of the entries
     * Insert new items only at the end
     */
    MENU_FILE,
    MENU_EDIT,
    MENU_VIEW,
    MENU_EXPORT_IMPORT,
    MENU_SAVED_SESSIONS,
    MENU_SPLIT_SESSION,
    MENU_TERM_WINDOW,
    MENU_TAB_BAR,
    MENU_FIND_OPTIONS,

    /*
     * Insert any new menus before this comment
     */

    MENU_ID_LAST
};

#define MENU_MAX_MENU       (MENU_ID_LAST - MENU_SEPARATOR - 1)

struct qutty_menu_actions_t {
    const char *name;
    const char *key;
    const char *slot;
    const char *tooltip;
};

struct qutty_menu_links_t {
    const char *name;
    int len;
    qutty_menu_id_t links[20];
};

extern qutty_menu_actions_t qutty_menu_actions[];
extern qutty_menu_links_t qutty_menu_links[];


class GuiToolbarTerminalTop : public QToolBar {
    Q_OBJECT

    enum {
        MENU_TERMTOP_MENU,
        MENU_TERMTOP_MOVE,
        MENU_TERMTOP_CLOSE,
        MENU_TERMTOP_MAX_SIZE
    };
    bool menuVisible;
    bool initSizes;
    int totalWidth;
    int totalHeight;
public:
    QToolButton btns[MENU_TERMTOP_MAX_SIZE];
    GuiToolbarTerminalTop(GuiMainWindow *p);
    void hideMe();
    void initializeToolbarTerminalTop(GuiMainWindow *p);
    void processMouseMoveTerminalTop(GuiTerminalWindow *term, QMouseEvent *e);
};

#endif // GUIMENU_H
