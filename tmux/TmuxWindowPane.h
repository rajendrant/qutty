#ifndef TMUXWINDOWPANE_H
#define TMUXWINDOWPANE_H
#include "tmux/tmux.h"
#include "tmux/TmuxGateway.h"

class TmuxWindowPane : public TmuxCmdRespReceiver
{
    TmuxGateway *_gateway;
    GuiTerminalWindow *_termWnd;
public:
    int id;
    string name;
    int width;
    int height;

    TmuxWindowPane(TmuxGateway *gateway, GuiTerminalWindow *termwnd);
    TmuxGateway *gateway() { return _gateway; }
    GuiTerminalWindow *termWnd() { return _termWnd; }
    int performCallback(tmux_cb_index_t index, string &response);
};

#endif // TMUXWINDOWPANE_H
