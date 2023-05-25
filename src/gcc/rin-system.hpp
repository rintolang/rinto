#ifndef GCC_RIN_SYSTEM_HPP
#define GCC_RIN_SYSTEM_HPP

#include "config.h"

/* Define this so that inttypes.h defines the PRI?64 macros even
   when compiling with a C++ compiler.  Define it here so in the
   event inttypes.h gets pulled in by another header it is already
   defined.  */
#define __STDC_FORMAT_MACROS

// These must be included before the #poison declarations in system.h.
#include <mpfr.h>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <cstring>
#include <list>
#include <map>
#include <set>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <stack>
#include <regex>
#include <iostream>

#include "system.h"
#include "ansidecl.h"
#include "coretypes.h"

#include "diagnostic-core.h"	/* For error_at and friends.  */
#include "intl.h"		/* For _().  */

/*
 * Defines the first line index in a file.
 * The frontend has 0-based lines, 0-based columns.
 * GCC has 1-based lines, 1-based columns.
 */

#ifndef LOC_LINE_BEGIN
#define LOC_LINE_BEGIN 1
#endif // LOC_LINE_BEGIN

#ifndef LOC_COLUMN_BEGIN
#define LOC_COLUMN_BEGIN 1
#endif // LOC_COLUMN_BEGIN

// When using gcc, BE_ASSERT is just gcc_assert.
#define BE_ASSERT(EXPR) gcc_assert(EXPR)

// When using gcc, BE_UNREACHABLE is just gcc_unreachable.
#define BE_UNREACHABLE() gcc_unreachable()

#endif // GCC_RIN_SYSTEM_HPP
