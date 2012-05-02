#include <tmux/tmux.h>
#include <iostream>
#include <sstream>
#include "tmux/TmuxWindowPane.h"
#include "tmux/TmuxGateway.h"
using namespace std;

TmuxGateway::TmuxGateway(GuiTerminalWindow *termWindow)
    : termGatewayWnd(termWindow),
      _currentCommand(NULL, CB_INDEX_MAX),
      _sessionID(0),
      _sessionName(NULL)
{
    bufchain_init(&buffer);
    _currentCommandResponse.reserve(128);
}

TmuxGateway::~TmuxGateway()
{
    if (_sessionName)
        sfree(_sessionName);
    closeAllPanes();
    bufchain_clear(&buffer);
}

int TmuxGateway::performCallback(tmux_cb_index_t index, string &response)
{
    qDebug("TMUX resp %d %s", index, response.c_str());
    switch(index) {
    case CB_NULL:
        return 0;
    case CB_LIST_WINDOWS:
        resp_hdlr_list_windows(response);
        return 0;
    case CB_OPEN_LISTED_WINDOWS:
        resp_hdlr_open_listed_windows(response);
        return 0;
    default:
        return -1;
    }
}

int TmuxGateway::fromBackend(int is_stderr, const char *data, int len)
{
    size_t i;
    const char *buf = NULL;
    int rc = 0;
    size_t resp_len = 0;
    size_t rem_len;

    term_data(termGatewayWnd->term, is_stderr, data, len);
    qDebug("%s len %d %.*s", __FUNCTION__, len, len, data);

    for (i=0; i<(size_t)len; i++) {
        if (data[i] == '\n') {
            resp_len = i+1;
            break;
        }
    }
    if (resp_len == 0) {
        // complete response not received
        bufchain_add(&buffer, data, len);
        return len;
    }
    rem_len = len - resp_len;
    if (bufchain_size(&buffer) > 0) {
        int buf_len = bufchain_size(&buffer);
        char *buf = snewn(buf_len + resp_len, char);
        bufchain_fetch(&buffer, buf, buf_len);
        memcpy(buf + buf_len, data, resp_len);
        bufchain_consume(&buffer, buf_len);
        rc = parseCommand(buf, buf_len + resp_len);
        sfree(buf);
        if (rc == -1)
            goto tmux_disconnected;
    } else {
        rc = parseCommand(data, resp_len);
        if (rc == -1)
            goto tmux_disconnected;
    }

    buf = data + resp_len;
    resp_len = 0;
    for (i=0; i<rem_len; i++) {
        if (buf[i] == '\n') {
            rc = parseCommand(buf+resp_len, i-resp_len+1);
            resp_len = i+1;
            if (rc == -1){
                rem_len -= i+1;
                goto tmux_disconnected;
            }
        }
    }
    if (resp_len < rem_len) {
        // pending data goes to buffer
        bufchain_add(&buffer, buf+resp_len, rem_len-resp_len);
    }

    return 0;
tmux_disconnected:
    // return the remaining data
    return rem_len;
}

int TmuxGateway::parseCommand(const char *command, size_t len)
{
#define strStartsWith(start, s, l) (strlen(start)<=l && !strncmp(start, s, strlen(start)))

    if (strStartsWith("%end", command, len)) {
        // command response received
        if (_currentCommand.receiver) {
            _currentCommand.receiver->performCallback(_currentCommand.callback,
                                                      _currentCommandResponse);
            _currentCommand.receiver = NULL;
            _currentCommand.callback = CB_INDEX_MAX;
            _currentCommandResponse.clear();
        }
    } else if (_currentCommand.receiver) {
        _currentCommandResponse.append(command, len);
    } else if (strStartsWith("%begin", command, len)) {
        if (_currentCommand.receiver) {
            qFatal("TMUX %%begin command without end");
        } else if (_commandQueue.empty()) {
            qDebug("TMUX %%begin command without command queue");
        } else {
            _currentCommand = _commandQueue.front();
            _commandQueue.pop();
            _currentCommandResponse.clear();
        }
    } else if (strStartsWith("%output ", command, len)) {
        cmd_hdlr_output(command, len);
    } else if (strStartsWith("%noop", command, len)) {
        qDebug("tmux noop command %d len %.*s", len, len, command);
    } else if (strStartsWith("%sessions-changed", command, len)) {
        cmd_hdlr_sessions_changed(command, len);
    } else if (strStartsWith("%session-changed ", command, len)) {
        cmd_hdlr_session_changed(command, len);
    } else if (strStartsWith("%window-renamed ", command, len)) {
        cmd_hdlr_window_renamed(command, len);
    } else if (strStartsWith("%window-renamed ", command, len)) {
        cmd_hdlr_window_renamed(command, len);
    } else if (strStartsWith("%window-add ", command, len)) {
        cmd_hdlr_window_add(command, len);
    } else if (strStartsWith("%window-close ", command, len)) {
        cmd_hdlr_window_close(command, len);
    } else if (strStartsWith("%exit ", command, len) ||
               strStartsWith("%exit\n", command, len)) {
        //luni_send(termGatewayWnd->ldisc, (wchar_t*)L"#ack-exit\n", 10, 0);
        termGatewayWnd->startDetachTmuxControllerMode();
        return -1;
    } else if (strStartsWith("%layout-change ", command, len)) {
        cmd_hdlr_layout_change(command, len);
    } else {
        qDebug("TMUX unrecognized command %d len %.*s", len, len, command);
    }
    return 0;
}

int TmuxGateway::cmd_hdlr_sessions_changed(const char *command, int len)
{
    const char *cmd = command + 17;  // skip command prefix
    if (*cmd != '\n')
        goto cu0;
    return 0;
cu0:
    qDebug("TMUX malformed command %.*s", len, command);
    return -1;
}

int TmuxGateway::cmd_hdlr_session_changed(const char *command, int len)
{
    const char *cmd = command + 17;  // skip command prefix
    long sessid;
    char *sessname;
    sessid = strtol(cmd, &sessname, 10);
    if (sessid==0 && sessname==cmd)
        goto cu0;
    sessname++;     // skip single whitespace

    this->_sessionID = sessid;
    this->_sessionName = dupstr(sessname);
    qDebug()<<"TMUX session-changed"<<sessid<<sessname;

    // TODO close all windows/panes
    openWindowsInitial();
    return 0;
cu0:
    qDebug("TMUX malformed command %.*s", len, command);
    return -1;
}

int TmuxGateway::cmd_hdlr_output(const char *command, int len)
{
    const char *cmd = command + 8;  // skip command prefix
    const char *fail_reason = NULL;
    int paneid, b64len, datalen, i;
    char *output;
    if (*cmd != '%') {
        fail_reason = "No %%";
        goto cu0;
    }
    cmd++;
    paneid = strtol(cmd, &output, 10);
    if (paneid==0 && output==cmd) {
        fail_reason = "No paneid";
        goto cu0;
    }
    output++;     // skip single whitespace
    if (_mapPanes.find(paneid) == _mapPanes.end()) {
        fail_reason = "Invalid paneid";
        goto cu0;
    }
    b64len = command + len - output - 1;
    datalen = b64len / 2;
    for (i=0; i<datalen; i++) {
        output[i] = hex_to_char(output[2*i])<<4 | hex_to_char(output[2*i+1]);
    }
    qDebug("TMUX output %.*s", datalen, output);
    _mapPanes[paneid]->termWnd()->from_backend(0, output, datalen);
    return 0;
cu0:
    qDebug("TMUX malformed command %s %.*s", fail_reason, len, command);
    return -1;
}

int TmuxGateway::cmd_hdlr_window_renamed(const char *command, int len)
{
    const char *cmd = command + 16;  // skip command prefix
    istringstream iresp(cmd);
    const char *fail_reason;
    int paneid;
    string wndtitle;
    iresp>>paneid;
    if (_mapPanes.find(paneid) == _mapPanes.end()) {
        fail_reason = "Invalid paneid";
        goto cu0;
    }
    iresp>>wndtitle;
    set_title(_mapPanes[paneid]->termWnd(), (char*)wndtitle.c_str());
    return 0;
cu0:
    qDebug("TMUX malformed command %s %.*s", fail_reason, len, command);
    return -1;
}

int TmuxGateway::cmd_hdlr_window_add(const char *command, int len)
{
    const char *cmd = command + 12;  // skip command prefix
    istringstream iresp(cmd);
    const char *fail_reason;
    int paneid;
    iresp>>paneid;
    if (_mapPanes.find(paneid) != _mapPanes.end()) {
        fail_reason = "Window already exists";
        goto cu0;
    }
    wchar_t cmd_display_pane[256];
    wsprintf(cmd_display_pane, L"display -p -F %s -t @%d\n",
             L"\"#{session_name}\t#{window_id}\t#{window_name}\t"
             L"#{window_width}\t#{window_height}\t#{window_layout}\t#{?window_active,1,0}\"",
             paneid);
    sendCommand(this, CB_OPEN_LISTED_WINDOWS, cmd_display_pane);
    return 0;
cu0:
    qDebug("TMUX malformed command %s %.*s", fail_reason, len, command);
    return -1;
}

int TmuxGateway::cmd_hdlr_window_close(const char *command, int len)
{
    const char *cmd = command + 14;  // skip command prefix
    istringstream iresp(cmd);
    const char *fail_reason;
    int paneid;
    iresp>>paneid;
    if (_mapPanes.find(paneid) == _mapPanes.end()) {
        fail_reason = "Invalid paneid";
        goto cu0;
    }
    mainWindow->tabArea->removeTab(mainWindow->tabArea->indexOf(_mapPanes[paneid]->termWnd()));
    _mapPanes.erase(paneid);
    return 0;
cu0:
    qDebug("TMUX malformed command %s %.*s", fail_reason, len, command);
    return -1;
}

int TmuxGateway::cmd_hdlr_layout_change(const char *command, int len)
{
    //TODO more work in removing deleted panes
    const char *cmd = command + 15;  // skip command prefix
    istringstream iresp(cmd);
    const char *fail_reason;
    int wndid;
    string strlayout;
    TmuxLayout layout;
    if (iresp.get() != '@') {
        fail_reason = "No @window found";
        goto cu0;
    }
    iresp>>wndid;
    iresp>>strlayout;
    layout.initLayout(strlayout.substr(5));
    createNewWindowPane(wndid, "", layout);
    return 0;
cu0:
    qDebug("TMUX malformed command %s %.*s", fail_reason, len, command);
    return -1;
}

int TmuxGateway::openWindowsInitial()
{
    // TODO hiddenwindows, affinities, origins
    wchar_t set_client_size[128];
    wsprintf(set_client_size,
             L"control set-client-size %d,%d\n",
             termGatewayWnd->term->cols, termGatewayWnd->term->rows);
    sendCommand(this, CB_NULL,
                set_client_size);
    sendCommand(this, CB_NULL,
                L"list-sessions -F \"#{session_name}\"\n");
    sendCommand(this, CB_LIST_WINDOWS,
                L"list-windows -F \"#{session_name}\t#{window_id}\t#{window_name}\t"
                L"#{window_width}\t#{window_height}\t#{window_layout}\t#{?window_active,1,0}\"\n");
    return 0;
}

int TmuxGateway::sendCommand(TmuxCmdRespReceiver *recv, tmux_cb_index_t cb,
                             const wchar_t cmd_str[], int cmd_str_len)
{
    qDebug()<<__FUNCTION__<<QString::fromWCharArray(cmd_str, cmd_str_len);
    if (cmd_str_len == -1)
        cmd_str_len = wcslen(cmd_str);
    luni_send(termGatewayWnd->ldisc, (wchar_t*)cmd_str, cmd_str_len, 0);
    _commandQueue.push(TmuxCmdResp(recv, cb));
    return 0;
}

int TmuxGateway::resp_hdlr_list_windows(string &response)
{
    string respline;
    char ch;
    string sessname, wndname, layout;
    int wndid, width, height, wndactive;
    istringstream iresp(response);
    while (std::getline(iresp, respline)) {
        istringstream irec(respline);
        irec>>sessname;
        irec>>ch;
        if (ch!='@')
            goto cu0;
        irec>>wndid;
        irec>>wndname;
        irec>>width;
        irec>>height;
        irec>>layout;
        irec>>wndactive;
        createNewWindow(wndid, sessname.c_str(), width, height, layout);
    }
    return 0;
cu0:
    return -1;
}

int TmuxGateway::resp_hdlr_open_listed_windows(string &response)
{
    string respline;
    char ch;
    string sessname, wndname, layout;
    int wndid, width, height, wndactive;
    istringstream iresp(response);
    while (std::getline(iresp, respline)) {
        istringstream irec(respline);
        irec>>sessname;
        irec>>ch;
        if (ch!='@')
            goto cu0;
        irec>>wndid;
        irec>>wndname;
        irec>>width;
        irec>>height;
        irec>>layout;
        irec>>wndactive;
        createNewWindow(wndid, sessname.c_str(), width, height, layout);
    }
    return 0;
cu0:
    return -1;
}

int TmuxGateway::createNewWindow(int id, const char *name, int width, int height, string layout)
{
    if (!_mapLayout[id].initLayout(layout.substr(5)))
        goto cu0;
    createNewWindowPane(id, name, _mapLayout[id]);
    sendCommand(this, CB_NULL, L"control set-ready\n");
    return 0;
cu0:
    return -1;
}

int TmuxGateway::createNewWindowPane(int id, const char *name, TmuxLayout &layout)
{
    switch (layout.layoutType) {
      case TmuxLayout::TMUX_LAYOUT_TYPE_LEAF:
        if (_mapPanes.find(layout.paneid) == _mapPanes.end()) {
            GuiTerminalWindow *newtermwnd = mainWindow->newTerminal();
            newtermwnd->cfg = termGatewayWnd->cfg;
            TmuxWindowPane *tmuxPane = newtermwnd->
                    initTmuxClientTerminal(this, layout.paneid,
                                           layout.width, layout.height);
            tmuxPane->name = name;
            wchar_t cmd_emu[256], cmd_hist[256], cmd_hist_alt[256];
            wsprintf(cmd_emu, L"control -t %%%d get-emulator\n", layout.paneid);
            wsprintf(cmd_hist, L"control -t %%%d -l %d get-history\n",
                     layout.paneid, termGatewayWnd->cfg.savelines);
            wsprintf(cmd_hist_alt, L"control -a -t %%%d -l %d get-history\n",
                     layout.paneid, termGatewayWnd->cfg.savelines);
            sendCommand(tmuxPane, CB_DUMP_HISTORY, cmd_hist);
            sendCommand(tmuxPane, CB_DUMP_HISTORY_ALT, cmd_hist_alt);
            sendCommand(tmuxPane, CB_DUMP_TERM_STATE, cmd_emu);
            _mapPanes[layout.paneid] = tmuxPane;
            break;
        } else {
            term_size(_mapPanes[layout.paneid]->termWnd()->term,
                      layout.height, layout.width,
                      _mapPanes[layout.paneid]->termWnd()->cfg.savelines);
        }
        break;
      case TmuxLayout::TMUX_LAYOUT_TYPE_HORIZONTAL:
      case TmuxLayout::TMUX_LAYOUT_TYPE_VERTICAL:
        for (int i=0; i<layout.child.size(); i++) {
            createNewWindowPane(id, name, layout.child[i]);
        }
        break;
      default:
        assert(0);
    }
    return 0;
}

void TmuxGateway::initiateDetach()
{
    sendCommand(this, CB_NULL, L"detach\n");
    closeAllPanes();
}

void TmuxGateway::detach()
{
    closeAllPanes();
}

void TmuxGateway::closeAllPanes()
{
    map<int, TmuxWindowPane*>::const_iterator it;
    for ( it=_mapPanes.begin() ; it != _mapPanes.end(); it++ ) {
        mainWindow->closeTerminal(it->second->termWnd());
        delete it->second;
    }
    _mapPanes.clear();
}

void TmuxGateway::closePane(int paneid)
{
    TmuxWindowPane *pane = _mapPanes[paneid];
    if (pane) {
        mainWindow->closeTerminal(pane->termWnd());
        _mapPanes.erase(paneid);
        delete pane;
    }
}
