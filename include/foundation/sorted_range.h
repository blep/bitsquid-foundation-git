#pragma once

#include <stdint.h>
#include <type_traits>

namespace foundation {

namespace sorted_range {
    template<typename It, typename Size, typename Key>
    typename std::enable_if<std::is_integral<Size>::value, Size>::type find( It begin, Size size, Key key );
} // namespace sorted_range {


template<typename It, typename Size, typename Key>
typename std::enable_if<std::is_integral<Size>::value, Size>::type
sorted_range::find( It begin, Size size, Key key )
{
    Size imin = 0;
    Size imax = size;
    while ( imin < imax )
    {
        Size imid = (imax + imin) / 2;
        if ( *( begin + imid ) < key )
            imin = imid + 1;
        else
            imax = imid;
    }
    if ( imin == imax  &&  *(begin+imin) == key )
        return imin;
    return Size(0)-1;
}


} // namespace foundation {

