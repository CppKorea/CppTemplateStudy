# Ch24. Typelists
* ## 발표 순서
    1. ### 개요
    1. ### 동기
    1. ### 응용 알고리즘
    1. ### Nontype Typelists
    1. ### Pack Expansion을 이용한 최적화 알고리즘
    1. ### Afternotes
    1. ### 여담

---

## 1. Overview
* ### 템플릿책 24장에서 배울 수 있는 것들
    - Typelist의 정의
        * A type that represents a list of types and can be manipulated by a *template metaprogram*.
    - 함수형 프로그래밍과의 관계
        * Scheme, ML, 그리고 Haskell 등의 `list`와 비슷한 기능
    - 응용 알고리즘들
        * 먼저 나오는 것들이 뒤따르는 것을 만드는 기초 도구로 작용
* ### 다른 곳에서 보충해야 할 것들
    - 왜 생겨난건지
    - 옛날 C++에서의 typelist 사용법
* ### 발표내용의 구성
    1. 왜 생겨났으며, 뭐할때 쓰는건지 알아보기
    1. 옛날 C++에서의 typelist 사용법과 비교하며 구현 가능한 알고리즘 살펴보기
        * 고전 vs 신형 C++ 예제들은 서로 개별적인 맥락을 가집니다. (출처가 상이합니다.)
    1. 여담...

---

## 2. Motive
```C++
class WidgetFactory
{
public:
    virtual Window* CreateWindow() = 0;
    virtual Button* CreateButton() = 0;
    virtual ScrollBar* CreateScrollBar() = 0;
};
```
> Abstract Factory 패턴을 사용하는, 그래픽 API 예시
* 이런 API를 좀 더 **generic하게** 사용하기
    - 시도 1: 클래스 템플릿 적용하기
    ```C++
    template <class T>
    T* MakeRedWidget(WidgetFactory& factory)
    {
        T* pW = factory.CreateT(); // huh???
        pW->SetColor(RED);
        return pW;
    }
    ```

    > 문제점: 템플릿 파라미터 `T`가 개체로 바뀔때, `CreateT()`의 `T`는 안바뀜

    - 시도 2: 템플릿 메소드도 함께 적용하기
    ```C++
    template <class T>
    T* MakeRedWidget(WidgetFactory& factory)
    {
        T* pW = factory.Create<T>(); // aha!
        pW->SetColor(RED);
        return pW;
    }
    ```
    > 문제점: 순수가상함수에 템플릿 적용이 안 됨.
* 결국, generic한 인터페이스 사용을 위해 Typelist와 같은 개념이 필요
```C++
template <class T, class U>
struct Typelist
{
    typedef T Head;
    typedef U Tail;
};

typedef Typelist<signed char, 
    Typelist<short, 
        Typelist<int,
            Typelist<long,
                Typelist<long long,
                    Typelist<NullType> > > > > >
    SignedIntegralTypes;
```
> 옛날 방식 Typelist
>> 뒷 부분에, 닫는 꺽쇠 사이의 **공백** 주의!<br>
>> 손으로 재귀하는 부분 -> 매크로를 이용하여 선형화...
```C++
template<typename... Elements>
class Typelist {};

using SignedIntegralTypes = 
    Typelist<signed char, short, int, long, long long>;
```
> 새로운 방식 Typelist
>> Parameter pack (_since C++11_)으로 애초에 선형화!
* Typelist의 결론적인 취지: **컴파일 타임**에 **자료형**을 ***유연하게*** 결정하기.

---

## 3. Algorithms
### 3.0. 기초 알고리즘 준비
* 목적: 옛날 C++에서 Head/Tail을 사용한 테크닉을 신형 C++에서도 사용하기 위하여.
1. Front: 가장 앞단의 원소 보기
```C++
template<typename List>
class FrontT;

template<typename Head, typename... Tail>
class FrontT<Typelist<Head, Tail...>>
{
public:
    using Type = Head;
};

template<typename List>
using Front = typename FrontT<List>::Type;
```
2. PopFront: 가장 앞단의 원소 빼내기
```C++
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
```
3. PushFront: 가장 앞단에 원소 집어넣기
```C++
template<typename List, typename NewElement>
class PushFrontT;

template<typename... Elements, typename NewElement> 
class PushFrontT<Typelist<Elements...>, NewElement>{
public:
    using Type = Typelist<NewElement, Elements...>;
};

template<typename List, typename NewElement>
using PushFront = typename PushFrontT<List, NewElement>::Type;
```
4. IsEmpty: 목록이 비어있는지 체크하기
```C++
template<typename List>
class IsEmpty
{
public:
    static constexpr bool value = false;
};

template<>
class IsEmpty<Typelist<>> {
public:
    static constexpr bool value = true;
};
```
### 3.1. typelist 원소 개수 구하기
```C++
class NullType {};

template <class TList> struct Length;

template <> 
struct Length<NullType>
{
    enum { value = 0 };
};

template <class T, class U>
struct Length< Typelist<T, U> >
{
    enum { value = 1 + Length<U>::value };
};
```

> 위: 옛날식 <br>
> 아래: 요즘식

```C++
template <typename TList, bool Empty = IsEmpty<TList>::value>
struct Length;

template <typename TList>
struct Length<TList, false>
{
    static constexpr int value = Length<PopFront<TList>>::value + 1;
};

template <typename TList>
struct Length<TList, true>
{
    static constexpr int value = 0;
};
```
### 3.2. Index로 type 원소 접근하기
* Q> Tail은 typelist, Head는 단위 type인데, Tail이 첫 번째 template argument로 올 때, typelist 그 자체로써 첫 번째 type만을 나타낼 수 있는가?
```C++
template <class TList, unsigned int index>
struct TypeAt;

template <class Head, class Tail>
struct TypeAt<Typelist<Head, Tail>, 0>
{
    typedef Head Result;
};

template <class Head, class Tail, unsigned int i>
struct TypeAt<Typelist<Head, Tail>, i>
{
    typedef typename TypeAt<Tail, i - 1>::Result Result;
};
```

> 위: 옛날식 (※ **Loki**, [[링크] loki-lib](https://sourceforge.net/projects/loki-lib/files/Source%20Code/Modern%20C%2B%2B%20Design/))<br>
> 아래: 요즘식

```C++
// recursive case:
template<typename List, unsigned N>
class NthElementT : public NthElementT<PopFront<List>, N-1> {};

// basis case:
template<typename List, 0> : public FrontT<List> {};

template<typename List, unsigned N>
using NthElement = typename NthElementT<List, N>::Type;
```
> !> 템플릿 책에서는 Loki와 다르게, FrontT<>를 이용하고 있다!
### 3.3. 검색하기
#### 3.3.1. 원소 type의 index 구하기 (옛날식 예제)
```c++
template <class TList, class T> struct IndexOf;

template <class T>
struct IndexOf<NullType, T>
{
    enum { value = -1 };
};

template <class T, class Tail>
struct IndexOf<Typelist<T, Tail>, T>
{
    enum { value = 0 };
};

template <class Head, class Tail, class T>
struct IndexOf<Typelist<Head, Tail>, T>
{
private:
    enum { temp = IndexOf<Tail, T>::value };
public:
    enum { value = temp == -1 ? -1 : 1 + temp };
}
```
#### 3.3.2. 주어진 요구사항과 best match 구하기 (새로운 예제)
```c++
template<typename List>
class LargestTypeT;

// recursive case:
template<typename List>
class LargestTypeT
{
private:
    using First = Front<List>;
    using Rest = typename LargestTypeT<PopFront<List>>::Type;
public:
    using Type = IfThenElse<(sizeof(First) >= sizeof(Rest)), First, Rest>;
};

// basis case:
template<>
class LargestTypeT<Typelist<>>
{
public:
    using Type = char;
};

template<typename List>
using LargestType = typename LargestTypeT<List>::Type;
```
> ※ first/rest idiom
>> 1. `First` type을 구한다.
>> 2. 재귀를 돌며, basis case에서부터 되감아 올라가며, 결과 type을 `Rest`에 담는다.
>> 3. 위의 '2.'에서, 되감아 올라갈 때 각각의 재귀 level(?)에서 `First`와 `Rest`를 비교한 결과로 도출된 type을 `Type`에 담는다.
#### 3.3.3. IsEmpty<>를 활용한 LargestTypeT 구현 예시
```c++
template<typename List, bool Empty = IsEmpty<List>::value>
class LargestTypeT;

// recursive case:
template<typename List>
class LargestTypeT<List, false>
{
private:
    using Contender = Front<List>;
    using Best = typename LargestTypeT<PopFront<List>>::Type;
public:
    using Type = IfThenElse<(sizeof(Contender) >= sizeof(Best)),
        Contender, Best>;
};

// basis case:
template<typename List>
class LargestTypeT<List, true>
{
public:
    using Type = char;
};
```
### 3.4. 원소 type을 list에 추가하기 (append)
```c++
template <class TList, class T> struct Append;

template <> struct Append<NullType, NullType>
{
    typedef NullType Result;
};

template <class T> struct Append<NullType, T>
{
    typedef TYPELIST_1 (T) Result;
};

template <class Head, class Tail>
struct Append<NullType, Typelist<Head, Tail> >
{
    typedef Typelist<Head, Tail> Result;
};

template <class Head, class Tail, class T>
struct Append<Typelist<Head, Tail> T>
{
    typedef Typelist<Head,
            typename Append<Tail, T>::Result>
        Result;
};
```

> 위: 옛날식 <br>
> 아래: 요즘식
1. parameter pack을 이용하기
```c++
template<typename List, typename NewElement>
class PushBackT;

template<typename... Elements, typename NewElement>
class PushBackT<Typelist<Elements...>, NewElement>
{
public:
    using Type = Typelist<Elements..., NewElement>;
};

template<typename List, typename NewElement>
using PushBack = typename PushBackT<List, NewElement>::Type;
```
2. 재귀 이용하기
```c++
template<typename List, typename NewElement, bool = IsEmpty<List>::value>
class pushBackRecT;

// recursive case:
template<typename List, typename NewElement>
class PushBackRecT<List, NewElement, false>
{
    using Head = Front<List>;
    using Tail = PopFront<List>;
    using NewTail = typename PushBackRecT<Tail, NewElement>::Type;

public:
    using Type = typename PushFront<Tail, NewTail>;
};

// basis case:
template<typename List, typename NewElement>
class PushBackRecT<List, NewElement, true>
{
public:
    using Type = PushFront<List, NewElement>;
};

// generic push-back operation:
template<typename List, typename NewElement>
class PushBackT : public PushBackRecT<List, NewElement> { };

template<typename List, typename NewElement>
class PushBack = typename PushBackT<List, NewElement>::Type;
```

### 3.5. typelist를 역순으로 뒤집기 (reversing)
```c++
template<typename List, bool Empty = IsEmpty<List>::value>
class ReverseT;

template<typename List>
using Reverse = typename ReverseT<List>::Type;

// recursive case:
template<typename List>
class ReverseT<List, false>
 : public PushBackT<Reverse<PopFront<List>>, Front<List>> {};

// basis case:
template<typename List>
class ReverseT<List, true>
{
public:
    using Type = List;
}
```

### 3.6. typelist에서 마지막 원소 빼내기
```c++
template<typename List>
class PopBackT {
public:
    using Type = Reverse<PopFront<Reverse<List>>>;
};

template<typename List>
using PopBack = typename PopBackT<List>::Type;
```
> Key point: 앞서서 정의한 Reverse<>의 사용.

### 3.7. 모든 원소를 동일한 방법으로 변형하기 (transforming)
* functional programming 용어로는 `map`이라고 함.
* C++는 표준 라이브러리에 `transform`이 있으므로, 그 방식을 따름.
```c++
template<typename T>
struct AddConstT
{
    using Type = T const;
};

// 개별 원소들에 적용될 함수
template<typename T>
using AddConst = typename AddConstT<T>::Type;

template<typename List, template<typename T> class MetaFun,
        bool Empty = IsEmpty<List>::value>
class TransformT;

// recursive case:
template<typename List, template<typename T> class MetaFun>
class TransformT<List, MetaFun, false>
 : public PushFrontT<typename TransformT<PopFront<List>, MetaFun>::Type,
    typename MetaFun<Front<List>>::Type>
    {};

// basis case:
template<typename List, template<typename T> class MetaFun>
class TransformT<List, MetaFun, true>
{
public:
    using Type = List;
};

template<typename List, template<typename T> class MetaFun>
using Transform = typename TransformT<List, MetaFun>::Type;
```
### 3.8. 모든 원소에 동일하게 수행된 작업의 누적값 구하기 (accumulating)
* functional programming 용어로는 `reduce`라고 함.
* C++는 표준 라이브러리에 `accumulate`가 있으므로, 그 방식을 따름.
#### 3.8.1. accumulate의 정의
```c++
template<typename List,
    template<typename X, typename Y> class F,
    typename I,
    bool = IsEmpty<List>::value>
class AccumulateT;

// recursive case:
template<typename List,
    template<typename X, typename Y> class F,
    typename I>
    class AccumulateT<List, F, I, false>
     : public AccumulateT<PopFront<List, F, typename F<I, Front<List>::Type>
     {};

// basis case:
template<typename List,
    template<typename X, typename Y> class F,
    typename I>
class AccumulateT<List, F, I, true>
{
public:
    using Type = I;
};

template<typename List,
    template<typename X, typename Y> class F,
    typename I>
using Accumulate = typename AccumulateT<List, F, I>::Type;
```
> * basis case와 `F<I, Front<List>::Type>`가 각각 최초로 instantiation 되는 시점은?<br>
> * `F`를 앞서서 정의한 `Transform`으로 치환한다면?
#### 3.8.2. accumulate 사용예시: 최대 크기의 type 구하기
```c++
// metafunction
template<typename T, typename U>
class LargerTypeT
 : public IfThenElseT<sizeof(T) >= sizeof(U), T, U>
 {};

// accumulate의 사용
template<typename Typelist>
class LargestTypeAccT
 : public AccumulateT<PopFront<Typelist>, LargerTypeT,
    Front<Typelist>> {};

template<typename Typelist>
using LargestTypeAcc = typename LargestTypeAccT<Typelist>::Type;
```
> 위의 `LargestTypeT`의 문제점은?<p>
대안:
```c++
template<typename T, typename U>
class LargerTypeT
 : public IfThenElseT<sizeof(T) >= sizeof(U), T, U>
 {};

template<typename Typelist, bool = IsEmpty<Typelist>::value>
class LargestTypeAccT;

template<typename Typelist>
class LargestTypeAccT<Typelist, false>
 : public AccumulateT<PopFront<Typelist>, LargerTypeT,
    Front<Typelist>>
{};

template<typename Typelist>
class LargestTypeAccT<Typelist, true>
{};

template<typename Typelist>
using LargestTypeAcc = typename LargestTypeAccT<Typelist>::Type;
```
> SFINAE-friendly

### 3.9. 삽입정렬
#### 3.9.1. 정의
```c++
template<typename List,
    template<typename T, typename U> class Compare,
    bool = IsEmpty<List>::value>
class InsertionSortT;

template<typename List,
    template<typename T, typename U> class Compare>
using InsertionSort = typename InsertionSortT<List, Compare>::Type;

// recursive case (insert first element into sorted list):
template<typename List,
    template<typename T, typename U> class Compare>
class InsertionSortT<List, Compare, false>
{};

// basis case (an empty list is sorted):
template<typename List,
    template<typename T, typename U> class Compare>
class InsertionSortT<List, Compare, true>
{
public:
    using Type = List;
};

template<typename List, typename Element,
    template<typename T, typename U> class Compare,
    bool = IsEmpty<List>::value>
class InsertSortedT;

// recursive case:
template<typename List, typename Element,
    template<typename T, typename U> class Compare>
class InsertSortedT<List, Element, Compare, false>
{
    // compute the tail of the resulting list:
    using NewTail =
        typename IfThenElse<Compare<Element, Front<List>>::value,
            IdentityT<List>,
            InsertSortedT<PopFront<List>, Element, Compare>
        }::Type;
    // compute the head of the resulting list:
    using NewHead = IfThenElse<Compare<Element, Front<List>>::value,
        Element,
        Front<List>>;

public:
    using Type = PushFront<NewTail, NewHead>;
};

// basis case:
template<typename List, typename Element,
    template<typename T, typename U> class Compare>
class InsertSortedT<List, Element, Compare, true>
 : public PushFrontT<List, Element>
{};

template<typename List, typename Element,
    template<typename T, typename U> class Compare>
using InsertSorted = typename InsertSortedT<List, Element, Compare>::Type;
```
> 위의 `InsertSortedT<>`는 아래와 같이 구현할 수도 있음.<br>
> 그렇다면, 어느쪽에 문제가?
```c++
template<typename List, typename Element,
    template<typename T, typename U> class Compare>
class InsertSortedT<List, Element, Compare, false>
 : public IfThenElseT<Compare<Element, Front<List>>::value,
    PushFront<List, Element>,
    PushFront<InsertSorted<PopFront<List>,
        Element, Compare>,
    Front<List>>>
    {};
```
> A> 효율성 중요! <p>
> ※ 아래: 19장(441~2p)의 `IfThenElse<>` 및 `Identity<>`
```c++
///-- IfThenElse<>

// primary template: yield the second argument by default and rely on
//              a partial specialization to yield the third argument
//              if COND is false
template<bool COND, typename TrueType, typename FalseType>
struct IfThenElseT {
    using Type = TrueType;
};

// partial specialization: false yields third argument
template<typename TrueType, typename FalseType>
struct IfThenElseT<false, TrueType, FalseType> {
    using Type = FalseType;
};

template<bool COND, typename TrueType, typename FalseType>
using IfThenElse = typename IfThenElseT<COND, TrueType, FalseType>::Type;

///-- Identity<>

// yield T when using member Type:
template<typename T>
struct IdentityT {
    using Type = T;
};

template<typename T>
 using Identity = typename IdentityT<T>::Type;
```
#### 3.9.2. 사용
```c++
template<typename T, typename U>
struct SmallerThanT {
    static constexpr bool value = sizeof(T) < sizeof(U);
};

void testInsertionSort()
{
    using Types = Typelist<int, char, short, double>;
    using ST = InsertionSort<Types, SmallerThanT>;
    std::cout << std::is_same<ST,Typelist<char, short, int, double>>::value
        << '\n';
}
```

## 4. Nontype Typelists
### 4.1. 언제 쓰는지?
* 고차원 배열의 경계를 나타낼 때
* 다른 typelist들이 배열된 목록상에서 특정 list의 서수(index)를 나타낼 때
#### 4.1.1. *compile-time value* class template
```c++
template<typename T, T Value>
struct CTValue
{
    static constexpr T value = Value;
};
```
> 위의 `CTValue<>`가 typelist내의 특정 type의 compile-time value를 나타내고 있음.
> 이와 대응되는 표준 라이브러리: **`std::integral_constant`**

#### 4.1.2. `CTValue`의 사용 예시
```c++
using Primes = Typelist<CTValue<int, 2>, CTValue<int, 3>,
                    CTValue<int, 5>, CTValue<int, 7>,
                    CTValue<int, 11>>;
```
> `accumulate<>`와 함께 쓰면?

#### 4.1.3. 소수들의 연속곱(***Π***)을 compile-time에 구하기

##### 4.1.3.1. 단위 곰셈의 정의
```c++
template<typename T, typename U>
struct MultiplyT;

template<typename T, T Value1, T Value2>
struct MultiplyT<CTValue<T, Value1>, CTValue<T, Value2>> {
public:
    using Type = CTValue<T, Value1 * Value2>;
};

template<typename T, typename U>
using Multiply = typename MultiplyT<T, U>::Type;
```
그리고 사용
```c++
Accumulate<Primes, MultiplyT, CTValue<int, 1>>::value;
```
> 모든 value의 type이 동일하다면? (예: `int`)

##### 4.1.3.2. *동질목록*을 구현해서 손가락 노동을 절약하기
* 앞서 구현된 `Primes`에 대하여...

```c++
template<typename T, T... Values>
using CTTypelist = Typelist<CTValue<T, Values>...>;

// Primes v2.0
using Primes = CTTypelist<int, 2, 3, 5, 7, 11>;
```
> 효과: `int`와 `CTValue`타이핑 절약
> 문제점: 오류가 발생하면 `alias`에서 메시지가 지저분하게 나옴.

##### 4.1.3.3. 디버깅이 쉬운 `Valuelist` 만들기
```c++
template<typename T, T... Values>
struct Valuelist {};

template<typename T, T... Values>
struct IsEmpty<Valuelist<T, Values...>> {
    static constexpr bool value = sizeof...(Values) == 0;
};

template<typename T, T Head, T... Tail>
struct FrontT<Valuelist<T, Head, Tail...>> {
    using Type = CTValue<T, Head>;
    static cosntexpr T value = Head;
};

template<typename T, T Head, T... Tail>
struct PopFrontT<Valuelist<T, Head, Tail...>> {
    using Type = Valuelist<T, Tail...>;
};

template<typename T, T... Values, T New>
struct PushFrontT<valuelist<T, Values...>, CTValue<T, New>> {
    using Type = Valuelist<T, New, Values...>;
};

template<typename T, T... Values, T New>
struct PushBackT<Valuelist<T, Values...>, CTValue<T, New>> {
    using Type = Valuelist<T, Values..., New>;
};
```

#### 4.1.4 응용: 정수 목록 정렬하기
```c++
template<typename T, typename U>
struct GreaterThanT;

template<typename T, T Frist, T Second>
struct GreaterThanT<CTValue<T, First>, CTValue<T, Second>> {
    static constexpr bool value = First > Second;
};

void valuelisttest()
{
    using Integers = Valuelist<int, 6, 2, 4, 9, 5, 2, 1, 7>;

    using SortedIntegers = InsertionSort<Integers, GreaterThanT>;

    static_assert(std::is_same_v<SortedIntegers,
                Valuelist<int, 9, 7, 6, 5, 4, 2, 2, 1>>,
                "insertion sort failed");
}
```

### 4.2. 추론된 Nontype Parameters
* Since ***C++17***
#### 4.2.1. 소수 목록 편하게 정의하기
```c++
template<auto Value>
struct CTValue
{
    static constexpr auto value = Value;
};

// Primes v3.0
using Primes = Typelist<CTValue<2>, CTValue<3>, CTValue<5>,
                    CTValue<7>, CTValue<11>>;
```

#### 4.2.2. *이종목록* 구현하기
```c++
template<auto... Values>
class Valuelist {};

int x;
using MyValueList = Valuelist<1, 'a', true, &x>;
```

## 5. Pack Expansion을 이용한 최적화 알고리즘
```c++
template<typename... Elements, template<typename T> class MetaFun>
class TransformT<Typelist<Elements...>, MetaFun, false>
{
public:
    using Type = Typelist<typename MetaFun<Elements>::Type...>;
};
```

```c++
template<typename Types, typename Indices>
class SelectT;

template<typename Types, unsigned... Indices>
class SelectT<Types, Valuelist<unsigned, Indices...>>
{
public:
    using Type = Typelist<NthElement<Types, Indices>...>;
};

template<typename Types, typename Indices>
using Select = typename SelectT<Types, Indices>::Type;
```

## 6. Cons-style Typelist
* *variadic template*이 등장하기 전에 사용하던 typelist 구현방식은 **LISP**언어의 *cons cell*이라는 object의 구성을 따르는 것임.


### 6.1. Cons-style Typelist의 구현과 사용 예시
```c++
class Nil { };

template<typename HeadT, typename TailT = Nil>
class Cons {
public:
    using Head = HeadT;
    using Tail = TailT;
};

using TwoShort = Const<short, Const<unsigned short>>;

using SignedIntegralTypes = Cons<signed char, Cons<short, Cons<int, Cons<long, Cons<long long, Nil>>>>>;
```

## 7. Afternotes
### 7.1. C++에 typelist가 도입된 계기
* Krysztof Czarnecki & Ulrich Eisenecker가 **LISP**의 cons cell에서 영향을 받아서 C++에 가져옴.
> * [Generative Programming - Methods, Tools, and Applications](https://www.amazon.com/Generative-Programming-Methods-Tools-Applications/dp/0201309777)
### 7.2. Typelist가 주목받게 된 계기
* Alexandrescu가 *Modern C++ Design*에서 typelist를 어디다가 쓰면 좋을지 구체적으로 보여줌.
> * [Modern C++ Design](https://www.amazon.com/Modern-Design-Generic-Programming-Patterns/dp/0201704315)
### 7.3. Typelist를 쓰는법을 안내한 다른 곳
* Abraham & Gurtovoy의 metaprogramming 책: 표준 라이브러리의 sequence, iterator, algorithm, (meta)function같은 기능들의 개념을 빌려와서 설명해줌.
> * [C++ Template Metaprogramming - Concepts, Tools, and Techniques from Boost and Beyond](https://www.amazon.com/Template-Metaprogramming-Concepts-Techniques-Beyond/dp/0321227255)
### 7.4. Typelist를 사용하는 라이브러리
* Boost.MPL: typelist를 조작하는데 쓰임.
> * [Boost.MPL](https://www.boost.org/doc/libs/1_63_0/libs/mpl/doc/index.html)