
#include "loaders/image_loader.h"

// std
#include <algorithm>
#include <memory>
#include <mutex>
#include <vector>

// local
#include "loaders/pnm_image_loader.h"
#include "loaders/tiff_image_loader.h"
#include "core/util.h"
#include "core/log.h"

namespace {
    std::once_flag registration_flag {};
}

namespace openpiv::core {

    using image_loader_container = std::vector<image_loader_ptr_t>;

    static image_loader_container& loaders()
    {
        static image_loader_container static_loaders;
        return static_loaders;
    }

    image_loader_registry& image_loader_registry::instance()
    {
        static image_loader_registry static_registry;
        std::call_once(
            registration_flag, 
            []()
            {
                static_registry.add< pnm_image_loader >();
                static_registry.add< tiff_image_loader >();
            });
        return static_registry;
    }

    image_loader_ptr_t image_loader_registry::find( std::istream& s )
    {
        for ( auto& loader: loaders() )
            if ( loader->can_load( s ) )
                return loader->clone();

        return {};
    }

    image_loader_ptr_t image_loader_registry::find( const std::string& n )
    {
        for ( auto& loader: loaders() )
            if ( loader->name() == n )
                return loader->clone();

        return {};
    }

    bool image_loader_registry::add( image_loader_ptr_t&& loader )
    {
        if ( !loader )
            exception_builder<std::runtime_error>() << "attempting to register null image loader";

        auto name{ loader->name() };
        loaders().emplace_back( std::move(loader) );
        std::sort( loaders().begin(), loaders().end(),
                   []( const auto& lhs, const auto& rhs ) -> bool
                   {
                       return lhs->priority() < rhs->priority();
                   }
            );

        logger::info("registered: {}", name);
        return true;
    }

}
