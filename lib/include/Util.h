
#pragma once

// std
#include <limits>
#include <istream>
#include <sstream>
#include <type_traits>
#include <utility>


/// wrapper to allow using stringstream to construct an
/// exception message; throw \ta E on destruction.
template < typename E >
class Thrower
{
public:
    Thrower() = default;
    ~Thrower() noexcept(false)
    {
        throw E(ss.str());
    }

    template < typename T >
    std::stringstream& operator<<( const T& v )
    {
        ss << v;
        return ss;
    }

    std::stringstream ss;
};

/// simple RAII for std::istream multi-char peek
class Peeker
{
public:
    Peeker( std::istream& is )
        : is_( is )
    {
        pos_ = is.tellg();
        // this will only work if we can read and rewind
        if ( pos_ == -1 || !is.good() )
            Thrower<std::runtime_error>() << "input stream doesn't support input position indicator";

        success_ = true;
    }

    ~Peeker()
    {
        if ( success_ )
            is_.seekg(pos_);
    }

private:
    std::istream& is_;
    std::istream::pos_type pos_ {-1};
    bool success_ {false};
};

/// allow safe conversion from unsigned to signed
template <typename To, typename From>
constexpr
typename std::enable_if<
    std::is_signed<To>::value && std::is_unsigned<From>::value && (sizeof(To) > sizeof(From)),
    To
    >::type
checked_unsigned_conversion(const From& v)
{
    return static_cast<To>(v);
}

/// allow safe conversion from unsigned to signed
template <typename To, typename From>
constexpr
typename std::enable_if<
    std::is_signed<To>::value && std::is_unsigned<From>::value && (sizeof(To) == sizeof(From)),
    To
    >::type
checked_unsigned_conversion(const From& v)
{
    if (v>std::numeric_limits<To>::max())
        Thrower<std::range_error>() << "unable to convert " << v << " to " << typeid(To).name() << " as value would be truncated";

    return static_cast<To>(v);
}

/// pair of helpers to determine if all types Ts are convertible to T
template <typename To, typename From, typename... R>
struct are_all_convertible {
    constexpr static bool value =
        std::is_convertible<From,To>::value &&
        are_all_convertible<To,R...>::value;
};

template <typename To, typename From>
struct are_all_convertible<To, From> {
    constexpr static bool value = std::is_convertible<From,To>::value;
};

/// pair of helpers to determine if all types Ts are equal to T
template <typename TypeA, typename TypeB, typename... R>
struct are_all_equal {
    constexpr static bool value =
        std::is_same<TypeA, TypeB>::value &&
        are_all_equal<TypeA, R...>::value;
};

template <typename TypeA, typename TypeB>
struct are_all_equal<TypeA, TypeB> {
    constexpr static bool value = std::is_same<TypeA, TypeB>::value;
};

/// helpers to convert std::array<T, N> to std::array<U, N>
template<typename Dest, typename Src, std::size_t N, std::size_t... Is>
auto convert_array_to_impl(const std::array<Src, N> &src, std::index_sequence<Is...>) {
    return std::array<Dest, N>{{static_cast<Dest>(src[Is])...}};
}

template<typename Dest, typename Src, std::size_t N>
auto convert_array_to(const std::array<Src, N> &src) {
    return convert_array_to_impl<Dest>(src, std::make_index_sequence<N>());
}
