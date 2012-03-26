/*
 * Find the platform-specific header for this platform.
 */

#ifndef PUTTY_PUTTYPS_H
#define PUTTY_PUTTYPS_H

#if defined(_WINDOWS) || defined(WIN32)

//#include "winstuff.h"
#include "QtStuff.h"

#elif defined(MACOSX)

#include "osx.h"

#else

#include "unix.h"

#endif

#endif
