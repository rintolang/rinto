#include <diagnostic.hpp>

const char open_quote[]  = "'";
const char close_quote[] = "'";

void rin_be_error_at(const Location loc, const std::string& errmsg)
{
        printf("[DEBUG ERROR] %s:%d:%d: %s\n", &loc.filename[0],
               loc.line, loc.column, &errmsg[0]);
}

void rin_be_warning_at(const Location loc, int opt, const std::string& warningmsg)
{
        printf("[DEBUG WARNING] %s:%d:%d: %s\n", &loc.filename[0],
               loc.line, loc.column, &warningmsg[0]);
}

void rin_be_fatal_error(const Location loc, const std::string& errmsg)
{
        printf("[DEBUG FATAL] %s:%d:%d: %s\n", &loc.filename[0],
               loc.line, loc.column, &errmsg[0]);
}

void rin_be_inform(const Location loc, const std::string& infomsg)
{
        printf("[DEBUG INFORM] %s:%d:%d: %s\n", &loc.filename[0],
               loc.line, loc.column, &infomsg[0]);
}

void rin_be_get_quotechars(const char** open_quo, const char** close_quo)
{
        *open_quo =  &open_quote[0];
        *close_quo = &close_quote[0];
}

// See: https://hg.mozilla.org/mozilla-central/file/98fa9c0cff7a/js/src/jsutil.cpp#l66
void do_recoverable_abort()
{
#if defined(WIN32)
        /*
         * We used to call DebugBreak() on Windows, but amazingly, it causes
         * the MSVS 2010 debugger not to be able to recover a call stack.
         */
        *((int *) NULL) = 0;
        exit(3);
#elif defined(__APPLE__)
        /*
         * On Mac OS X, Breakpad ignores signals. Only real Mach exceptions are
         * trapped.
         */
        *((int *) NULL) = 0;  /* To continue from here in GDB: "return" then "continue". */
        raise(SIGABRT);  /* In case above statement gets nixed by the optimizer. */
#else
        raise(SIGABRT);  /* To continue from here in GDB: "signal 0". */
#endif
}
