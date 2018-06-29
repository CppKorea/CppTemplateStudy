#pragma once

#include "SArray.hpp"
#include "operators.hpp"

template <typename T, typename Data = SArray<T>>
class Array
{
private:
    Data data_;

public:
    /* ----------------------------------------------------------
        CONSTRUCTORS
    ---------------------------------------------------------- */
    explicit Array(size_t s)
        : data_(s)
    {
    };

    Array(const Data &obj)
        : data_(obj)
    {
    };

    Array& operator=(const Array &obj)
    {
        assert(size() == obj.size());
        for (size_t i = 0; i < size(); ++i)
            data_[i] = obj[i];
        return *this;
    };

    /* ----------------------------------------------------------
        ACCESSORS
    ---------------------------------------------------------- */
    size_t size() const
    {
        return data_.size();
    };

    Data& data()
    {
        return data_;
    };
    const Data& data() const
    {
        return data_;
    };

    inline T& operator[](size_t index)
    {
        assert(index < size());
        return data_[index];
    };
    inline const T& operator[](size_t index) const
    {
        assert(index < size());
        return data_[index];
    };
};

template <typename T, typename D1, typename D2>
auto operator+(const Array<T, D1> &x, const Array<T, D2> &y) -> Array<T, A_Add<T, D1, D2>>
{
    return Array<T, A_Add<T, D1, D2>>
    (
        A_Add<T, D1, D2>(x.data(), y.data())
    );
}

template <typename T, typename D1, typename D2>
auto operator*(const Array<T, D1> &x, const Array<T, D2> &y) -> Array<T, A_Mult<T, D1, D2>>
{
    return Array<T, A_Mult<T, D1, D2>>
    (
        A_Mult<T, D1, D2>(x.data(), y.data())
    );
}

template <typename T, typename D2>
auto operator*(T x, const Array<T, D2> &y) -> Array<T, A_Mult<T, A_Scalar<T>, D2>>
{
    auto op = A_Mult<T, A_Scalar<T>, D2>(A_Scalar<T>(x), y.data());
    return Array<T, A_Mult<T, A_Scalar<T>, D2>>(op);
}