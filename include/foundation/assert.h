#pragma once

#define FDT_ASSERT( mustBeTrue ) \
    (void)(!!(mustBeTrue) ||  foundation::handleAssertionFailure( #mustBeTrue, __FILE__, __LINE__ ))

namespace foundation {
    
int handleAssertionFailure( const char *expr, const char *file, int lineNo );

} // namespace foundation {