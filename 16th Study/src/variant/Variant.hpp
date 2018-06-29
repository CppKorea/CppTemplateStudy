#pragma once

#include "VariantStorage.hpp"
#include "VariantGetter.hpp"

#include "functional.hpp"
#include <exception>

#include <iostream>
#include <typeinfo>

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
        index_(FindIndexOfT<TypeList<Types...>, T>::value)
    {
    };

    //----
    // ACCESSORS
    //----
    template <typename T>
    bool is() const
    {
		constexpr size_t N = FindIndexOfT<TypeList<Types...>, T>::value;
        return N == index_;
    };
    
    template <typename T>
    auto get()
    {
        constexpr size_t N = FindIndexOfT<TypeList<Types...>, T>::value;
        return get<N>();
    };

    template <size_t N>
    auto get()
    {
        return VariantGetter<N>::apply(data_);
    };

    template <typename Func>
    void visit(Func &&func) const
    {
        if (index_ == -1)
            throw std::invalid_argument("value is not assigned.");

        _Visit<Func, Types...>(&func);
    };

private:
    template <typename Func, typename Head, typename ...Tail>
    void _Visit(Func *func) const
    {
        if (is<Head>())
            (*func)(get<Head>());
        else if constexpr (sizeof...(Tail) > 0)
            _Visit<Func, Tail...>(func);
    };
};