
#pragma once

// openpiv
#include "core/util.h"

namespace openpiv::core {

    std::vector<core::rect>
    generate_cartesian_grid( const core::size& image_size,
                             const core::size& interrogation_size,
                             double percentage_overlap )
    {
        if ( image_size.area() == 0 )
            core::exception_builder<std::runtime_error>() << "image size must be non-zero";

        if ( interrogation_size.area() == 0 )
            core::exception_builder<std::runtime_error>() << "interrogation size must be non-zero";

        auto x_overlap = (uint32_t)(interrogation_size.width() * percentage_overlap);
        auto y_overlap = (uint32_t)(interrogation_size.height() * percentage_overlap);

        if ( percentage_overlap < 0.0 || percentage_overlap > 1.0 || x_overlap == 0 || y_overlap == 0 )
            core::exception_builder<std::runtime_error>() << "overlaps must be between 0.0 and 1.0";

        if ( interrogation_size.width() > image_size.width() ||
             interrogation_size.height() > image_size.height() )
            core::exception_builder<std::runtime_error>() << "interrogation size is bigger than image";

        auto x_count = 1 + (image_size.width()  - interrogation_size.width())  / x_overlap;
        auto y_count = 1 + (image_size.height() - interrogation_size.height()) / y_overlap;

        auto x_offset = (image_size.width()  - interrogation_size.width()  - (x_overlap * (x_count-1)))/2;
        auto y_offset = (image_size.height() - interrogation_size.height() - (y_overlap * (y_count-1)))/2;

        std::vector<core::rect> result;
        for ( size_t y=0; y < y_count; ++y )
        {
            for ( size_t x=0; x < x_count; ++x )
            {
                rect r{ {x_offset + x * x_overlap, y_offset + y * y_overlap}, interrogation_size };
                result.emplace_back( std::move(r) );
            }
        }

        return result;
    }

}
