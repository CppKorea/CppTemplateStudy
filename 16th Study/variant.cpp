#include <iostream>
#include <type_traits>

/* ===========================================================
    UTILITIES
        - BASE
        - ACCESSORS
        - MODIFIERS
        - FINDERS
==============================================================
    BASE
----------------------------------------------------------- */
// TYPE-LIST FOR POP
template<typename ...Elements>
class Typelist {};

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
class FrontT<Typelist<Head, Tail...>>
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
struct TypeAt<Typelist<Head, Tail...>, 0>
{
    using Result = Head;
};

template <class Head, class ...Tail, size_t index>
struct TypeAt<Typelist<Head, Tail...>, index>
{
    using Result = typename TypeAt<Typelist<Tail...>, index - 1>::Result;
};

/* -----------------------------------------------------------
    MODIFIERS
----------------------------------------------------------- */
template<typename List>
class PopFrontT;

template<typename Head, typename... Tail>
class PopFrontT<Typelist<Head, Tail...>>
{
public:
    using Type = Typelist<Tail...>;
};

template<typename List>
using PopFront = typename PopFrontT<List>::Type;

/* -----------------------------------------------------------
    FINDERS
----------------------------------------------------------- */
// TYPE TO INDEX
template <typename List, typename T, size_t N>
struct FindIndexOfT
    : public IfThenElseT<std::is_same<Front<List>, T>::value,
                         std::integral_constant<size_t, N>,
                         FindIndexOfT<PopFront<List>, T, N+1>>
{
};

template <typename List, typename T>
using FindIndexOf = typename FindIndexOfT<List, T, 0>::Type;

// INDEX TO TYPE

/* ===========================================================
    VARIANT
        - STORAGE
        - ACCESSOR
        - FINAL CLASS
==============================================================
    STORAGE
----------------------------------------------------------- */
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

/* -----------------------------------------------------------
    ACCESSOR
----------------------------------------------------------- */
template <size_t index>
struct VariantGetter
{
    template <typename Data>
    static auto apply(const Data &obj)
    {
        return VariantGetter<index-1>::apply(obj.tail_);
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

template <typename Index, typename Container>
auto get(const Container &container)
{
	return container.get<Index>();
}

/* -----------------------------------------------------------
    FINAL CLASS
----------------------------------------------------------- */
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
        : index_(-1) {};
    
    template <typename T>
    Variant(const T &val)
        : data_(val)
    {
    };

	//----
	// ACCESSORS
	//----
	template <size_t index>
	auto get()
	{
		return VariantGetter<index>::apply(this->data_);
	}
};

int main()
{
    Variant<char, int, double> s(3.0);
    std::cout << s.get<2>() << std::endl;
     
    return 0;
}