# Chapter 5: Tricky Basics

## 5.1 Keyword ```typename```

```C++
template <typename T>
class MyClass
{
public:
    void foo()
    {
        typename T::SubType* ptr;
    }
};
```

```typename```은 템플릿 내부의 식별자가 타입임을 나타내기 위해 도입된 키워드입니다.

따라서 위 코드에서 ```ptr```은 ```T::SubType``` 타입을 가리키는 포인터입니다.

만약 ```typename```이 없다면, ```SubType```을 타입이 없는 멤버라고 가정합니다.

따라서 ```T::SubType* ptr```은 ```T::SubType```과 ```ptr```의 곱셈으로 해석합니다.

보통 템플릿 매개 변수에 의존하는 이름이 타입일 때 ```typename```을 사용합니다.

가장 흔하게 다루는 예제로 표준 컨테이너의 반복자를 선언하는 코드가 있습니다.

```C++
template <typename T>
void printcoll(T const& coll)
{
    typename T::const_iterator pos;
    typename T::const_iterator end(coll.end());

    for (pos = coll.begin(); pos != end; ++pos)
    {
        std::cout << *pos << ' ';
    }
    std::cout << '\n';
}
```

```const_iterator```는 표준 컨테이너 클래스에 선언되어 있습니다.

```C++
class stlContainer
{
public:
    using iterator = ...;
    using const_iterator = ...;
};
```

템플릿 타입 ```T```의 ```const_iterator``` 타입에 접근하려면 ```typename```을 붙여 한정해야 합니다.

```C++
typename T::const_iterator pos;
```

## 5.2 Zero Initialization

기본 타입(```int```, ```double```, 포인터 타입)은 디폴트 생성자가 없습니다.

이 때문에, 초기화하지 않은 지역 변수는 정의되지 않은 값을 갖습니다.

```C++
void foo()
{
    int x;
    int* ptr;
}
```

템플릿을 작성할 때 템플릿 타입의 변수가 디폴트 값을 갖게 만들고 싶다고 합시다.

하지만 기본 타입은 디폴트 생성자가 없어서 정의되지 않은 값을 갖습니다.

```C++
template <typename T>
void foo()
{
    T x;
}
```

이 때문에 내장 타입을 0(```bool``` 타입은 ```false```, 포인터 타입은 ```nullptr```)으로 초기화하는 디폴트 생성자를 명시적으로 호출할 수 있습니다.

```C++
template <typename T>
void foo()
{
    T x{};
}
```

이러한 초기화 방식을 <b>값 초기화(Value Initialization)</b>라고 합니다.

값 초기화를 사용하면 개체의 생성자를 호출하거나 0으로 초기화합니다.

클래스 템플릿의 멤버를 디폴트 값으로 초기화하고 싶다면, 중괄호 초기화를 사용해 멤버를 초기화하는 클래스 생성자를 정의하면 됩니다.

```C++
template <typename T>
class MyClass
{
public:
    MyClass() : x{} { }

private:
    T x;
};
```

다음과 같이 사용할 수도 있습니다.

```C++
template <typename T>
class MyClass
{
private:
    T x{};
}
```

하지만 디폴트 인수는 이 문법을 사용할 수 없습니다.

```C++
template <typename T>
void foo(T p{})
{
    ...
}
```

대신, 다음과 같이 써야 합니다.

```C++
template <typename T>
void foo(T p = T{})
{
    ...
}
```

## 5.3 using ```this->```

템플릿 매개 변수에 의존하는 클래스 템플릿이 베이스 클래스를 갖고 있다고 가정합시다.

```x```라는 이름을 사용했을 때, 멤버 ```x```를 상속받았더라도 항상 ```this->x```와 같지는 않습니다.

```C++
template <typename T>
class Base
{
public:
    void bar();
};

template <typename T>
class Derived : Base<T>
{
public:
    void foo()
    {
        bar();
    }
};
```

이 예제에서 ```foo()``` 안에 있는 ```bar```라는 심볼을 해석할 때 ```Base```에 정의되어 있는 ```bar()```는 전혀 고려하지 않습니다.

따라서 오류가 발생하거나 다른 ```bar()```(예를 들어, 전역 함수인 ```bar()```)를 호출합니다.

이 문제를 해결하기 위해서는 심볼을 ```this->```나 ```Base<T>::```로 한정해야 합니다.

## 5.4 Templates for Raw Arrays and String Literals

배열이나 문자열 리터럴을 템플릿으로 전달할 때는 주의해야 합니다.

- 템플릿 매개 변수가 레퍼런스로 선언되어 있다면, 인수는 붕괴되지 않습니다. 즉, ```"hello"```를 인수로 전달하면 타입은 ```char const[6]```이 됩니다. 하지만 타입이 다르기 때문에 길이가 다른 배열이나 문자열 리터럴을 전달하면 문제가 발생할 수 있습니다.

- 값을 통해 인수를 전달할 때는 타입이 붕괴됩니다. 따라서 문자열 리터럴은 ```char const*``` 타입으로 변환됩니다.

우리는 배열이나 문자열 리터럴을 처리하는 템플릿을 만들 수 있습니다.

```C++
template <typename T, int N, int M>
bool less(T(&a)[N], T(&b)[M])
{
    for (int i = 0; i < N && i < M; ++i)
    {
        if (a[i] < b[i]) return true;
        if (b[i] < a[i]) return false;
    }
    return N < M;
}
```

다음처럼 배열을 템플릿으로 전달할 때,

```C++
int x[] = {1, 2, 3};
int y[] = {1, 2, 3, 4, 5};
std::cout << less(x, y) << '\n';
```

```less<>()```는 ```T```는 ```int```, ```N```은 ```3```, ```M```은 ```5```로 인스턴스화됩니다.

문자열 리터럴을 템플릿으로 전달할 수도 있습니다.

```C++
std::cout << less("ab", "abc") << '\n';
```

이 경우, ```less<>()```는 ```T```는 ```char const```, ```N```은 ```3```, ```M```은 ```4```로 인스턴스화됩니다.

문자열 리터럴(그리고 ```char``` 배열)만 처리하는 템플릿 만들고 싶다면, 다음처럼 하면 됩니다.

```C++
template <int N, int M>
bool less(char const(&a)[N], char const(&b)[M])
{
    for (int i = 0; i < N && i < M; ++i)
    {
        if (a[i] < b[i]) return true;
        if (b[i] < a[i]) return false;
    }
    return N < M;
}
```

범위를 알 수 없는 배열을 오버로드하거나 부분 특수화할 수도 있습니다.

```C++
#include <iostream>

template <typename T>
struct MyClass;

template <typename T, std::size_t SZ>
struct MyClass<T[SZ]>
{
    static void print() { std::cout << "print() for T[" << SZ << "]\n"; }
}

template <typename T, std::size_t SZ>
struct MyClass<T(&)[SZ]>
{
    static void print() { std::cout << "print() for T(&)[" << SZ << "]\n"; }
}

template <typename T>
struct MyClass<T[]>
{
    static void print() { std::cout << "print() for T[]\n"; }
}

template <typename T>
struct MyClass<T(&)[]>
{
    static void print() { std::cout << "print() for T(&)[]\n"; }
}

template <typename T>
struct MyClass<T*>
{
    static void print() { std::cout << "print() for T*\n"; }
}
```

이제 다양한 타입을 사용해 호출하는 예제를 살펴봅시다.

```C++
#include "arrays.hpp"

template <typename T1, typename T2, typename T3>
void foo(int a1[7], int a2[], int (&a3)[42], int (&x0)[], T1 x1, T2& x2, T3&& x3)
{
    MyClass<decltype(a1)>::print();
    MyClass<decltype(a2)>::print();
    MyClass<decltype(a3)>::print();
    MyClass<decltype(x0)>::print();
    MyClass<decltype(x1)>::print();
    MyClass<decltype(x2)>::print();
    MyClass<decltype(x3)>::print();
}

int main()
{
    int a[42];
    MyClass<decltype(a)>::print();

    extern int x[];
    MyClass<decltype(x)>::print();

    foo(a, a, a, x, x, x, x);
}

int x[] = {0, 8, 16};
```

참고로 언어 규칙에 의해 (길이 존재 여부와 관계 없이) 배열로 선언된 <b>호출 매개 변수(Call Parameter)</b>의 타입은 포인터 타입입니다.

## 5.5 Member Templates

```Stack<>``` 클래스 템플릿으로 멤버 템플릿의 장점을 살펴보도록 합시다.

일반적으로 두 스택의 타입이 같을 때만 스택을 할당할 수 있습니다.

두 스택의 타입이 서로 다르다면, 암시적 타입 변환을 통해 변환이 가능하더라도 할당할 수 없습니다.

```C++
Stack<int> intStack1, intStack2;
Stack<float> floatStack;
...
intStack1 = intStack2;
floatStack = intStack1;
```

디폴트 할당 연산자는 할당 연산자의 양변이 모두 같은 타입이어야 한다고 요구합니다.

하지만 할당 연산자를 템플릿으로 정의하면 적절한 타입 변환을 통해 스택에 할당할 수 있도록 만들 수 있습니다.

```C++
template <typename T>
class Stack
{
public:
    void push(T const&);
    void pop();
    T const& top() const;
    bool empty() const
    {
        return elems.empty();
    }

    template <typename T2>
    Stack& operator=(Stack<T2> const&);

private:
    std::deque<T> elems;
};
```

새 할당 연산자는 다음과 같이 구현합니다.

```C++
template <typename T>
template <typename T2>
Stack<T>& Stack<T>::operator=(Stack<T2> const& op2)
{
    Stack<T2> tmp(op2);

    elems.clear();
    while (!tmp.empty())
    {
        elems.push_front(tmp.top());
        tmp.pop();
    }
    return *this;
}
```

```operator=```의 두 스택이 서로 다른 타입을 갖고 있으므로, ```public``` 인터페이스만 사용할 수 있습니다.

```op2```의 모든 멤버에 접근하고 싶다면, ```friend```로 만들어주면 됩니다.

```C++
template <typename T>
class Stack
{
public:
    void push(T const&);
    void pop();
    T const& top() const;
    bool empty() const
    {
        return elems.empty();
    }

    template <typename T2>
    Stack& operator=(Stack<T2> const&);

    template <typename> friend class Stack;

private:
    std::deque<T> elems;
};
```

템플릿 매개 변수의 이름을 사용하지 않는 경우 생략할 수 있습니다.

이제 템플릿 할당 연산자를 다음과 같이 구현할 수 있습니다.

```C++
template <typename T>
template <typename T2>
Stack<T>& Stack<T>::operator=(Stack<T2> const& op2)
{
    elems.clear();
    elems.insert(elems.begin(), op2.elems.begin(), op2.elems.end());
    return *this;
}
```

이제 ```int``` 타입의 스택을 ```float``` 타입의 스택에 할당할 수 있습니다.

```C++
Stack<int> intStack;
Stack<float> floatStack;
...
floatStack = intStack;
```

내부 컨테이너 타입을 매개 변수화하기 위해 코드를 변경해 봅시다.

```C++
template <typename T, typename Cont = std::deque<T>>
class Stack
{
public:
    void push(T const&);
    void pop();
    T const& top() const;
    bool empty() const
    {
        return elems.empty();
    }

    template <typename T2, typename Cont2>
    Stack& operator=(Stack<T2, Cont2> const&);

    template <typename, typename> friend class Stack;

private:
    Cont elems;
};
```

템플릿 할당 연산자의 코드도 변경합니다.

```C++
template <typename T, typename Cont>
template <typename T2, typename Cont2>
Stack<T, Cont>& Stack<T, Cont>::operator=(Stack<T2, Cont2> const& op2)
{
    elems.clear();
    elems.insert(elems.begin(), op2.elems.begin(), op2.elems.end());
    return *this;
}
```

클래스 템플릿은 호출하는 멤버 함수만 인스턴스화합니다.

따라서 다른 타입을 갖는 스택을 할당하지 않는다면, 벡터를 내부 컨테이너를 사용해도 오류가 발생하지 않습니다.

```C++
Stack<int, std::vector<int>> vStack;
...
vStack.push(42);
vStack.push(7);
std::cout << vStack.top() << '\n';
```

할당 연산자 템플릿이 필요없기 때문에, ```push_front()``` 멤버 함수가 없다는 오류 메시지를 출력하지 않습니다.

#### 멤버 함수 템플릿의 특수화

멤버 함수 템플릿은 부분 및 전체 특수화도 할 수 있습니다.

예를 들어, 다음과 같은 클래스가 있다고 합시다.

```C++
class BoolString
{
public:
    BoolString(std::String const& s) : value(s) { }
    
    template <typename T = std::String>
    T get() const
    {
        return value;
    }

private:
    std::string value;
}
```

멤버 함수 템플릿인 ```get()```에 전체 특수화를 적용해 봅시다.

```C++
template<>
inline bool BoolString::get<bool>() const
{
    return value == "true" || value == "1" || value == "on";
}
```

전체 특수화는 선언만 할 수 없으며, 헤더 파일에 정의해야 합니다.

```C++
std::cout << std::boolalpha;
BoolString s1("hello");
std::cout << s1.get() << '\n';
std::cout << s1.get<bool>() << '\n';
BoolString s2("on");
std::cout << s2.get<bool>() << '\n';
```

#### 특별한 멤버 함수 템플릿

템플릿 멤버 함수는 특별한 멤버 함수가 개체를 복사 또는 이동할 수 있도록 허용한 곳에서 사용할 수 있습니다.

복사 할당 연산자 뿐만 아니라 생성자도 구현할 수 있습니다.

하지만 템플릿 생성자나 템플릿 할당 연산자가 미리 정의된 생성자나 할당 연산자를 대체하진 않습니다.

멤버 템플릿은 개체를 복사 또는 이동하는 특별한 멤버 함수로 고려하지 않습니다.

따라서 같은 타입을 갖는 스택을 할당한다면 디폴트 할당 연산자를 호출합니다.

- 장점 : 템플릿 생성자나 할당 연산자가 미리 정의된 복사/이동 생성자나 할당 연산자보다 더 잘맞는 경우가 있습니다.

- 단점 : 복사/이동 생성자를 "템플릿화"하기 쉽지 않습니다.

### 5.5.1 The ```.template``` Construct

멤버 템플릿을 호출할 때 템플릿 인수를 명시적으로 한정해야 하는 경우가 있습니다.

이 때, ```<```이 템플릿 인수 목록의 시작 부분이라고 보장하는 ```template``` 키워드를 사용해야 합니다.

```C++
template <unsigned long N>
void printBitset(std::bitset<N> const& bs)
{
    std::cout << bs.template to_string<char, std::char_traits<char>, std::allocator<char>>();
}
```

```bs```는 멤버 함수 템플릿 ```to_string()```을 호출합니다.

만약 ```.template```을 사용하지 않는다면, 컴파일러는 실제로 ```<```이 작다가 아니라 템플릿 인수 목록의 시작 부분이라고 판단하지 못합니다.

이 문제는 매개 변수가 템플릿 매개 변수에 의존하는 경우에만 발생합니다.

예제에서는 매개 변수 ```bs```가 템플릿 매개 변수 ```N```에 의존합니다.

이러한 문제가 발생했을 경우에만 ```.template``` 표기법을 사용하기 바랍니다.

### 5.5.2 Generic Lambdas and Member Templates

C++14에 도입된 제네릭 람다는 멤버 템플릿을 편하게 사용하는 방법입니다.

임의의 타입을 갖는 두 인수의 합을 구하는 간단한 람다가 있다고 합시다.

```C++
[](auto x, auto y)
{
    return x + y;
}
```

이 람다식을 다음 클래스의 디폴트 생성된 개체에 편하게 사용할 수 있습니다.

```C++
class SomeCompilerSpecificName
{
public:
    SomeCompilerSpecificName();

    template <typename T1, typename T2>
    auto operator()(T1 x, T2 y) const
    {
        return x + y;
    }
};
```

## 5.6 Variable Templates

## 5.7 Template Template Parameters

# Chapter 6: Move Semantics and ```enable_if<>```

## 6.1 Perfect Forwarding

## 6.2 Special Member Function Templates

## 6.3 Disable Templates with ```enable_if<>```

## 6.4 Using ```enable_if<>```

## 6.5 Using Concepts to Simplify ```enable_if<>``` Expressions