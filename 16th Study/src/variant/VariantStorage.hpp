#pragma once

template <typename ...Types>
union VariantStorage;

template <typename Head, typename ...Tail>
union VariantStorage<Head, Tail...>
{
public:
    Head head_;
    VariantStorage<Tail...> tail_;

public:
    VariantStorage() {};
    VariantStorage(const Head &head)
        : head_(head) {};

    template <typename T>
    VariantStorage(const T &val)
        : tail_(val) {};
};

template<>
union VariantStorage<>
{
};