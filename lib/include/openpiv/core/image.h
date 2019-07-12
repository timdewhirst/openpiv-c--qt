
#pragma once

// std
#include <array>
#include <cstdint>
#include <exception>
#include <iostream>
#include <typeinfo>
#include <type_traits>
#include <utility>
#include <vector>

// local
#include "core/image_expression.h"
#include "core/image_type_traits.h"
#include "core/pixel_types.h"
#include "core/point.h"
#include "core/rect.h"
#include "core/size.h"
#include "core/util.h"
#include "core/exception_builder.h"

namespace openpiv::core {

/// basic 2-dimensional image; data is stored as a
/// contiguous array of type T
template < typename T >
class image
{
public:
    using type = T;
    using pixel_t = T;
    using index_t = size_t;
    using data_t = typename std::vector<T>;

    using iterator = std::add_pointer_t<T>;
    using const_iterator = std::add_pointer_t<std::add_const_t<T>>;
    // using reverse_iterator = std::add_pointer_t<T>;
    // using const_reverse_iterator = std::add_pointer_t<std::add_const_t<T>>;

    // ctor
    image()
        : image( 0, 0 )
    {}

    image( const image& rhs ) { operator=(rhs); }
    image( image&& rhs ) { operator=(std::move(rhs)); }

    /// empty image
    image( uint32_t w, uint32_t h )
        : image( core::size{w, h} )
    {}
    image( const core::size& s )
    {
        resize( s );
    }

    /// image with default value
    image( uint32_t w, uint32_t h, T value )
        : image( {w, h}, value )
    {}
    image( const core::size& s, T value )
    {
        resize( s, value );
    }

    /// conversion from another similar image; expensive!
    template < template<typename> class ImageT,
               typename ContainedT,
               typename = typename std::enable_if_t<
                   pixeltype_is_convertible_v< ContainedT, T > &&
                   is_imagetype_v< ImageT<ContainedT> > >
               >
    explicit image( const ImageT<ContainedT>& p )
    {
        operator=(p);
    }

    template <typename E,
              typename = typename std::enable_if_t< is_imageexpression_v<E> > >
    explicit image( const E& e )
    {
        operator=(e);
    }

    /// resize the image; this is destructive and any data contained
    /// after a re-size should be considered invalid
    inline void resize( uint32_t w, uint32_t h ) { resize({w,h}); }
    inline void resize( uint32_t w, uint32_t h, T value ) { resize({w,h}, value); }

    /// resize the image; this is destructive and any data contained
    /// after a re-size should be considered invalid
    inline void resize( const core::size& s )
    {
        if ( s == size() )
            return;

        width_ = s.width();
        height_ = s.height();
        associate_data();
    }
    inline void resize( const core::size& s, T value )
    {
        if ( s == size() )
            return;

        width_ = s.width();
        height_ = s.height();
        associate_data(value);
    }

    /// assignment
    inline image& operator=(const image& rhs)
    {
        resize( rhs.size() );
        typed_memcpy( data(), rhs.data(), rhs.pixel_count() );

        return *this;
    }

    /// move assignment
    inline image& operator=(image&& rhs)
    {
        small_data_   = std::move(rhs.small_data_);
        large_data_   = std::move(rhs.large_data_);
        width_        = std::move(rhs.width_);
        height_       = std::move(rhs.height_);
        associate_data();

        return *this;
    }

    /// conversion assignment
    template < template<typename> class ImageT,
               typename ContainedT,
               typename = typename std::enable_if_t<
                   pixeltype_is_convertible_v< ContainedT, T > &&
                   is_imagetype_v< ImageT<ContainedT> > >
               >
    inline image& operator=( const ImageT<ContainedT>& p )
    {
        resize( p.size() );
        for ( index_t i=0; i<p.pixel_count(); ++i )
            convert( p[i], data()[i] );

        return *this;
    }

    template <typename E,
              typename = typename std::enable_if_t< is_imageexpression_v<E> > >
    inline image& operator=(const E& e)
    {
        resize( e.size() );
        for ( index_t i=0; i<pixel_count(); ++i )
            data()[i] = e[i];

        return *this;
    }

    /// equality
    inline bool operator==(const image& rhs) const
    {
        return
            width_ == rhs.width_ &&
            height_ == rhs.height_ &&
            typed_memcmp( data(), rhs.data(), pixel_count() ) == 0;
    }
    inline bool operator!=(const image& rhs) const { return !operator==(rhs); }

    /// pixel accessor
    inline T& operator[](size_t i) { return data()[i]; }
    inline const T& operator[](size_t i) const { return const_cast<image*>(this)->operator[](i); }

    /// pixel accessor by point
    inline T& operator[]( const point2<uint32_t>& xy ) { return data()[xy[1]*width_ + xy[0]]; }
    inline const T& operator[]( const point2<uint32_t>& xy ) const
    {
        return const_cast<image*>(this)->operator[](xy);
    }

    /// raw data accessor
    inline T* data() { return (pixel_count() > small_data_.size()) ? &large_data_[0] : &small_data_[0]; }
    inline const T* data() const { return const_cast<image*>(this)->data(); }

    /// raw data by line
    inline T* line( size_t i )
    {
        if (i>height_)
            exception_builder<std::range_error>() << "line out of range (" << i << ", max is: " << height_ << ")";

        return &data()[i*width_];
    }
    inline const T* line( size_t i ) const { return const_cast<image*>(this)->line(i); }

    /// iterators
    inline iterator begin() { return data(); }
    inline iterator end() { return data() + pixel_count(); }
    inline const_iterator begin() const { return data(); }
    inline const_iterator end() const { return data() + pixel_count(); }
    // reverse_iterator rbegin() { return std::rbegin( data_ ); }
    // reverse_iterator rend() { return std::rend( data_ ); }
    // const_reverse_iterator rbegin() const { return std::rbegin( data_ ); }
    // const_reverse_iterator rend() const { return std::rend( data_ ); }

    /// geometry accessors
    inline uint32_t width() const { return width_; }
    inline uint32_t height() const { return height_; }
    inline core::size size() const { return { width_, height_ }; }
    inline index_t pixel_count() const { return width_ * height_; }
    inline core::rect rect() const { return core::rect{ {}, { width_, height_ } }; }

    /// swap
    inline void swap( image& rhs )
    {
        std::swap( width_, rhs.width_ );
        std::swap( height_, rhs.height_ );
        std::swap( small_data_, rhs.small_data_ );
        std::swap( large_data_, rhs.large_data_ );
        associate_data();
    }

private:
    uint32_t width_ = {};
    uint32_t height_ = {};

    inline void associate_data()
    {
        if ( pixel_count() > small_data_.size() )
        {
            large_data_.resize( pixel_count() );
        }
    }

    inline void associate_data(T value)
    {
        associate_data();
        typed_memset( data(), value, pixel_count() );
    }

    std::array<T, 32*32> small_data_;
    data_t large_data_;
};


template <typename PixelT>
void swap( image<PixelT>& lhs, image<PixelT>& rhs )
{
    lhs.swap(rhs);
}

/// ostream operator
template < typename T >
std::ostream& operator<<( std::ostream& os, const image<T>& p )
{
    os << "image<" << typeid(T).name() << ">[" << p.width() << ", " << p.height() << "] "
       << "data @ " << (void*)p.data();

    return os;
}


/// standard image types
using g8_image     = image< g_8 >;
using g16_image    = image< g_16 >;
using gf_image     = image< g_f >;
using rgba8_image  = image< rgba_8 >;
using rgba16_image = image< rgba_16 >;
using cf_image     = image< c_f >;

}
