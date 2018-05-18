# Chapter 20. Overloading on Type Properties

함수 오버로딩은 같은 이름에 매개변수만 다르게 사용되고 있다.

``` c++
template<typename T> void f(T*);

template<typename T> void f(Array<T>);

```

이렇게 오버로딩을 하게 되면 귀찮고 반복을 많이 해야한다.

그래서 템플릿을 사용해서 개선하자.

 

## 20.1 Algorithme Specialization

```c++
template<typename T>

void swap(T& x, T& y) 

{

    T tmp(x);

    x = y;

    y = tmp;

}

```

이 함수는 Array<T>를 사용하면 더 효과적으로 개선할수 있다.

```c++
template<typename T>

void swap(Array<T>& x, Array<T>& y)

{

    swap(x.ptr, y.ptr);

    swap(x.len, y.len);

}

```

둘다 swap함수지만 Array<T>를 사용한 부분이 더 정확히 데이터 교환을 했다.(사실 더 나은 선택은 std::move를 사용하는 것이다. std::move를 사용하면 템플릿의 복사를 막을 수 있다.)

또한 2번째가 [partial ordering](https://msdn.microsoft.com/en-us/library/zaycz069.aspx "title")에 더 부합하게 작성되었다. 

 generic 알고리즘([일반화 알고리즘](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.61.9764&rep=rep1&type=pdf "title"))보다 특수하게 최적화 되어있는 것을 algorithm specialization 이라고 부른다. 

algorithm specialization의 모든 함수가 개념적으로 완벽하게 partial ordering을 지원하는건 아니다.

 

std::advance함수를 만들어보면

```c++
template<typename InputIterator, typename Distance>
void advanceIter(InputIterator& x, Distance n)

{

    whiel(n > 0)  // linear time

    {

        ++x;

        --n;

    }

}

template<typename RandomAccessIterator, typename Distance>

void advanceIter(RandomAccessIterator& x, Distance n) {

    x += n;     // constant time

}

```

이 두 함수는 컴파일 에러를 낸다. 왜냐하면 템플릿 파라미터의 갯수가 같기 때문이다. 

그래서 이 챕터에서 함수 템플릿의 overloading에 관한 방법을 설명한다. 

 

## 20.2 Tag Dispatching

한가지 방법으로는 다른 implimentation을 사용해서 유니크한 매개변수를 만드는 방법이다.

```c++
template<typename InputIterator, typename Distance>

void advanceIterImpl(InputIterator& x, Distance n, std::input_iterator_tag)

{

    whiel(n > 0)  // linear time

    {

        ++x;

        --n;

    }

}

template<typename RandomAccessIterator, typename Distance>

void advanceIterImpl(RandomAccessIterator& x, Distance n, std::random_access_iterator_tag) {

    x += n;     // constant time

}

 

template<typename Iterator, typename Distance>

void advanceIter(Iterator& x, Distance n)

{

    advanceIterImpl(x, n, typename std::iterator_traits<Iterator>::iterator_category());

}

```

[std::iterator_traits](https://msdn.microsoft.com/ko-kr/library/zdxb97eh.aspx) 는 iterator의 카테고리를 알려주는 클래스이다.

카테고리 종류에는 

```c++
namespace std {
    struct input_iterator_tag();				// 전진만 가능, 한번에 하나만 읽기 가능
    struct output_iterator_tag();				// 전진만 가능, 한번에 하나만 쓰기 가능
    
    // 전진만 가능, 한번에 여러개 읽고 쓰기 가능
    struct forward_iterator_tag : public input_iterator_tag {};	
    // 전/후진 모두 가능, 한번에 여러개 읽고 쓰기 가능
    struct bidirectional_iterator_tag : public forward_iterator_tag {}; 
    //  반복자를 임의의 위치만큼 전/후진 가능
    struct random_access_iterator_tag : public bidirectonal_iterator_tag {};
}
```

그래서 advanceIter는 Iterator의 태그에 따라서 advanceIterImpl를 호출하게 된다.

어떤 T 타입에 따라서 작동하기에는 적절하지 않지만 tag값을 갖고 올수 있는 어떤것이라면 적절하게 사용 가능하다.

 

## 20.3 Enabling/Disabling Function Templates

EnableIf의 이름 충돌을 피해 [alias template](http://en.cppreference.com/w/cpp/language/type_alias)에 상응하는 방법을 알아보자

[std::enable_if](https://msdn.microsoft.com/ko-kr/library/ee361639.aspx) 같은 alias template은 어떤 상황에서 Enable/Disable 함수 템플릿으로 사용할 수 있다.

```c++
template<typename Iterator>
constexpr bool IsRandomAccessIterator = IsConvertible<typename std::iterator_traits<Iterator>::iterator_category, std::random_access_iterator_tag>;

template<typename Iterator, typename DIstance>
EnableIf<IsRandomAccessIterator<Iterator>> advanceIter(Iterator& x, Distance n) {
	x += n;
}
```

[EnableIf](https://msdn.microsoft.com/ko-kr/library/br244914.aspx)는 advanceIter에 Iterator가 random_access_iterator 카테고리를 가질때만 사용할 수 있다.

```c++
template<typename Iterator, typename Distance>
EnableIf<!IsRandomAccessIterator<Iterator>> advanceIter(Iterator& x, Distance n) {
    while(n > 0) {
		++x;
		--n;
    }
}
```



### 20.3.1 Providing Multiple Specializations

이전의 패턴은 2개이상의 구현이 필요할 경우에 사용한다.

만약에 이전의 패턴에 거꾸로 접근하는 패턴이 필요할 경우에는 어떻게 할까?

- Random access iterator : constant time, forward or backward
- Bidirectional iterator and not random access : linear time, forward or backward
- Input iterator and not bidirectional : General case, linear time, forward 

```c++
namespace std {
    struct input_iterator_tag();				// 전진만 가능, 한번에 하나만 읽기 가능
    struct output_iterator_tag();				// 전진만 가능, 한번에 하나만 쓰기 가능
        
    // 전진만 가능, 한번에 여러개 읽고 쓰기 가능
    struct forward_iterator_tag : public input_iterator_tag {};	
    // 전/후진 모두 가능, 한번에 여러개 읽고 쓰기 가능
    struct bidirectional_iterator_tag : public forward_iterator_tag {}; 
    //  반복자를 임의의 위치만큼 전/후진 가능
    struct random_access_iterator_tag : public bidirectonal_iterator_tag {};
}
```



모든 tag 상태를 처리할 수 있는 코드

```c++
#include <iterator>

// implementation for random access iterators:
template<typename Iterator, typename Distance>
EnableIf<IsRandomAccessIterator<Iterator>> advanceIter(Iterator& x, Distance n) {
	x += n;
}

template<typename Iterator>
constexpr bool IsBidirectionalIterator = IsConvertible<typename std::iterator_traits<Iterator>::iterator_category, std::bidirectional_iterator_tag>;

// implimentation for bidirectional iterators:
EnableIf<IsBidirectionalIterator<Iterator> && ! IsRandomAccessIterator<Iterator>> advancedIter(Iterator& x, Distance n) {
    if(n > 0) {
        for( ; n > 0; ++x, --n) {
            
        }
    }
    else {
         for( ; n > 0; ++x, --n) {
            
        }
    }
}

// implementation for all other iterators:
template<typename Iterator, typename Distance>
EnableIf<!IsBidirectionalIterator<Iterator>> advanceIter(Iterator& x, Distance n) {
    if(n < 0) {
		throw "advanceIter() : invalid iterator category for negative n";
    }
    
    while (n > 0) {
    	++x;
    	--n;
    }
}
```

이전의 두 방식의 단점은 조건이 늘어날때마다 전체의 조건을 모두 검토해야 한다는 점이다.



### 20.3.2 Where Does the EnableIf Go?

Constructor template이나 conversion 함수 템플릿은 지정된 return type이 없어서 이전의 두 방식을 사용할 수가 없다.

또 EnableIf는 return type이 뭔지 확실히 알기 힘들다.

```c++
#include <iterator>
#include "enableif.hpp"
#include "isconvertible.hpp"

template<typename Iterator>
constexpr bool IsInputIterator = IsConvertible<typename std::iterator_traits<Iterator>::Iterator_category, std::input_iterator_tag>;

template<typename T>
Class Container {
public:
	// construct from an input iterator sequence:
    template<typename Iterator, typename = EnableIf<IsInputIterator<Iterator>>>
    Container(Iterator first, Iterator last);
    
    // convert to a container so long as the value types are convertible:
    template<typename U, typename = EnableIf<IsConvertible<T, U>>>
    operator container<U>() const;
}
```

근데 이 상황에서 또 다른 overload함수를 추가하려고 하면 문제가 생긴다.

```c++
// construct from an input iterator sequence:
template<typename Iterator, typename = EnableIf<IsInputIterator<Iterator> && !IsRandomAccessIterator<Iterator>>> 
Container(Iterator first, Iterator last);

// ERROR : redeclaration of constructor template
template<typename Iterator, typename = EnableIf<IsRandomAccessIterator<Iterator>>>
Container(Iterator first, Iterator last);	
```

문제는 두개의 생성자 템플릿이 기본 템플릿 인수를 제외하고는 동일하지만 기본 생성자의 타입이 동일한지는 체크하지 않아서 컴파일 에러 처리 된다.

```c++
// construct from an input iterator sequence:
template<typename Iterator, typename = EnableIf<IsInputIterator<Iterator> && !IsRandomAccessIterator<Iterator>>> 
Container(Iterator first, Iterator last);

// extra dummy parameter to enable both constructors
template<typename Iterator, typename = EnableIf<IsRandomAccessIterator<Iterator>>, typename = int>
Container(Iterator first, Iterator last);
```

그래서 가상의 typename을 하나 더 만들어서 사용하면 우회 가능... 



### 20.3.3 Compile-Time if

[c++17의 constexpr](http://en.cppreference.com/w/cpp/language/if)을 사용하여 advenceIter를 개선해보자

```c++
template<typename Iterator, typename Distance>
void advanceIter(Iterator& x, Distance n) {
    if constexpr(IsRandomAccessIterator<Iterator>) {
		// implementation for random access iterators:
		x += n;
    }
    else if constexpr(IsBidirectionalIterator<Iterator>) {
    	// implementation for bidirectional iterator
        if(n > 0) {
            for( ; n > 0; ++X, --n) {
				// linear time for positive n
            }
        }
        else {
            for( ; n < 0; --x, ++n) {
				// linear time for negative n
            }
        }
    }
    else {
    	// implementation for all other iterators that are at least imput iterators:
        if(n < 0) {
        	throw "advanceIter(): invalid iterator category for negative n";
        }
        while (n > 0) {
        	++x;
        	--n;
        }
    }
}

```

if constexpr을 사용함으로써 코드가 깔끔해졌고 += 같은 연산을 좀 더 안전하게 사용할수 있게 되었다.

그러나 if constexpr은 generic component가 완전히 함수 템플릿 안에서 표현될수 있을때만 사용할 수 있다.

- 다른 interface를 포함하고 있을 때
- 다른 클래스 정의가 필요할때
- 특정 템플릿 매개변수 리스트에는 유효한 인스턴스가 있으면 안된다.

이렇게 코드 작성을 하는건 [SFINAE](https://github.com/jwvg0425/ModernCppStudy/wiki/SFINAE)가 발생한다. 왜냐하면 candidates 리스트에서 삭제되지 않기 때문이다.

```c++
template<typename T>
void f(T p) {
    if constexpr (condition<T>::value) {
        // do someting here...
    }
    else {
        // not a T for which f() makes sence:
        static_assert(condition<T>::value, "can't call f() for such a T");	
    }
}
```

그러니까 EnableIf<…> 대신에 EnableIf f<T>를 호출해서 사용하면 된다.



### 20.3.4 Concepts

위 방법들은 잘 동작하지만 종종 많은 양의 컴파일 리소스를 차지할 수 있고 오류가 발생할 경우 찾기 힘들 수 있다.

```c++
template<typename T>
class Container {
public:
    // construct from an input iterator sequence:
    template <typename Iterator>
    requires IsInputIterator<Iterator>
    Container(Iterator first, Iterator last);
    
    // construct from a random access iterator sequence:
    template<typename Iterator>
    requires IsRandomAccessIterator<Iterator>
    Container(Iterator first, Iterator last);
    
    // convert to a container so long as the value types are convertible:
    template<typename U>
    requires IsConvertible<T, U>
    operator Container<U>() const;
};
```

해당 개념은 언어가 지원하고 있다.

[require](http://en.cppreference.com/w/cpp/language/constraints, "714쪽 E1에서 설명")은 템플릿의 요구사항이다. 요구사항중 한개라도 충족되지 않으면 템플릿이 후보로 간주되지 않는다. 그래서 EnableIf로 표현된것보다 더 직접적으로 표현한 것이다.

EnableIf에 비해 나은점은 template중에서 순서를 제공함으로 tag를 따로 작성 할 필요가 없다. 또 require를 비 템플릿에 연결할 수 있다.

```c++
template<typename T>
class Container {
public:
...

	requires HasLess<T>
	void sort() {
        ...
	}
};
```



## 20.4 Class Specialization

간단한 Dictionary class를 구현해보자

```c++
template<typename Key, typename Value>
class Dictionary
{
private:
	vector<pair<Key const, Value>> data;
public:
	// subscripted access to data:
	Value& operator[](key const& key) 
	{
        // Search for the element with this key:
        for(auto& element : data) {
            if(element.first == key)  {
                return element.second;
            }
        }
        
        // there is no element with this key; add one
        data.push_back(pair<Key const, Value>(key, Value()));
        return data.back().second;
	}
    ...
};
```

키 타입이 <연산자를 지원하면 표준 라이브러리의 맵 턴테이너를 효쥴적으로 구현할수 있다. 마찬가지로 키해싱 작업을 지원하면 표준 라이브러리의 unordered_map을 더 효율적으로 구현할 수 있다.

### 20.4.1 Enabling/Disabling Class Templates

Enable/Disable class template을 사용해서 Dictionary를 구현해보자 

```c++
template<typename Key, typename Value, typename = void>
class Dictionary {
    ... // vector implementation as above
};
```

새로운 템플릿 매개변수는 EnableIf의 앵커 역할을 한다. 

```c++
template<typename Key, typename Value>
class Dictionary<Key, Value, EnableIf<HasLess<Key>>>
{
private:
	map<Key, Value> data;
public:
	value& operator[](Key const& key) {
        return data[key];
	}
	...
};
```

오버로드 된 함수 템플릿과 달리, 이 클래스는 어떤 함수도 비활성화 할 필요가 없다. 왜냐하면 부분 특수화는 기본 템플릿보다 우선하기 때문이다. 그러나 키 해싱 operator를 추가할 때, 부분 특수화의 조건이 상호 배타적인지 확인해야 한다.

```c++
template<typename Key, typename Value, typename = void>
class Dictionary
{
    ... // vector implementation as above
};

template<typename Key, typename Value>
class Dictionary <Key, Value, EnableIf<HasLess<Key> && !HasHash<Key>>> 
{
	... // map implementation as above
};

template<typename Key, typename Value>
class Dictionary<Key, Value, EnableIf<HasHash<Key>>
{
private:
	unordered_map<Key, Value> data;
public:
	value& operator[](Key const& key) {
        return data[key];
	}
	...
};
```



### 20.4.2 Tag Dispatching for Class Templates

Tag Dispatching도 부분 특수화 중에 하나로 사용할수 있다.

예를 들어 이전 섹션에서 사용 된 advanceIter알고리즘과 비슷한 객체 함수 advance<Iterator>를 정의한다.

iterator의 category태그에 제일 잘 맞는 것을 선택하기 위해 BestMatchInSet에 의존하는 bidirectional 과 random access iterator에 대한 구현 뿐만 아니라 일반 input iterator구현도 구현한다.

```c++
// primary template (intentionally undifined):
template<typename Iterator, 
		typename Tag = BestMatchInSet<
            			typename std::iterator_traits<Iterator>::iterator_category, 
							std::input_iterator_tag, 
							std::bidirectional_iterator_tag, 
							std::random_access_iterator_tag>>
class Advance;

// general, linear-time implementation for input iterators:
template<typename Iterator>
class Advance<Iterator, std::input_iterator_tag>
{
public:
	using DifferenceType = typename std::iterator_traits<Iterator>::difference_type;
	
    void operator() (Iterator& x, DifferenceType n) const {
        while(n > 0) {
			++X;
			--n;
        }
    } 
};

// bidirectional, linear-time algorithm for bidirectional iterstor:
template<typename Iterator>
class Advance<Iterator, std::bidirectional_iterator_tag>
{
public:
	using DifferenceType = typename std::iterator_traits<Iterator>::difference_type;
	
	void operator() (Iterator& x, DifferenceType n) const
    {
        if(n > 0) {
            while (n > 0) {
				++x;
				--n;
            }
        } 
        else {
            while (n < 0) {
				--x;
				++n;
            }
        }
    }
};

// bidirectional, constant-time algorithm for random access iterators:
template<typename Iterator>
class Advance<Iterator, std::random_access_iterator_tag>
{
public:
	using DifferenceType = typename std::iterator_traits<Iterator>::difference_type;
	
	void operator() (Iterator& x, DifferenceType n) const 
    {
    	x += n;
    }
};
```

이 방식은 함수 템플릿의 tag dispatching 방식과 유사하다. 그러나 반복자 태그를 판별해 BestMatchInSet를 선택하는데 어려움이 있다. 

In essence, this trait is intended to tell us which of the following overloads would be picked given a value of the iterator's category tag and to report its parameter type.

```c++
void f(std::input_iterator_tag);
void f(std::bidirectional_iterator_tag);
void f(std::random_access_iterator_tag);

// construct a set of match() overloads for the types in Types...
template<typename... Types>
struct MatchOverloads;

//basis case: nothing matched:
template<>
struct MatchOverloads<> {
	static void match(...);
};

// recursive case: introduce a new match() overload:
template<typename T1, typename... Rest>
struct MatchOverloads<T1, Rest...> : public MatchOverloads<Rest...> {
	static T1 match(T1);						// introduce overload for T1
	using MatchOverloads<Rest...>::match;		// collect overloads from bases
};

// find the best match for T in Types...:
template<typename T, typename... Types>
struct BestMatchInSetT {
	using Type = decltype(MatchOverloads<Types...>::match(declval<T>()));
};

template<typename T, typename... Types>
using BestMatchInSet = typename BestMatchInSetT<T, Types...>::Type;
```



## 20.5 Instantiation-Safe templates

EnableIf의 기술의 본질은 템플릿 매개변수가 특정 기준을 충족시키는 경우에만 특정 템플릿 또는 부분 특수화를 하는것이다. 예를 들어 advanceIter() 알고리즘의 가장 효율적인 방식은 iterator의 카테고리가 std::random_access_iterator_tag로 변환 가능한지 확인하는것이다. 

What if we took this notion to the extreme and encoded every operation that the template performs on its template arguments as part of the EnableIf condition? The instantiation of such a template could never fail, because template arguments that do not provide the required operations would cause a deduction failure(via EnableIf) rather than allowing the instantiation to proceed.

이 인스턴스화는 절대 실패하지 않습니다 왜냐하면 템플릿 매개변수가 적절한 operation을 제공하지 않으면 인스턴화를 진행하는 대신 EnableIf를 통해 deduction을 유발하기 때문이다..?? (instantiation을 하기전에 deduction 에러나서..??)

이러한 템플릿은 인스턴스화에 안전한 템플릿이라고 부른다.

```c++
template<typename T>
T const& min(T const& x, T const& y)
{
    if(y < x) {
        return y;
    }
    return x;
}
```

<연산자를 검사하고 결과 유형을 계산하는 특성은 424 페이지의 19.4.4 절에서 논의된 SFINAE-friendly한 PlusResultT특성과 유사하지만 편리를 위해서 LessResultT 특성을 표시한다.



**lessresult.hpp**

```c++
#include <utility> 			// for declval()
#include <type_traits>		// for true_type and false_type

template<typename T1, typename T2>
class HasLess {
	template<typename T> struct Identity;
	template<typename U1, typename U2> static std::true_type test(Identity<decltype(std::declval<U1> < std::declval<U2>())>*);
	template<typename U1, typename U2> static std::false_type test(...);
public:
	static constexpr bool value = decltype(test<T1, T2>(nullptr))::value;
};

template<typename T1, typename T2, bool HasLess>
class LessResultImpl {
public:
	using Type = dycltype(std::declval<T1>() < std::declval<T2>());
};

template<typename T1, typename T2>
class LessResultImpl<T1, T2, calse> {

};

template<typename T1, typename T2>
class LessResultT : public LessResultImpl<T1, T2, HasLess<T1, T2>::value> {

};

template<typename T1, typename T2>
using LessResult = typename LessResultT<T1, T2>::Type;
```

**isconvertible.hpp**

```c++
#include <type_traits>		// for true_type and false_type
#include <utility>			// for declval

template<typename FROM, typename TO>
struct IsConvertibleHelper {
private:
	// test() trying to call the helper aux(TO) for a FROM passed as F:
	static void aux(TO);
	
	template<typename F, typename T, typename = decltype(auz(std::declval<F>()))>
	static std::true_type test(void*);
	
	// test() fallback:
	template<typename, typename>
	static std::false_type test(...);
public:
	using Type = decltype(test<FROM>(nullptr));
}

template<typename FROM, typename TO>
struct IsCOnvertibleT : IsConvertibleHelper<FROM, TO>::Type {} ;

template<typename FROM, typename TO>
using IsConvertible = typename IsConvertibleT<FROM, TO>::Type;

template<typename FROM, typename TO>
constexpr bool isConvertible = IsConvertibleT<FROM, TO>::value;
```

**min2.hpp**

```c++
#include "isconvertible.hpp"
#include "lessresult.hpp"

template<typename T>
EnavleIf<IsConvertible<LessResult<T const&, T Const&>, bool>, T const&>
min(T const& x, T const& y)
{
    if(y < x) {
        return y;
    }
    return x;
}
```

**min.cpp**

```c++
#include "min.hpp"

struct X1 {};
bool operator< (X1 const&, X1 const&) { return true; }

struct X2 {};
bool operator< (X2, X2) { return true; }

struct X3 {};
bool operator< (X3&, X3&) { return true; }

struct X4 {};

struct BoolConvertible {
    operator bool() const { return true; }		// implicit conversion to bool
};

struct X5 {};
BoolConvertible operator< (X5 const&, X5 const&)
{
    return BoolConvertible();
}

struct NotBoolConvertible {}; 					// no conversion to bool

struct X6 {};

NotBoolConvertible operator< (X6 const&, X6 const&)
{
    return NotBoolConvertible();
}

struct BoolLike {
    explicit operator bool() const { return true; }	// explicit conversion to bool
}

struct X7 {};
BoolLike operator< (X7 const&, X7 const&) { return BoolLike(); }

int main()
{
    min(X1(), X1());		// X1 can be passed to min()
    min(X2(), X2());		// X2 can be passed to min()
    min(X3(), X3());		// ERROR: X3 cannot be passed to min()
    min(X4(), X4());		// ERROR: X4 can be passed to min()
    min(X5(), X5());		// X5 can be passed to min()
    min(X6(), X6());		// ERROR: X6 cannot be passed to min()
    min(X7(), X7());		// UNEXPECTED ERROR: X7 cannot be passed to min()
}
```

이 프로그램을 컴파일하면 X3, X4, X6, X7에 대해서  서로 다른 min()을 호출하게 되고 error는 min()의 함수 본문에서 나오지 않고 비 인스턴스화에서 에러가 난다. 오히려 SFINAE가 유일한 옵션을 제거했기 때문에 적절한 min()함수가 없다고 불평한다. Clang은 다음과 같은 결론을 낸다.

```c++
min.cpp:41:3: error: no matching function for call to 'min' 
				min(X3(), X3()); // ERROR: X3 cannot be passed to min~~
./min.hpp:8:1: note: candidate template ignored: substitution failure[with T = X3]: no 	type named 'Type' in 'LessResultT<const X3 &, const X3 &>' min(T const& x, T const& y)
```

따라서 EnableIf는 템플릿 (X1, X2, X5)의 요구사항을 충족하는 템플릿 인수에 대한 인스턴스화만 허용하므로 min() 본문에서 오류가 발생하지 않는다. 

X7의 경우는 안전한 instantiation을 구현한다. 특히 X7이 안전하지 않은 instantiation이 min()에 전달되면 인스턴스화가 성공한다. 그러나 BoolLike과 암시적으로 bool로 변환할수 없으므로 안전한 instantiation min()은 실패합니다. 여기서의 구분은 특히 미묘하다. explicit구문은 절대적으로 확실한 문맥으로 bool을 사용할수 있다. (if, while, for, do, !, && ,||, 삼항연산자(?)은 bool 값으로 변환)

그러나 일반적인 암시적 변환은 구체적으로 안전한 instantiation을 하는 효과가 있다. 이것은 실제 요구사항(템플릿이 올바르게 instatiate하는것)보다 강한 요구사항(EnableIf)을 요구한다. 반대로 bool로의 변환을 잊어버리면 min() 템플릿은 제한되지 않고 instantiation실패를 야기하는 템플릿 매개변수를 허용했을것이다.(X6)

안전한 Instantation min()을 만들려면  타입 T는 문맥에 따라 bool로 변환이 가능해야 한다. Control-flow 문은 SFINAE 컨텍스트내에서 발생할수 없으며 임의의 형식에 대해 오버로드 될 수 있는 논리 연산이 아니기 때문에 이 특성을 정의하는데 도움이 되지 않는다. 다행히 삼항연산자(?) 는 표현식이며 오버로드 될 수 없으므로 문맥상 bool로 변환할수 있는지 여부를 테스트 할 수 있다.

```c++
#include <utility>				// for declval()
#include <type_traits>			// for true_type and false_type	

template<typename T>
class IsContextualBoolT {
private:
    template<typename T> struct Identity;
    template<typename U> static 
        std::true_type test(Identity<decltype(decival<U>() ? 0 : 1)>*);
    template<typename U> static std::false_type test(...);
    
public:
    static constexpr bool value = decltype(test<T>(nullptr))::value;
}

template<typename T>
constexpr bool IsContextualBool = IsContextualBoolT<T>::value;
```

이 특성을 사용하면 EnableIf의 요구를 지키면서 instantiation-safe한 min()함수를 사용할수 있다.

**min3.hpp**

```c++
#include "iscontextualbool.hpp"
#include "lessresult.hpp"

template<typename T>
EnableIf<IsContextualBool<LessResult<T const&, T const&>>, T const&>
min(T const& x, T const& y)
{
    if(y < x) {
        return y;
    }
    return x;
}
```

The technique used here to make min() instantiation-safe can be extended to describe requirements for nontrivial templates by composing various requirement checks into traits that describe some class of types, such as forward iterators, and combining those traits within EnableIf.

이렇게 하면 오버로딩된 템플릿 인스턴스 내에서 컴파일러가 생성하는 오류 소설? 을 없애고 오버로드 동작을 향상시키는 장점이 있다. 다른 한편으로 제공된 오류메세지는 구체적이지 못하다. 이러한 경우에 디버깅 하는 방법은 28.2절에서...



## 20.6 In the standard Liabrary

C++ STL은 이 장에서 배웠던 iterator tag(input, output, forward, bidirectional, random access)를 제공한다. 이 반복자 테그는 표준 iterator 특성(std::iterator_traits)의 일부며 iterator에 대한 요구사항 이므로 tag dispatching 목적으로 안전하게 사용할 수 있다.

C++11의 std::enable_if는 위에서 구현한 EnableIf클래스 템플릿과 동일한 동작이다. 유일한 차이점은 표준이 대문자 유형이 아닌 소문자 유형을 사용한다는 것이다.

algorithm specialization은 C++ 표준의 여러군데에서 사용된다. 예를들면, std::advance(), std::distance(). 대부분의 표준 라이브러리는 태그 디스패칭을 사용하는 경향이 있지만 최근에는 일부가 algorithm specialization을 구현하기 위해 std::enable_if를 사용하고 있다.(VS2017 기준 advance, distance도 아직은 tag dispatching을 사용함)

```c++
template<class _BidIt,
	class _Diff>
	_CONSTEXPR17 void _Advance1(_BidIt& _Where, _Diff _Off, bidirectional_iterator_tag)
	{	// increment iterator by offset, bidirectional iterators
	for (; 0 < _Off; --_Off)
		{
		++_Where;
		}

	// the following warning is triggered if _Diff is unsigned
#pragma warning(suppress: 6294)	// Ill-defined for-loop: initial condition does not satisfy test.
								// Loop body not executed.
	for (; _Off < 0; ++_Off)
		{
		--_Where;
		}
	}

template<class _RanIt,
	class _Diff>
	_CONSTEXPR17 void _Advance1(_RanIt& _Where, _Diff _Off, random_access_iterator_tag)
	{	// increment iterator by offset, random-access iterators
	_Where += _Off;
	}

template<class _InIt,
	class _Diff>
	_CONSTEXPR17 void advance(_InIt& _Where, _Diff _Off)
	{	// increment iterator by offset, arbitrary iterators
		// we remove_const_t before _Iter_cat_t for better diagnostics if the user passes an iterator that is const
	_Advance1(_Where, _Off, _Iter_cat_t<remove_const_t<_InIt>>());
	}

```

예를 들어, iterator가 인접 메모리를 참조하고 값 유형에 사소한 복사 할당 연산자가 있는 경우 std::copy를 std::memcpy()나 std::memmove()를 호출하도록 specialized할 수 있다. 비슷하게 std::fill()은 std::memset()을 사용하도록 최적화 할 수 있고, 여러 알고리즘 유형에 사소한 소멸자가 있는 것으로 알려진 경우 소멸자를 호출하지 않을 수 있다. 이러한 algorithm specialization은 std::advance()나 std::distance()와 동일한 방식으로 C++에서 요구하지는 않지만 사용자가 효율성을 위해 선택하도록 만들었다.

```c++
template<typename InputIterator>
vector(InputIterator first, InputIterator second, allocator_type const& alloc = allocator_type());
```



## 20.7 Afternotes

tag dispatching은 오랫동안 C++로 알려져 있다. 이것은 STL의 원래 구현에서 사용되고 있었고 흔히 alongside traits과 함께 사용되었다. SFINAE와 EnableIf의 사용은 훨씬 더 새롭다. 이 책의 첫번째 판에서는 SFINAE라는 용어를 소개하고 멤버타입의 존재를 발견하는 방법을 알려줬다.

enable if라는 기술과 용어는 JaakkoJ¨arvi, Jeremiah Will-cock, Howard Hinnant, and Andrew Lumsdaine의 책 [OverloadingProperties] 에 처음 나온다. 이 책은 EnableIf템플릿, EnableIf 를 사용하여 함수 overloading을 구현하는 방법에 대해서 설명하고 있다. 그 이후로 EnableIf와 같은 유사한 기술은 C++ STL을 포함하여 고급 템플릿 라이브러리 구현에 널리 보급되었다. 더욱이 이러한 기법의 인기는 C++11에서 확장된 SFINAE동작을 유발했다. Peter Dimov는 함수템플릿의 기본템플릿 인수가 다른 함수 매개변수를 도입하지 않고 생성자 템플릿에 EnableIf를 사용할 수 있다는 사실을 처음으로 지적했다.

이 개념 언어 기능은 C++17 이후에 표준이 될거라고 예상한다. 