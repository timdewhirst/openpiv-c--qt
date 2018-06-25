
#pragma once

// std
#include <iosfwd>
#include <type_traits>

// local
#include "ImageExpression.h"
#include "Point.h"

/// basic 2-dimensional image interface; designed to
/// be inherited using CRTP
template < template<typename> class ImageT, typename ContainedT >
class ImageInterface
{
public:
    using ContainedType = ContainedT;
    using DerivedType = ImageT<ContainedT>;

    inline const DerivedType* derived() const { return static_cast<const DerivedType*>( this ); }
    inline DerivedType* derived() { return static_cast<DerivedType*>( this ); }

    /// equality
    inline bool operator==(const ImageInterface& rhs) const
    {
        return derived()->operator==(rhs);
    }
    inline bool operator!=(const ImageInterface& rhs) const
    {
        return derived()->operator!=(rhs);
    }

    /// pixel accessor
    constexpr inline ContainedT& operator[](size_t i)
    {
        return derived()->operator[](i);
    }
    constexpr inline const ContainedT& operator[](size_t i) const
    {
        return derived()->operator[](i);
    }

    /// pixel accessor by point
    constexpr inline ContainedT& operator[]( const Point2<uint32_t>& xy )
    {
        return derived()->operator[](xy);
    }
    constexpr inline const ContainedT& operator[]( const Point2<uint32_t>& xy ) const
    {
        return derived()->operator[](xy);
    }

    /// raw data accessor
    constexpr inline ContainedT* data()
    {
        return derived()->data();
    }
    constexpr inline const ContainedT* data() const
    {
        return derived()->data();
    }

    /// raw data by line
    constexpr inline ContainedT* line( size_t i )
    {
        return derived()->line();
    }
    constexpr inline const ContainedT* line( size_t i ) const
    {
        return derived()->line();
    }

    /// geometry accessors
    constexpr inline const uint32_t width() const { return derived()->width(); }
    constexpr inline const uint32_t height() const { return derived()->height(); }
    constexpr inline const uint32_t pixel_count() const { return derived()->pixel_count(); }

    /// apply an operation to all pixels
    template <typename Op>
    DerivedType& apply( Op op )
    {
        for ( decltype(pixel_count()) i=0; i<pixel_count(); ++i )
            operator[](i) = op(operator[](i));

        return *derived();
    }

    // image expression assignment/evaluation
    template <typename Op, typename LeftExpr, typename RightExpr>
    DerivedType& operator=(const ImageExpression<Op, LeftExpr, RightExpr>& e)
    {
        for ( decltype(pixel_count()) i=0; i<pixel_count(); ++i )
        {
            operator[](i) = e[i];
        }

        return *derived();
    }
};

/// ostream operator
template < template<typename> class ImageT, typename ContainedT >
std::ostream& operator<<( std::ostream& os, const ImageInterface<ImageT, ContainedT>& p )
{
    os << typeid(ImageT<ContainedT>).name() << "[" << p.width() << ", " << p.height() << "]";

    return os;
}
