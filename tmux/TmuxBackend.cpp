extern "C" {
#include "putty.h"
}
#include "tmux/tmux.h"
#include "tmux/TmuxWindowPane.h"
#include "tmux/TmuxGateway.h"

extern "C" {
typedef struct __tmux_window_pane_t {
    const struct plug_function_table *fn;
    /* the above field _must_ be first in the structure */

    TmuxGateway *gateway;
    int paneid;
} tmux_window_pane_t;
}

extern "C" void tmux_log(Plug /*plug*/, int /*type*/, SockAddr /*addr*/, int /*port*/,
               const char * /*error_msg*/, int /*error_code*/)
{
    qDebug()<<__FUNCTION__;
}

extern "C" int tmux_closing(Plug /*plug*/, const char * /*error_msg*/, int /*error_code*/,
              int /*calling_back*/)
{
    qDebug()<<__FUNCTION__;
    return 0;
}

extern "C" int tmux_receive(Plug /*plug*/, int /*urgent*/, char * /*data*/, int /*len*/)
{
    qDebug()<<__FUNCTION__;
    return 1;
}

extern "C" void tmux_sent(Plug /*plug*/, int /*bufsize*/)
{
    qDebug()<<__FUNCTION__;
}

/*
 * Called to set up the Tmux client connection.
 *
 * Returns an error message, or NULL on success.
 */
extern "C" const char *tmux_client_init(void *frontend_handle, void **backend_handle,
                   Config * /*cfg*/,
                   char * /*host*/, int port, char ** /*realhost*/,
                   int /*nodelay*/, int /*keepalive*/)
{
    static const struct plug_function_table fn_table = {
        tmux_log,
        tmux_closing,
        tmux_receive,
        tmux_sent
    };

    GuiTerminalWindow *termWnd = static_cast<GuiTerminalWindow*>(frontend_handle);
    assert(termWnd->tmuxGateway());

    tmux_window_pane_t *handle = new tmux_window_pane_t;
    *backend_handle = handle;
    handle->gateway = termWnd->tmuxGateway();
    handle->paneid = port;  // HACK - port is actually paneid
    handle->fn = &fn_table;

    return NULL;
}

extern "C" void tmux_free(void *plug)
{
    tmux_window_pane_t *handle = (tmux_window_pane_t*)plug;
    delete handle;
}

extern "C" void tmux_reconfig(void * /*handle*/, Config * /*cfg*/)
{
}

/*
 * Called to send data down the backend connection.
 */
extern "C" int tmux_send(void *handle, char *buf, int len)
{
    tmux_window_pane_t *tmuxpane = static_cast<tmux_window_pane_t*>(handle);
    const size_t wbuf_len = 20480;
    wchar_t wbuf[wbuf_len];        // for plenty of speed
    char *rem_buf = buf;
    int i, rem_len = len;
    do {
        size_t ptrlen = 0;
        ptrlen += _snwprintf(wbuf, wbuf_len,
                             L"send-keys -t %%%d ", tmuxpane->paneid);
        for (i=0; i<rem_len && ptrlen<wbuf_len-6; i++) {
            ptrlen += _snwprintf(wbuf+ptrlen, wbuf_len-ptrlen,
                                 L"0x%02x ", rem_buf[i]);
        }
        wbuf[ptrlen-1] = '\n';
        tmuxpane->gateway->sendCommand(tmuxpane->gateway, CB_NULL,
                                       wbuf, ptrlen);
        rem_buf += i;
        rem_len -= i;
    } while (rem_len > 0);
    return len;
}

/*
 * Called to query the current socket sendability status.
 */
extern "C" int tmux_sendbuffer(void * /*handle*/)
{
    qDebug()<<__FUNCTION__;
    return 1;
}

extern "C" void tmux_unthrottle(void * /*handle*/, int /*backlog*/)
{
    qDebug()<<__FUNCTION__;
    //sk_set_frozen(telnet->s, backlog > TELNET_MAX_BACKLOG);
}

extern "C" int tmux_ldisc(void * /*handle*/, int option)
{
    if (option == LD_ECHO)
        return FALSE;
    if (option == LD_EDIT)
        return FALSE;
    return FALSE;
}

extern "C" void tmux_provide_ldisc(void * /*handle*/, void * /*ldisc*/)
{
    //telnet->ldisc = ldisc;
}

extern "C" void tmux_provide_logctx(void * /*handle*/, void * /*logctx*/)
{
    /* This is a stub. */
}

extern "C" int tmux_cfg_info(void * /*handle*/)
{
    return 0;
}

static char tmux_client_backend_name[] = "tmux_client_backend";

Backend tmux_client_backend = {
    tmux_client_init,
    tmux_free,
    tmux_reconfig,
    tmux_send,
    tmux_sendbuffer,
    NULL,NULL,NULL,NULL,NULL,NULL,
/*
    telnet_size,
    telnet_special,
    telnet_get_specials,
    telnet_connected,
    telnet_exitcode,
    telnet_sendok,
*/
    tmux_ldisc,
    tmux_provide_ldisc,
    tmux_provide_logctx,
    tmux_unthrottle,
    tmux_cfg_info,
    tmux_client_backend_name,
    PROT_TMUX_CLIENT,
    -1
};
