#pragma once

#include "VariantStorage.hpp"
#include "VariantGetter.hpp"

#include "functional.hpp"

template <typename ...Types>
class Variant
{
private:
    VariantStorage<Types...> data_;
    size_t index_;

public:
    //----
    // CONSTRUCTORS
    //----
    Variant()
        : index_(0) {};

    template <typename T>
    Variant(const T &val)
        : data_(val), 
        index_(FindIndexOfT<Typelist<Types...>, T>::value)
    {
    };

    //----
    // ACCESSORS
    //----
    template <typename T>
    bool is() const
    {
		constexpr size_t N = FindIndexOfT<Typelist<Types...>, T>::value;
        return N == index_;
    };
    
    template <typename T>
    auto get()
    {
        constexpr size_t N = FindIndexOfT<Typelist<Types...>, T>::value;
        return get<N>();
    };

    template <size_t N>
    auto get()
    {
        return VariantGetter<N>::apply(data_);
    };
};