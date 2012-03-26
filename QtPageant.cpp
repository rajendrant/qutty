/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

extern "C" {
#include "putty.h"
#include "ssh.h"
}

#define FLAG_SYNCAGENT 0x1000
int flags = FLAG_SYNCAGENT;

int agent_exists(void)
{
    return FALSE;
}

int agent_query(void *in, int inlen, void **out, int *outlen,
        void (*callback)(void *, void *, int), void *callback_ctx)
{
    return 0;
}
