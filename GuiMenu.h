#ifndef GUIMENU_H
#define GUIMENU_H

#include <QString>
#include <QKeySequence>

#define QUTTY_MENU_COMMON_LIST \
    QUTTY_MENU_ENTRY(MENU_NEW_SESSION,       "New Session",         "Ctrl+Shift+T") \
    QUTTY_MENU_ENTRY(MENU_RESTART_SESSION,   "Restart Session",     "") \
    QUTTY_MENU_ENTRY(MENU_DUPLICATE_SESSION, "Duplicate Session",   "") \
    QUTTY_MENU_ENTRY(MENU_SAVED_SESSIONS,    "Saved Sessions",      "") \
    QUTTY_MENU_ENTRY(MENU_CHANGE_SETTINGS,   "Change Settings",     "") \
    QUTTY_MENU_ENTRY(MENU_PASTE,             "Paste",               "")\
    QUTTY_MENU_ENTRY(MENU_NEW_TAB,           "New Tab",             "Ctrl+Shift+T") \
    QUTTY_MENU_ENTRY(MENU_NEW_WINDOW,        "New Window",          "") \
    QUTTY_MENU_ENTRY(MENU_CLOSE_SESSION,     "Close",               "") \
    QUTTY_MENU_ENTRY(MENU_SPLIT_SESSION,     "Split Session",       "") \
    QUTTY_MENU_ENTRY(MENU_SPLIT_HORIZONTAL,  "Horizontally",        "") \
    QUTTY_MENU_ENTRY(MENU_SPLIT_VERTICAL,    "Vertically",          "") \
    QUTTY_MENU_ENTRY(MENU_SWITCH_LEFT_TAB,   "Switch to Left Tab",  "Shift+Right") \
    QUTTY_MENU_ENTRY(MENU_SWITCH_RIGHT_TAB,  "Switch to Right Tab", "Shift+Left") \
    QUTTY_MENU_ENTRY(MENU_MAX_ENTRY,         "",                    "")

enum qutty_menu_id_t {
#define QUTTY_MENU_ENTRY(id, str, key) id,
    QUTTY_MENU_COMMON_LIST
#undef QUTTY_MENU_ENTRY
};

extern QString qutty_menu_common_name[];
extern QKeySequence qutty_menu_common_shortcut[];

// helper for adding actions
#define QUTTY_MENU_ADD_ACTION(menu, id, recv, slot) \
    (menu)->addAction(qutty_menu_common_name[id], recv, SLOT(slot), qutty_menu_common_shortcut[id]);

#define QUTTY_MENU_ADD_ACTION2(menu, id) \
    (menu)->addAction(qutty_menu_common_name[id]);

#endif // GUIMENU_H
