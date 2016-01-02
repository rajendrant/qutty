/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#ifndef QTCOMMON_H
#define QTCOMMON_H

#include <stdlib.h>
#include <QTimer>
#include <QtNetwork/QTcpSocket>
#include <QKeyEvent>
extern "C" {
#include "putty.h"
}

#define SUCCESS 0

#ifdef __linux
#define _snprintf snprintf
#define wsprintf(dst, fmt...) swprintf(dst, sizeof(dst)/sizeof(wchar_t), fmt)
#define _snwprintf swprintf
#else
#define snprintf _snprintf
#endif

class GuiMainWindow;
class GuiTerminalWindow;

extern QTimer *qtimer;
extern long timing_next_time;

typedef struct Socket_tag *Actual_Socket;

struct Socket_tag {
    const struct socket_function_table *fn;
    /* the above variable absolutely *must* be the first in this structure */
    const char *error;
    //SOCKET s;
    Plug plug;
    void *private_ptr;
    bufchain output_data;
    int connected;
    int writable;
    int frozen; /* this causes readability notifications to be ignored */
    int frozen_readable; /* this means we missed at least one readability
                          * notification while we were frozen */
    int localhost_only;		       /* for listening sockets */
    char oobdata[1];
    int sending_oob;
    int oobinline, nodelay, keepalive, privport;
    SockAddr addr;
    //SockAddrStep step;
    int port;
    int pending_error;		       /* in case send() returns error */
    /*
     * We sometimes need pairs of Socket structures to be linked:
     * if we are listening on the same IPv6 and v4 port, for
     * example. So here we define `parent' and `child' pointers to
     * track this link.
     */
    Actual_Socket parent, child;
    QTcpSocket *qtsock;
};

typedef struct telnet_tag {
    const struct plug_function_table *fn;
    /* the above field _must_ be first in the structure */

    Socket s;

    void *frontend;
    void *ldisc;
    int term_width, term_height;

    int opt_states[20];

    int echoing, editing;
    int activated;
    int bufsize;
    int in_synch;
    int sb_opt, sb_len;
    unsigned char *sb_buf;
    int sb_size;
    int state;

    Config cfg;

    Pinger pinger;
} *Telnet;

void qstring_to_char(char *dst, QString src, int dstlen);

#endif // QTCOMMON_H
