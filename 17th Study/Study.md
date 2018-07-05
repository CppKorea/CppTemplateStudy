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

```