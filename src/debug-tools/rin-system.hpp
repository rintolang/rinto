#ifndef RIN_DEBUG_SYSTEM_HPP
#define RIN_DEBUG_SYSTEM_HPP

#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include "jswin.h"
#else
#include <signal.h>
#endif // WIN32

// debug-diagnostics.cc
extern void do_recoverable_abort();

#define BE_UNREACHABLE()                                                    \
{                                                                           \
        fprintf(stderr, "[DEBUG FATAL] Received UNREACHABLE signal\n");     \
        fflush(stderr);                                                     \
        do_recoverable_abort();                                             \
}

#define BE_ASSERT(EXPR)                                                     \
{                                                                           \
        if (!(EXPR)) {                                                      \
                fprintf(stderr, "[DEBUG FATAL]: ASSERTION FAILURE\n");      \
                fflush(stderr);                                             \
                do_recoverable_abort();                                     \
        }                                                                   \
}

#endif // RIN_DEBUG_SYSTEM_HPP
