#pragma once
#include <cassert>
#include <cstddef>

template <typename T>
class SArray
{
private:
    T *data_;
    size_t size_;

public:
    /* ----------------------------------------------------------
        CONSTRUCTORS
    ---------------------------------------------------------- */
    // CAPACITY CONSTRUCTOR
    explicit SArray(size_t size)
        : data_(new T[size]),
        size_(size)
    {
        init();
    };

    // COPY CONSTRUCTOR
    template <typename X>
    SArray(const SArray<X> &obj)
        : data_(new T[size]),
        size_(size)
    {
        copy(obj);
    };

    template <typename X>
    SArray<T>& operator=(const SArray<X> &obj)
    {
        copy(obj);
        return *this;
    };

    // DESTRUCTOR
    ~SArray()
    {
        delete[] data_;
    };

protected:
    void init()
    {
        for (size_t i = 0; i < size(); ++i)
            data_[i] = T();
    };

    template <typename X>
    void copy(SArray<X> &obj)
    {
        assert(size() == obj.size());
        for (size_t i = 0; i < size(); ++i)
            data_[i] = obj[i];
    };

public:
    /* ----------------------------------------------------------
        ACCESSORS
    ---------------------------------------------------------- */
    size_t size() const
    {
        return size_;
    }

    T& operator[](size_t index)
    {
        return data_[index];
    };
    const T& operator[](size_t index) const
    {
        return data_[index];
    };

    /* ----------------------------------------------------------
        SELF OPERATORS
    ---------------------------------------------------------- */
    template <typename X>
    SArray<T>& operator+=(const SArray<X> &obj)
    {
        assert(size() == obj.size());
        for (size_t i = 0; i < obj.size(); ++i)
            data_[i] += obj[i];
        return *this;
    };

    template <typename X>
    SArray<T>& operator*=(const SArray<X> &obj)
    {
        assert(size() == obj.size());
        for (size_t i = 0; i < obj.size(); ++i)
            data_[i] *= obj[i];
        return *this;
    };

    template <typename X>
    SArray<T>& operator*=(const T &val)
    {
        for (size_t i = 0; i < a.size(); ++i)
            data_[i] *= val;
        return *this;
    };
};

template <typename X, typename Y, typename Ret = decltype(X() + Y())>
SArray<Ret> operator+(const SArray<X> &x, SArray<Y> &y)
{
    assert(x.size() == y.size());

    SArray<Ret> ret(x.size());
    for (size_t i = 0; i < x.size(); ++i)
        ret[i] = x[i] + y[i];

    return ret;
}

template <typename X, typename Y, typename Ret = decltype(X() * Y())>
SArray<Ret> operator*(const SArray<X> &x, SArray<Y> &y)
{
    assert(x.size() == y.size());

    SArray<Ret> ret(x.size());
    for (size_t i = 0; i < x.size(); ++i)
        ret[i] = x[i] * y[i];

    return ret;
}

template <typename X, typename Y, typename Ret = decltype(X() * Y())>
SArray<Ret> operator*(const X &val, SArray<Y> &elems)
{
    SArray<Ret> ret(elems.size());
    for (size_t i = 0; i < elems.size(); ++i)
        ret[i] = val * elems[i];

    return ret;
}