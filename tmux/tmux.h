#ifndef TMUX_H
#define TMUX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "putty.h"
extern Backend tmux_client_backend;

enum tmux_mode_t {
    TMUX_MODE_NONE,
    TMUX_MODE_GATEWAY,
    TMUX_MODE_GATEWAY_DETACH_INIT,
    TMUX_MODE_CLIENT
};

/*
 * called to initalize tmux mode
 */
int tmux_init_tmux_mode(void *frontend, char *tmux_version);
int tmux_from_backend(void *frontend, int is_stderr, const char *data, int len);

#ifdef __cplusplus
}

class TmuxGateway;
class TmuxWindowPane;
class TmuxCmdRespReceiver;
#endif

#endif // TMUX_H
