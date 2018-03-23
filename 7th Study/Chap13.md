
18/03/24 진행: [박동하](https://github.com/luncliff) luncliff@gmail.com

### 추가 자료
 - [Templates and Name Resolution](https://docs.microsoft.com/en-us/cpp/cpp/templates-and-name-resolution)
 - [Name Resolution for Locally Declared Names](https://docs.microsoft.com/en-us/cpp/cpp/name-resolution-for-locally-declared-names)
 - [Function Overloading](https://docs.microsoft.com/en-us/cpp/cpp/function-overloading)
 - [Overload Resolution for Template Calls](https://docs.microsoft.com/en-us/cpp/cpp/overload-resolution-of-function-template-calls)


# Chapter 13: 템플릿과 이름
> Names in Templates

 - 이름은 프로그래머가 부여하는 것
 - C++ 컴파일러는 이름을 발견하면, 어떤 존재(entity)를 가리키는 것인지 "탐색"해야함  
   (C++ 언어를 구현하는 입장에선 굉장히 어려운 일)

C++은 문맥 인식(context sensitive) 언어: 
 - 하나를 이해하기 위해선 더 넓은 문맥(context)를 알아야만 한다는 의미 

```
x * y;    // x가 변수인 경우 - 곱셈(아마도)
          // x가 타입인 경우 - x 포인터 변수 y의 선언
```

템플릿과의 상관성?
  1. 템플릿이 발견된 문맥
  1. 템플릿이 실체화되는 문맥
  1. 실체화되는 템플릿 인자와 관련된 문맥

## 1. 이름 분류
> Name Taxonomy

C++ 에서의 이름에 대한 정리(요약). 필요하면 찾아보는 정도면 충분

| 분류 |  |
|:-----|:-----|
| Identifier | 식별자
| Operator-function-id | `new`, `operator[]`
| Conversion-function-id | `operator int()`
| Literal-operator-id | `100_km`
| Template-id | `List<T, int>`. 꺾음 괄호 `<>`로 묶인 템플릿 인자가 따라옴(후속)
| Uniqualified-id | 일반적인 식별자. Operator, Conversion, Literal, Destructor
| Qualified-id | 한정된(조건부) 식별자. `class`, `union`, `namespace`등의 이름. Global scope resolution operator(`::`) 포함. 
| Qualified name | 한정된(조건부) 이름. Qualified Lookup의 대상. Qualified ID 혹은 명시적인 멤버 접근(`.` 또는 `->`)을 사용하는 Unqualified ID
| Unqualified name | 제약이 걸리지 않은 이름. Unqualified Lookup의 대상
| Dependent name | 템플릿 인자에 (어떤 형태로든) 의존하는 이름
| Nondependent name | 의존적이지 않은 모든 이름

## 2. 이름 탐색
> Looking Up Names


**Qualified Lookup**: 지정된 조건 하에서의 탐색.
`void f(D* pd)` 함수 내에서 `i`와 `x`는 Qualified Name. 
```c++
int x;

class B {
  public:
    int i;
};

class D : public B {
};

void f(D* pd)
{
    pd->i = 3;  // B::i 를 찾아낸다. (상위 클래스는 탐색 범위에 포함됨)
    D::x = 2;   // ERROR: ::x 가 D 안에 없음 (바깥 Scope는 탐색 범위에 포함하지 않음)
}
```

**Unqualified Lookup**: 한정되지 않은(특별한 제약이 없는) 경우, 계속해서 바깥 Scope를 탐색. 
```c++
extern int count; // #1

int lookup_example(int count) // #2
{
    if(count < 0){
        int count = 1; // #3
        lookup_example(count); // 제약이 없으므로 가장 가까운 #3 선택
    }
    return count  // 가장 가까운 #2 선택
            + ::count; // 전역 한정(Qualified). #1 선택
}
```

앞서까지의 방법은 Ordinary Lookup. 템플릿을 위해 추가된 방법이 ADL.

```c++
template <typename T>
T max(T a, T b)
{
    return b < a ? a : b;
}

namespace BigMath
{
    class BigNum
    {
        // ...
    };

    bool operator < (BigNum const&, BigNum const&);
    // ...
}

using BigMath::BigNum;

void g(BigNum const& a, BigNum const& b)
{
    // ...
    BigNum x = ::max(a, b);
}
```
`max()` 템플릿 함수는 BigMath 네임스페이스(이름공간)를 볼 수 었음. 따라서 Ordinary Lookup으로는 `operator <`를 적용 불가. 이것을 가능하게 하는 **특별한 규칙**이 필요.

### 2.1 Argument-Dependent Lookup

정의: 템플릿 함수를 호출하면(소괄호가 `()` 따라오면), 인자들의 타입과 관련된 네임스페이스/타입에서 탐색을 수행한다  

조건: Ordinary Lookup이 다음 중 하나라도 찾아내면 일어나지 않음
  - 멤버 함수
  - 변수
  - 타입
  - 블록에서의 함수 선언(?)

| Type | Associated Namespace/Class |
|:-----|:-----|
| Built-In Types    |  없음
| Pointer/Array     |  기반 타입 T와 해당 타입이 포함된 네임스페이스
| Enum              |  선언된 네임스페이스
| Class Member      |  멤버가 포함된 클래스
| Class (+ Union)   |  클래스 자신(itself), 자신을 내포하는 클래스, 직/간접적 상위 클래스. 이들이 선언된 네임스페이스들
| Function          |  모든 인자와 리턴 타입의 연관된 네임스페이스/클래스
| Pointer to Class Member Variable  | Class의 연관된 네임스페이스/클래스
| Pointer to Class Member Function  | Class와 함수 인자/리턴 타입들의 연관된 네임스페이스/클래스들

예제:
```c++
#include <iostream>

namespace X
{
    template <typename T> 
    void f(T);
}

namespace N
{
    using namespace X;

    enum E
    {
        e1
    };

    void f(E){
        std::cout << "N::f(N::E) called" << std::endl;
    }
}

void f(int){
    std::cout << "::f(int) called" << std::endl;
}

int main()
{
    ::f(N::e1); // f는 Qualified name. ADL 적용하지 않음
    f(N::e1);   // Ordinary --> ::f(int)
                // ADL --> N::f(N::E)  
                // ADL 의 함수가 선택됨 (인자의 적합성)
}
```

### 2.2 Friend 선언에서의 ADL (생략)
> ADL of Friend Declarations

### 2.3 클래스 이름 주입
> Injected Class Name

**이름 주입**: 클래스를 선언하는 경우, **선언하는 도중에도** 클래스 이름에 접근이 가능. 단, Unqualified Name이어야 함

```c++
#include <iostream>

int C;

class C
{
    private:
        int i[2];

    public:
        static int f() {
            // Name injection: Size of this type
            return sizeof(C); 
        }
};

int f()
{
    // size of the variable
    return sizeof(C);
}
```

 - 템플릿에 대해서도 마찬가지로 적용
 - +) 템플릿 인자가 따라올 수 있음

```c++
template <template <typename> 
          class TT>
class X
{
};

template <typename T>
class C
{
    C* a;       // OK: C<T>*
    C<void>& b; // OK
    X<C> c:     // OK: 템플릿 클래스 X의 인자로 템플릿 C<T>를 사용
    X<::C> d;   // OK: 주입된 이름은 아니지만 템플릿 C를 사용하도록 명시
};
```

### 2.4 현재 정의되고 있는 타입
> Current Instantiations

템플릿 클래스에 대해서는, 주입된 이름이 곧 현재 정의되고 있는 타입(Current Instantiation)을 의미하게 됨. 현재 정의되고 있는 타입이 아니면서, 
템플릿 인자에 의존적인 타입을 사용해서 실체화를 시키는 경우, 이를 Unknown Instantiation(미확인 실체화?))이라고 부름.

```c++
template <typename T>
class Node
{
    using Type = T;

    Node* next;         // current instantiation
    Node<Type>* prev;   // current instantiation
    Node<T*>* parent;   // unknown instantiation
};
```

## 3. 템플릿 구문 분석
> Parsing Templates

Q. 컴파일러는 템플릿을 어떻게 '인식'하게 될까?

### 3.1 템플릿이 아닌 경우의 문맥 인식
> Context Sensitivity in Nontemplates

이런 문장(string)을 발견했다고 하면...
```
X<1>(0)
```

 - `X`가 템플릿? 생성자(혹은 타입 변환) 호출
 - `X`가 템플릿이 아닌 경우? 아래와 동일한 의미

```c
( X < 1 ) > 0
```

이게 다 `<>`(Angle Bracket)을 템플릿 인자기호로 사용했기 때문.
```c++
template <bool B>
class Invert
{
    public:
        static bool const result = !B;
};

void g()
{
    bool test = Invert<(1>0)>::result; // 소괄호 ()가 필요하다!
}
```
함수 `g`에서, `(1>0)`형태로 감싸지 않으면 템플릿 인자를 의도와는 다르게 인식하게 된다.


### 3.2 의존적인 이름: 타입
> Dependent Names of Types

 - 템플릿에서는 이름들이 명확하지 않은 경우가 있음
 - 다른 템플릿을 사용해야 하는 경우, 소스코드 상의 명시적 특수화로 인해 잘못될 수 있음

```c++
template <typename T>
class Trap{
    public:
        enum {x}; // #1 x 는 상수값
};

template <typename T>
class Victim{
    public:
        int y;
        void proof(){
            Trap<T>::x * y; // #2 타입 선언인가? 곱셈인가?
        }
};

template <>
class Trap<void>{       // 사악한 특수화
    public:
        using x = int;  // #3 x 는 여기서 타입을 의미한다
};

// 컴파일 트리거
void bool(Victim<void>& bomb)
{
    bomb.proof(); 
}
```

 - `Trap<T>::x` 가 타입인 경우? `y`는 포인터 변수가 된다.  
 - `Trap<T>::x` 가 변수인 경우? `x * y`는 곱셈이 된다.
 - 템플릿에서 사용된 표현식에서는 이런 타입/상수 모호함이 발생
 - **모호한 상황에서** 타입을 의미한다면 `typename`을 사용해 명시
    - 교재에 언어 규칙들이 나오는데.. 눈으로 보는게 더 편할것 같아서 정답으로 bypass

```c++
template <typename T> // #1 템플릿 인자에 이름을 부여한다
struct S : /* typename */ X<T>::Base  // #2 부모 타입이 오는 자리. 모호하지 않음
{
    S() : /* typename */ X<T>::Base(       // #3: #2와 동일
        typename X<T>::Base(0))  // #4 Base가 상수라면 잘못된 표현식이 됨
    {
        // ...        
    }

    // #5 X<T>가 qualified name이 아님
    /* typename */ X<T> f() {
        // #6 모호함. X<T>::C가 상수(static constexpr)일 수 있음
        typename X<T>::C * p;   // 포인터 선언

        // 명시하지 않으면 값이라고 전제한다
        X<T>::D * q;            // 곱셈
    }
    // #7 모호함. X<T>::C가 상수(static constexpr)일 수 있음
    typename X<int>::C * s;

    using Type = T;
    // #8 선택적으로 사용 가능
    using OtherType = /* typename */ S<T>::Type;
};

```

### 3.3 의존적인 이름: 템플릿
> Dependent Names of Templates

 - 템플릿의 이름이 템플릿 인자에 의존적인 경우, 앞서와 같이 모호함의 문제가 발생할 수 있음
 - 일반적인 경우, C++ 컴파일러는 템플릿 이름 뒤에 `<`이 나오면, 템플릿 인자 목록의 시작이라고 판단
 - 이외의 경우는 모두 less-than 연산자

```c++
template <typename T>
class Shell
{
}
```


### 3.4 의존적인 이름: Using 선언
> Dependent Names in Using Declarations

Using 선언은 일종의 '바로가기' 같은 기능.

```c++
class BX
{
    public:
        void f(int);
        void f(char const*);
        void g();
};

class DX : private BX
{
    public:
        using BX::f;
};
```

하지만 템플릿이라면? 템! 플! 릿!

```c++
template <typename T>
class BXT
{
    public:
        using Mystery = T;
        template <typename U>
        struct Magic;
};

template <typename T>
class DXTT : private BXT<T>
{
    public:
        using typename BXT<T>::Mystery;
        Mystery* p;
};
```

 - `DXTT`내에서 사용된 `Mystery`는 `T`에 의존적이기 떄문에, 타입/상수 모호성이 발생
 - `typename`을 명시


### 3.5 명시적 템플릿 인자와 ADL
> ADL and Explicit Template Arguments

컴파일러의 고난을 엿볼 수 있는 예제
```c++
namespace N
{
    class X
    {
        // ...
    };

    template <int I>
    void select(X*);
}


void g(N::X* xp)
{
    select<3>(xp); // ERROR: ADL이 적용되지 않는다!
}
```

 - Ordinary Lookup: select 라는 symbol을 찾지 못함
 - ADL: `select`가 템플릿 함수 인가? 

ADL 수행여부를 결정하려다 보면, 컴파일러는 아래와 같은 순환에 빠지게 됩니다.

 1. `select`가 템플릿 함수인지를 알기 위해선 `select<3>(...)` 표현식에서 사용된 `<3>`이 템플릿 인자 목록인지 알아야 한다
 1. `<3>`이 템플릿 인자 목록이라는 것을 알기 위해선 `select`가 템플릿 함수인지 알아야 한다

결국 ADL은 적용되지 않게 됩니다.

### 3.6 의존적인 표현
> Dependent Expressions

간단히 구분할 수 있도록 책의 예제를 확인해보겠습니다.

#### 타입 의존
```c++
template <typename T> 
void typeDependent(T x)
{
    x;      // T에 따라 x가 달라지므로, type-dependent expr

    f(x);   // x가 type-dependent 하므로, 이 expr도 type-dependent
}
```

#### 값 의존
```c++
template <int N, typename T>
void valueDependent(T x)
{
    N;          // Fixed type(int) 이면서 값만 달라진다.
                // type-dependent 하지 않고, value-dependent

    sizeof(x);  // sizeof(...)가 반환하는 값은 항상 같은 타입(size_t)  
                // type-dependent 하지 않고, value-dependent
}
```

#### 실체화 의존
템플릿 코드가 실체화되면서 표현식이 평가되는 경우.

#### 기타 등등
이외의 모든 표현식. 기타 둥둥


## 4. 상속과 클래스 탬플릿
> Inheritance and Class Templates

템플릿에서의 의존성은 인자에 대한 것. 클래스의 정의는 상위 클래스에 따라서 달라질 수 있는데, 그러면 클래스 템플릿에는 의존성이 어떻게 적용될까?

### 4.1 템플릿 인자와 무관한 상위 클래스
> Nondependent Base Classes

```c++
template <typename X>
class Base
{
    public:
        int basefield;
        using T = int;
};

// Base<Base<void>>는
// class Base_Base_void 같은 클래스를 상속받은 것과 같다.
// 템플릿이 아닌 경우
class D1 : public Base<Base<void>>
{
    public:
        void f(){
            basefield = 3; // (평범한) 상위 클래스 멤버에 대한 접근
        }
};

template <typename T>
class D2 
    : public Base<double> // non-dependent base. 
                          // 템플릿 인자와 무관하다 
{
    public:
        void f(){
            basefield = 7; // (평범한) 상위 클래스 멤버에 대한 접근
        }

        // 놀랍게도, Base<double>::T 를 의미한다. 
        // 즉, 템플릿 인자가 아니다!
        T strange; 
}
```

### 4.2 템플릿 인자에 따라 변화하는 상위 클래스
> Dependent Base Classes


```c++
template <typename T>
class DD 
    : public Base<T> // Dependent base. 
                     // 템플릿 인자 T에 따라 달라진다
{
public:
    void f(){
        basefield = 0; // #1 상속한 클래스의 멤버에 접근한다
    }
};

template <> // 특수화
class Base<bool>
{
    public:
        // #2 여기서는 basefield가 상수를 의미한다.
        enum { basefield = 42 }; 
};

void g(DD<bool>& d)
{
    d.f();  // #3 어라?
}
```

`g` 함수의 `d.f()`에서 `Base<bool>`이 실체화 하는데, 특수화로 인해 `basefield`가 상수가 되면서 에러를 발생시킵니다.
