#pragma once

template <size_t index>
struct VariantGetter
{
    template <typename Data>
    static auto apply(const Data &obj)
    {
        return VariantGetter<index - 1>::apply(obj.tail_);
    };
};

template <>
struct VariantGetter<0>
{
    template <typename Data>
    static auto apply(const Data &obj)
    {
        return obj.head_;
    };
};

