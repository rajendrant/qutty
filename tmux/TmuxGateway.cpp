#include <tmux/tmux.h>
#include <iostream>
#include <sstream>
#include "tmux/TmuxWindowPane.h"
#include "tmux/TmuxGateway.h"
#include "GuiMainWindow.h"
#include "GuiTabWidget.h"

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
    qDebug("%s %s %s", __FUNCTION__, get_tmux_cb_index_str(index), response.c_str());
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

size_t TmuxGateway::fromBackend(int is_stderr, const char *data, size_t len)
{
    size_t i;
    const char *buf = NULL;
    int rc = 0;
    size_t resp_len = 0;
    size_t rem_len;

    term_data(termGatewayWnd->term, is_stderr, data, (int)len);

    for (i=0; i<(size_t)len; i++) {
        if (data[i] == '\n') {
            resp_len = i+1;
            break;
        }
    }
    if (resp_len == 0) {
        // complete response not received
        bufchain_add(&buffer, data, (int)len);
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
        bufchain_add(&buffer, buf+resp_len, (int)(rem_len-resp_len));
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
        return 0;
    } else if (_currentCommand.receiver) {
        _currentCommandResponse.append(command, len);
        return 0;
    } else if (strStartsWith("%begin", command, len)) {
        if (_currentCommand.receiver) {
            qFatal("TMUX %%begin command without end");
        } else if (_commandQueue.empty()) {
            qFatal("TMUX %%begin command without command queue");
        } else {
            _currentCommand = _commandQueue.front();
            _commandQueue.pop();
            _currentCommandResponse.clear();
        }
        return 0;
    }
    qDebug() << __FUNCTION__ << command;

    if (strStartsWith("%output ", command, len)) {
        cmd_hdlr_output(command, len);
    } else if (strStartsWith("%noop", command, len)) {
        qDebug("tmux noop command %d len %.*s", len, len, command);
    } else if (strStartsWith("%sessions-changed", command, len)) {
        cmd_hdlr_sessions_changed(command, len);
    } else if (strStartsWith("%session-changed ", command, len)) {
        cmd_hdlr_session_changed(command, len);
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
        qFatal("TMUX unrecognized command %d len %.*s", len, len, command);
    }
    return 0;
}

int TmuxGateway::cmd_hdlr_sessions_changed(const char *command, size_t len)
{
    const char *cmd = command + 17;  // skip command prefix
    if (*cmd != '\n')
        goto cu0;
    return 0;
cu0:
    qDebug("TMUX malformed command %.*s", len, command);
    return -1;
}

int TmuxGateway::cmd_hdlr_session_changed(const char *command, size_t len)
{
    const char *cmd = command + 17 + 1;  // skip command prefix
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

int TmuxGateway::cmd_hdlr_output(const char *command, size_t len)
{
    const char *cmd = command + 8;  // skip command prefix
    const char *fail_reason = NULL;
    char data[128];
    int paneid, datalen = 0, i;
    char *bytes;
    if (*cmd != '%') {
        fail_reason = "No %%";
        goto cu0;
    }
    paneid = strtol(++cmd, &bytes, 10);
    if (paneid==0 && bytes==cmd) {
        fail_reason = "No paneid";
        goto cu0;
    }
    if (_mapPanes.find(paneid) == _mapPanes.end()) {
        fail_reason = "Invalid paneid";
        goto cu0;
    }
    if (!_mapPanes[paneid]->ready) {
        fail_reason = "Not yet ready";
        goto cu0;
    }
    bytes++;     // skip single whitespace
    int byteslen = command + len - bytes - 1;
    for (i=0; i<byteslen; i++) {
        char c = bytes[i];
        if (c < ' ')
            continue;
        if (c == '\\') {
            for (int j = 0; j < 3; j++) {
                i++;
                if (bytes[i] == '\r') {
                    // Ignore \r's that the line driver sprinkles in at its pleasure.
                    continue;
                }
                if (bytes[i] < '0' || bytes[i] > '7') {
                    c = '?';
                    i--;  // Back up in case bytes[i] is a null; we don't want to go off the end.
                    break;
                }
                c *= 8;
                c += bytes[i] - '0';
            }
        }
        data[datalen++] = c;
        if (datalen == sizeof(data)) {
            qDebug("TMUX output %d %.*s", datalen, datalen, data);
            _mapPanes[paneid]->termWnd()->from_backend(0, data, datalen);
            datalen = 0;
        }
    }
    if (datalen > 0) {
        qDebug("TMUX output %d %.*s", datalen, datalen, data);
        _mapPanes[paneid]->termWnd()->from_backend(0, data, datalen);
    }
    return 0;
cu0:
    qDebug("TMUX malformed command %s %.*s", fail_reason, len, command);
    return -1;
}

int TmuxGateway::cmd_hdlr_window_renamed(const char *command, size_t len)
{
    const char *cmd = command + 16 + 1;  // skip command prefix
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
    _mapPanes[paneid]->termWnd()->setSessionTitle(QString::fromLatin1(wndtitle.c_str()));
    return 0;
cu0:
    qDebug("TMUX malformed command %s %.*s", fail_reason, len, command);
    return -1;
}

int TmuxGateway::cmd_hdlr_window_add(const char *command, size_t len)
{
    const char *cmd = command + 12 + 1;  // skip command prefix
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

int TmuxGateway::cmd_hdlr_window_close(const char *command, size_t len)
{
    const char *cmd = command + 14 + 1;  // skip command prefix
    istringstream iresp(cmd);
    const char *fail_reason;
    int paneid;
    iresp>>paneid;
    if (_mapPanes.find(paneid) == _mapPanes.end()) {
        fail_reason = "Invalid paneid";
        goto cu0;
    }
    termGatewayWnd->getMainWindow()->tabArea->removeTab(termGatewayWnd->getMainWindow()->tabArea->indexOf(_mapPanes[paneid]->termWnd()));
    _mapPanes.erase(paneid);
    return 0;
cu0:
    qDebug("TMUX malformed command %s %.*s", fail_reason, len, command);
    return -1;
}

int TmuxGateway::cmd_hdlr_layout_change(const char *command, size_t len)
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
    //createNewWindowPane(wndid, "", layout);
    return 0;
cu0:
    qCritical("TMUX malformed command %s %.*s", fail_reason, len, command);
    return -1;
}

int TmuxGateway::openWindowsInitial()
{
    // TODO hiddenwindows, affinities, origins
    wchar_t set_client_size[128];
    wsprintf(set_client_size,
             L"refresh-client -C %d,%d\n",
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
                             const wchar_t cmd_str[])
{
    return sendCommand(recv, cb, cmd_str, wcslen(cmd_str));
}

int TmuxGateway::sendCommand(TmuxCmdRespReceiver *recv, tmux_cb_index_t cb,
                             const wchar_t cmd_str[], size_t cmd_str_len)
{
    qDebug()<<__FUNCTION__<<QString::fromWCharArray(cmd_str, (int)cmd_str_len);
    luni_send(termGatewayWnd->ldisc, (wchar_t*)cmd_str, (int)cmd_str_len, 0);
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
        createNewWindow(wndid, wndname.c_str(), width, height, layout);
    }
    return 0;
cu0:
    qCritical("TMUX malformed response %s %.*s", response);
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
        createNewWindow(wndid, wndname.c_str(), width, height, layout);
    }
    return 0;
cu0:
    qCritical("TMUX malformed response %s %.*s", response);
    return -1;
}

int TmuxGateway::createNewWindow(int id, const char *name, int /*width*/, int /*height*/, string layout)
{
    if (!_mapLayout[id].initLayout(layout.substr(5)))
        goto cu0;
    createNewWindowPane(id, name, _mapLayout[id]);
    return 0;
cu0:
    return -1;
}

int TmuxGateway::createNewWindowPane(int id, const char *name, TmuxLayout &layout)
{
    switch (layout.layoutType) {
      case TmuxLayout::TMUX_LAYOUT_TYPE_LEAF:
        if (_mapPanes.find(layout.paneid) == _mapPanes.end()) {
            GuiTerminalWindow *newtermwnd = new GuiTerminalWindow(termGatewayWnd->getMainWindow()->tabArea,
                                                                  termGatewayWnd->getMainWindow());
            newtermwnd->cfg = termGatewayWnd->cfg;
            TmuxWindowPane *tmuxPane = newtermwnd->
                    initTmuxClientTerminal(this, layout.paneid,
                                           layout.width, layout.height);
            termGatewayWnd->getMainWindow()->setupLayout(newtermwnd, GuiBase::TYPE_LEAF, -1);
            tmuxPane->name = name;
            newtermwnd->setSessionTitle(name);
            wchar_t modes[2000], cmd_state[2000], cmd_hist[256], cmd_hist_alt[256];
            wchar_t *modesformat[] = {L"pane_id", L"alternate_on", L"alternate_saved_x", L"alternate_saved_y",
                                     L"saved_cursor_x", L"saved_cursor_y", L"cursor_x", L"cursor_y",
                                     L"scroll_region_upper", L"scroll_region_lower", L"pane_tabs", L"cursor_flag",
                                     L"insert_flag", L"keypad_cursor_flag", L"keypad_flag", L"wrap_flag",
                                     L"mouse_standard_flag", L"mouse_button_flag", L"mouse_any_flag",
                                     L"mouse_utf8_flag"};
            for(int i=0, l=0; i< sizeof(modesformat)/sizeof(modesformat[0]); i++)
                l += wsprintf(modes+l, L"%s=#{%s}\t", modesformat[i], modesformat[i]);
            modes[wcslen(modes)-1] = '\0';
            wsprintf(cmd_state, L"list-panes -t %%%d -F \"%s\"\n", layout.paneid, modes);
            wsprintf(cmd_hist, L"capture-pane -peqJ -t %%%d -S -%d\n",
                     layout.paneid, termGatewayWnd->cfg.savelines);
            wsprintf(cmd_hist_alt, L"capture-pane -peqJ -a -t %%%d -S -%d\n",
                     layout.paneid, termGatewayWnd->cfg.savelines);
            sendCommand(tmuxPane, CB_DUMP_HISTORY, cmd_hist);
            sendCommand(tmuxPane, CB_DUMP_HISTORY_ALT, cmd_hist_alt);
            sendCommand(tmuxPane, CB_DUMP_TERM_STATE, cmd_state);
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
        for (int i=0; i<(signed)layout.child.size(); i++) {
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
        it->second->termWnd()->closeTerminal();
        delete it->second;
    }
    _mapPanes.clear();
}

void TmuxGateway::closePane(int paneid)
{
    TmuxWindowPane *pane = _mapPanes[paneid];
    if (pane) {
        pane->termWnd()->closeTerminal();
        _mapPanes.erase(paneid);
        delete pane;
    }
}

const char *get_tmux_cb_index_str(tmux_cb_index_t index)
{
    char *tmux_cb_index_str[] = {
    #undef T
    #define T(a) #a
        TMUX_CB_INDEX_LIST
    #undef T
    };
    if (index > CB_INDEX_MAX) {
        static char tmp[100];
        snprintf(tmp, sizeof(tmp), "Invalid cb index %d", index);
        return tmp;
    }
    return tmux_cb_index_str[index];
}
