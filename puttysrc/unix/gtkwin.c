#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "putty.h"
#include "terminal.h"

/*
 * Default settings that are specific to pterm.
 */
FontSpec platform_default_fontspec(const char *name)
{
    FontSpec ret;
    if (!strcmp(name, "Font"))
        strcpy(ret.name, "server:fixed");
    else
        *ret.name = '\0';
    return ret;
}

Filename platform_default_filename(const char *name)
{
    Filename ret;
    if (!strcmp(name, "LogFileName"))
        strcpy(ret.path, "putty.log");
    else
        *ret.path = '\0';
    return ret;
}

char *platform_default_s(const char *name)
{
    if (!strcmp(name, "SerialLine"))
        return dupstr("/dev/ttyS0");
    return NULL;
}

int platform_default_i(const char *name, int def)
{
    if (!strcmp(name, "CloseOnExit"))
        return 2;  /* maps to FORCE_ON after painful rearrangement :-( */
    if (!strcmp(name, "WinNameAlways"))
        return 0;  /* X natively supports icon titles, so use 'em by default */
    return def;
}

