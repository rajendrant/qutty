#include "tmux/TmuxWindowPane.h"
#include <iostream>
#include <sstream>

extern "C" void swap_screen(Terminal *, int, int, int);

TmuxWindowPane::TmuxWindowPane(TmuxGateway *gateway, GuiTerminalWindow *termwnd)
    : _gateway(gateway),
      _termWnd(termwnd),
      old_state(NULL)
{
}

TmuxWindowPane::~TmuxWindowPane()
{
    if (old_state)
        delete old_state;
}

int TmuxWindowPane::performCallback(tmux_cb_index_t index, string &response)
{
    qDebug("TMUX resp %d %s", index, response.c_str());
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
        return 0;
    default:
        return -1;
    }
}

int TmuxWindowPane::resp_hdlr_dump_term_state(string &response)
{
    string key;
    istringstream iresp(response);

    old_state = new tmux_old_state_t;
    memset(old_state, 0, sizeof(tmux_old_state_t));
    while (std::getline(iresp, key, '=')) {
        if (!key.compare("in_alternate_screen")) {
            iresp>>old_state->in_alt_screen;
        } else if (!key.compare("base_cursor_x")) {
            iresp>>old_state->base_cursor_x;
        } else if (!key.compare("base_cursor_y")) {
            iresp>>old_state->base_cursor_y;
        } else if (!key.compare("cursor_x")) {
            iresp>>old_state->cursor_x;
        } else if (!key.compare("cursor_y")) {
            iresp>>old_state->cursor_y;
        } else if (!key.compare("scroll_region_upper")) {
            iresp>>old_state->scroll_region_upper;
        } else if (!key.compare("scroll_region_lower")) {
            iresp>>old_state->scroll_region_lower;
        } else if (!key.compare("decsc_cursor_x")) {
            iresp>>old_state->decsc_cursor_x;
        } else if (!key.compare("decsc_cursor_y")) {
            iresp>>old_state->decsc_cursor_y;
        } else if (!key.compare("tabstops")) {
            iresp>>old_state->tabstops;
        }
    }
    if (old_state->in_alt_screen) {
        _termWnd->term->curs.x = old_state->base_cursor_x;
        _termWnd->term->curs.y = old_state->base_cursor_y;
        swap_screen(_termWnd->term, TRUE, TRUE, TRUE);
    }
    _termWnd->term->curs.x = old_state->cursor_x;
    _termWnd->term->curs.y = old_state->cursor_y;
    _termWnd->term->marg_t = old_state->scroll_region_upper;
    _termWnd->term->marg_b = old_state->scroll_region_lower;
    return 0;
cu0:
    delete old_state;
    old_state = NULL;
    return -1;
}

int TmuxWindowPane::resp_hdlr_dump_history(string &response)
{
    ostringstream fail_reason;
    istringstream iresp(response);
    string line, hist;
    while (std::getline(iresp, line)) {
        struct termchar tchar;
        bool softeol = false;
        char c;
        bool is_utf8 = false;
        string utf8str;
        if (line.length() > 0 && line.at(line.length()-1) == '+') {
            line.erase(line.length()-1);
            softeol = true;
        }
        istringstream iss(line);
        while ( iss>>c ) {
            if (c == ':') {
                if (is_utf8) {
                    fail_reason << "Malformed text no closing ']'";
                    goto cu0;
                }
                int values[4];
                for (int j = 0; j < 4; j++) {
                    char tmp;
                    iss>>std::hex>>values[j]>>tmp;
                    if (tmp != ',') {
                        fail_reason << "Malformed text after ':'' no ','";
                        goto cu0;
                    }
                }
                term_data(_termWnd->term, 0, hist.c_str(), hist.length());
                hist.clear();

                int attr, fg, bg;
                attr = values[0];
                fg = values[2]==8 ? ATTR_DEFFG : values[2]<<ATTR_FGSHIFT;
                bg = values[3]==8 ? ATTR_DEFBG : values[3]<<ATTR_BGSHIFT;
                _termWnd->term->curr_attr = 0;
                _termWnd->term->curr_attr &= ~ATTR_FGMASK;
                _termWnd->term->curr_attr |= fg;
                _termWnd->term->curr_attr &= ~ATTR_BGMASK;
                _termWnd->term->curr_attr |= bg;

                if (attr & GRID_ATTR_BRIGHT)
                    _termWnd->term->curr_attr |= ATTR_BOLD;
                if (attr & GRID_ATTR_UNDERSCORE)
                    _termWnd->term->curr_attr |= ATTR_UNDER;
                if (attr & GRID_ATTR_BLINK)
                    _termWnd->term->curr_attr |= ATTR_BLINK;
                if (attr & GRID_ATTR_REVERSE)
                    _termWnd->term->curr_attr |= ATTR_REVERSE;

                tchar.attr = 0;
                qDebug()<<"attr, flags, fg, bg"<<values[0]<<values[1]<<values[2]<<values[3];
            } else if (c == '*') {
                if (is_utf8) {
                    fail_reason << "Malformed text no closing ']'";
                    goto cu0;
                }
                // We have a "*<number> " sequence.
                int repeats;
                char c2;
                iss>>std::dec>>std::noskipws>>repeats>>c2;
                if (c2 != ' ') {
                    fail_reason << "lacks a space after *n line:"<<line<<" pos:"<<iss.tellg();
                    goto cu0;
                }
                if (!iss) {
                    fail_reason << "lacks a number after * line:"<<line<<" pos:"<<iss.tellg();
                    goto cu0;
                }
                qDebug("repeats %d %x\n", repeats, tchar.chr);
                for (int j = 0; j < repeats - 1; j++) {
                    hist.push_back((char)tchar.chr);
                }
            } else if (c == '[') {
                if (iss.rdbuf()->in_avail() < 3) {
                    fail_reason << "Malformed text after '[' in history";
                    goto cu0;
                }
                is_utf8 = true;
                utf8str.clear();
            } else if (c == ']') {
                if (!is_utf8) {
                    fail_reason << "']' without '[' in history";
                    goto cu0;
                }
                is_utf8 = false;
            } else if (is_hex_char(c) && is_hex_char((char)iss.peek())) {
                char c2;
                iss>>c2;
                c2 = hex_to_char(c)<<4 | hex_to_char(c2);
                if (is_utf8) {
                    utf8str.push_back(c2);
                } else {
                    tchar.chr = c2;
                }
                hist.push_back(c2);
            } else {
                fail_reason << "char not expected "<<c<<" line:"<<line<<" pos:"<<iss.tellg();
                goto cu0;
            }
        }
        qDebug()<<"history"<<hist.c_str();
        term_data(_termWnd->term, 0, hist.c_str(), hist.length());
        term_data(_termWnd->term, 0, "\r\n", 2);
        hist.clear();
    }
    return 0;
cu0:
    qDebug("Malformed history dump response reason: %s\n", fail_reason.str().c_str());
    return -1;
}
