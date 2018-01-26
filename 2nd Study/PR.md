# Chapter 3 Nontype Template Parameters

template parameter는 반드시 type일 필요는 없다. 값을 template parameter로 사용하는 것도 가능하다.

## 3.1 Nontype Class Template Patameters

이전 장에서 예로 든 Stack의 배열 크기를 template parameter로 지정하는 예제.
Stack의 사용자가 적절한 Stack 크기를 지정하도록 할 수 있다.

```cpp
template<typename T, std::size_t Maxsize>
class Stack {
  private:
    std::array<T,Maxsize> elems;  // elements
    ...
};
```

그리고 push()를 호출할 때 Stack이 가득 찼는지 확인할 수 있다.

```cpp
template<typename T, std::size_t Maxsize>
void Stack<T,Maxsize>::push (T const& elem)
{
  assert(numElems < Maxsize);
  elems[numElems] = elem;       // append element
  ++numElems;                   // increment number of elements
}
```

이 클래스 템플릿을 사용하기 위해서 element type과 최대 크기를 지정해서 사용해야 한다.

```cpp
#include "stacknontype.hpp"
#include <iostream>
#include <string>

int main()
{
  Stack<int,20>         int20Stack;   // stack of up to 20 ints
  Stack<int,40>         int40Stack;   // stack of up to 40 ints
  Stack<std::string,40> stringStack;  // stack of up to 40 strings

  // manipulate stack of up to 20 ints
  int20Stack.push(7);
  std::cout << int20Stack.top() << '\n';
  int20Stack.pop();

  // manipulate stack of up to 40 strings
  stringStack.push("hello");
  std::cout << stringStack.top() << '\n';
  stringStack.pop();
}
```

여기서 int20Stack과 int40Stack은 다른 type인 점에 주목해야 한다. 이 둘은 서로의 type간에 변환 방법이 정의되어 있지 않기 때문에, 복사, 할당 등이 불가능하다.

default argument 역시 nontype template parameter에서도 사용이 가능하다.

```cpp
template<typename T = int, std::size_t Maxsize = 100>
class Stack {
  ...
};
```

## 3.2 Nontype function Template Parameters

함수에서도 역시 nontype parameter를 정의할 수 있다.

```cpp
template<int Val, typename T>
T addValue (T x)
{
  return x + Val;
}
```

이 함수가 쓰이는 예를 들어 보면, 어떤 collection의 각 element에 특정 숫자를 더하도록 std::transform을 사용해서 쓸 수 있다.

```cpp
std::transform (source.begin(), source.end(), // start and end of source
                dest.begin(),                 // start of destination
                addValue<5,int>);
```

std::transform이 호출될 때, 4번째 parameter의 완전한 타입이 필요하지만, 특정한 상황에서 어떤 template parameter가 맞을 지 추론하는 기능은 없다.
하지만 여기서 이전 parameter의 타입을 추론하도록 프로그래밍 하는 것은 가능하다.

```cpp
template<auto Val, typename T = decltype(Val)>
T foo();
```

또는 전달된 type과 같은 type을 값이 갖도록 하기 위해 다음과 같이 할 수 있다.

```cpp
template<typename T, T Val = T{}>
T bar();
```

## 3.3 Restrictions for Nontype Template Parameters

Nontype template parameter는 몇몇 제약사항이 있다.
- 정수 형태의 상수
- 객체, 함수, 멤버의 포인터
- 함수 또는 객체의 lvalue reference
- `std::nullptr_t`

부동소수점 숫자나 class-type 객체는 nontype template parameter로 허용되지 않는다.

```cpp
template<double VAT>        // ERROR: floating-point values are not
double process (double v)   //        allowed as template parameters
{
  return v * VAT;
}

template<std::string name>  // ERROR: class-type objects are not
class MyClass {             //        allowed as template parameters
  ...
};
```

template parameter로 포인터나 레퍼런스를 전달할 때는 문자열 이거나 temporaried이거나 data member이거나 subobject이면 안된다.
C++17 이전의 표준들은 아래의 제약사항이 추가된다.
- In C++11, the objects also had to have external linkage.
- In C++14, the objects also had to have external or internal linkage.
그러므로 다음의 코드는 오류이다.

```cpp
template<char const* name>
class MyClass {
  ...
};

MyClass<"hello"> x; // ERROR: string literal "hello" not allowed
```

하지만 C++ 버전에 따라서 다음은 가능하다.

```cpp
extern char const s03[] = "hi";   // external linkage
char const s11[] = "hi";          // internal linkage

int main()
{
  Message<s03> m03;               // OK (all versions)
  Message<s11> m11;               // OK since C++11
  static char const s17[] = "hi"; // no linkage
  Message<s17> m17;               // OK since C++17
}
```

#### Avoiding Invalid Expressions

```cpp
template<int I, bool B>
class C;
...
C<sizeof(int) + 4, sizeof(int)==4> c;
```

nontype template parameter의 argument는 어떤 compile-time expression도 가능하다.

## 3.4 Template Parameter Type auto

C++17 이후로 nontype template parameter의 type을 generic하게 선언하는 것이 가능하다.
auto를 Stack의 maxsize의 타입으로 선언함으로써 어떤 타입이든 nontype template parameter의 type이 될 수 있다.
```cpp
template<typename T, auto Maxsize>
class Stack {
  ...
};
```

C++14 이후로 size() 함수의 반환 type도 auto로 쓰는 것이 가능하다.
```cpp
auto size() const {
  return numElems;
}
```

이런 방법으로 element의 최대 갯수는 Stack 클래스를 선언할 때가 아닌, 사용할 때 결정된다.
stackauto.cpp의 코드를 살펴보면 20u를 주기때문에 int20Stack의 size type은 unsigned int가 되고, stringStack의 40을 주기 때문에 type은 int가 된다.

3.3에서 살펴본 제약사항은 여기서도 그대로 적용된다. 때문에 다음의 코드는 에러이다.
```cpp
Stack<int,3.14> sd; // ERROR: Floating-point nontype argument
```

message.cpp 예제를 통해 nontype template parameter로 문자열을 constant array로 전달하는 방법을 볼 수 있다.

`template<decltype(auto) N>`을 이용해서 N을 reference로 instantiation 할 수 있다.

```cpp
template<decltype(auto) N>
class C {
  ...
};

int i;
C<(i)> x; // N is int&
```

# Chapter 4 Variadic Templates

C++11 이래로 임의의 type을 갖는 임의의 수의 template argument를 쓸 수 있다.

## 4.1 Variadic Templates

template parameter는 수의 제한이 없는 template argument를 정의할 수 있으며, 이를 variadic templates라고 부른다.

### 4.1.1 Variadic Templates by Example

![varprint1](https://github.com/CppKorea/CppTemplateStudy/blob/master/2nd Study/varprint1.hpp)

`function parameter pack`: "args"를 `function parameter pack`라고 부른다.
`template parameter pack`: "Type"를 `template parameter pack`라고 부른다.

예를 들어서 varprint1.hpp의 동작을 살펴보자.

```cpp
std::string s("world");
print (7.5, "hello", s);
```

위 코드의 출력은 다음과 같다.

```cpp
7.5
hello
world
```

단계적으로 어떻게 동작하는지를 설명하면 다음과 같다.

```cpp
print<double, char const*, std::string> (7.5, "hello", s);
```
여기에서 `firstArg`는 7.5이고 `type T`는 double로 추론된다.
그러므로 나머지 argument들 `"hello"`와 `s`는 `function parameter pack`인 args이고, 이들의 type은 `template parameter pack`인 `char const*`와 `std::string`이 된다.
args...로 호출하는 print는 다음과 같이 호출된다.

```cpp
print<char const*, std::string> ("hello", s);
```

여기에서 `firstArg`는 `"hello"`이고 `type T`는 `char const*`로 추론된다.
그러므로 나머지 argument `s`는 `function parameter pack`인 args이고, type은 `template parameter pack`인 `std::string`이 된다.
args...로 호출하는 print는 다음과 같이 호출된다.

```cpp
print<std::string> (s);
```
여기에서 `firstArg`는 `s`이고 `type T`는 `std::string`로 추론된다.
그러므로 나머지 argument가 없기 때문에 argument가 없는 print()를 호출하고 끝이난다.

### 4.1.2 Overloading Variadic and Nonvariadic Templates

만약 trailing parameter pack이 있는 함수와 없는 함수 모두를 호출 가능할 경우, trailing parameter pack이 없는 함수가 우선 호출 된다.

### 4.1.3 Operator sizeof...

C++11에서는 `sizeof`의 variadic template를 위한 새로운 연산자인 `sizeof...`를 쓸 수 있다.

```cpp
template<typename T, typename... Types>
void print (T firstArg, Types... args)
{
  std::cout << sizeof...(Types) << '\n';  // print number of remaining types
  std::cout << sizeof...(args) << '\n';   // print number of remaining args
}
```

Types와 args가 각각 몇개의 type과 arguement가 남았는지가 출력된다.
하지만 sizeof...의 동작에 대해 오해를 할 수 있는데, 다음의 코드를 보자.

```cpp
template<typename T, typename... Types>
void print (T firstArg, Types... args)
{
  std::cout << firstArg << '\n';
  if (sizeof...(args) < 0) {      // error if sizeof... (args) == 0
    print(args...);               // and no print() for no arguements declared
  }
}
```

if는 run-time에 동작하며 sizeof...은 compile-time에 동작하기 때문에 위 코드는 에러가 발생한다.
C++17부터는 compile-time에 동작하는 if가 추가되었다. Section 8.5에서 더 자세히 살펴본다.

## 4.2 Fold Expressions

C++17 이후로 이항연산자를 `function parameter pack`에 쓰면 모든 argument들에 적용하는 문법이 추가되었다.

```cpp
template<typename... T>
auto foldSum (T... s) {
  return (... + s);     // ((s1 + s2) + s3) ...
}
```

만약 parameter pack이 empty라면 일반적으로 ill-formed 표현으로 보지만 몇가지 예외가 있다. (&&: true, ||: false, .: void())
다음의 표에 fold expression들을 정리했다.

| Fold Expression         | Evaluation                                  |
| :---------------------- | :-----------------------------------------: |
| ( ... op pack )         | (((pack1 op pack2) op pack3) ... op packN)  |
| ( pack op ... )         | (pack1 op ( ... (packN-1 op PackN)))        |
| ( init op ... op pack ) | (((init op pack1) op pack2) ... op packN)   |
| ( pack op ... op init ) | (pack1 op ( ... (packN op init)))           |

foldtraverse.cpp 예제를 살펴보면 세번째 Fold Expression의 사용법을 볼 수 있다.

그리고 모든 argument들을 출력하는 간단한 함수를 Fold Expression을 활용해서 만들 수 있다.
```cpp
template<typename... Types>
void print (Types const&... args)
{
  (std::cout << ... << args) << '\n';
}
```

하지만 이 예제는 argument들 사이에 space로 구분을 할 수 없다. space로 구분하는 예제는 addspace.hpp를 보자.

## 4.3 Application of Variadic Templates

variadic templates를 실제로 응용하는 예를 들어보면 아래와 같다.

* shared pointer에서 생성자에 전달 할 argument들을 받는다.
```cpp
// create shared pointer to complex<float> initialized by 4.2 and 7.7:
auto sp = std::make_shared<std::complex<float>>(4.2, 7.7);
```

* thread에 호출 할 함수와 전달할 argument들을 받는다.
```cpp
std::thread t (foo, 42, "hello"); // call foo(42,"hello") in a separate thread
```

* vector에 추가할 새로운 element 생성자의 argument들을 전달한다.
```cpp
std::vector<Customer> v;
...
v.emplace("Tim", "Jovi", 1962); // insert a Customer initilized by three arguments
```

일반적으로 move semantics로 argument들은 "perfectly forwarded"된다. 그러므로 이에 대한 선언은 예를 들어 다음과 같다.
```cpp
namespace std {
  template<typename T, typename... Args> shared_ptr<T>
  make_shared(Args&&... args);

  class thread {
    public:
      template<typename F, typename... Args>
      explicit thread(F&& f, Args&&... args);
      ...
  };

  template<typename T, typename Allocator = allocator<T>>
  class vector {
    public:
      template<typename... Args> reference emplace_back(Args&&... args);
  };
}
```

## 4.4 Variadic Class Templates and Variadic Expressions

앞절의 예제말고도 parameter packs는 다양한 template에서 쓰일 수 있다. (expressions, class templates)

### 4.4.1 Variadic Expressions

parameter pack을 forward하는 것 말고도, 이를 이용해서 연산을 할 수 있다.

```cpp
template<typename... T>
void printDoubled (T const&... args)
{
  print (args + args...);
}
```

이 예제는 각각의 argument들을 더해서 print 함수에 전달한다. 코드로 표현하면 아래와 같다.

```cpp
printDoubled(7.5, std::string("hello"), std::complex<float>(4,2));
print(7.5 + 7.5, std::string("hello") + std::string("hello"), std::complex<float>(4,2) + std::complex<float>(4,2));
```

그리고 compile-time expression도 parameter pack의 연산에 사용할 수 있다.
```cpp
template<typename T1, typename... TN>
constexpr bool isHomogeneous (T1, TN...)
{
  return (std::is_same<T1,TN>::value && ...); // since C++17
}
```

이 예제는 Fold expression의 한 예로서 parameter pack의 모든 argument의 type을 같은지 비교한다.

### 4.4.2 Variadic Indices

다양한 index를 지정하도록 variadic parameter를 이용할 수 있다.
```cpp
template<typename C, typename... Idx>
void printElems (C const& coll, Idx... idx)
{
  print (coll[idx]...);
}

std::vector<std::string> coll = {"good", "times", "say", "bye"};
printElems(coll,2,0,3);

print (coll[2], coll[0], coll[3]);
```

그리고 nontype template parameter 역시 index로 사용이 가능하다.
```cpp
template<std::size_t... Idx, typename C>
void printIdx (C const& coll)
{
  print(col[Idx]...);
}

std::vector<std::string> coll = {"good", "times", "say", "bye"};
printIdx<2,0,3>(coll);
```

### 4.4.3 Variadic Class Templates

variadic tmeplate는 class에도 적용이 가능하다.

```cpp
template<typename... Elements>
class Tuple;

Tuple<int, std::string, char> t;  // t can hold integer, string, and character

template<typename... Types>
class Variant;

Variant<int, std::string, char> v;  // t can hold integer, string, or character
```

위 예제는 Tuple과 Variant를 예로 든 것으로 다음 챕터에서 자세히 다룰 것이다.
그리고 아래의 예제처럼 index들을 나타내는 클래스도 정의할 수 있다.

```cpp
// type for arbitrary number of indices:
template<std::size_t...>
struct Indices {
};

template<typename T, std::size_t... Idx>
void printByIdx(T t, Indices<Idx...>)
{
  print(std::get<Idx>(t)...);
}

std::array<std::string, 5> arr = {"Hello", "my", "new", "!", "World"};
printByIdx(arr, Indices<0,4,3>();

auto t = atd::make_tuple(12, "monkeys", 2.0);
printByIdx(t, Indices<0, 1, 2>());
```

### 4.4.4 Variadic Deduction Guides

variadic에서 deduction guide를 적용할 수 있다.
```cpp
namespace std {
  template<typename T, typename... U> array(T, U...)
    -> array<enable_if_t<(is_same_v<T, U> && ...), T>, 
             (1 + sizeof...(U))>;
}

std::array a{42,45,77};
```

여기서 template parameter를 guide대로 deduce해서 다음과 같이 추론한다.
```cpp
std::array<int, 3> a{42,45,77};
```

`is_same_v`를 계속 호출하면서 type이 같지 않으면 전체적인 deduction은 실패한다. 즉, 모든 variadic parameter들의 type이 일치해야 한다.

### 4.4.5 Variadic Base Classes and using

