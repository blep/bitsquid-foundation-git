#pragma once

#include <stdint.h>
#include <assert.h>


namespace foundation {
inline uint32_t size_cast( ptrdiff_t delta )
{
    assert( delta >= 0 && delta <= INT32_MAX );
    return static_cast<uint32_t>( delta );
}

inline uint32_t size_cast( size_t delta )
{
    assert( delta >= 0 && delta <= INT32_MAX );
    return static_cast<uint32_t>( delta );
}

} // namespace foundation {
