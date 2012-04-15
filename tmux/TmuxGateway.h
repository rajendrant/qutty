#ifndef TMUXGATEWAY_H
#define TMUXGATEWAY_H

class TmuxGateway;
#include "GuiTerminalWindow.h"
extern "C" {
#include "misc.h"
// min/max interferes with std::min/max
#undef min
#undef max
}
#include <string>
#include <queue>
#include <map>
using namespace std;

enum tmux_cb_index_t {
    CB_NULL,
    CB_LIST_WINDOWS,
    CB_OPEN_LISTED_WINDOWS,
    CB_DUMP_HISTORY,
    CB_DUMP_HISTORY_ALT,
    CB_INDEX_MAX
};

class TmuxCmdRespReceiver
{
public:
    virtual int performCallback(tmux_cb_index_t index, string &response) = 0;
};

class TmuxCmdResp
{
public:
    TmuxCmdRespReceiver *receiver;
    tmux_cb_index_t callback;
    TmuxCmdResp(TmuxCmdRespReceiver *recv, tmux_cb_index_t cb)
    {
        receiver = recv;
        callback = cb;
    }
};


class TmuxGateway : public TmuxCmdRespReceiver
{
    GuiTerminalWindow *termWnd;
    bufchain buffer;
    queue<TmuxCmdResp> _commandQueue;
    TmuxCmdResp _currentCommand;
    string _currentCommandResponse;
    long _sessionID;
    char *_sessionName;
    map<int, TmuxWindowPane*> _mapPanes;

    int cmd_hdlr_sessions_changed(const char *command, int len);
    int cmd_hdlr_session_changed(const char *command, int len);
    int cmd_hdlr_output(const char *command, int len);
    int cmd_hdlr_window_renamed(const char *command, int len);
    int cmd_hdlr_window_add(const char *command, int len);
    int cmd_hdlr_window_close(const char *command, int len);

    int resp_hdlr_list_windows(string &response);
    int resp_hdlr_open_listed_windows(string &response);

public:
    TmuxGateway(GuiTerminalWindow *termWindow);
    ~TmuxGateway();
    int performCallback(tmux_cb_index_t index, string &response);
    int fromBackend(int is_stderr, const char *data, int len);
    int parseCommand(const char *command, size_t len);
    int openWindowsInitial();
    int createNewWindow(int id, const char *name, int width, int height);
    int sendCommand(TmuxCmdRespReceiver *recv, tmux_cb_index_t cb,
                    const wchar_t cmd_str[], int cmd_str_len=-1);
    int sendCommand(TmuxCmdResp cmd_list[], wstring cmd_str[], int len=1);
};

#endif // TMUXGATEWAY_H
