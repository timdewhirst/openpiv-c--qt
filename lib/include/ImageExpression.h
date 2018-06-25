
#pragma once

// std
#include <type_traits>
#include <utility>

// local
#include "PixelTypes.h"

// forwards
template < typename ContainedT > class ImageView;
template < typename ContainedT > class Image;
template < template <typename> class ImageT, typename ContainedT > class ImageInterface;


/// wrapper around a constant to adhere to requirements of
/// ImageExpression
template <typename T>
class ConstImageExpressionNode
{
public:
    using type = T;

    ConstImageExpressionNode() = default;
    ConstImageExpressionNode(const ConstImageExpressionNode&) = default;
    ConstImageExpressionNode(ConstImageExpressionNode&&) = default;
    explicit ConstImageExpressionNode(const T& t)
        : t_(t)
    {}

    explicit ConstImageExpressionNode(T&& t)
        : t_(std::move(t))
    {}

    constexpr T operator[](size_t) const
    {
        return t_;
    }

private:
    T t_ {};
};

/// wrapper around an image interface, allowing storage of
/// an expression node without copying the image data
template <template <typename> class ImageT,
          typename ContainedT,
          typename T = ImageInterface<ImageT, ContainedT>>
class ImageInterfaceExpressionNode
{
public:
    using type = T;

    ImageInterfaceExpressionNode() = default;
    ImageInterfaceExpressionNode(const ImageInterfaceExpressionNode&) = default;
    ImageInterfaceExpressionNode(ImageInterfaceExpressionNode&&) = default;
    explicit ImageInterfaceExpressionNode(const T& im)
        : im_(im)
    {}

    constexpr ContainedT operator[](size_t i) const
    {
        return im_[i];
    }

private:
    const T& im_;
};


/// template expression to construct chains of operations to perform
/// on a per-pixel level for images; \ta Op is always passed
/// as a template parameter to allow full expansion without
/// indirection
template <typename Op, typename LeftExpr, typename RightExpr>
class ImageExpression
{
public:
    using type = typename LeftExpr::type;

    ImageExpression() = delete;
    ImageExpression(const ImageExpression&) = default;
    ImageExpression& operator=(const ImageExpression&) = default;
    ImageExpression(ImageExpression&&) = default;
    ImageExpression& operator=(ImageExpression&&) = default;

    template < typename LE, typename RE >
    ImageExpression(LE&& le, RE&& re)
        : le_(std::forward<LE>(le))
        , re_(std::forward<RE>(re))
    {}

    inline const LeftExpr& le() const { return le_; }
    inline const RightExpr& re() const { return re_; }

    inline auto operator[](size_t index) const ->
        decltype( Op::apply(this->le()[index], this->re()[index]) )
    {
        return Op::apply(le()[index], re()[index]);
    }

private:
    LeftExpr le_;
    RightExpr re_;
};

// fundamental pixel operators
template <typename T>
struct plus_op
{
    static T apply(const T& lhs, const T& rhs)
    {
        return lhs + rhs;
    }
};

template <typename T>
struct minus_op
{
    static T apply(const T& lhs, const T& rhs)
    {
        return lhs - rhs;
    }
};

template <typename T>
struct mult_op
{
    static T apply(const T& lhs, const T& rhs)
    {
        return lhs * rhs;
    }
};

template <typename T>
struct div_op
{
    static T apply(const T& lhs, const T& rhs)
    {
        return lhs / rhs;
    }
};

template <typename T>
struct mod_op
{
    static T apply(const T& lhs, const T& rhs)
    {
        return lhs % rhs;
    }
};


///
template <typename T>
struct is_ie_inputtype : std::false_type
{};

template <typename LE, typename RE, typename OP>
struct is_ie_inputtype<ImageExpression<OP, LE, RE>> : std::true_type
{
    using type = typename LE::type;
    using node_type = ImageExpression<OP, LE, RE>;
};

template <typename ContainedT>
struct is_ie_inputtype<Image<ContainedT>> : std::true_type
{
    using type = ContainedT;
    using node_type = ImageInterfaceExpressionNode<Image, ContainedT>;
};

template <typename ContainedT>
struct is_ie_inputtype<ImageView<ContainedT>> : std::true_type
{
    using type = ContainedT;
    using node_type = ImageInterfaceExpressionNode<ImageView, ContainedT>;
};

template <typename T>
struct is_ie_inputtype<ConstImageExpressionNode<T>> : std::true_type
{
    using type = T;
    using node_type = ConstImageExpressionNode<T>;
};

///
template <
    typename LE,
    typename RE,
    typename T = typename std::enable_if<
        is_ie_inputtype<LE>::value && is_ie_inputtype<RE>::value,
        typename LE::type>::type,
    typename Op = plus_op<T>,
    typename LEWrapped = typename is_ie_inputtype<LE>::node_type,
    typename REWrapped = typename is_ie_inputtype<RE>::node_type>
ImageExpression<Op, LEWrapped, REWrapped>
operator+(const LE& le, const RE& re)
{
    return ImageExpression<Op, LEWrapped, REWrapped>{
        LEWrapped{ le },
        REWrapped{ re } };
}

template <
    typename LE,
    typename T,
    typename Op = plus_op<T>,
    typename E_ = typename std::enable_if<is_ie_inputtype<LE>::value && is_pixeltype<T>::value>::type,
    typename LEWrapped = typename is_ie_inputtype<LE>::node_type,
    typename REWrapped = ConstImageExpressionNode<T>>
ImageExpression<Op, LEWrapped, REWrapped>
operator+(const LE& le, T&& v)
{
    return ImageExpression<Op, LEWrapped, REWrapped>{
        LEWrapped{ le },
        REWrapped{ std::forward<T>(v) } };
}

template <
    typename T,
    typename RE,
    typename Op = plus_op<T>,
    typename E_ = typename std::enable_if<is_ie_inputtype<RE>::value && is_pixeltype<T>::value>::type,
    typename LEWrapped = ConstImageExpressionNode<T>,
    typename REWrapped = typename is_ie_inputtype<RE>::node_type>
ImageExpression<Op, LEWrapped, REWrapped>
operator+(T&& v, const RE& re)
{
    return ImageExpression<Op, LEWrapped, REWrapped>{
        LEWrapped{ std::forward<T>(v) },
        REWrapped{ re } };
}

///
template <
    typename LE,
    typename RE,
    typename T = typename std::enable_if<
        is_ie_inputtype<LE>::value && is_ie_inputtype<RE>::value,
        typename LE::type>::type,
    typename Op = minus_op<T>,
    typename LEWrapped = typename is_ie_inputtype<LE>::node_type,
    typename REWrapped = typename is_ie_inputtype<RE>::node_type>
ImageExpression<Op, LEWrapped, REWrapped>
operator-(const LE& le, const RE& re)
{
    return ImageExpression<Op, LEWrapped, REWrapped>{
        LEWrapped{ le },
        REWrapped{ re } };
}

template <
    typename LE,
    typename T,
    typename Op = minus_op<T>,
    typename E_ = typename std::enable_if<is_ie_inputtype<LE>::value && is_pixeltype<T>::value>::type,
    typename LEWrapped = typename is_ie_inputtype<LE>::node_type,
    typename REWrapped = ConstImageExpressionNode<T>>
ImageExpression<Op, LEWrapped, REWrapped>
operator-(const LE& le, T&& v)
{
    return ImageExpression<Op, LEWrapped, REWrapped>{
        LEWrapped{ le },
        REWrapped{ std::forward<T>(v) } };
}

template <
    typename T,
    typename RE,
    typename Op = minus_op<T>,
    typename E_ = typename std::enable_if<is_ie_inputtype<RE>::value && is_pixeltype<T>::value>::type,
    typename LEWrapped = ConstImageExpressionNode<T>,
    typename REWrapped = typename is_ie_inputtype<RE>::node_type>
ImageExpression<Op, LEWrapped, REWrapped>
operator-(T&& v, const RE& re)
{
    return ImageExpression<Op, LEWrapped, REWrapped>{
        LEWrapped{ std::forward<T>(v) },
        REWrapped{ re } };
}

///
template <
    typename LE,
    typename RE,
    typename T = typename std::enable_if<
        is_ie_inputtype<LE>::value && is_ie_inputtype<RE>::value,
        typename LE::type>::type,
    typename Op = div_op<T>,
    typename LEWrapped = typename is_ie_inputtype<LE>::node_type,
    typename REWrapped = typename is_ie_inputtype<RE>::node_type>
ImageExpression<Op, LEWrapped, REWrapped>
operator/(const LE& le, const RE& re)
{
    return ImageExpression<Op, LEWrapped, REWrapped>{
        LEWrapped{ le },
        REWrapped{ re } };
}

template <
    typename LE,
    typename T,
    typename Op = div_op<T>,
    typename E_ = typename std::enable_if<is_ie_inputtype<LE>::value && is_pixeltype<T>::value>::type,
    typename LEWrapped = typename is_ie_inputtype<LE>::node_type,
    typename REWrapped = ConstImageExpressionNode<T>>
ImageExpression<Op, LEWrapped, REWrapped>
operator/(const LE& le, T&& v)
{
    return ImageExpression<Op, LEWrapped, REWrapped>{
        LEWrapped{ le },
        REWrapped{ std::forward<T>(v) } };
}

template <
    typename T,
    typename RE,
    typename Op = div_op<T>,
    typename E_ = typename std::enable_if<is_ie_inputtype<RE>::value && is_pixeltype<T>::value>::type,
    typename LEWrapped = ConstImageExpressionNode<T>,
    typename REWrapped = typename is_ie_inputtype<RE>::node_type>
ImageExpression<Op, LEWrapped, REWrapped>
operator/(T&& v, const RE& re)
{
    return ImageExpression<Op, LEWrapped, REWrapped>{
        LEWrapped{ std::forward<T>(v) },
        REWrapped{ re } };
}

///
template <
    typename LE,
    typename RE,
    typename T = typename std::enable_if<
        is_ie_inputtype<LE>::value && is_ie_inputtype<RE>::value,
        typename LE::type>::type,
    typename Op = mult_op<T>,
    typename LEWrapped = typename is_ie_inputtype<LE>::node_type,
    typename REWrapped = typename is_ie_inputtype<RE>::node_type>
ImageExpression<Op, LEWrapped, REWrapped>
operator*(const LE& le, const RE& re)
{
    return ImageExpression<Op, LEWrapped, REWrapped>{
        LEWrapped{ le },
        REWrapped{ re } };
}

template <
    typename LE,
    typename T,
    typename Op = mult_op<T>,
    typename E_ = typename std::enable_if<is_ie_inputtype<LE>::value && is_pixeltype<T>::value>::type,
    typename LEWrapped = typename is_ie_inputtype<LE>::node_type,
    typename REWrapped = ConstImageExpressionNode<T>>
ImageExpression<Op, LEWrapped, REWrapped>
operator*(const LE& le, T&& v)
{
    return ImageExpression<Op, LEWrapped, REWrapped>{
        LEWrapped{ le },
        REWrapped{ std::forward<T>(v) } };
}

template <
    typename T,
    typename RE,
    typename Op = mult_op<T>,
    typename E_ = typename std::enable_if<is_ie_inputtype<RE>::value && is_pixeltype<T>::value>::type,
    typename LEWrapped = ConstImageExpressionNode<T>,
    typename REWrapped = typename is_ie_inputtype<RE>::node_type>
ImageExpression<Op, LEWrapped, REWrapped>
operator*(T&& v, const RE& re)
{
    return ImageExpression<Op, LEWrapped, REWrapped>{
        LEWrapped{ std::forward<T>(v) },
        REWrapped{ re } };
}

///
template <
    typename LE,
    typename RE,
    typename T = typename std::enable_if<
        is_ie_inputtype<LE>::value && is_ie_inputtype<RE>::value,
        typename LE::type>::type,
    typename Op = mod_op<T>,
    typename LEWrapped = typename is_ie_inputtype<LE>::node_type,
    typename REWrapped = typename is_ie_inputtype<RE>::node_type>
ImageExpression<Op, LEWrapped, REWrapped>
operator%(const LE& le, const RE& re)
{
    return ImageExpression<Op, LEWrapped, REWrapped>{
        LEWrapped{ le },
        REWrapped{ re } };
}

template <
    typename LE,
    typename T,
    typename Op = mod_op<T>,
    typename E_ = typename std::enable_if<is_ie_inputtype<LE>::value && is_pixeltype<T>::value>::type,
    typename LEWrapped = typename is_ie_inputtype<LE>::node_type,
    typename REWrapped = ConstImageExpressionNode<T>>
ImageExpression<Op, LEWrapped, REWrapped>
operator%(const LE& le, T&& v)
{
    return ImageExpression<Op, LEWrapped, REWrapped>{
        LEWrapped{ le },
        REWrapped{ std::forward<T>(v) } };
}

template <
    typename T,
    typename RE,
    typename Op = mod_op<T>,
    typename E_ = typename std::enable_if<is_ie_inputtype<RE>::value && is_pixeltype<T>::value>::type,
    typename LEWrapped = ConstImageExpressionNode<T>,
    typename REWrapped = typename is_ie_inputtype<RE>::node_type>
ImageExpression<Op, LEWrapped, REWrapped>
operator%(T&& v, const RE& re)
{
    return ImageExpression<Op, LEWrapped, REWrapped>{
        LEWrapped{ std::forward<T>(v) },
        REWrapped{ re } };
}