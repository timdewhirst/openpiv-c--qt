
#pragma once

// std
#include <tuple>

// local
#include "Image.h"

template < template<typename> class ImageT, typename ContainedT >
std::tuple< ContainedT, ContainedT >
find_image_range( const ImageInterface< ImageT, ContainedT >& im )
{
    ContainedT min, max;
    auto p = std::cbegin( im );
    auto e = std::cend( im );
    min = max = *p++;
    while ( p != e )
    {
        min = *p < min ? *p : min;
        max = *p > max ? *p : max;
        ++p;
    }

    return std::make_tuple( min, max );
}
