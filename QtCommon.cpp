/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include "QtCommon.h"
#include "GuiMainWindow.h"
#include "GuiTerminalWindow.h"
#include <QKeyEvent>
extern "C" {
#include "putty.h"
#include "terminal.h"
#include "ssh.h"
}
#include "windows.h"

using namespace Qt;

char sshver[] = "PuTTY-Local: " __DATE__ " " __TIME__;

QTimer *qtimer = NULL;
long timing_next_time;

void timer_change_notify(long next)
{
    long ticks = next - GetTickCount();
    if (ticks <= 0) ticks = 1;	       /* just in case */
    if(!qtimer) {
        qtimer = new QTimer(mainWindow);
        QObject::connect(qtimer, SIGNAL(timeout()), mainWindow, SLOT(timerHandler()));
        qtimer->setSingleShot(true);
    }
    qtimer->start(ticks);
    timing_next_time = next;
}

int get_remote_username(Config *cfg, char *user, size_t len) {
    if (*cfg->username) {
        strncpy(user, cfg->username, len);
        user[len-1] = '\0';
    } else {
        // TODO
        *user = '\0';
    }
    return (*user != '\0');
}

int GuiTerminalWindow::TranslateKey(QKeyEvent *keyevent, char *output)
{
    Config *cfg = &term->cfg;
    char *p = output;
    int keystate = keyevent->modifiers();
    int ctrlshiftstate = keystate & (Qt::ControlModifier|Qt::ShiftModifier);
    int ctrlstate = keystate & Qt::ControlModifier;
    int shiftstate = keystate & Qt::ShiftModifier;
    int key = keyevent->key();

    /* Disable Auto repeat if required */
    if (term->repeat_off && keyevent->isAutoRepeat())
        return 0;

    // optimization - handle it as a normal unicode key
    if (keystate==Qt::NoModifier || keystate==Qt::ShiftModifier)
        if (key>=Qt::Key_Space && key<=Qt::Key_AsciiTilde)
            return -1;

    // Treat Alt by just inserting an Esc before everything else
    if (keystate & Qt::AltModifier){
        *p++ = 0x1b;
    }

    // First some basic control characters
    if ( (key == Qt::Key_Backspace) && (ctrlshiftstate == 0) ) {
        // Backspace
        *p++ = (char) (cfg->bksp_is_delete ? 0x7F : 0x08);
        *p++ = 0;
        return -2;
    }
    if ( (key == Qt::Key_Backspace) && (ctrlshiftstate == Qt::ShiftModifier) ) {
        // Shift-Backspace
        *p++ = (char) (cfg->bksp_is_delete ? 0x08 : 0x7F);
        *p++ = 0;
        return -2;
    }
    if ( (key == Qt::Key_Tab) && (ctrlshiftstate == Qt::ShiftModifier) ) {
        // Shift-Tab
        *p++ = 0x1B;
        *p++ = '[';
        *p++ = 'Z';
        return p - output;
    }
    if ( (key == Qt::Key_Space) && (ctrlshiftstate & Qt::ControlModifier) ) {
        // Ctrl-Space OR Ctrl-Shift-Space
        *p++ = (ctrlshiftstate & Qt::ShiftModifier) ? 160 : 0;
        return p - output;
    }
    if ( (ctrlshiftstate == Qt::ControlModifier) && (key >= Qt::Key_2) && (key <= Qt::Key_8) ) {
        // Ctrl-2 through Ctrl-8
        *p++ = "\000\033\034\035\036\037\177"[key - Qt::Key_2];
        return p - output;
    }
    if ( (key == Qt::Key_Enter) && term->cr_lf_return ) {
        // Enter (if it should send CR + LF)
        *p++ = '\r';
        *p++ = '\n';
        return p - output;
    }
    if ( (ctrlshiftstate == Qt::ControlModifier) && (key >= Qt::Key_A) && (key <= Qt::Key_Z) ) {
        // Ctrl-a through Ctrl-z, when sent with a modifier
        *p++ = (char) (key - Qt::Key_A + 1);
        return p - output;
    }
    if ( (ctrlshiftstate & Qt::ControlModifier) && (key >= Qt::Key_BracketLeft) && (key <= Qt::Key_Underscore) ) {
        // Ctrl-[ through Ctrl-_
        *p++ = (char) (key - Qt::Key_BracketLeft + 27);
        return p - output;
    }
    if (ctrlshiftstate==0 && key==Qt::Key_Return) {
        // Enter
        *p++ = 0x0D;
        *p++ = 0;
        return -2;
   }

    // page-up page-down with ctrl/shift
    if (key==Key_PageUp && ctrlshiftstate) {
        if (ctrlstate && shiftstate) { // ctrl + shift + page-up
            term_scroll_to_selection(term, 0);
        } else if (ctrlstate) { // ctrl + page-up
            term_scroll(term, 0, -1);
        } else { // shift + page-up
            term_scroll(term, 0, -term->rows / 2);
        }
        return 0;
    }
    if (key==Key_PageDown && ctrlshiftstate) {
        if (ctrlstate && shiftstate) { // ctrl + shift + page-down
            term_scroll_to_selection(term, 1);
        } else if (ctrlstate) { // ctrl + page-down
            term_scroll(term, 0, +1);
        } else { // shift + page-down
            term_scroll(term, 0, +term->rows / 2);
        }
        return 0;
    }

    // shift-insert -> paste
    if (key==Key_Insert && ctrlshiftstate==ShiftModifier) {
        this->requestPaste();
        return 0;
    }

    // Arrows
    char xkey = 0;
    switch ( key ) {
        case Qt::Key_Up:
            xkey = 'A';
            break;
        case Qt::Key_Down:
            xkey = 'B';
            break;
        case Qt::Key_Right:
            xkey = 'C';
            break;
        case Qt::Key_Left:
            xkey = 'D';
            break;
        default: // keep gcc happy
            break;
    }
    if ( xkey ) {
        if (term->vt52_mode)
            p += sprintf((char *) p, "\x1B%c", xkey);
        else {
            int app_flg = (term->app_cursor_keys && !cfg->no_applic_c);

            /* Useful mapping of Ctrl-arrows */
            if ( ctrlshiftstate == Qt::ControlModifier )
                app_flg = !app_flg;

            if (app_flg)
                p += sprintf((char *) p, "\x1BO%c", xkey);
            else
                p += sprintf((char *) p, "\x1B[%c", xkey);
        }
        return p - output;
    }

    /*
     * Next, all the keys that do tilde codes. (ESC '[' nn '~',
     * for integer decimal nn.)
     *
     * We also deal with the weird ones here. Linux VCs replace F1
     * to F5 by ESC [ [ A to ESC [ [ E. rxvt doesn't do _that_, but
     * does replace Home and End (1~ and 4~) by ESC [ H and ESC O w
     * respectively.
     */
    int code = 0;
    switch (key) {
      case Key_F1:
        code = (shiftstate ? 23 : 11);
        break;
      case Key_F2:
        code = (shiftstate ? 24 : 12);
        break;
      case Key_F3:
        code = (shiftstate ? 25 : 13);
        break;
      case Key_F4:
        code = (shiftstate ? 26 : 14);
        break;
      case Key_F5:
        code = (shiftstate ? 28 : 15);
        break;
      case Key_F6:
        code = (shiftstate ? 29 : 17);
        break;
      case Key_F7:
        code = (shiftstate ? 31 : 18);
        break;
      case Key_F8:
        code = (shiftstate ? 32 : 19);
        break;
      case Key_F9:
        code = (shiftstate ? 33 : 20);
        break;
      case Key_F10:
        code = (shiftstate ? 34 : 21);
        break;
      case Key_F11:
        code = 23;
        break;
      case Key_F12:
        code = 24;
        break;
      case Key_F13:
        code = 25;
        break;
      case Key_F14:
        code = 26;
        break;
      case Key_F15:
        code = 28;
        break;
      case Key_F16:
        code = 29;
        break;
      case Key_F17:
        code = 31;
        break;
      case Key_F18:
        code = 32;
        break;
      case Key_F19:
        code = 33;
        break;
      case Key_F20:
        code = 34;
        break;
    }
    if (ctrlstate==0) switch (key) {
      case Key_Home:
        code = 1;
        break;
      case Key_Insert:
        code = 2;
        break;
      case Key_Delete:
        code = 3;
        break;
      case Key_End:
        code = 4;
        break;
      case Key_PageUp:
        code = 5;
        break;
      case Key_PageDown:
        code = 6;
        break;
    }
    /* Reorder edit keys to physical order */
    if (cfg->funky_type == FUNKY_VT400 && code <= 6)
        code = "\0\2\1\4\5\3\6"[code];

    if (term->vt52_mode && code > 0 && code <= 6) {
        p += sprintf((char *) p, "\x1B%c", " HLMEIG"[code]);
        return p - output;
    }

    if (cfg->funky_type == FUNKY_SCO &&     /* SCO function keys */
        code >= 11 && code <= 34) {
        char codes[] = "MNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz@[\\]^_`{";
        int index = 0;
        switch (key) {
          case Key_F1: index = 0; break;
          case Key_F2: index = 1; break;
          case Key_F3: index = 2; break;
          case Key_F4: index = 3; break;
          case Key_F5: index = 4; break;
          case Key_F6: index = 5; break;
          case Key_F7: index = 6; break;
          case Key_F8: index = 7; break;
          case Key_F9: index = 8; break;
          case Key_F10: index = 9; break;
          case Key_F11: index = 10; break;
          case Key_F12: index = 11; break;
        }
        if (shiftstate) index += 12;
        if (ctrlstate) index += 24;
        p += sprintf((char *) p, "\x1B[%c", codes[index]);
        return p - output;
    }
    if (cfg->funky_type == FUNKY_SCO &&     /* SCO small keypad */
        code >= 1 && code <= 6) {
        char codes[] = "HL.FIG";
        if (code == 3) {
        *p++ = '\x7F';
        } else {
        p += sprintf((char *) p, "\x1B[%c", codes[code-1]);
        }
        return p - output;
    }
    if ((term->vt52_mode || cfg->funky_type == FUNKY_VT100P) && code >= 11 && code <= 24) {
        int offt = 0;
        if (code > 15)
        offt++;
        if (code > 21)
        offt++;
        if (term->vt52_mode)
        p += sprintf((char *) p, "\x1B%c", code + 'P' - 11 - offt);
        else
        p +=
            sprintf((char *) p, "\x1BO%c", code + 'P' - 11 - offt);
        return p - output;
    }
    if (cfg->funky_type == FUNKY_LINUX && code >= 11 && code <= 15) {
        p += sprintf((char *) p, "\x1B[[%c", code + 'A' - 11);
        return p - output;
    }
    if (cfg->funky_type == FUNKY_XTERM && code >= 11 && code <= 14) {
        if (term->vt52_mode)
        p += sprintf((char *) p, "\x1B%c", code + 'P' - 11);
        else
        p += sprintf((char *) p, "\x1BO%c", code + 'P' - 11);
        return p - output;
    }
    if (cfg->rxvt_homeend && (code == 1 || code == 4)) {
        p += sprintf((char *) p, code == 1 ? "\x1B[H" : "\x1BOw");
        return p - output;
    }
    if (code) {
        p += sprintf((char *) p, "\x1B[%d~", code);
        return p - output;
    }

    // OK, handle it as a normal unicode key
    return -1;
}


Filename filename_from_str(const char *str)
{
    Filename ret;
    strncpy(ret.path, str, sizeof(ret.path));
    ret.path[sizeof(ret.path)-1] = '\0';
    return ret;
}

const char *filename_to_str(const Filename *fn)
{
    return fn->path;
}

int filename_equal(Filename f1, Filename f2)
{
    return !strcmp(f1.path, f2.path);
}

int filename_is_null(Filename fn)
{
    return !*fn.path;
}

int from_backend_untrusted(void *frontend, const char *data, int len)
{
    GuiTerminalWindow *f = static_cast<GuiTerminalWindow*>(frontend);
    return term_data_untrusted(f->term, data, len);
}

int from_backend(void *frontend, int is_stderr, const char *data, int len)
{
    GuiTerminalWindow *f = static_cast<GuiTerminalWindow*>(frontend);
    return f->from_backend(is_stderr, data, len);
}


char *get_ttymode(void *frontend, const char *mode)
{
    GuiTerminalWindow *f = static_cast<GuiTerminalWindow*>(frontend);
    return term_get_ttymode(f->term, mode);
}

struct tm ltime(void)
{
    time_t rawtime;
    struct tm * timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    return *timeinfo;
}

void set_busy_status(void *frontend, int status)
{
    // TODO not implemented
    //busy_status = status;
    //update_mouse_pointer();
}

char *platform_get_x_display(void) {
    /* We may as well check for DISPLAY in case it's useful. */
    return dupstr(getenv("DISPLAY"));
}

/*
 * called to initalize tmux mode
 */
int tmux_init_tmux_mode(void *frontend, char *tmux_version) {
    GuiTerminalWindow *f = static_cast<GuiTerminalWindow*>(frontend);
    return f->initTmuxControllerMode(tmux_version);
}

int tmux_from_backend(void *frontend, int is_stderr, const char *data, int len)
{
    GuiTerminalWindow *f = static_cast<GuiTerminalWindow*>(frontend);
    return f->tmuxGateway()->fromBackend(is_stderr, data, len);
}
