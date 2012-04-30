#ifndef TMUXWINDOWPANE_H
#define TMUXWINDOWPANE_H
#include "tmux/tmux.h"
#include "tmux/TmuxGateway.h"

// -- begin section copied from tmux.h --
/* Grid attributes. */
#define GRID_ATTR_BRIGHT 0x1
#define GRID_ATTR_DIM 0x2
#define GRID_ATTR_UNDERSCORE 0x4
#define GRID_ATTR_BLINK 0x8
#define GRID_ATTR_REVERSE 0x10
#define GRID_ATTR_HIDDEN 0x20
#define GRID_ATTR_ITALICS 0x40
#define GRID_ATTR_CHARSET 0x80  /* alternative character set */

class TmuxWindowPane : public TmuxCmdRespReceiver
{
    TmuxGateway *_gateway;
    GuiTerminalWindow *_termWnd;

    /*
     * To maintain previous state of terminal temporarily.
     * sent by tmux gateway upon attach session.
     */
    struct tmux_old_state_t {
        int8_t in_alt_screen;
        uint8_t base_cursor_x;
        uint8_t base_cursor_y;
        uint8_t cursor_x;
        uint8_t cursor_y;
        uint8_t scroll_region_upper;
        uint8_t scroll_region_lower;
        uint8_t decsc_cursor_x;
        uint8_t decsc_cursor_y;
        string tabstops;
        /*
        uint cursor_mode;
        uint insert_mode;
        uint kcursor_mode;
        uint kkeypad_mode;
        uint wrap_mode;
        uint mouse_standard_mode;
        uint mouse_button_mode;
        uint mouse_any_mode;
        uint mouse_utf8_mode;
        */
        tmux_old_state_t()
        {
            in_alt_screen = 0;
            base_cursor_x = base_cursor_y = cursor_x = cursor_y = 0;
            scroll_region_upper = scroll_region_lower = 0;
            decsc_cursor_x = decsc_cursor_y = 0;
            tabstops ="";
        }
    };
    tmux_old_state_t *old_state;

public:
    int id;
    string name;
    int width;
    int height;

    TmuxWindowPane(TmuxGateway *gateway, GuiTerminalWindow *termwnd);
    ~TmuxWindowPane();

    TmuxGateway *gateway() { return _gateway; }
    GuiTerminalWindow *termWnd() { return _termWnd; }
    int performCallback(tmux_cb_index_t index, string &response);

    int resp_hdlr_dump_term_state(string &response);
    int resp_hdlr_dump_history(string &response, bool is_alt=false);
};

#endif // TMUXWINDOWPANE_H
