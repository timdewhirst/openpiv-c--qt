
// catch
#include "catch2/catch.hpp"

// std
#include <cstdlib>
#include <fstream>

// local
#include "test_utils.h"

// to be tested
#include "core/image_utils.h"
#include "loaders/image_loader.h"

using namespace openpiv::core;

TEST_CASE("image_loader_test - tiff_loader_mono")
{
    std::ifstream is("test-mono.tiff", std::ios::binary);
    REQUIRE(is.is_open());

    std::shared_ptr<image_loader> loader{ image_loader::find_loader(is) };
    REQUIRE(!!loader);
    REQUIRE(loader->name() == std::string("image/tiff") );

    g16_image im;
    loader->load( is, im );

    REQUIRE(im.width() == 511);
    REQUIRE(im.height() == 369);
}

TEST_CASE("image_loader_test - tiff_loader_rgba")
{
    std::ifstream is("test-rgb.tiff", std::ios::binary);
    REQUIRE(is.is_open());

    std::shared_ptr<image_loader> loader{ image_loader::find_loader(is) };
    REQUIRE(!!loader);
    REQUIRE(loader->name() == std::string("image/tiff"));

    rgba16_image im;
    loader->load( is, im );

    REQUIRE(im.width() == 511);
    REQUIRE(im.height() == 369);
}

TEST_CASE("image_loader_test - pnm_loader_save_p5")
{
    std::ifstream is("test-mono.tiff", std::ios::binary);
    REQUIRE(is.is_open());

    std::shared_ptr<image_loader> loader{ image_loader::find_loader(is) };
    REQUIRE(!!loader);

    g16_image im;
    loader->load( is, im );

    // write data
    std::shared_ptr<image_loader> writer{ image_loader::find_loader("image/x-portable-anymap") };
    REQUIRE(!!writer);
    REQUIRE(writer->name() == std::string("image/x-portable-anymap") );

    std::fstream os( "test-mono.pgm", std::ios_base::trunc | std::ios_base::out );
    writer->save( os, im );
}

TEST_CASE("image_loader_test - pnm_loader_save_p6")
{
    std::ifstream is("test-rgb.tiff", std::ios::binary);
    REQUIRE(is.is_open());

    std::shared_ptr<image_loader> loader{ image_loader::find_loader(is) };
    REQUIRE(!!loader);

    g16_image im;
    loader->load( is, im );

    // write data
    std::shared_ptr<image_loader> writer{ image_loader::find_loader("image/x-portable-anymap") };
    REQUIRE(!!writer);
    REQUIRE(writer->name() == std::string("image/x-portable-anymap") );

    std::fstream os( "test-rgb.ppm", std::ios_base::trunc | std::ios_base::out );
    writer->save( os, im );
}
