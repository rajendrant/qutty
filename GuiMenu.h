#ifndef GUIMENU_H
#define GUIMENU_H

#include <QString>
#include <QKeySequence>

#define QUTTY_MENU_ACTIONS \
    QUTTY_ENTRY(MENU_NEW_SESSION,       "New Session",         "Ctrl+Shift+T",  on_openNewTab) \
    QUTTY_ENTRY(MENU_RESTART_SESSION,   "Restart Session",     "",              contextMenuRestartSessionTriggered) \
    QUTTY_ENTRY(MENU_DUPLICATE_SESSION, "Duplicate Session",   "",              contextMenuDuplicateSessionTriggered) \
    QUTTY_ENTRY(MENU_CHANGE_SETTINGS,   "Change Settings",     "",              contextMenuChangeSettingsTriggered) \
    QUTTY_ENTRY(MENU_PASTE,             "Paste",               "",              contextMenuPaste)\
    QUTTY_ENTRY(MENU_NEW_TAB,           "New Tab",             "Ctrl+Shift+T",  on_openNewTab) \
    QUTTY_ENTRY(MENU_NEW_WINDOW,        "New Window",          "",              on_openNewWindow) \
    QUTTY_ENTRY(MENU_CLOSE_SESSION,     "Close",               "",              contextMenuCloseSessionTriggered) \
    QUTTY_ENTRY(MENU_SPLIT_HORIZONTAL,  "Horizontally",        "Ctrl+Shift+H",  "") \
    QUTTY_ENTRY(MENU_SPLIT_VERTICAL,    "Vertically",          "Ctrl+Shift+V",  "") \
    QUTTY_ENTRY(MENU_SWITCH_LEFT_TAB,   "Switch to Left Tab",  "Shift+Left",    tabPrev) \
    QUTTY_ENTRY(MENU_SWITCH_RIGHT_TAB,  "Switch to Right Tab", "Shift+Right",   tabNext) \
    QUTTY_ENTRY(MENU_IMPORT_FILE,       "Import from File",    "",              "") \
    QUTTY_ENTRY(MENU_IMPORT_REGISTRY,   "Import PuTTY sessions", "",            "") \
    QUTTY_ENTRY(MENU_EXPORT_FILE,       "Export from File",    "",              "") \
    QUTTY_ENTRY(MENU_EXIT,              "Exit",                "",              contextMenuCloseWindowTriggered) \
    QUTTY_ENTRY(MENU_MENUBAR,           "Show Menubar",        "",              contextMenuMenuBar) \
    QUTTY_ENTRY(MENU_FULLSCREEN,        "Fullscreen",          "",              contextMenuFullScreen) \
    QUTTY_ENTRY(MENU_ALWAYSONTOP,       "Always on top",       "",              contextMenuAlwaysOnTop) \
    QUTTY_ENTRY(MENU_PREFERENCES,       "Preferences",         "",              "")

enum qutty_menu_id_t {
#define QUTTY_ENTRY(id, str, key, slot) id,
    QUTTY_MENU_ACTIONS
#undef QUTTY_ENTRY
    MENU_SEPARATOR,
    MENU_FILE,
    MENU_EDIT,
    MENU_VIEW,
    MENU_EXPORT_IMPORT,
    MENU_SAVED_SESSIONS,
    MENU_SPLIT_SESSION
};

#define MENU_MAX_ACTION     20
#define MENU_MAX_MENU       6

struct qutty_menu_links_t {
    char *name;
    int len;
    qutty_menu_id_t links[20];
};
extern qutty_menu_links_t qutty_menu_links[];

#endif // GUIMENU_H
