#include <foundation/assert.h>
#include <foundation/snprintf_msvc.h>
#include <stdlib.h>
#include "windows_lean.h"

namespace foundation {

int handleAssertionFailure( const char *expr, const char *file, int lineNo )
{
    static char buffer[ 1024 ];
    snprintf( buffer, sizeof( buffer ), "Assertion failed in %s(%d): %s\n", file, lineNo, expr );
    buffer[ sizeof( buffer ) - 1 ] = 0;
    OutputDebugStringA( buffer );

    DebugBreak();
    abort();
}


} // namespace foundation {
