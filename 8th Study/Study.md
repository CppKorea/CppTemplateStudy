# Chapture 15. Template Argument Deduction #


## 15.1 The Deduction Process ##
기본적인 deduction 과정: argument와 parameter를 비교하고, 적당한 substitution을 시도합니다. 각각의 argument-parameter pair는 독립적으로 처리됩니다. 

```C++
template <typename T> T max (T a, T b) {
  return b < a ? a : b; 
} 

int main() {
  auto g = max(1, 1.0);
  return 0;
}
```

* 첫 번째 argement가 int이므로 임시적으로 parameter T는 int가 됩니다. 
* 두 번째 argument가 double이고, T는 double이 되어야 합니다.
* 충돌 발생. 

```C++
 In function 'int main()':
6:23: error: no matching function for call to 'max(int, double)'
6:23: note: candidate is:
1:27: note: template<class T> T max(T, T)
1:27: note:   template argument deduction/substitution failed:
6:23: note:   deduced conflicting types for parameter 'T' ('int' and 'double')
```

-----
* If all the deduced template parameters are consistently determined, the deduction process can still fail if substituting the arguments in the rest of the function declaration results in an invalid construct.
* template parameter deduction이 잘 맞더라도, 함수의 다른 argument를 치환하는데 실패하면 deduction process 가 실패합니다.

```c++
template <typename T>
typename T::ElementT at(T a, int i) {
  return a[i];
}

void f(int* p) {
  int x = at(p, 7);
}
```
* `int*` 에는 ElementT type이 없..

-----

* We still need to explore how argument-parameter matching proceeds. We describe it in terms of matching a type A (derived from the call argument type) to a parameterized type P (derived from the call parameter declaration). If the call parameter is declared with a reference declarator, P is taken to be the type referenced, and A is the type of the argument. Otherwise, however, P is the declared parameter type, and A is obtained from the type of the argument by decaying2 array and function types to pointer types, ignoring top-level const and volatile qualifiers. For example:

* type A : derived from the call argument type
* type P : derived from the call parameter declaration
* with & => P는 레퍼런스된 타입, A는 argument type
* without & => P는 parameter type, A는 decaying 된 결과. (array, function type은 pointer로, const, volatile은 무시)

```C++
template <typename T> void f(T); // parameterized type P is T
template <typename T> void g(T&); // parameterized type P is also T

double arr[20];
int const seven = 7;

f(arr); // nonreference parameter: T is double*
g(arr); // reference parameter: T is double[20]
f(seven); // nonreference parameter: T is int
g(seven); // reference parameter: T is int const
f(7); // nonreference parameter: T is int 
g(7); // reference parameter: T is int = > ERROR: can’t pass 7 to int&
```

* with reference declarator. 
  * 직관적인 deduction.
* without reference declarator.
  * 배열과 함수 타입이 포인터로 decaying 된다. const, volatile도 날라갑니다.

## 15.2 Deduced Contexts ##

복잡한 parameterized types 도 주어진 argument type에 대응될 수 있습니다.

```C++
template <typename T>
void f1(T*);

template <typename E, int N>
void f2(E(&)[N]);

template <typename T1, typename T2, typename T3>
void f3(T1 (T2::*)(T3*));

class S {
public:
  void f(double*);
};

void g (int*** ppp) {
  bool b[42];
  f1(ppp); // deduces T to be int**
  f2(b); // deduces E to be bool and N to be 42
  f3(&S::f); // deduces T1 = void, T2 = S, and T3 = double
}
```

복잡한 타입이라고 하더라도 기본 요소들로 구성되어 있습니다. (pointer, reference, array, function, pointer-to-member, template-ids)

matching 과정은 상위레벨부터 이뤄지고, 이것을 `deduced contexts`라고 합니다.

* duduced context가 아닌 것들의 예.. 
  * Qualified type names
    * Q<T>::X는 T를 deduce하는데 사용되지 않습니다.
  * nontype expressions that are not just a nontype paramater
    * `S<I+1>`은 `I`를 deduce 하는데 사용되지 않습니다. 
    * `int(&)[sizeof(S<T>)]` T는 deduce되지 않습니다.
* These limitations should come as no surprise because the deduction would, in general, not be unique (or even finite), although this limitation of qualified type names is sometimes easily overlooked. A nondeduced context does not automatically imply that the program is in error or even that the parameter being analyzed cannot participate in type deduction.
* 이런 제한이 있지만, nondeduced context를 사용할 수 없는것은 아닙니다. ??

```C++
template <int N>
class X {
public:
  using I = int;
  void f(int) { }
};

template <int N>
void fppm(void (X<N>::*p)(typename X<N>::I));

int main() {
  fppm(&X<33>::f); // fine: N deduced to be 33
}
```

* fppm()에서 X<N>::I 는 nondeduced context이지만, X<N>이 X<33>으로 deduce됩니다.

* 반대로 deduced context에서 deduce된 parameter type에 문제가 있을 수 있습니다.
```
template<typename T>
void f( X< Y <T>, Y <T> > );

void g() {
  f(X<Y<int>, Y<int> >());  // OK
  f(X<Y<int>, Y<char> >()); // ERROR: deduction fails
}
```

## 15.3 Special Deduction Situations ##
* type A : derived from the call argument type
* type P : derived from the call parameter declaration

(A, P) pair가 함수호출의 인자와 템플릿 파라미터에 의해 추론되지 않는 몇가지 경우가 있습니다.

1. 함수 템플릿의 주소가 쓰이는 경우.
   ```
   template<typename T>
   void f(T, T);

   void (*pF)(char, char) = &f;
   ```
2. Determining partial ordering between overloaded function templates. ?
3. Matching an explicit specialization to a function template
4. Matching an explicit instantiation to a template
5. Matching a friend function template specialization to a template specialization to a template
6. Matching a placement operator delete or operator delete[] to a corresponding placement operator new or operator new[] template.
7. Conversion function templates.
  ```
  class S {
  public:
      template<typename T> operator T&()
  };

  void f(int (&)[20]);
  void g(S s) {
      f(s);
  }
  ```
  T를 int[20]으로 치환 성공.
8. `auto` placeholder type에 특별한 처리. 15.10.4에서 설명


## 15.4 Initializer Lists ##

함수 호출 argement가 initializer list인 경우, 그 argument는 특정한 형식을 가지지 않고 주어진 pair(A, P)에서 일반적으로 아무런 deduction도 이뤄지지 않습니다. 

```
#include <initiaizer_list>

template<typename T> void f(T p);

int main() {
  f({1,2,3}); // ERROR braced list로는 deduce할 수 없음.
}
```

그러나, parameter type P에서 레퍼런스와 최상위 const, volatile qualifier를 제거한 이후에 std::initializer_list<P`>와 같은 경우, P`와 initializer list의 각 아이템과 비교를 통해 추론이 진행됩니다.

```
#include <initializer_list>

template<typename T> void f(std::initializer_list<T>);

int main() {
  f({2, 3, 5, 7, 9});                // OK: T is deduced to int
  f({'a', 'e', 'i', 'o', 'u', 42});  // ERROR: T deduced to both char and int
}
```

## 15.5 Parameter Packs ##

추론 과정은 template argument를 결정하기 위해 각 argument를 각 parameter에 매치합니다.
그러나, variadic template에 대한 추론 과정에서는 1:1 관계가 적용되지 않고 parameter pack이 여러 argument를 매치합니다. 
```
template<typename First, typename... Rest>
void f(First first, Rest... rest);

void g(int i, double j, int* k) {
  f(i, j, k); // First : int, Rest : {double, int*}
}
```

parameter packs을 추론은 복잡합니다. 
```
template <typename T, typename U> class pair { };

template <typename T, typename… Rest>
void h1(pair <T, Rest> const&…);
template <typename… Ts, typename… Rest>
void h2(pair <Ts, Rest> const&…);

void foo(pair <int, float> pif,
    pair<int, double> pid,
    pair<double, double> pdd) {
  h1(pif, pid); // OK: deduces T to int, Rest to {float, double}=
  h2(pif, pid); // OK: deduces Ts to {int, int}, Rest to {float, double}
  h1(pif, pdd); // ERROR: T deduced to int from the 1st arg, but to double from the 2nd
                // pif 첫 번째 T는 int, Rest는 float으로 추론되는데. pdd는 T가 double 이다.
  h2(pif, pdd); // OK: deduces Ts to {int, double}, Rest to {float, double}
}
```

parameter pack의 추론은 함수의 argument-parameter 가 처리되는 추론에 제한되지는 않습니다.

```
template<typename... Types> class Tuple { };

template<typename... Types>
bool f1(Tuple<Types...>, Tuple<Types...>);

template<typename... Types1, typename... Types2>
bool f1(Tuple<Types1...>, Tuple<Types2...>);

void bar(
    Tuple<short, int, long> sv,
    Tuple<unsigned short, unsigned, unsigned long> uv)
{
  f1(sv, sv); // OK: Types is deduced to {short, int, long} 
  f2(sv, sv); // OK: Types1 is deduced to {short, int, long}, // Types2 is deduced to {short, int, long} 
  f1(sv, uv); // ERROR: Types is deduced to {short, int, long} from the 1st arg, but // to {unsigned short, unsigned, unsigned long} from the 2nd 
  f2(sv, uv); // OK: Types1 is deduced to {short, int, long}, // Types2 is deduced to {unsigned short, unsigned, unsigned long}
}
```

## 15.5.1 Literal Operator Templates ##

Literal operator templates은 unique한 방식으로 argument가 결정됩니다.

```
template<char...> int operator ""_B7();
...
int a = 121_B7;

template<char… cs>
int operator""_B7() {
  std::array<char, sizeof…(cs)> chars{cs…}; // initialize array of passed chars
  for (char c : chars) {
    // and use it (print it here)
    std:: cout < < "’" < < c < < "’";
  }
  std:: cout < < ’\ n’;
  return …;
}

auto b = 01.3_B7; // OK: deduces <’0’, ’1’, ’.’, ’3’> 
auto c = 0xFF00_B7; // OK: deduces <’0’, ’x’, ’F’, ’F’, ’0’, ’0’> 
auto d = 0815_B7; // ERROR: 8 is no valid octal literal 
auto e = hello_B7; // ERROR: identifier hello_B7 is not defined 
auto f = "hello"_B7; // ERROR: literal operator _B7 does not match
```

친절한 설명: https://msdn.microsoft.com/ko-kr/library/dn919277.aspx

## 15.6 Rvalue References ##
### 15.6.1 Reference Collapsing Rules ###
reference의 reference를 명시적으로 선언할 수 없습니다. 

```
int const& r = 42;
int const& & ref2ref = i; // ERROR: reference to reference is invalid
```

그러나, template 치환이나 type alias 또는 decltype 를 통해서는 가능합니다. 
``` c++
using RI = int&;
int i = 42;
RI r = i;
RI const& rr = r; // rr has type int&
```

이런 구성에 대해 타입이 결정되는 규칙을 reference collapsing rules라고 합니다. 내부 reference의 최상위부에 있는 const 나 volatile qualifier 는 무시됩니다. 

|inner reference|oouter refrence|resulting reference|
|-|-|--|
|&|&|&|
|&|&&|&|
|&&|&|&|
|&&|&&|&&|

``` c++
#include <boost/type_index.hpp>
using boost::typeindex::type_id_with_cvr;

#define print_type(var) do { \
  std::cout << type_id_with_cvr<decltype(var)>().pretty_name() << std::endl; \
} while(0)

int main() {
  using RCI = int const&;
  RCI volatile&& r = 42; // OK: r has type int const&
  using RRI = int&&;
  RRI const&& rr = 42; // OK: rr has type int&&

  print_type(r);
  print_type(rr);

  RCI const r1 = 42;
  int i = 0;
  RRI const& r2 = i;

  return 0;
};
```

```
RCI const r1 = 42; // int const&

int i = 0;
RRI const& r2 = i; // int&
```

### 15.6.2 Forwarding References ###

template argement deduction은 함수 파라미터가 forwarding reference인 경우에 특별하게 동작합니다. 
forwarding reference: rvalue referencto to a template parameter of that function template
추론 과정에서 argument의 타입만 신경쓰는게 아니라, rvalue 인지 lvalue인지도 고려합니다.

```
template <typename T> void f(T&& p); // p is a forwarding reference

void g() {
  int i;
  int const j = 0;
  f(i); // argument is an lvalue; deduces T to int& and 
        // parameter p has type int& 
  f(j); // argument is an lvalue; deduces T to int const& 
        // parameter p has type int const& 
  f(2); // argument is an rvalue; deduces T to int 
        // parameter p has type int&& 
}
```

In the call f(i) the template parameter T is deduced to int&, since the expression i is an lvalue of type int. Substituting int& for T into the parameter type T&& requires reference collapsing, and we apply the rule & + && ! & to conclude that the resulting parameter type is int&, which is perfectly suited to accept an lvalue of type int.
* 아? 첫 번째 구절.. 

* T가 reference type으로 추론되는 경우에 템플릿 instantiation에서 이런 일이 일어납니다.
``` c++
template<typename T> void f(T&& p) {
  T x = p;
  print_type(x);
}

template<typename T> void g(T&& p) {
  std::remove_reference_t<T> x = p;
  print_type(x);
}

int main() {
  int i  = 0;
  f(i);
  g(i);
  
  return 0;
}
```

### 15.6.3 Perfect Forwarding ###

rvalue reference에 대한 특별한 추론 규칙과 reference collapsing 규칙은 거의 모든 argument를 받아들이고 중요한 특징을 유지하는 function template을 작성할 수 있게 합니다. 

``` c++
class C { 
public:
  C() {}
  int i;
};

void g(C&) { std::cout << "C&" << std::endl; }
void g(C const&) { std::cout << "C const&" << std::endl; }
void g(C&&) { std::cout << "C&&" << std::endl; }

template<typename T> 
void forwardToG(T&& x) {
  g(static_cast<T&&>(x)); // forward x to g()
} 

int main() {
  C v;
  C const c;
  forwardToG(v); // eventually calls g(C&)
  forwardToG(c); // eventually calls g(C const&)
  forwardToG(C()); // eventually calls g(C&&)
  forwardToG(std::move(v)); // eventually calls g(C&&)

  g(v);
  g(c);
  g(C());
  g(std::move(v));

  return 0;
}
```

이런 기법을 perfect forwarding이라고 부릅니다. 간접적으로 g()를 호출하지만, 바로 g()를 호출하는 것과 동일하기 때문입니다. 어떤 부가적인 copy도 일어나지 않고 오버로드된 g()가 선택됩니다. 

static_cast에 대해서는 설명이 더 필요합니다. forwardToG의 각각의 instantiation에서 parameter x는 lvalue 또는 rvalue reference type 입니다. 그렇지만, 표현식 x는 lvalue입니다. static_cast는 x를 기존 형식과 lvalue/rvalue-ness를 포함하여 캐스팅합니다. perfect forwarding을 위해서는 static_cast보다 std::forward를 쓰는게 더 좋습니다. 의도를 확실히 할 수 있다는 점에서, 그리고 &를 빠트릴 수 도 있습니다.

``` c++
template<typename T> 
void forwardToG(T&& x) {
  g(std::forward<T>(x)); // forward x to g()
} 
```

#### Perfect Forwarding for Variadic Templates ####
``` c++
template<typename... Ts>
void forwardToG(Ts&&... xs) {
  g(std::forward<Ts>(xs)...); // forward all xs to g()
}
```

아.. 생략.. 

### 15.6.4 Deduction Surprises ###

rvalue reference에 대한 특별한 추론 규칙은 perfect forwarding에 매우 유용합니다. 

``` c++
void int_lvalues(int&); // accepts lvalues of type int 

template<typename T> void lvalues(T&); // accepts lvalues of any type 

void int_rvalues(int&&); // accepts rvalues of type int template<typename T> 

void anything(T&&); // SURPRISE: accepts lvalues and rvalues of any type
```

깜짝! 저자가 의도하는 서프라이즈 포인트를 잘 모르겠습니다.
이 추론은 함수 파라미터가 동작하는 조건이 있습니다.
1. 템플릿 파라미터 && 형식이고
2. 함수 템플릿의 한 부분이
3. named template parameter is declared by that function template

그래서 아래에 나오는 어떤 상황에서도 적용되지 않습니다.

``` c++
template<typename T> class X {
public:
  X(X&&); // X is not a template parameter
  X(T&&); // this constructor is not a function template
  template<typename U> X(X<U>&&); // X<U> is not a template parameter 
  template<typename U> X(U,T&&); // T is a template parameter from an outer template
};
```

## 15.7 SFINAE (Substitution Failure Is Not An Error) ##
``` c++
template<typename T, unsigned N>
T* begin(T (&array)[N]) {
  return array;
}

template <typename Container> 
typename Container::iterator begin(Container& c) {
  return c.begin();
}

int main() {
  std::vector<int> v;
  int a[10];
  ::begin(v); // OK: only container begin() matches, because the first deduction fails 
  ::begin(a); // OK: only array begin() matches, because the second substitution fails
}
```
argument가 vector인 첫 번째 begin()호출에서 array begin에 대한 추론에서 실패합니다. array가 아니니까. 
argument가 array인 두 번째 호출에서는 T, N이 int, 10으로 추론됩니다.

### 15.7.1 Immediate Context ###

생략

## 15.8 Limitations of Deduction ##
### 15.8.1 Allowable Argument Conversions ###

* type A : derived from the call argument type
* type P : derived from the call parameter declaration

템플릿 추론은 함수 템플릿 파라미터와 A 타입과 동일한 parameterized P 타입을 찾습니다. 하지만, 이게 불가능하더라도 P가 deduced context에 있는 템플릿 파라미터를 갖고 있다면, 아래와 같은 차이는 문제 없이 추론됩니다. 
* 원래 파라미터에 &가 붙어서 선언되어 있는 경우, substitude P 타입에는 A 타입에 const/volatile가 붙어 있을 수 있다.
* A 형식이 포인터 또는 멤버에 대한 포이터 형식이고 qualification conversion으로 변환 가능합니다. 이 변환 과정에서 const or volatile qualifier가 붙습니다.
* Unless deduction occurs for a conversion operator template, the substituted P type may be a base class type of the A type or a pointer to a base class type of the class type for which A is a pointer type.

``` c++
template<typename T>
class B { };

template<typename T>
class D : public B<T> { };

template<typename T> void f(B<T>*);

void g(D<long> dl) {
  f(&dl); // deduction succeeds with T substituted with long
}
```

P에 deduced context의 template parameter가 포함되어 있지 않다면, 모든 암묵적인 변환이 가능합니다.

``` c++
template <typename T> int f(T, typename T::X);

struct V {
  V();
  struct X {
    X(double);
  };
} v;

int r = f(v, 7.0); // OK: T is deduced to v through the first parameter, 
                   // which causes the second parameter to have type V::X
                   // which can be constructed from a double value
```

이런 규칙은 매우 한정된 범위에만 적용됩니다. 
``` c++
template<typename T> T max (T a, T b) {
  return b < a ? a : b;
}

std:: string maxWithHello(std:: string s) {
  return :: max(s, "hello");
}
```

첫 번째 T는 std::string이 되고, 두 번째는 char[6]이 됩니다. 이렇게 추론이 실패하지만, `::max<std::string>(s, "hello")`에서는 "hello"가 암묵적으로 std::string으로 변환되어서 추론에 성공합니다. 

### 15.8.2 Class Template Arguments ###
c++17 이전까지 template argument deduction이 함수와 멤버함수에 명시적으로 적용됩니다. 특히 클래스 템플릿에 대한 인수는 해당 생성자 중 하나의 호출에 대한 인수에서 추론되지 않았습니다.

``` c++
template<typename T>
class S {
public:
  S(T b) : a(b) {
  }
private:
  T a;
};

int main() {
  // S x(1); // Error before c++17: 
             // the class template parameter T was not deduced 
             // from the constructor call argument 1
  S<int> x(1);
  return 0;
}
```

### 15.8.3 Default Call Arguments ###

디폴트 호출 argument도 일반 함수처럼 지정할 수 있습니다.

``` c++
template<typename T>
void init(T* loc, T const& val = T()) {
  *loc = val;
}
```

사실, 이 예에서 알 수 있듯이 기본 함수 호출 argument는 템플릿 매개 변수에 따라 달라질 수 있습니다. 이러한 의존적인 default argument는 명시적 argument가 제공되지 않는 경우에만 인스턴스화됩니다.

``` c++
class S {
public:
  S(int, int);
};

S s(0, 0);

int main() {
  init(&s, S(7, 42)); // T() is invalid for T = S,
                      // but the default call argument T() needs no instantiation
                      // because an explicit argument is given
  return 0;
}
```

### 15.8.4 ###
Like default call arguments, exception specifications are only instantiated when they are needed. This means that they do not participate in template argument deduction.

``` c++
template <typename T>
void f(T, int) noexcept(nonexistent(T()));// #1

template <typename T>
void f(T, …); // #2 (C-style vararg function)

void test(int i) {
  f(i, i); // ERROR: chooses #1 , but the expression nonexistent(T()) is ill-formed
}
```
예외는 템플릿 argument deduction에는 관여하지 않습니다.

``` c++
template <typename T>
void g(T, int) throw(typename T:: Nonexistent); //# 1

template <typename T>
void g(T, …); //# 2

void test(int i) {
  g(i, i);// ERROR: chooses #1 , but the type T::Nonexistent is ill-formed
}
```

## 15.9 Explicit Function Template Arguments ##

함수 템플릿 argument가 추론되지 않는 경우, 명시적으로 지정할 수 있습니다. 
``` c++
template<typename T> T default_value() {
  return T{};
}

int main() {
  return default_value<int>();
}
```

추론이 성공하는 경우에도 사용할 수 있습니다.
``` c++
template<typename T> void compute(T p) {
  ...
}

int main() {
  compute<double>(2);
}
```

template argument가 명시적으로 지정되면 대응되는 파라미터는 추론 결과를 따르지 않습니다. deduced call에서는 불가능한 함수 호출 파라미터의 변환이 허용됩니다. 예를 들어 `compute<double>(2)`에서 2가 암묵적으로 double로 변환됩니다.

명시적으로 특정 템플릿 argument를 지정하면서 다른 argument는 추론이 되게 할 수 있습니다. 그러나, 명시적으로 지정한 template argument는 언제나 왼쪽에서 오른쪽으로 매칭됩니다. 그러므로 추론이 어려운 parameter는 반드시 처음에 나와야 합니다. 

``` c++
template <typename Out, typename In>
Out convert(In p) { 
  …
}

int main() {
  auto x = convert<double>(42); // the type of parameter p is deduced, 
                                // but the return type is explicitly specified
}
  ```

빈 template argument를 지정하면 선택된 함수가 템플릿 인스턴스인지 알 수 있습니다. 

``` c++
int f(int); // #1 
template <typename T> T f(T); // #2 

int main() {
  auto x = f(42); // calls #1
  auto y = f <>(42); // calls #2
}
```

`f(42)`는 비템플릿 함수를 선택합니다. 왜냐하면 overload resolution이 보통 함수를 선호하기 떄문입니다. (모든 다른 조건이 동일한 경우)


friend에 대한 얘기... 

생략


## 15.10 Deduction from Initializers and Expressions ##

c++11에는 형식을 initialier에서 추론한 변수를 선언하는 기능이 포함됩니다. 또한 변수 또는 함수 또는 표현식의 타입을 표현하는 메커니즘을 제공합니다. 이러한 기능은 매우 편리하다는 것이 밝혀졌고 c++14 및 c++ 17에서 이에 대한 내용이 추가되었습니다.


### 15.10.1 The auto Type Specifier ###

`auto` type specifier는 여러 곳에서 타입 추론에 사용됩니다. 이 경우 `auto`를 `placeholder type`이라고 합니다. 

``` c++
template<typename Container>
void useContainer(Container const& container) {
  auto pos = container.begin();
  while (pos != container.end()) {
    auto& element = *pos++;
    … // operate on the element
  }
}
```

`auto` 사용으로 길고 복잡한 타입을 적을 필요가 없어졌습니다. 

``` c++
typename Container::iterator pos = container.begin();
...
typename std::iterator_traits<typename Container::iterator>::reference element = *pos++;
```

`auto`에 대한 추론은 템플릿 argument 추론 메커니즘과 같습니다. type specifier `auto`는 template type parameter라고 하고, 추론 과정은 변수가 함수 파라미터이고 초기화값은 함수의 argument로 바꿔서 생각하면 됩니다.

``` c++
template<typenameT> void deducePos(T pos);
deducePos(container.begin());
```

auto 유형의 변수가 참조 형식이 될 수 없습니다. 두 번째 `auto` 예제에서 `auto&`를 사용하면 추론된 형식에 대한 참조가 어떻게 생성되는지 보여줍니다. 이 추론은 다음 함수 템플릿 호출과 동일합니다.

``` c++
template<typename T> deduceElement(T& element);
deduceElement(*pos++);
```

여기서 `element`는 항상 참조 형식이며 초기화는 임시값을 만들지 않습니다. 

`auto`에 rvalue reference를 지정하는것도 가능합니다. 이렇게 하는 것은 `forwarding reference`와 비슷하게 동작합니다. 

``` c++
auto&& fr = ...; // deduction model is based on a function template
...
template<typename T> void f(T&& fr);// auto replaced by template parameter T
```

``` c++
int x;
auto&& rr = 42; // OK: rvalue reference binds to an rvalue (auto = int)
auto&& lr = x; // Also OK: auto = int& and reference collapsing makes lr an lvalue reference
```

이 기법은 일반 코드에서 자주 사용되어 value catecory(lvalue와 rvalue)를 알 수 없는 함수 또는 연산자 호출의 결과를 바인드합니다. 결과의 복사본을 만들지 않아도됩니다. 예를 들어 범위 기반 for 루프에서 반복 값을 편하게 선언할 수 있습니다.

``` c++
template<typename Container>
void g(Container c) {
  for (auto&& x : c) {
    ...
  }
}
```

여기서 Container의 interation interface를 알 수 없지만, `auto`를 사용하면 iterating하는 값이 대한 복사가 없다고 확신할 수 있습니다. std::forward<T>() can be invoked as usual on the variable as usual, if perfect forwarding of the bound value is desired.

`auto` specifier로 변수를 const, 포인터, 멤버 포인터로 만들 수 있고, `auto`가 매인 형식 지정자가 되어야 합니다.
``` c++
template<typename T> struct X { T const m; };
auto const N = 400u; // OK: constant of type unsigned int
auto* gp = (void*)nullptr; // OK: gp has type void*
auto const S::*pm = &X<int>:: m; // OK: pm has type int const
X<int>::* X<auto> xa = X<int>(); // ERROR: auto in template argument 
int const auto::* pm2 = &X<int>:: m; // ERROR: auto is part of the “declarator”
```

c++에서 이런 구문을 지원하지 못하는 것에 대한 아무런 기술적인 이유도 없지만, c++ 위원회에서는 이런 구문의 장점이 추가 구현 비용과 남용 가능성보다 더 크다고 생각합니다. 

구식 `auto` 저장 지정자는 c++11과 그 이후에서 더 이상 사용하지 않습니다.
``` c++
int g() {
  auto int r = 24; // valid in c++03 but invalid in c++11
  return r;
}
```

#### Deduced Return Type ####
c++14에서 auto placeholder type이 함수의 리턴 타입으로 추론될 수 있게 되었습니다. 

``` c++
auto f() { return 42; }
...
auto f() -> auto { return 42; }
```
