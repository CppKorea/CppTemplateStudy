#pragma once
#include <cassert>
#include <numeric>

/* ----------------------------------------------------------
    TRAITS
---------------------------------------------------------- */
template <typename T>
class A_Traits
{
public:
    using Reference = const T&;
};

template<typename T>
class A_Scalar
{
private:
    const T &s;

public:
    constexpr A_Scalar(const T &v)
        : s(v)
    {
    };

    constexpr const T& operator[](size_t) const
    {
        return s;
    };

    constexpr size_t size() const
    {
        return 0;
    }
};

/* ----------------------------------------------------------
    OPERATORS
---------------------------------------------------------- */
template <typename T, typename X, typename Y, typename Operator>
class A_Operator
{
public:
    typename A_Traits<X>::Reference x_;
    typename A_Traits<Y>::Reference y_;

public:
    A_Operator(const X &x, const Y &y)
        : x_(x), y_(y)
    {
    };

    auto operator[](size_t index) const
    {
        return Operator()(x_[index], y_[index]);
    };

    size_t size() const
    {
        assert(x_.size() == 0 || y_.size() == 0 || x_.size() == y_.size());
        return x_.size() != 0
            ? x_.size()
            : y_.size();
    };
};

template <typename T, typename X, typename Y>
using A_Add = A_Operator<T, X, Y, std::plus<T>>;

template <typename T, typename X, typename Y>
using A_Mult = A_Operator<T, X, Y, std::multiplies<T>>;