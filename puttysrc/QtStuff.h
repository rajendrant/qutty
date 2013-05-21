#ifndef QTSTUFF_H
#define QTSTUFF_H

#include <stdio.h>
#include <windows.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct Filename {
    char path[FILENAME_MAX];
};

struct FontSpec {
    char name[64];
    int isbold;
    int height;
    int charset;
};

typedef uint32_t uint32;

typedef void* Context;

void init_ucs(Config *, struct unicode_data *);

/*
 * On Windows, copying to the clipboard terminates lines with CRLF.
 */
#define SEL_NL { 13, 10 }

#define DEFAULT_CODEPAGE CP_ACP

#define TICKSPERSEC 1000	       /* GetTickCount returns milliseconds */

#define GETTICKCOUNT GetTickCount

#define f_open(filename, mode, isprivate) ( fopen((filename).path, (mode)) )

/*
 * sk_getxdmdata() does not exist under Windows (not that I
 * couldn't write it if I wanted to, but I haven't bothered), so
 * it's a macro which always returns NULL. With any luck this will
 * cause the compiler to notice it can optimise away the
 * implementation of XDM-AUTHORIZATION-1 in x11fwd.c :-)
 */
#define sk_getxdmdata(socket, lenp) (NULL)


#define APPNAME "QuTTY"
#define QUTTY_RELEASE_VERSION "0.3-beta"

void qt_message_box(void * frontend, const char *title, char *msg);

#define qt_critical_msgbox(frontend, fmt, ...) do { \
    char qt_message_box_msg[1000]; \
    _snprintf(qt_message_box_msg, sizeof(qt_message_box_msg), fmt, __VA_ARGS__); \
    qt_message_box(frontend, APPNAME " Fatal Error", qt_message_box_msg); \
} while(0)

#define modalfatalbox(fmt, ...) qt_message_box(NULL, APPNAME " Fatal Error", fmt)

void qutty_connection_fatal(void *frontend, char *msg);

#define connection_fatal(frontend, fmt, ...) do { \
    char buf[1000]; \
    _snprintf(buf, sizeof(buf), fmt, __VA_ARGS__); \
    qutty_connection_fatal(frontend, buf); \
} while(0);

#define fatalbox(fmt, ...) qt_critical_msgbox(NULL, fmt, __VA_ARGS__)

#define assert(cond) do {\
    if(!(cond)) fatalbox("fatal assert %s(%d)"#cond, __FUNCTION__, __LINE__); \
} while(0)

void notify_remote_exit(void *frontend);

#ifdef __cplusplus
}
#endif
#endif // QTSTUFF_H
