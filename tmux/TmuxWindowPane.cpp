#include "tmux/TmuxWindowPane.h"
#include <iostream>
#include <sstream>

extern "C" void swap_screen(Terminal *, int, int, int);

TmuxWindowPane::TmuxWindowPane(TmuxGateway *gateway, GuiTerminalWindow *termwnd)
    : _gateway(gateway),
      _termWnd(termwnd),
      ready(false)
{
}

TmuxWindowPane::~TmuxWindowPane()
{
}

int TmuxWindowPane::performCallback(tmux_cb_index_t index, string &response)
{
    qDebug("%s %s %s", __FUNCTION__, get_tmux_cb_index_str(index), response.c_str());
    switch(index) {
    case CB_NULL:
        return 0;
    case CB_LIST_WINDOWS:
        return 0;
    case CB_DUMP_TERM_STATE:
        return resp_hdlr_dump_term_state(response);
    case CB_DUMP_HISTORY:
        return resp_hdlr_dump_history(response);
    case CB_DUMP_HISTORY_ALT:
        return resp_hdlr_dump_history(response, true);
    default:
        return -1;
    }
}

int TmuxWindowPane::resp_hdlr_dump_term_state(string &response)
{
    string key, tstr;
    int n;
    istringstream iresp(response);

    while (std::getline(iresp, key, '=')) {
        if (!key.compare("pane_id")) {
            char ch;
            iresp>>ch;
            if(ch != '%') {
                qCritical() << "Invalid value for key paneid";
                goto cu0;
            }
            iresp>>n;
            if(n != id) {
                qCritical() << "Invalid paneid" << id << n;
                goto cu0;
            }
        } else if (!key.compare("alternate_on")) {
            iresp>>n;
            _termWnd->term->alt_which = n;
        } else if (!key.compare("alternate_saved_x")) {
            iresp>>n;
            _termWnd->term->alt_x = n;
            _termWnd->term->alt_savecurs.x = n;
        } else if (!key.compare("alternate_saved_y")) {
            iresp>>n;
            _termWnd->term->alt_y = n;
            _termWnd->term->alt_savecurs.y = n;
        } else if (!key.compare("saved_cursor_x")) {
            iresp>>n;
            _termWnd->term->savecurs.x = n;
        } else if (!key.compare("saved_cursor_y")) {
            iresp>>n;
            _termWnd->term->savecurs.y = n;
        } else if (!key.compare("cursor_x")) {
            iresp>>n;
            _termWnd->term->curs.x = n;
        } else if (!key.compare("cursor_y")) {
            iresp>>n;
            _termWnd->term->curs.y = n;
        } else if (!key.compare("scroll_region_upper")) {
            iresp>>n;
            _termWnd->term->marg_t = n;
        } else if (!key.compare("scroll_region_lower")) {
            iresp>>n;
            _termWnd->term->marg_b = n;
        } else if (!key.compare("pane_tabs")) {
            string tabstops;
            iresp>>tabstops;
        } else if (!key.compare("cursor_flag")) {
            iresp>>n;
        } else if (!key.compare("insert_flag")) {
            iresp>>n;
            _termWnd->term->insert = n;
        } else if (!key.compare("keypad_cursor_flag")) {
            iresp>>n;
            _termWnd->term->app_cursor_keys = n;
        } else if (!key.compare("keypad_flag")) {
            iresp>>n;
            _termWnd->term->app_keypad_keys = n;
        } else if (!key.compare("wrap_flag")) {
            iresp>>n;
            _termWnd->term->wrap = n;
        } else if (!key.compare("mouse_standard_flag")) {
            iresp>>n;
        } else if (!key.compare("mouse_button_flag")) {
            iresp>>n;
        } else if (!key.compare("mouse_any_flag")) {
            iresp>>n;
        } else if (!key.compare("mouse_utf8_flag")) {
            iresp>>n;
        } else {
            qCritical()<<"Invalid key "<<key.c_str();
            goto cu0;
        }
        iresp.ignore(128, '\t');
    }
    ready = true;
    return 0;
cu0:
    qCritical()<<"Error in dump_term_state";
    return -1;
}

int TmuxWindowPane::resp_hdlr_dump_history(string &response, bool is_alt)
{
    Terminal *term = _termWnd->term;

    // make sure we start with clean slate
    if (!is_alt)
        assert(count234(term->scrollback) == 0);
    assert(count234(term->screen) == term->rows);
    assert(count234(term->alt_screen) == term->rows);

    if (response.length() > 1) {
        if (is_alt) {
            std::swap(term->screen, term->alt_screen);
            _termWnd->term->curs.x = 0;
            _termWnd->term->curs.y = 0;
        }
        _termWnd->from_backend(0, response.c_str(), response.length()-1);
        if (is_alt)
            std::swap(term->screen, term->alt_screen);
    }

    return 0;
}
