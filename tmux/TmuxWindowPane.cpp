#include "tmux/TmuxWindowPane.h"

TmuxWindowPane::TmuxWindowPane(TmuxGateway *gateway, GuiTerminalWindow *termwnd)
    : _gateway(gateway),
      _termWnd(termwnd)
{
}

int TmuxWindowPane::performCallback(tmux_cb_index_t index, string &response)
{
    qDebug("TMUX resp %d %s", index, response.c_str());
    switch(index) {
    case CB_NULL:
        return 0;
    case CB_LIST_WINDOWS:
        return 0;
    case CB_DUMP_HISTORY:
        return 0;
    case CB_DUMP_HISTORY_ALT:
        return 0;
    default:
        return -1;
    }
}
