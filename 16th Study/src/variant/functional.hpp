#pragma once
#include <type_traits>

/* ===========================================================
    META FUNCTIONS
        - BASE
        - ACCESSORS
        - MODIFIERS
        - FINDERS
==============================================================
    BASE
----------------------------------------------------------- */
// TYPE-LIST FOR POP
template<typename ...Elements>
class TypeList {};

// MP - IF THEN ELSE
template <bool Condition, typename TrueType, typename FalseType>
struct IfThenElseT
{
    using Type = TrueType;
};

template <typename TrueType, typename FalseType>
struct IfThenElseT<false, TrueType, FalseType>
{
    using Type = FalseType;
};

template <bool Condition, typename TrueType, typename FalseType>
using IfThenElse = typename IfThenElseT<Condition, TrueType, FalseType>::Type;

/* -----------------------------------------------------------
    ACCESSORS
----------------------------------------------------------- */
// FRONT
//----
template<typename List>
class FrontT;

template<typename Head, typename ...Tail>
class FrontT<TypeList<Head, Tail...>>
{
public:
    using Type = Head;
};

template<typename List>
using Front = typename FrontT<List>::Type;

//----
// AT
//----
template <class List, size_t index>
struct TypeAt;

template <class Head, class ...Tail>
struct TypeAt<TypeList<Head, Tail...>, 0>
{
    using Result = Head;
};

template <class Head, class ...Tail, size_t index>
struct TypeAt<TypeList<Head, Tail...>, index>
{
    using Result = typename TypeAt<TypeList<Tail...>, index - 1>::Result;
};

/* -----------------------------------------------------------
    MODIFIERS
----------------------------------------------------------- */
template<typename List>
class PopFrontT;

template<typename Head, typename... Tail>
class PopFrontT<TypeList<Head, Tail...>>
{
public:
    using Type = TypeList<Tail...>;
};

template<typename List>
using PopFront = typename PopFrontT<List>::Type;

/* -----------------------------------------------------------
    FINDERS
----------------------------------------------------------- */
// TYPE TO INDEX
template <typename List, typename T, size_t N = 0>
struct FindIndexOfT
    : public IfThenElse<std::is_same<Front<List>, T>::value,
                        std::integral_constant<size_t, N>,
                        FindIndexOfT<PopFront<List>, T, N+1>>
{
};