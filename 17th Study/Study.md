# Chapter 28: Debugging Templates

## 28.1 Shallow Instantiation

다음 코드를 살펴봅시다.

```C++
template <typename T>
void clear(T& p)
{
    *p = 0; // assumes T is a pointer-like type
}

template <typename T>
void core(T& p)
{
    clear(p);
}

template <typename T>
void middle(typename T::Index p)
{
    core(p);
}

template<typename T>
void shell(T const& env)
{
    typename T::Index i;
    middle<T>(i);
}
```

위 코드는 소프트웨어 개발에 있어서 전형적인 계층을 보여줍니다.

- shell() 함수는 middle() 함수에 의존
- middle() 함수는 core() 함수에 의존
- core() 함수는 clear() 함수에 의존

shell() 함수에서 T::index의 타입이 int라고 가정한다면, 모든 템플릿 함수에서 T는 int로 인스턴스화됩니다. 이 때 clear() 함수에서 p를 역참조할 때 오류가 발생하게 됩니다.

문제는 이 오류를 인스턴스화 타임이 되어서야 알 수 있다는 것입니다.

```C++
class Client
{
public:
    using Index = int;
};

int main()
{
    Client mainClient;
    shell(mainClient);
}
```

그리고 인스턴스화 과정이 여러 함수 계층을 통해 이뤄지기 때문에 오류 메시지도 굉장히 깁니다.

이 문제를 해결할 수 있는 방법으로 얕은 인스턴스화(Shallow Instantiation)가 있습니다.

얕은 인스턴스화는 템플릿 인수가 제약 조건 집합를 충족시키는 가의 여부를 미리 결정하기 위해 두 가지 접근 방식을 식별합니다. 바로 언어 확장 또는 이전 매개 변수 사용을 통해서 말입니다.

이전 예제에서 shell() 함수에 T::Index 타입의 값을 역참조하는 코드를 추가할 수 있습니다.

```C++
template <typename T>
void ignore(T const&)
{

}

template <typename T>
void shell(T const& env)
{
    class ShallowChecks
    {
        void deref(typename T::Index ptr)
        {
            ignore(*ptr);
        }
    };

    typename T::Index i;
    middle(i);
}
```

이제 T::Index가 역참조할 수 없는 타입이라면 ShallowChecks 클래스에서 오류가 발생합니다. 물론 ShallowChecks는 실제로 사용하지 않기 때문에 shall() 함수의 런타임 비용에는 아무런 영향이 없습니다. 대신 컴파일러에서 사용하지 않는 클래스라고 경고 메시지를 출력합니다.

### Concept Checking

C++20에 추가될 Concept을 사용하면 템플릿의 요구 사항과 예상 행동을 더욱 명확하게 표현할 수 있습니다.

```C++
#include <iostream>
#include <string>
#include <locale>
#include <vector>
#include <cassert>
#include <list>

using namespace std::literals;
using namespace std;

// Declaration of the concept "EqualityComparable",
// which is satisfied by any type T such that for values a and b of type T
// the expression a == b compiles and its result is convertible to bool
template <typename T>
concept bool EqualityComparable = requires(T a, T b)
{
    { a == b } -> bool;
};

// Forces you to implement to_string method
template <typename T>
concept bool Stringable = requires(T a)
{  
	{ a.to_string() } -> string;
};

// Has a to_string function which returns a string
template <typename T>
concept bool HasStringFunc = requires(T a)
{
	{ to_string(a) } -> string;  	 
};

struct Person
{
	double height;
	int weight;

	Person(double a, int b)
    {
		height = a;
		weight = b;
	}

	string to_string()
    {	
		return "Weight: " + std::to_string(height) + " Height: " + std::to_string(weight);
	}
};

namespace std
{
    string to_string(list<int> l)
    {
	    // Better ways to do this but just as example
	    string s = "";

	    for (int a : l)
        {
		    s += (" " + to_string(a) + " ");
	    }

	    return s;
    }
}

string to_string(std::vector<int> v)
{
	string s = "";

	for (int a : v)
    {
		s += (" " + to_string(a) + " ");
	}

	return s;
}

void pretty_print(Stringable a)
{
	// We know that if the compiler made it this far we are good to go.
	cout << a.to_string() << endl;
}

void pretty_print(HasStringFunc a)
{
	cout << to_string(a) << endl;
}

// declaration of a constrained function template
// template<typename T>
// void f(T&&) requires EqualityComparable<T>; // long form of the same
 
int main()
{
    assert(__cpp_concepts >= 201500); // check compiled with -fconcepts
    assert(__cplusplus >= 201500);    // check compiled with --std=c++1z
    
    std::list<int> l{ 1, 2, 3 };
    Person jonathan(5.7, 130);
    std::vector<int> v { 34, 23, 34, 56, 78};
    
    // we can make it possible to pretty print a
 	// vector we just need to implement -> string to_string(vector<int> a);
 	// If we want to pretty print
    pretty_print(jonathan); // uses concept overload pretty_print(Stringable a)
    pretty_print(3);        // uses concept overload pretty_print(HasStringFunc a) 
    pretty_print(l);
 
    // pretty_print(v);
    // This will result in an error, first you get the old template garbage you are used to
    // but you can ignore that because at the end highlighted in blue you will see
    // main.cpp:41:6: note:   constraints not satisfied
    // void pretty_print(Stringable a)
    // ^~~~~~~~~~~~
    // main.cpp:18:14: note: within ‘template<class T> concept const bool Stringable<T> [with T = std::vector<std::__cxx11::basic_string<char> >]’
    // concept bool Stringable = requires(T a)
}
```

## 28.2 Static Assertions

assert() 매크로를 사용하면 런타임에 특정 조건을 검사할 수 있습니다. 이 때 검사가 실패하면 프로그램이 멈추게 되고 프로그래머는 원인을 분석해 문제를 해결할 수 있습니다.

C++11에 도입된 static_assert 키워드도 같은 동작을 합니다. 다만 차이가 있다면 런타임이 아닌 컴파일 타임에 검사합니다. 검사가 실패하면 컴파일러는 오류 메시지를 출력합니다. 이 때 무엇이 잘못되었는지를 나타내는 출력 메시지를 포함할 수 있습니다.

```C++
static_assert(sizeof(void*) * CHAR_BIT == 64, "Not a 64-bit platform");
```

정적 단언문을 사용하면 템플릿 인자가 템플릿의 제약 조건을 만족하지 않을 경우 유용한 오류 메시지를 출력할 수 있습니다. 예를 들어 주어진 타입이 역참조할 수 있는지를 확인하는 타입 특성을 만든다고 합시다.

```C++
#include <utility>      // for declval()
#include <type_traits>  // for true_type and false_type

template <typename T>
class HasDereference
{
private:
    template<typename U> struct Identity;
    template<typename U> static std::true_type
        test(Identity<decltype(*std::declval<U>())>*);
    template<typename U> static std::false_type
        test(…);
        
public:
    static constexpr bool value = decltype(test<T>
    (nullptr))::value;
};
```

이제 정적 분석을 사용하면 shell() 함수에서 역참조 할 수 없는 타입으로 인스턴스화 할 경우 좀 더 명확한 오류 메시지를 출력합니다.

```C++
template<typename T>
void shell(T const& env)
{
    static_assert(HasDereference<T>::value, "T is not dereferenceable");
    
    typename T::Index i;
    middle(i);
}
```

정적 분석은 클래스 템플릿에도 적용할 수 있고 모든 타입 특성을 사용할 수 있습니다.

```C++
template <typename T>
class C
{
    static_assert(HasDereference<T>::value, "T is not dereferenceable");
    static_assert(std::is_default_constructible<T>::value, "T is not default constructible");
    ...
};
```

## 28.3 Archetypes

템플릿을 작성할 때 템플릿 인수가 해당 템플릿에 지정된 제약 조건을 충족하는 경우에만 템플릿 정의를 컴파일하도록 구현하는게 좋습니다.

예를 들어 배열에서 특정 값을 찾는 find() 함수를 고려해 봅시다.

```C++
// T must be EqualityComparable, meaning:
// two objects of type T can be compared with == and
// the result converted to bool
template <typename T>
int find(T const* array, int n, T const& value);
```

이 함수를 간단하게 구현해 봅시다.

```C++
template <typename T>
int find(T const* array, int n, T const& value)
{
    int i = 0;
    while (i != n && array[i] != value)
        ++i;
    return i;
}
```

이 템플릿 정의는 두 가지 문제가 있습니다. 두 문제 모두 기술적으로는 템플릿의 요구 사항을 충족하지만 예상한 것과 약간 다르게 동작하는 특정 템플릿 인수가 주어지면 컴파일 오류가 발생합니다. 이 때 원형(Archetype)의 개념을 사용하면 find() 템플릿에 지정된 요구 사항에 충족하는 템플릿 매개 변수의 실제 동작을 검사합니다.

원형은 사용자 정의 클래스를 템플릿 인수로 사용할 경우 해당 템플릿 매개 변수의 제약 조건을 준수하는지 검사합니다. 원형은 가능한 최소한의 방법으로 템플릿의 요구 사항을 충족시키도록 특별히 제작되었습니다. 원형을 템플릿 인수로 사용해 템플릿 정의를 인스턴스화하면 템플릿 정의가 명시적으로 필요하지 않은 작업을 사용하지 않는다는 사실을 알 수 있습니다.

예를 들어 find() 알고리즘의 문서에 설명된 EqualityComparable의 요구 사항을 충족시키기 위한 원형은 다음과 같습니다.

```C++
class EqualityComparableArchetype
{

};

class ConvertibleToBoolArchetype
{
public:
    operator bool() const;
};

ConvertibleToBoolArchetype operator==(
    EqualityComparableArchetype const&,
    EqualityComparableArchetype const&);
```

EqualityComparableArchetype에는 멤버 함수나 데이터가 없습니다. 오직 find()에서 필요한 요구 사항을 충족하기 위해 오버로드된 operator==만 제공합니다. 한편 operator==는 다른 원형인 ConvertibleToBoolArchetype을 반환하는데, 여기에는 사용자 정의 bool 타입 변환 연산만 정의되어 있습니다.

EqualityComparableArchetype은 템플릿 함수 find()에 명시된 요구 사항을 충족합니다. 따라서 EqualityComparableArchetype을 사용해 find() 함수를 인스턴스화했을 때 예상한 대로 동작하는지 검사할 수 있습니다.

```C++
template int find(EqualityComparableArchetype const*, int,
                  EqualityComparableArchetype const&);
```

하지만 컴파일하면 find<EqualityComparableArchetype>의 인스턴스화가 실패했다는 오류가 발생할 것입니다. 여기서 우리는 첫번째 문제를 발견하게 됩니다. 바로 EqualityComparable에는 ==만 필요하지만 find()의 구현 코드에서 T 객체를 !=로 비교한다는 것입니다. 우리가 구현한 코드는 ==와 !=을 쌍으로 구현하는 대부분의 사용자 정의 타입에서 작동했지만 실제로는 그렇지 않았습니다. 원형은 템플릿 라이브러리 개발 초기에 이러한 문제를 발견하는데 많은 도움을 줍니다.

find() 함수에서 != 대신 ==을 사용하도록 변경하면 첫번째 문제를 해결할 수 있습니다. 그리고 find() 함수는 성공적으로 컴파일됩니다.

```C++
template <typename T>
int find(T const* array, int n, T const& value)
{
    int i = 0;
    while (i != n && !(array[i] == value))
        ++i;
    return i;
}
```

원형을 사용하는 find()의 두번째 문제를 발견하려면 약간 더 독창적인 작업이 필요합니다. find()의 새로운 정의는 이제 ==의 결과에 ! 연산자를 적용합니다. 우리가 만든 원형의 경우 사용자 정의 bool 타입 변환 연산과 내장된 논리 부정 연산자 operator!에 의존합니다. ConvertibleToBoolArcheType의 operator!를 오염시켜 부적절하게 사용하는 경우를 방지하도록 신경써서 구현해야 합니다.

```C++
class ConvertibleToBoolArchetype
{
public:
    operator bool() const;
    bool operator!() = delete; // logical negation was not explicitly required
};
```

= delete를 사용해 && 및 || 연산자를 오염시켜 원형을 더 확장할 수 있습니다. 이는 다른 템플릿 정의에서 문제를 찾는 데 도움이됩니다. 일반적으로 템플릿을 구현하는 사람은 템플릿 라이브러리에서 식별된 모든 컨셉에 대한 원형을 개발한 다음 이 원형을 사용해 명시된 요구 사항에 따라 각 템플릿 정의를 검사합니다.

## 28.4 Tracers

