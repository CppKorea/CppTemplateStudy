# Ch. 25 [Tuples](http://en.cppreference.com/w/cpp/utility/tuple)

그동안 이 책에서는 단일 타입(homogeneous) 컨테이너와 배열 형태의 타입들을 사용했습니다. 이런 단일타입 구조들은 C/C++의 배열을 확장한 것이고, 많은 응용 프로그램들에 스며들어 있죠. C++ (그리고 C)는 또한 다수의 타입을 묶을 수 있는 기능: `class` (또는 `struct`)를 가지고 있습니다. 이번 챕터에서는 struct와 class처럼 데이터들을 결합할 수 있는 _tuple_에 대해서 알아볼 것입니다. 예를 들어, `int`, `double`, `std::string`을 가진 tuple은 `int`, `double`, `std::string`을 멤버변수로 가진 struct와 유사합니다. 다만 이름이 아니라 순서(0번째, 1번째 같은)를 통해서 참조된다는 점이 다릅니다. 이런 위치를 사용한 방법과 typelist로부터 쉽게 생성할 수 있는 기능은 tuple을 struct보다 템플릿 메타프로그래밍에 더 적합하게 만들어줍니다.

다른 관점으로는, tuple은 실행 프로그램에서 사용하는 typelist라는 점입니다. 예컨대, `Typelist<int, double, std::String>`은 `int`, `double`, `std::string` 타입들을 순서대로 묶은 것입니다. 이 묶음은 컴파일 시간에 변경될 수 있지요. `Tuple<int, double,std::string>`은 실행 시간에 변경할 수 있는 `int`, `double`, `std::string`의 저장공간(storage)를 표현한 것입니다. 다른 예로, 아래와 같이 프로그램은 tuple 개체를 생성할 수 있습니다.

```c++
template <typename... Types>
class Tuple
{
    // 구현은 잠시 후에...
};

Tuple<int, double, std::string> t(17, 3.14, "Hello, World!");
```  
템플릿 메타프로그래밍에서는 데이터를 저장할 tuple을 생성하기 위해 typelist를 사용하는 것은 흔한 방법입니다. 앞서는 예시를 위해 임의로 `int`, `double`, `std::string`을 선택하긴 했지만, 다른 구성으로 메타 프로그램에서 사용할 타입들의 집합을 생성할 수 있었을 겁니다.

이 챕터에서는 `Tuple` 클래스 템플릿의 구현과 조작법에 대해서 다룹니다. `std::tuple`의 단순화된 버전이죠.

## 1 Basic Tuple Design
> 기초적인 Tuple 설계

### 1.1 Storage

Tuple은 템플릿 인자로 전달받은 각 타입들에 대해 저장공간을 가집니다. 이 저장공간들은 템플릿 함수 `get`을 써서 접근 할 수 있습니다. tuple `t`에 대해서 `get<I>(t)` 형태로 사용하죠. 가령, `get<0>(t)`는 좀 전의 예제라면 17을 참조하게 되고, `get<1>(t)`는 3.14를 참조하게 됩니다.

재귀적으로 tuple의 저장공간을 형성하는 것은,  N > 0 개의 원소를 가진 tuple이 하나의 원소와 나머지( N-1 개의 원소 리스트)로 표현될 수 있다는 생각에 기반한 것입니다. 따라서, 3개의 원소를 가진 `Tuple<int, double, std::string>`은 `int`와 `Tuple<double, std::string>`으로 저장될 수 있습니다. 2개의 원소를 가진 tuple은 `double`과 `Tuple<std::string>`으로 저장되겠죠. 마지막으로 `std::string`과 `Tuple<>`으로 분해됩니다. typelist 알고리즘의 재귀적 분해와 같은 방법으로, tuple의 저장공간도 구분될 수 있는 것이죠.

```c++
// tuples/tuple0.hpp

template <typename... Types>
class Tuple;

// recursive case
template<typename Head, typename... Tail>
class Tuple<Head, Tail...>
{
  private:
    Head head;
    Tuple<Tail...> tail;
  public:
    // constructors
    Tuple() = default;
    Tuple(Head const& head, Tuple<Tail...> const& tail)
       : head(head), tail(tail)
    {}
    // ...    
};

// basis case
template<>
class Tuple<>
{
    // no storage required
};
```
반복적으로 사용되는 경우에 대해선, 각 Tuple 개체는 head와 tail에 대해서 저장공간을 가집니다. 빈 tuple은 연관된 저장공간이 없게 되죠.

>
```
 let Tuple = { elements }

 EmptyTuple == { }
 e          == { e, {} }
 { a, b }   == { a, { b } } == { a, { b, {} } }
```
>

`get` 함수 템플릿은 재귀적으로 요청받은 원소를 찾아내게 됩니다. 
```c++
// tuples/tupleget.hpp

// recursive case:
template <unsigned N>
struct TupleGet
{
    template<typename Head, typename... Tail>
    static auto apply(Tuple<Head, Tail...> const& t){
        return TupleGet<N-1>::apply( t.getTail() );
    }
};

// basis case
template <>
struct TupleGet<0>
{
    template<typename Head, typename... Tail>
    static auto apply(Tuple<Head, Tail...> const& t){
        return t.getHead();
    }
}

template <unsigned N, typename... Type>
auto get(Tuple<Types...> const& t)
{
    return TupleGet<N>::apply(t);
}
```
`get`함수는 `TupleGet` 템플릿을 감싸기만 할 뿐입니다. 함수 템플릿의 부분 특수화(Section 17.3 page 356)가 어려울 때 이는 유용하게 쓰일 수 있는 기술이죠. 재귀적으로 적용되는 경우(N > 0), static 멤버함수인 `apply()`가 현재 tuple의 tail에 적용됩니다. 마지막 단계(N = 0)에 도달하면, tuple의 head가 반환되도록 구현되어있는 것을 확인할 수 있습니다.

### 1.2 Construction

```c++
template<typename Head, typename... Tail>
class Tuple<Head, Tail...>
{
  private:
    Head head;
    Tuple<Tail...> tail;
  public:
    // constructors
    Tuple() = default;
    Tuple(Head const& head, Tuple<Tail...> const& tail)
       : head(head), tail(tail)
    {}
    // ...    
};

```

## 2 Basic Tuple Operation
> 기초적인 Tuple 연산

### 2.1 Comparison
Tuple은 값들의 집합을 구조적인 타입으로 만든 것입니다. 두 tuple을 비교하기 위해선, 그안의 원소들을 비교해야합니다. 따라서,  `operator==`의 정의는 아래처럼 원소마다 비교하는 형태로 작성할 수 있습니다.
```cpp
// tuples/tupleeq.hpp

// basis case
bool operator==(Tuple<> const&, Tuple<> const&)
{
    // empty tuples are always equivalent
    return true;
}

// recursive case
template < typename H1, typename... T1,
           typename H2, typename... T2,
           typename = std::enable_if_t<sizeof...(T1) == sizeof...(T2)>
         >
bool operator==(Tuple<H1, T1...> const& lhs,
                Tuple<H2, T2...> const& rhs)
{
    return lhs.getHead() == rhs.getHead() &&
            lhs.getTail() == rhs.getTail();
}
```
다른 알고리즘들처럼, 원소마다 비교하는 과정은 서로의 첫 원소를 비교하고, 재귀적으로 나머지 원소들을 비교하며 진행됩니다. 마지막에는 서로 비어있는 Tuple을 비교하게 되겠죠. `!=`, `<`, `>`, `<=`, `>=` 연산자들도 이와 유사하게 구현할 수 있습니다.

### 2.2 Output
이번 장 전반에 걸쳐서, 우리는 새로운 tuple 타입을 만들어낼 것입니다. tuple이 프로그램에서 어떻게 사용되는지 볼 수 있다면 유용하겠죠. 아래의 `operator <<` 구현은 어떤 원소 타입을 가지고 있더라도 tuple의 값을 출력하도록 해줍니다.

```cpp
// tuples/tupleio.hpp

#include <iostream>

void printTuple(std::ostream& s, Tuple<> const&, bool isFirst = true)
{
    s << ( isFirst ? '(' : ')' );
}

template <typename Head, typename...Tail>
void printTuple(std::ostream& s, Tuple<Head, Tail...> const& t, bool isFirst = true)
{
    s << ( isFirst ? '(' : ')' );
    s << t.getHead();
    printTuple(s, t.getTail(), false);
}

template <typename... Types>
std::ostream& operator<<(std::ostream& s, Tuple<Types...> const& t)
{
    printTuple(s, t);
    return s;
}
```

이제 tuple을 출력해볼 수 있겠군요. 
```cpp
std::cout << makeTuple(1, 2.5, std::strein("hello")) << '\n';
```

이 코드는 아래와 같이 출력됩니다.
```console
(1, 2.5, hello)
```

## 3 Tuple Algorithm
> Tuple과 알고리즘

Tuple은 3가지 기능을 가진 컨테이너입니다. 멤버의 접근/변경(`get` 함수), 새로운 tuple의 생성(직접 생성 또는 `makeTuple` 함수), 이미 있는 tuple의 분해(`getHead`와 `getTail` 함수). 이 기본적인 기능들은 tuple 알고리즘을 만들기에 충분하죠. 새로운 원소를 더하거나 제거하는 것, 원소들의 순서를 바꾸는 것, 혹은 일부 원소들의 부분집합을 만들어내는 것들 말입니다.

Tuple에 적용되는 알고리즘이 흥미로운 점은 컴파일 시간과 실행 시간을 모두 고려해야 한다는 것입니다. 24장에서 본 Typelist 알고리즘들 처럼, 하나의 tuple 타입에 알고리즘을 적용하면 완전히 다른 tuple 타입을 생성할 수 있습니다. 이는 컴파일 시간에 이루어지죠. 예컨대 `Tuple<int, double, string>`을 역순으로 만드는 것은 `Tuple<string, double, int>` 타입을 만들어냅니다. 하지만, 단일 타입 컨테이너에 사용되는 알고리즘 처럼(예컨대, `std::vector`에 적용되는 `std::reverse`), Tuple 알고리즘의 코드는 실행 시간에 수행됩니다. 컴파일러에 의해 생성되는 코드의 효율성을 고려해야 하는 것이죠.

### 3.1 Tuples as Typelists
`Tuple` 템플릿에서 실행 시간 요소들을 제외하면, 앞서 24장에서 다루었던 `Typelist`와 같은 구조를 가지고 있다는 것을 알 수 있습니다. 실제로, 몇가지 부분 특수화를 사용하면, `Tuple`을 typelist로 변모시킬 수 있죠.

```cpp
// tuples/tupletypelist.hpp

// determine whether the tuple is empty
template <>
struct IsEmpty<Tuple<>>
{
    static cosntexpr bool value = true;
};

// extract front element
tempalte <typename Head, typename... Tail>
class FrontT< Tuple<Head, Tail...> >
{
  public:
    using Type = Head;
};

// remove front element
template <typename Head, typename... Tail>
class PopFrontT<Tuple<Head, Tail...>>
{
  public:
    using Type = Tuple<Tail...>;
};

// add element to the front
template <typename... Types, typename Elem>
class PushFrontT<Tuple<Types...>, Element>
{
  public:
    using Type = Tuple< Elem, Types... >;  
};

// add element to the back
template <typename... Types, typename Elem>
class PushBackT<Tuple<Types...>, Elem>
{
  public:
    using Type = Tuple<Types..., Elem>;
};
```
이제, 24장에서 다뤘던 typelist 모든 알고리즘들이 `Tuple`에 대해서도 동일하게 동작합니다. tuple안의 타입들을 다루는 게 수월해졌죠. 

```cpp
Tuple<int, double, std::string> t1(17, 3.14, "Hello, World!");
using T2 = PopFront<PushBack<decltype(t1), bool>>;
T2 t2(get<1>(t1), get<2>(t1), true);
std::cout << t2;
```
예를 들어, 위의 코드는 아래와 같이 출력합니다.
```console
( 3.14, Hello, World!, 1 )
```
곧 보겠지만, tuple에 적용된 typelist 알고리즘들은 tuple 알고리즘의 결과 타입들을 결정하는데 사용됩니다.

### 3.2 Adding to and Removing from Tuple

>
```
 function popFront  
    == ( { h, t } ) -> { t } || 
       ( {}       ) -> {}

 function popBack
    == ( { h, t } ) -> { h, popBack( t ) } || 
       ( { e }    ) -> {}

 function pushFront 
    == ( { h, t }, v ) -> { v, { h, t }} ||
       ( {},       v ) -> { v }

 function pushBack  
    == ( { h, t }, v ) -> { h, t, v } ||
       ( {},       v ) -> { v }
```
>

보다 유용한 알고리즘을 작성하기 위해선 tuple의 시작이나 끝에 원소를 더하는 기능이 필요합니다. typelist와 마찬가지로, tuple의 앞(front)에 원소를 추가하는 것은 뒤쪽(bacK)에 원소를 추가하는 것보다 쉽습니다. 그러니 `pushFront` 부터 시작하도록 하죠.

```cpp
// tuples/pushfront.hpp

template <typename... Types, typename V>
PushFront<Tuple<Types...>, V>
    pushFront(Tuple<Types...> const& tuple, V const& value)
{
    return PushFront<Tuple<Types...>, V>(value, tuple);
}
```
새로운 원소(`value`)를 이미 존재하는 tuple의 앞에 붙이면서, 결과 타입으로 새로운 `Tuple<V, Tuple<Types...>`타입 tuple 개체를 생성하게 됩니다. 이 과정에서 typelist 알고리즘인 `PushFront`를 사용하면서 컴파일 시간과 실행 시간 측면이 강하게 결합된 것을 볼 수 있습니다. 실행 시간에 적합한 개체를 생성하도록 `PushFront`가 새로운 타입을 컴파일 시간에 생성하는 것이죠.

tuple의 뒤쪽에 원소를 추가하는 것은 조금 더 복잡한데, tuple을 재귀적으로 파고들어야 하기 때문입니다. 그리고 다시 새로운 tuple을 생성하면서 재귀를 풀어나가게 되죠. Section 24.2.4 555 page에서 typelist `PushBack()`의 재귀적 형성을 어떻게 따라했는지 확인해보세요.
```cpp
// tuples/pushback.hpp

// basis case
template <typename V>
Tuple<V> pushBack(Tuple<> const&, V const& value)
{
    return Tuple<V>(value);
}

// recursive case
template <typename Head, typename... Tail, typename V>
Tuple<Head, Tail..., V>
    pushBack(Tuple<Head, Tail...> const& tuple, V const& value)
{
    return Tuple<Head, Tail..., V>(
        tuple.getHead(),
        pushBack(tuple.getTail(), value);
    );
}
```
빈 tuple에 `value`를 더하는 경우는, `value` 하나만 담은 tuple을 생성합니다. 재귀적으로 새로운 tuple을 생성하는 경우는, tuple의 tail에 `value`를 더해서 새로운 tuple을 생성하는 방식으로 진행되죠. 예제에서는 `Tuple<Head, Tail..., V>`를 사용했지만, 컴파일 시간에 이는 `PushBack<Tuple<Head, Tail...>, V>`와 동일합니다. 

`popFront()`도 간단하게 작성할 수 있습니다.
```cpp
// tuples/popfront.hpp
template<typename... Types>
PopFront<Tuple<Types...>>
    popFront(Tuple<Types...> const& tuple)
{
    return tuple.getTail();
}
```
이제 Section 25.3.1 582 page의 프로그램을 작성할 수 있게 되었군요.
```cpp
Tuple<int, double, std::string> t1(17, 3.14, "Hello, World!");
auto t2 = popFront(pushBack(t1, true));
std::cout << std::boolalpha << t2 << '\n';
```
위의 코드는 아래와 같이 출력합니다.
```console
( 3.14, Hello, World!, true )
```

### 3.3 Reversing a Tuple
>
```
 function reverse
    == ( { h, t } ) -> { reverse(t), h } || 
       ( {}       ) -> {}
```
>

tuple의 원소들은 앞서 Section 24.2.4, 557 page에서 소개된 typelist 뒤집기를 적용하여 역순으로 만들 수도 있습니다.

```cpp
// tuples/reverse.hpp

// basis case
Tuple<> reverse(Tuple<> const& t)
{
    return t;
}

// recursive case
template <typename Head, typename... Tail>
Reverse< Tuple<Head, Tail...> > reverse( Tuple<Head, Tail...> const& t)
{
    return pushBack(reverse(t.getTail()), t.getHead());
}
```
원소가 없는 경우는 아무것도 하지 않고 그대로 반환합니다. 재귀적으로 뒤집는 경우는 원본 리스트의 head를 역순 리스트의 tail로 추가하는 것이죠. 이 말인 즉, 아래와 같이 사용한다면...
```cpp
reverse( makeTuple(1, 2.5, std::string("hello")) );
```
결과로는 `Tuple<string, double, int>` 타입에 각 원소가 `string("hello")`, `2.5`, `1` 값을 가진 tuple이 반환된다는 뜻이죠.

Typelist와 같은 방법으로, `popBack()` 함수를 `popFront()`를 써서 지원할 수도 있습니다. 일시적으로 생성된 역순 리스트의 첫번째 원소를 반환하는 것이죠.

```cpp
// tuples/popback.hpp
template <typename... Types>
PopBack< Tuple<Types...> >
    popBack( Tuple<Types...> const& tuple )
{
    return reverse(popFront(reverse(tuple)));
}
```

### 3.4 Index Lists
Tuple을 역순으로 만들기 위해 반복하는 것은 정확한 방법이지만, 실행 시간에는 불필요할 정도로 비효율적입니다. 문제를 확인하기 위해서, 간단히 복사 횟수를 세는 클래스를 사용할 겁니다.
```cpp
// tuples/copycounter.hpp
template <int N>
struct CopyCounter
{
    inline static unsigned numCopies = 0;
    CopyCounter(){}
    CopyCounter(CopyCounter const& ){
        ++numCopies;
    }
};
```
그리고, tuple 개체를 만들어서 뒤집어 볼 겁니다.

```cpp
void copycountertest()
{
    Tuple< CopyCounter<0>, CopyCounter<1>, CopyCounter<2>, CopyCounter<3>, CopyCounter<4> > copies;

    auto reversed = reverse(copies);
    std::cout << "0: " <<  CopyCounter<0>::numCopies << " copies\n"; 
    std::cout << "1: " <<  CopyCounter<1>::numCopies << " copies\n"; 
    std::cout << "2: " <<  CopyCounter<2>::numCopies << " copies\n"; 
    std::cout << "3: " <<  CopyCounter<3>::numCopies << " copies\n"; 
    std::cout << "4: " <<  CopyCounter<4>::numCopies << " copies\n"; 
}
```
이 프로그램은 아래와 같이 출력합니다.
```console
0: 5 copies
1: 8 copies
2: 9 copies
3: 8 copies
4: 5 copies
```
굉장히 빈번한 복사가 발생하는군요! tuple을 역순으로 만드는 이상적인 구현은, 각 원소가 원본 개체에서 뒤집힌 개체로 한번만 복사되도록 하는 것입니다. 참조를 사용해서 만드는 것이 가능하겠지만, 그 방법은 심각하게 복잡할 수 있죠.

불필요한 복사를 없애기 위해서, 이미 길이를 알고 있는(예제에서는 5개의 원소가 있다고 해보죠) tuple을 뒤집는 코연산을 어떻게 작성할 지 생각해봅시다. `makeTuple()`과 `get()` 함수만 사용할 수 있습니다.
```cpp
auto reversed = makeTuple( get<4>(copies), get<3>(copies), 
                           get<2>(copies), get<1>(copies),
                           get<0>(copies) );
```
이 프로그램은 우리가 원하는 결과를 그대로 출력합니다. 즉, 각 원소를 한번만 복사한 것이죠.
```console
0: 1 copies
1: 1 copies
2: 1 copies
3: 1 copies
4: 1 copies
```

_Index list_ ( _index sequence_ 라고도 불립니다. Section 24.4, 570 page를 참고하세요)는 이런 생각을 일반화하여, tuple이 가진 index들의 집합을 parameter pack으로 만든 것입니다. 예제의 경우는 -- 4, 3, 2, 1, 0 -- 이 되겠네요. 이렇게 하면 일련의 `get` 함수 호출을 pack expansion을 통해서 생성해낼 수 있죠. 이 접근법은 메타 프로그램에서는 경우에 따라 복잡할 수 있는 index 계산을 분리해낼 수 있도록 해줍니다. C++ 14 표준에 정의된 [`std::integer_sequence`](https://en.cppreference.com/w/cpp/utility/integer_sequence)이 종종 사용됩니다.

### 3.5 Reversal with Index Lists
index list를 사용해서 tuple을 뒤집기 위해서, index list를 표현할 방법이 필요합니다. index list는 index로 사용될 수 있는 값으로 구성된 typelist이거나, Section 24.4 page 570에서 다룬 서로 다른 타입으로 구성된 자료구조일 수 있습니다. 여기서는 `Valuelist ` 타입을 사용할 겁니다. (Section 24.3 page 566) 앞서 tuple을 뒤집기 위해 사용된 index list는 이렇게 표현할 수 있겠죠.
```cpp
Valuelist<unsigned, 4, 3, 2, 1, 0>
```
index list는 어떻게 생성할 수 있을까요? 하나는 `MakeIndexList` 메타프로그램을 사용해서 0 부터 N-1 까지 하나씩 생성하는 것입니다. 여기서 N은 tuple의 길이에 해당하죠.

>
```
 function makeIndexList
    ==  ( N, Result ) -> [ makeIndexList( N-1, [ N-1, Result ] ) ] || 
        ( 0, Result ) -> [ Result ]


 makeIndexList( 0, [] )
    == []
 makeIndexList( 1, [] )
    == makeIndexList( 0, [ 0 ])
    == [ 0 ]
 makeIndexList( 2, [] ) 
    == makeIndexList( 1, [ 1 ] )
    == makeIndexList( 0, [ 0, 1 ] )
    == [ 0, 1 ]
```
>

```cpp
// tuples/makeindexlist.hpp

// recursive case
template <unsigned N, typename Result = ValueList<unsigned>>
struct MakeIndexListT
 : MakeIndexListT<N-1, PushFront<Result, CTValue<unsigned, N-1>>>
{
};

// basis case
template <typename Result>
struct MakeIndexListT<0, Result>
{
    using Type = Result;
};

template <unsigned N>
using MakeIndexList = typename MakeIndexListT<N>::Type;
```
이렇게 하면 typelist `Reverse`를 사용해서 뒤집는 것이 가능합니다.

```cpp
using MyIndexList = Reverse<MakeIndexList<5>>;
            // == ValueList<unsigned, 4, 3, 2, 1, 0>
```
실제로 역순 재배치를 수행할때는, index list에 있는 index들이 non-type parameter pack으로 사용되어야 합니다. 이 부분은 index 집합인 tuple을 `reverse` 알고리즘으로 뒤집는 것으로 구현할 수 있습니다. 

```cpp
// tuples/indexlistreverse.hpp

template <typename...Elems, unsigned... Indices>
auto reverseImpl(Tuple<Elems...> const& t
                 Valuelist<unsigned, Indices...>)
{
    return makeTuple(get<Indices>(t)...);
}

template <typename... Elems>
auto reverse(Tuple<Elems...> const& t)
{
    return reverseImpl(t,
                       Reverse<MakeIndexList< sizeof...(Elems) >>() );
}
```
C++ 11 환경에서라면, 반환 타은 아래와 같이 선언되어야 합니다.
```cpp
 -> decltype(makeTuple(get<Indices>(t)...))
```
다르게는 이렇게 작성할 수 있습니다.
```cpp
 -> decltype(reverseImpl(t, Reverse<MakeIndexList< sizeof...(Elems) >>() ))
```

`reverseImpl()` 함수 템플릿이 `Valuelist`를 템플릿 인자 `Indices`로 받고 있습니다. 그 다음엔 tuple에 `get()`을 사용해서 지정된 index 집합으로 참조하게 되죠. 이들은 `makeTuple()`의 인자가 되어 

`reverse` 알고리즘은 앞서 언급했던 `TupleGet`처럼, 적합한 index list를 생성해 `reverseImpl`에 전달하는 역할을 하고 있습니다. index들은 템플릿 메타프로그램을 사용해서 변경되기 떄문에 실행 시간을 소모하는 코드는 생성되지 않습니다. 유일하게 실행 시간에 수행되는 코드는 `reverseImpl()` 입니다. 이 함수는 `makeTuple()`로 새로운 원소들을 한번만 복사하여 tuple을 생성하기 때문이죠.

### 3.6 Shuffle and Select
앞서 본 `reverseImpl()` 함수 템플릿은 `reverse()`연산에서만 사용되는 코드를 가지고 있지 않습니다. 대신, 목적에 맞게 index들을 골라내서 새로운 tuple을 만드는데 사용하죠. 

```cpp
// tuples/select.hpp

template <typename... Elems, unsigned... Indices>
auto select(Tuple<Elems,...> const& t,
            Valuelist<unsigned, Indices...>)
{
    return makeTuple<get<Indices>(t)...>();
}
```
`select()`에서 생각할 수 있는 간단한 알고리즘 중 하나는 "splat" 연산입니다. tuple의 한 원소를 골라 복제해 새로운 tuple을 생성하는 것이죠. 

```cpp
Tuple<int, double, std::string> t1( 42, 7.7, "hello" );
auto a = splat<1, 4>(t);
std::cout << a << '\n';
```
위 코드에선 `get<1>(t)`을 사용해 `Tuple<double, double, double, double>`을 생성해냅니다. 그 결과로, 이렇게 출력될 것입니다.

```console
 ( 7.7, 7.7, 7.7, 7.7 )
```
"복제된" index를 생성해내는 `splat()`은 `select()`을 그대로 활용한 것이라 할 수 있습니다.

```cpp
// tuples/splat.hpp

template <unsigned I, unsigned N, typename IndexList = Valuelist<unsigned>>
class ReplicatedIndexListT;

template <unsigned I, unsigned N, typename... Indices>
class ReplicatedIndexListT<I, N, Valuelist<unsigned, Indices...>>
    : public ReplicatedIndexListT<I, N-1,
                                  Valuelist<unsigned, Indices..., I>>
{
};

template <unsigned I, unsigned... Indices>
class ReplicatedIndexListT<I, 0, Valuelist<unsigned, Indices...>>
{
  public:
    using Type = Valuelist<unsigned, Indices...>;
};

template <unsigned I, unsigned N>
using RelicatedIndexList = typename ReplicatedIndexListT<I, N>::Type;

template <unsigned I, unsigned N, typename... Elems>
auto splat(Tuple<Elems...> const& t)
{
    return select(t, ReplicatedIndexList<I,N>());
}
```
`select()`를 통해 index list를 생성해 사용하는 템플릿 메타프로그램이라면 복잡한 tuple 알고리즘도 구현할 수 있습니다. Section 24.2.7 page 563에서 구현했던 삽입 정렬을 만들어볼 수도 있을 겁니다. 원소들의 크기를 받아서 비교를 수행하는 템플릿 메타함수를 사용한 `sort()` 함수같은걸 말입니다. 

```cpp
// tuples/tuplesorttest.hpp
#include <complex>

template <typename T, typename U>
class SmallerThanT
{
  public:
    static constexpr bool value = sizeof(T) < sizeof(U);
};

void testTupleSort()
{
    auto t1 = makeTuple(17LL, std::complex<double>(42.77), 'c', 42, 7.7);
    std::cout << t1 << '\n';

    auto t2 = sort<SmallerThanT>(t1);
    std::cout << "sorted by size: " << t2 << '\n';
}
```
결과는 다음과 같습니다.
```console
(17, (42,77), c, 42, 7.7)
sorted by size: (c, 42, 7.7, 17, (42,77))
```
실제 `sort()`구현은 `select()`와 `InsertionSort`를 사용한 것입니다.

```cpp
// tuples/tuplesort.hpp

// meta function wrapper that compares the elements in a tuple
template <typename List, template<typename T, typename U> class F>
class MetafunOfNthElementT
{
  public:
    template <typename T, typename U> class Apply;

    template <unsigned N, unsigned M>
    class Apply<CTValue<unsigned, M>, CTValue<unsigned M>>
        : public F<NthElement<List, M>, NthElement<List, N>> {};
};

// sort a tuple based on comparing the element types:
template <template<typename T, typename U> class Compare,
          typename... Elems>
auto sort(Tuple<Elems...> const& t)
{
    return select(t,
        InsertionSort< MakeIndexList<sizeof...(Elems)> ,
                       MetafunOfNthElementT<
                            Tuple<Elems...>,
                            Compare>::template Apply>());
}
```
`InsertionSort`의 사용을 주의깊게 보시기 바랍니다: 실제 index list는 `MakeIndexList`를 통해 typelist로 생성됩니다.따라서, `InsetionSort`의 결과는 tuple에서 사용되는 index의 목록이 됩니다. 이는 바로 `select()`로 전달됩니다. 하지만 `InsertionSort`는 index에 대해서 수행되기 때문에, 그대로 두면 index 두개를 비교하게 될 것입니다. 메타프로그램이 아닌 경우의 예를 들자면 아래와 같은 상황인 것이죠.

```cpp
// tuples/indexsort.hpp
#include <vector>
#include <algorithm>
#include <string>

int main()
{
    std::vector<std::string> strings = { "banana", "apple", "cherry" };
    std::vector<unsigned> indices = { 0, 1, 2 };
    std::sort(indices.begin(), indices.end(), 
              [&strings](unsigned i, unsigned j){
                  return strings[i] < strings[j];
              });
}
```  
여기서, `indices`는 `strings` 벡터의 index들을 의미합니다. `sort()` 연산은 index들을 정렬하죠. 대신, 람다 함수의 내부에서는 `strings` 벡터의 값을 비교합니다. 결과적으로 정렬순서는 `strings`에 따라서 수행되는 것이죠. `sort()`를 마치면, 어떤 순서로 정렬해야 하는지를 알려주는 index들을 얻을 수 있을 것입니다.

tuple `sort()`에서 사용한 `InsertionSort`도 이같은 접근법을 사용한 것입니다. `MetafunOfNthElementT` 템플릿은 `Apply`라는 템플릿 메타 함수를 제공하고, 특수화된 `CTValue`를 받아 해당 위치에 있는 원소들을 `NthElement`로 획득합니다. 비유하자면, `Apply` 멤버 함수가 자신을 내포한 템플릿 `MetafunOfNthElement`에 제공된 typelist를 참조(capture)한 것이죠. `Apply`는 그 후에 상속받은 메타함수 `F`를 사용합니다.

정렬을 위한 모든 계산은 컴파일 시간에 수행되고, 정렬의 결과로 생성된 tuple은 불필요한 복사를 수행하지 않습니다.

## 4 Expanding Tuples
> Tuple 확장

Tuple은 타입의 개수 혹은 값의 개수와 상관없이 서로 연관된 값들을 묶어서 저장하는데 유용합니다. 때에 따라서는, 예를 들자면, 각각의 원소를 함수의 인자로 전달하기 위해서 이를 풀어내야만 할수도 있을 겁니다. 간단한 예로, tuple을 받아 `print()` 연산(Section 12.4, 200 page)에 전달하고 싶을 수고 있겠죠.

```cpp
Tuple<std::string, char const*, int, char> t("Pi", "is roughly", 3, '\n');

print(t...); // Error: cannot expand tuple; it isn't a parameter pack
```

예제에서 보듯이, tuple을 풀어내기 위한 시도는 실패하게 됩니다. tuple은 parameter pack이 아니기 때문이죠. index list를 사용해서 이를 해결할 수 있습니다.
아래에서 템플릿 함수 `apply()`는 함수와 tuple을 받아서, 각 원소에 대해서 함수를 호출합니다.

```cpp
// tuples/apply.hpp
template <typename F, typename... Elems, unsigned... Indices>
auto applyImpl(F f, Tuple<Elems...> const& t, Valuelist<unsigned, Indices...>)
    -> decltype(f(get<Indeices>(t)...))
{
    return f(get<Indices>(t)...);
}

template <typename F, typename... Elems, unsigned N = sizeof(Elems...)>
auto apply(F f, Tuple<Elems...> const& t)
    -> decltype(applyImpl(f, t, MakeIndexList<N>()))
{
    return applyImpl(f, t, MakeIndexLists<N>());
}
```

`applyImpl()` 함수 템플릿은 주어진 index list를 사용해서 tuple을 함수 개체 인자 f에 전달할 argument list로 만들어냅니다. 사용자가 실제로 쓰게 되는 `apply()`는 최초의 index list를 만들어내는 역할만을 위한 것이죠. 이런 방법으로, tuple을 `print()`에 인자로 전달할 수 있습니다.

```cpp
Tuple<std::string, char const*, int, char> t("Pi", "is roughly", 3, '\n');

apply(print, t); // OK: prints Pi is roughly 3
``` 
C++17에서는 tuple과 같은 타입에 대해서 동작하는 유사한 함수를 제공합니다.

## 5 Optimizing Tuple
> Tuple 최적화

### 5.1 Tuples and the EBCO
> EBCO: Empty Base Class Optimization
> 추후 보충 예정

### 5.2 Constant-time `get()`
> 추후 보충 예정

## 6 Tuple Subscript
> Tuple과 아래 첨자

원리적으로는, tuple의 원소에 접근할 수 있도록 `operator[]`를 정의할 수 있습니다. `std::vector`와 유사하게 말이죠. 하지만 `std::vector`와는 달리 tuple의 각 원소는 다른 타입을 가지고 있습니다. 따라서 tuple의 `operator[]`는 index에 따라 결과타입이 달라지는 템플릿 함수여야만 합니다. 달리 생각하면, 다른 타입을 선택하기 위해 각각의 index를 사용한다는 말이죠. 그러니 index의 타입은 원소의 타입을 결정하는데 사용될 수 있습니다.

앞서 24.3절(566 page)에서 소개한 `CTValue` 클래스 템플릿을 사용하면 index를 타입으로 바꿀 수 있습니다. 아래첨자 연산자를 사용하기 위해 `Tuple`에 아래와 같은 멤버 함수를 추가합니다.
```cpp
template <typename T, T index>
auto& operator[](CTValue<T, Index>){
    return get<Index>(*this);
}
```
여기선 `get<>()`호출과 일치하도록 index를 `CTValue`로 감싸서 전달할 것입니다. 

```c++
auto t = makeTuple(0, '1', 2.2f, std::String{"hello"});
auto a = t[CTValue<unsigned, 2>{}];
auto b = t[CTValue<unsigned, 3>{}];
```

> [User defined literals에 대한 소개로 대체](http://en.cppreference.com/w/cpp/language/user_literal)

```cpp
auto t = makeTuple(0, '1', 2.2f, std::String{"hello"});
auto c = t[2_c];
auto d = t[3_c];
```


## 25.7 Afternotes
Boost Hana
