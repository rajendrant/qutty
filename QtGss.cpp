/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

extern "C" {
#include "putty.h"
#include "sshgss.h"
}

const int ngsslibs = 3;

extern "C" struct ssh_gss_liblist *ssh_gss_setup(const Config *) { return NULL; }

extern "C" void ssh_gss_cleanup(struct ssh_gss_liblist *) {}
