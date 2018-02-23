# Chapter 6: Move Semantics and ```enable_if<>```

C++11에 도입된 특징들 중 가장 중요한 특징을 하나 꼽으라면 바로 이동 문법(Move Semantics)이라고 말할 수 있습니다.

개체를 복사하거나 할당할 때, 내부 리소스를 복사하는 대신 이동해 최적화 할 수 있습니다.

## 6.1 Perfect Forwarding

인수의 기본적인 특성을 유지한 채 전달 받는 템플릿 함수를 만들고 싶다고 가정해 봅시다.

전달 받는 인수의 종류는 크게 다음과 같습니다.

- 수정 가능한 개체 : 수정 가능한 개체로 전달 받아야 함
- 상수 개체 : 읽기만 가능한 개체로 전달 받아야 함
- 이동 가능한 개체 : 이동 가능한 개체로 전달 받아야 함

템플릿을 사용하지 않을 경우, 전달 받는 인수의 종류마다 함수를 하나씩 만들어야 합니다.

```C++
#include <utility>
#include <iostream>

class X
{
    ...
};

void g(X&)
{
    std::cout << "g() for variable\n";
}

void g(X const&)
{
    std::cout << "g() for constant\n";
}

void g(X&&)
{
    std::cout << "g() for movable object\n";
}

void f(X& val)
{
    g(val);
}

void f(X const& val)
{
    g(val);
}

void f(X&& val)
{
    g(std::move(val));
}

int main()
{
    X v;
    X const c;

    f(v);
    f(c);
    f(X());
    f(std::move(v));
}
```

이동 가능한 개체는 Rvalue 레퍼런스를 사용하는데, 인수를 전달할 때 ```std::move()```를 사용해야 합니다.

```std::move()```는 언뜻 이름만 봐서는 이동시켜 줄 것 같지만, 실제로는 값을 <b>무조건</b> Rvalue로 바꾸는 동작을 합니다.

C++ 표준 규칙에 따르면 이동 문법은 전달되지 않기 때문에 ```std::move()```를 통해 값을 Rvalue로 바꿔줘야 합니다.

만약 ```g(std::move(val));```에서 ```std::move()```를 뺀다면, ```g(&&)```이 아닌 ```g(X&)```이 호출됩니다.

위 예제의 세 함수를 하나의 템플릿 함수로 만들려고 하면, 문제가 발생하게 됩니다.

```C++
template <typename T>
void f(T val)
{
    g(T);
}
```

앞의 두 경우는 잘 동작하지만, 이동 가능한 개체를 전달 받는 세번째 경우는 동작하지 않기 때문입니다.

이러한 이유로 C++11은 <b>퍼펙트 포워딩(Perfect Forwarding)</b> 매개 변수를 위한 특별한 규칙을 도입했습니다.

```C++
template <typename T>
void f(T&& val) {
    g(std::forward<T>(val));
}
```

```std::move()```가 값을 <b>무조건</b> Rvalue로 바꾸는 동작을 했다면, ```std::forward()```는 <b>조건에 따라</b> 값을 Rvalue로 바꾸는 동작을 합니다.

알아둬야 할 중요한 사실은 템플릿 매개 변수에 사용하는 ```T&&```와 특정 타입 ```X```에 사용하는 ```X&&```이 서로 다르다는 점입니다.

- 특정 타입 ```X```에 사용하는 ```X&&```는 Rvalue 레퍼런스입니다.

    - 이동 가능한 개체(임시 개체와 같은 prvalue와 ```std::move()```로 전달된 개체인 xvalue)에만 사용 가능합니다.

- 템플릿 매개 변수에 사용하는 ```T&&```는 <b>포워딩 레퍼런스(Forwarding Reference)</b>입니다.

    - 포워딩 레퍼런스는 <b>유니버셜 레퍼런스(Universal Reference)</b>라고도 합니다.

    - 변경 가능한, 변경 불가능한(불변, 예 : ```const```), 이동 가능한 개체에 모두 사용 가능합니다.

    - 여기서 ```T```는 템플릿 매개 변수의 이름이어야 합니다.

참고로 템플릿 매개 변수에 의존하는 타입, 예를 들어 ```typename T::iterator&&```는 포워딩 레퍼런스가 아닌 Rvalue 레퍼런스입니다.

예제에 퍼펙트 포워딩을 적용한 코드는 다음과 같습니다.

```C++
#include <utility>
#include <iostream>

class X
{
    ...
};

void g(X&)
{
    std::cout << "g() for variable\n";
}

void g(X const&)
{
    std::cout << "g() for constant\n";
}

void g(X&&)
{
    std::cout << "g() for movable object\n";
}

template <typename T>
void f(T&& val)
{
    g(std::forward<T>(val));
}

int main()
{
    X v;
    X const c;

    f(v);
    f(c);
    f(X());
    f(std::move(v));
}
```

퍼펙트 포워딩은 가변 템플릿과 함께 사용할 수 있습니다.

## 6.2 Special Member Function Template

생성자와 같은 특별한 멤버 함수에도 멤버 함수 템플릿을 사용할 수 있습니다. 하지만 예상한 것과 다른 동작을 합니다.

예를 들어, 다음과 같은 코드가 있다고 합시다.

```C++
#include <utility>
#include <string>
#include <iostream>

class Person
{
public:
    explicit Person(std::string const& n) : name(n)
    {
        std::cout << "copying string-CONSTR for '" << name << "'\n";
    }

    explicit Person(std::string&& n) : name(std::move(n))
    {
        std::cout << "moving string-CONSTR for '" << name << "'\n";
    }

    Person(Person const& p) : name(p.name)
    {
        std::cout << "COPY-CONSTR Person '" << name << "'\n";
    }

    Person(Person&& p) : name(std::move(p.name))
    {
        std::cout << "MOVE-CONSTR Person '" << name << "'\n";
    }

private:
    std::string name;
};
```

위 코드는 예상한 대로 동작합니다.

```C++
std::string s = "sname";

Person p1(s);
Person p2("tmp");
Person p3(p1);
Person p4(std::move(p1));
```

이제 문자열을 받는 두 생성자를 퍼펙트 포워딩을 적용한 하나의 템플릿 생성자로 만들어 보겠습니다.

```C++
#include <utility>
#include <string>
#include <iostream>

class Person
{
public:
    template <typename STR>
    explicit Person(STR&& n) : name(std::forward<STR>(n))
    {
        std::cout << "TMPL-CONSTR for '" << name << "'\n";
    }

    Person(Person const& p) : name(p.name)
    {
        std::cout << "COPY-CONSTR Person '" << name << "'\n";
    }

    Person(Person&& p) : name(std::move(p.name))
    {
        std::cout << "MOVE-CONSTR Person '" << name << "'\n";
    }

private:
    std::string name;
};
```

이제 정상적으로 동작하는지 확인해 봅시다. 문자열을 인수로 받는 생성자는 예상한 대로 동작합니다.

```C++
std::string s = "sname";

Person p1(s);
Person p2("tmp");
```

하지만 복사 생성자를 호출하면 오류가 발생합니다.

```C++
Person p3(p1);
```

반면 이동 가능한 개체로 새 Person 개체를 생성하는 코드는 잘 동작합니다.

```C++
Person p4(std::move(p1));
```

상수 ```Person```을 복사해 생성하는 코드 또한 잘 동작합니다.

```C++
Person const p2c("ctmp");
Person p3c(p2c);
```

오류가 발생하게 된 원인은 무엇일까요? 바로 C++의 오버로드 해결 규칙 때문입니다.

상수가 아닌 Lvalue ```Person p```의 경우, (일반적으로 미리 정의된) 복사 생성자

```C++
Person(Person const& p)
```

보다 멤버 템플릿 생성자

```C++
template <typename STR>
Person(STR&& n)
```

이 더 잘 일치합니다.

```STR```은 ```Person&```으로 치환되는데, 복사 생성자는 ```const```로 변환해야 하기 때문입니다.

이 문제를 해결하기 위해 상수가 아닌 Lvalue를 받는 복사 생성자를 제공하는 방법을 생각해 볼 수 있습니다.

```Person(Person& p)```

하지만 이 방법은 완벽한 해결책이 아닙니다.

우리가 정말로 원하는 건 ```Person``` 타입을 인수로 전달하거나 표현식이 ```Person```으로 변환될 수 있는 경우를 막는 것입니다.

```std::enable_if<>```를 사용하면 이 문제를 해결할 수 있습니다.

## 6.3 Disable Templates with ```enable_if<>```

C++11의 표준 라이브러리는 컴파일 타임의 특정 조건에서 함수 템플릿을 무시하는 헬퍼 템플릿 ```std::enable_if<>```를 제공합니다.

예를 들어, 함수 템플릿 ```foo<>()```이 있다고 합시다.

```C++
template <typename T>
typename std::enable_if<(sizeof(T) > 4)>::type
foo() 
{

}

만약 ```sizeof(T) > 4```가 ```false```라면, ```foo<>()```의 정의는 무시됩니다.

만약 ```sizeof(T) > 4```가 ```true```라면, 함수 템플릿 인스턴스는 다음과 같이 확장합니다.

```C++
void foo()
{

}
```

즉, ```std::enable_if<>```는 첫번째 템플릿 인수로 주어진 컴파일 타임 표현식을 계산하는 타입 특성입니다.

이 때 표현식의 결과에 따라 다음과 같이 동작합니다.

- 표현식의 결과가 ```true```라면, ```std::enable_if<>```의 멤버 ```type```은
    - 두번째 템플릿 인수를 전달하지 않았다면, ```void``` 타입이 됩니다.
    - 전달했다면, 두번째 템플릿 인수 타입이 됩니다.
- 표현식의 결과가 ```false```라면, ```std::enable_if<>```의 멤버 ```type```은 정의되지 않습니다.
    - <b>SFINAE(Substitution Failure Is Not An Error)</b>라는 템플릿 특성 때문에 무시됩니다.

C++14부터는 별칭 템플릿 ```std::enable_if_t<>```이 추가되어 ```typename```과 ```::type```을 생략할 수 있습니다.

```C++
template <typename T>
std::enable_if_t<(sizeof(T) > 4)>
foo()
{

}
```

```enable_if<>```나 ```enable_if_t<>```에 두번째 인자를 전달할 수 있습니다.

```C++
template <typename T>
std::enable_if_t<(sizeof(T) > 4)>
foo()
{
    return T();
}
```

만약 ```size(T) > 4```가 ```true```이고 ```T```가 ```MyType()```이었다면, 위 함수 템플릿은

```C++
MyType foo();
```

로 인스턴스화됩니다.

```enable_if``` 표현식이 선언 중간에 있어서 보기에 껄끄럽다면, 디폴트 값을 갖는 함수 템플릿 인수를 사용하면 됩니다.

```C++
template <typename T,
          typename = std::enable_if_t<(sizeof(T) > 4)>>
void foo()
{

}
```

```sizeof(T) > 4```라면, 위 함수 템플릿은

```C++
template <typename T,
          typename = void>
void foo()
{

}
```

로 확장합니다.

여전히 껄끄럽다면, 별칭 템플릿을 사용해 요구 사항이나 제약 사항을 더욱 명시적으로 표현할 수 있습니다.

```C++
template <typename T>
using EnableIfSizeGreater4 = std::enable_if_t<(sizeof(T) > 4)>;

template <typename T,
          typename = EnableIfSizeGreater4<T>>
void foo()
{

}
```

## 6.4 Using ```enable_if<>```

6.2절에서 ```std::enable_if<>```를 사용해 문제를 해결할 수 있다고 말했었습니다.

전달된 인수 ```STR```이 올바른 타입(```std::string```이거나 ```std::string``` 타입으로 변환됨)을 갖지 않을 경우 템플릿 생성자의 선언을 막아야 합니다.

이를 위해, 다른 표준 타입 특성인 ```std::is_convertible<FROM, TO>```를 사용합니다.

C++17이라면 다음과 같이 선언할 수 있습니다.

```C++
template <typename STR,
          typename = std::enable_if_t<std::is_convertible_v<STR, std::string>>>
Person(STR&& n);
```

만약 ```STR``` 타입을 ```std::string``` 타입으로 변환할 수 있다면, 함수 템플릿은

```C++
template <typename STR,
          typename = void>
Person(STR&& n);
```

으로 확장합니다.

만약 ```STR``` 타입을 ```std::string``` 타입으로 변화할 수 없다면, 함수 템플릿은 무시됩니다.

별칭 템플릿을 사용해 제약 사항을 더욱 명시적으로 표현할 수 있습니다.

```C++
template <typename T>
using EnableIfString = std::enable_if_t<std::is_convertible_v<T, std::string>>;
...
template <typename STR, typename = EnableIfString<STR>>
Person(STR&& n);
```

이를 적용한 ```Person``` 클래스 코드는 다음과 같습니다.

```C++
#include <utility>
#include <string>
#include <iostream>
#include <type_traits>

template <typename T>
using EnableIfString = std::enable_if_t<std::is_convertible_v<T, std::string>>;

class Person
{
public:
    template <typename STR, typename = EnableIfString<STR>>
    explicit Person(STR&& n) : name(std::forward<STR>(n))
    {
        std::cout << "TMPL-CONSTR for '" << name << "'\n";
    }

    Person(Person const& p) : name(p.name)
    {
        std::cout << "COPY-CONSTR Person '" << name << "'\n";
    }

    Person(Person&& p) : name(std::move(p.name))
    {
        std::cout << "MOVE-CONSTR Person '" << name << "'\n";
    }

private:
    std::string name;
};
```

이제 모든 코드가 예상한 대로 동작합니다.

```C++
std::string s = "sname";

Person p1(s);
Person p2("tmp");
Person p3(p1);
Person p4(std::move(p1));
```

C++14라면 값을 나타내는 타입 특성인 ```_v```가 정의되어 있지 않기 때문에 별칭 템플릿을 선언해야 합니다.

```C++
template <typename T>
using EnableIfString = std::enable_if_t<std::is_convertible<T, std::string>::value>;
```

C++11이라면 타입을 나타내는 타입 특성인 ```_t```가 정의되어 있지 않기 떄문에 특별한 멤버 템플릿을 선언해야 합니다.

```C++
template <typename T>
using EnableIfString = std::enable_if<std::is_convertible<T, std::string>::value>::type;
```

참고로 ```std::is_convertible<>```은 타입이 암시적으로 변환 가능해야 한다는 조건이 있습니다.

대체 가능한 타입 특성으로 ```std::is_constructible<>```이 있습니다. 이 타입 특성은 초기화에 사용하는 명시적 변환도 허용합니다.

두 타입 특성의 차이가 있다면, 인수의 순서가 서로 반대라는 점입니다.

```C++
template <typename T>
using EnableIfString = std::enable_if_t<std::is_constructible_v<std::string, T>>;
```

#### 특별한 멤버 함수를 사용하지 못하게 만들기

```enable_if<>```를 사용해 미리 정의된 복사/이동 생성자, 그리고 복사/이동 할당 연산자를 사용하지 못하게 만들 수 없습니다.

왜냐하면 멤버 함수 템플릿을 특별한 멤버 함수로 간주하지 않기 때문입니다. 따라서 복사 생성자가 필요한 경우 무시됩니다.

다음과 같은 코드가 있다고 합시다.

```C++
class C
{
public:
    template <typename T>
    C(T const&)
    {
        std::cout << "tmpl copy constructor\n";
    }
    ...
};
```

C의 복사본이 필요할 경우, 여전히 미리 정의된 복사 생성자를 사용합니다.

(멤버 템플릿의 템플릿 매개 변수 ```T```를 지정하거나 추론할 방법이 없기 때문에 사용할 수 없습니다.)

```C++
C x;
C y{x};
```

미리 정의된 복사 생성자를 삭제하는 방법은 해결책이 되지 못합니다. 그렇다면 어떻게 사용하지 못하게 만들 수 있을까요?

```const volatile```을 인수로 받는 복사 생성자를 선언하고 삭제하면 됩니다.

이렇게 하면 암시적으로 선언되는 또 다른 복사 생성자를 막을 수 있습니다.

이제 여기에 ```volatile```이 아닌 타입을 받는 (삭제된) 복사 생성자보다 일치하는 생성자 템플릿을 정의할 수 있습니다.

```C++
class C
{
public:
    C(C const volatile&) = delete;

    template <typename T>
    C(T const&)
    {
        std::cout << "tmpl copy constructor\n";
    }
    ...
};
```

이제 "일반적인" 복사를 할 때도 템플릿 생성자를 사용합니다.

```C++
C x;
C y{x};
```

여기에 ```enable_if<>```를 사용해 제약 사항을 추가할 수 있습니다.

예를 들어, 템플릿 매개 변수가 정수 타입일 경우 복사 생성할 수 없도록 만들 수 있습니다.

```C++
template <typename T>
class C
{
public:
    ...
    C(C const volatile&) = delete;

    template <typename U,
              typename = std::enable_if_t<!std::is_integral<U>::value>>
    C(C<U> const&)
    {
        ...
    }
    ...
};
```

## 6.5 Using Concepts to Simplify ```enable_if<>``` Expressions

별칭 템플릿을 사용하더라도 ```enable_if``` 문법은 꽤 껄끄럽습니다.

함수의 요구 사항이나 제약 사항에 따라 확장하거나 무시하는 언어 기능이 있다면 좋겠습니다.

C++17 표준에는 들어가지 않았지만, 곧 들어갈 언어 기능인 <b>컨셉(Concept)</b>이 있다면 가능합니다.

컨셉이 있다면, ```enable_if```를 사용하지 않고 다음과 같이 간단하게 작성할 수 있을 것입니다.

```C++
template <typename STR>
requires std::is_convertible_v<STR, std::string>
Person(STR&& n) : name(std::forward<STR>(n))
{
    ...
}
```

요구 사항을 일반적인 컨셉으로 지정할 수도 있습니다.

```C++
template <typename T>
concept ConvertibleToString = std::is_convertible_v<T, std::string>;
```

이제 이 컨셉을 요구 사항으로 표현해 봅시다.

```C++
template <typename STR>
requires ConvertibleToString<STR>
Person(STR&& n) : name(std::forward<STR>(n))
{
    ...
}
```

다음처럼 표현할 수도 있습니다.

```C++
template <ConvertibleToString STR>
Person(STR&& n) : name(std::forward<STR>(n))
{
    ...
}
```

# Chapter 7: By Value or by Reference?

C++의 함수 호출 방식에는 크게 두 가지가 있습니다. 바로 <b>값에 의한 호출(call-by-value)</b>과 <b>레퍼런스에 의한 호출(call-by-reference)</b>입니다.

일반적으로 레퍼런스에 의한 호출 방식은 비용이 저렴하지만 더 복잡합니다. C++11에 이동 문법이 추가되면서 레퍼런스에 의한 호출 방식이 다양해졌습니다.

- ```X const&``` (상수 Lvalue 레퍼런스) : 매개 변수는 전달된 개체를 나타냅니다. 개체를 수정할 수 없습니다.

- ```X&``` (상수가 아닌 Lvalue 레퍼런스) : 매개 변수는 전달된 개체를 나타냅니다. 개체를 수정할 수 있습니다. 

- ```X&&``` (Rvalue 레퍼런스) : 매개 변수는 전달된 개체를 나타냅니다. 이동 문법으로 전달되었으므로, 값을 수정하거나 "훔칠" 수 있습니다.

## 7.1 Passing by Value

인수로 값을 전달하는 간단한 함수 템플릿을 살펴 봅시다.

```C++
template <typename T>
void printV(T arg)
{
    ...
}
```

```int``` 타입의 인수로 함수 템플릿을 호출하면,

```C++
void printV(int arg)
{
    ...
}
```

으로 확장합니다.

매개 변수 ```arg```는 전달된 인수의 종류(개체, 리터럴, 함수에 의해 반환된 값)에 상관없이 복사본을 갖습니다.

만약 ```std::string``` 타입의 변수를 정의하고 이 변수를 인수로 사용해 함수 템플릿을 호출한다면,

```C++
std::string s = "hi";
printV(s);
```

템플릿 매개 변수 ```T```는 ```std::string```으로 인스턴스화되어,

```C++
void printV(std::string arg)
{
    ...
}
```

으로 확장합니다.

이 때 ```std::string``` 클래스의 복사 생성자를 통해 문자열의 복사본을 만들게 되는데, 비용이 꽤 비쌉니다.

복사본을 만드는 이유는 원칙적으로 복사 동작은 <b>깊은 복사(Deep Copy)</b>를 하기 때문입니다. 따라서 내부적으로 값을 저장하는 메모리를 할당하게 됩니다.

하지만 복사 생성자가 항상 호출되지는 않습니다. 다음 코드를 보도록 합시다.

```C++
std::string returnString();
std::string s = "hi";
printV(s);
printV(std::string("hi"));
printV(returnString());
printV(std::move(s));
```

- ```printV(s)``` : Lvalue, 복사 생성자가 호출됩니다.
- ```printV(std::string("hi"))```, ```printV(returnString())``` : PRvalue, 컴파일러가 인수 전달을 최적화하므로 복사 생성자가 호출되지 않습니다.
    - C++17부터는 이 최적화를 적용합니다.
    - C++14까지는 복사를 최적화하지는 않지만 복사 비용이 저렴한 이동 문법을 사용하려고 시도합니다.
- ```printV(std::move(s))``` : Xvalue, 이동 생성자가 호출됩니다.

따라서 인수로 값을 전달해 ```printV()``` 함수를 호출하는 동작은 Lvalue를 전달할 때만 비용이 비싸다고 할 수 있습니다.

#### 인수로 값을 전달하면 타입이 붕괴됩니다

인수로 값을 전달하는 동작은 타입이 붕괴된다는 특징이 있습니다.

즉, 배열은 포인터로 변환되며 ```const```나 ```volatile```과 같은 한정자는 삭제됩니다.

```C++
template <typename T>
void printV(T arg)
{
    ...
}

std::string const c = "hi";
printV(c);

printV("hi");

int arr[4];
printV(arr);
```

문자열 리터럴 ```"hi"```를 전달하면 ```char const[3]``` 타입이 ```char const*```로 붕괴되어 ```T```의 추론된 타입이 됩니다.

따라서, 템플릿은

```C++
void printV(char const* arg)
{
    ...
}
```

로 인스턴스화됩니다.

## 7.2 Passing by Reference

### 7.2.1 Passing by Constant Reference

임시가 아닌 개체를 전달할 때 (불필요한) 복사를 피하고 싶다면 상수 레퍼런스를 사용하면 됩니다.

```C++
template <typename T>
void printR(T const& arg)
{
    ...
}
```

위 함수의 인수로 개체를 전달하면, 복사본을 절대로 생성하지 않습니다.

```C++
std::string returnString();
std::string s = "hi";
printR(s);
printR(std::string("hi"));
printR(returnString());
printR(std::move(s));
```

```int```를 레퍼런스로 전달해도 결과는 마찬가지입니다.

```C++
int i = 42;
printR(i);
```

따라서, 템플릿은

```C++
void printR(int const& arg)
{
    ...
}
```

으로 인스턴스화됩니다.

#### 인수로 레퍼런스를 전달하면 타입이 붕괴되지 않습니다

인수로 레퍼런스를 전달하는 동작은 타입이 붕괴되지 않는다는 특징이 있습니다.

즉, 배열은 포인터로 변환되지 않으며 ```const```나 ```volatile```과 같은 한정자도 삭제되지 않습니다.

그러나 ```call``` 매개 변수가 ```T const&```로 선언되었기 때문에 템플릿 매개 변수 ```T```는 ```const```로 추론되지 않습니다.

예를 들어,

```C++
template <typename T>
void printR(T const& arg)
{
    ...
}

std::string const c = "hi";
printR(c);

printR("hi");

int arr[4];
printR(arr);
```

따라서 ```printR()```에서 타입 ```T```로 선언된 지역 개체는 상수가 아닙니다.

### 7.2.2 Passing by Nonconstant Reference

전달한 인수를 통해 값을 반환하고 싶다면, 상수가 아닌 레퍼런스를 사용해야 합니다.

```C++
template <typename T>
void outR(T& arg)
{
    ...
}
```

인수로 임시 개체(를 전달하거나 존재하는 개체를 ```std::move()```로 변환해 전달할 경우 ```outR()```을 호출할 수 없습니다.

```C++
std::string returnString();
std::string s = "hi";
outR(s);
outR(std::string("hi"));
outR(returnString());
outR(std::move(s));
```

상수가 아닌 타입으로 된 배열도 전달할 수 있으며, 타입이 붕괴되지 않습니다.

```C++
int arr[4];
outR(arr);
```

하지만 조금 까다로운 부분이 있습니다. ```const``` 인수를 전달하면 ```arg```의 타입을 상수 레퍼런스라고 추론합니다.

이는 Lvalue를 전달할 거라고 예상하던 인수에 Rvalue를 전달해도 된다는 말입니다.

```C++
std::string const c = "hi";
outR(c);
outR(returnConstString());
outR(std::mvoe(c));
outR("hi");
```

물론 함수 템플릿 내부에서 전달된 인자를 수정하려고 하면 오류가 발생합니다.

호출 표현식 자체에서 ```const``` 개체를 전달할 수 있는 있지만, 함수가 전부 인스턴스화되었을 때 값을 수정하려고 하면 오류가 발생할 것입니다.

상수 개체를 상수가 아닌 레퍼런스로 전달하는 경우를 막고 싶다면, 다음과 같이 하면 됩니다.

- ```static_assert```를 사용해 컴파일 오류를 발생시키는 방법

```C++
template <typename T>
void outR(T& arg)
{
    static_assert(!std::is_const<T>::value, "out parameter of foo<T>(T&) is const");
    ...
}
```

- ```std::enable_if<>```를 사용해 ```const``` 개체를 전달할 경우 템플릿을 사용하지 못하게 만드는 방법

```C++
template <typename T,
          typename = std::enable_if_t<!std::is_const<T>::value>
void outR(T& arg)
{
    ...
}
```

- (두번째 방법의 대안) 컨셉을 지원할 경우 사용할 수 있는 방법
    
```C++
    template <typename T>
    requires !std::is_const_v<T>
    void outR(T& arg)
    {
        ...
    }
```

### 7.2.3 Passing by Forwarding Reference

레퍼런스에 의한 호출을 사용하는 한 가지 이유는 매개 변수를 퍼펙트 포워드 할 수 있기 때문입니다.

포워딩 레퍼런스를 사용할 때 기억해야 될 점은 템플릿 매개 변수의 Rvalue 레퍼런스처럼 보이지만 특별한 규칙을 적용한다는 점입니다.

```C++
template <typename T>
void passR(T&& arg)
{
    ...
}
```

이제 모든 것들을 포워딩 레퍼런스에 전달할 수 있습니다. 일반적으로 레퍼런스를 전달하면 복사본이 만들어지지 않습니다.

```C++
std::string s = "hi";
passR(s);
passR(std::string("hi"))
passR(returnString());
passR(std::move(s));
```

타입 추론의 특별한 규칙 때문에 어떨 때는 놀라운 결과가 나오기도 합니다.

```C++
std::string const c = "hi";
passR(c);
passR("hi");
int arr[4];
paassR(arr);
```

하지만 퍼펙트 포워딩이라는 이름처럼 "완벽"하지는 않습니다. 거의 완벽하긴 하죠.

템플릿 매개 변수 ```T```가 암시적으로 레퍼런스 타입이 될 수 있는 경우가 있는데, 이 때 문제가 발생할 가능성이 있습니다.

```C++
template <typename T>
void passR(T&& arg)
{
    T x;
    ...
}

foo(42);
int i;
foo(i);
```

## 7.3 Using ```std::ref()``` and ```std::cref()```

C++11부터는 호출자가 함수 템플릿의 인자를 값으로 전달할 것인지 레퍼런스로 전달할 것인지를 결정할 수 있습니다.

인수를 값으로 받는다고 선언한 템플릿이 있다고 가정합시다.

이 때 호출자는 ```<functional>``` 헤더 파일에 있는 ```std::cref()```와 ```std::ref()```를 통해 인수를 레퍼런스로 전달할 수 있습니다.

```C++
template <typename T>
void printT(T arg)
{
    ...
}

std::string s = "hello";
printT(s);
printT(std::cref(s));
```

```std::cref()```는 템플릿에서 매개 변수의 처리 방식을 바꾸지 않습니다.

다만 전달한 인수 ```s``` 개체가 레퍼런스처럼 행동하도록 래핑하는 역할을 합니다.

사실, ```std::cref()```는 기존 인수를 참조하는 ```std::reference_wrapper<>``` 타입의 개체를 만들어 이 개체를 값으로 전달합니다.

이 래퍼는 오직 한 가지 동작만 지원합니다. 바로 기존 개체를 넘겨주기 위해 래핑하기 전의 타입으로 되돌아가는 암시적 타입 변환 동작입니다.

그래서 전달한 개체를 받을 수 있는 연산자를 갖고 있다면, 레퍼런스 래퍼를 대신 사용할 수 있습니다. 예를 들어,

```C++
#include <functional>
#include <string>
#include <iostream>

void printString(std::string const& s)
{
    std::cout << s << '\n';
}

template <typename T>
void printT(T arg)
{
    printString(arg);
}

int main()
{
    std::string s = "hello";
    printT(s);
    printT(std::cref(s));
}
```

```printT(std::cref(s))```는 ```std::reference_wrapper<string const>``` 타입의 개체를 값으로 전달합니다.

매개 변수 ```arg```는 ```printString``` 함수에 전달받은 값을 전달하고 래핑하기 전의 타입인 ```std::string```으로 변환합니다.

유의할 점은 컴파일러가 래핑하기 전의 타입으로 되돌아가는 암시적 타입 변환이 필요하다는 사실을 알고 있어야 한다는 점입니다.

이러한 이유로 ```std::ref()```와 ```std::cref()```는 개체를 제네릭 코드를 통해 전달할 때만 잘 동작합니다.

```C++
template <typename T>
void printV(T arg)
{
    std::cout << arg << '\n';
}
...
std::string s = "hello";
printV(s);
printV(std::cref(s));
```

```C++
template <typename T1, typename T2>
bool isless(T1 arg1, T2 arg2)
{
    return arg1 < arg2;
}
...
std::string s = "hello";
if (isless(std::cref(s) < "world")) ...
if (isless(std::cref(s) < std::string("world"))) ...
```

## 7.4 Dealing with String Literals and Raw Arrays

우리는 앞부분에서 문자열 리터럴과 배열을 사용할 때 템플릿 매개 변수에서 서로 다른 결과가 나온다는 점을 살펴봤었습니다.

- 값에 의한 호출은 요소 타입을 가리키는 포인터로 붕괴시킵니다.

- 레퍼런스에 의한 호출은 붕괴시키지 않기 때문에 인수는 배열을 가리키는 레퍼런스가 됩니다.

두 호출은 각자 장단점이 있습니다.

만약 배열이 포인터로 붕괴된다면, 실제로 포인터인지 전달된 배열이 포인터로 붕괴되었는지 구별할 수 없다는 문제가 있습니다.

반면 문자열 리터럴을 전달했을 때 붕괴되지 않는다면 서로 다른 크기를 갖는 문자열 리터럴이 서로 다른 타입을 갖기 때문에 문제가 될 수 있습니다.

```C++
template <typename T>
void foo(T const& arg1, T const& arg2)
{
    ...
}

foo("hi", "guy");
```

여기서 ```foo("hi", "guy")```로 인해 컴파일을 실패합니다.

왜냐하면 ```"hi"```의 타입은 ```char const[3]```이고, ```"guy"```의 타입은 ```char const[4]```인데 템플릿은 같은 타입 ```T``를 요구하기 때문입니다.

두 문자열 리터럴의 크기가 같을 경우에만 컴파일이 됩니다. 따라서 테스트 케이스를 작성할 때 길이가 서로 다른 문자열 리터럴을 사용하길 권합니다.

레퍼런스에 의한 호출을 값에 의한 호출로 바꾸면 컴파일이 됩니다.

```C++
template <typename T>
void foo(T arg1, T arg2)
{
    ...
}

foo("hi", "guy");
```

그러나 이렇게 한다고 모든 문제가 해결된 것은 아닙니다. 설상가상으로 컴파일 타임의 문제가 런타임의 문제로 바뀝니다.

다음 코드를 보면, ```operator==```를 사용해 전달한 인수들을 비교합니다. 하지만 문자열의 내용이 아닌 주소를 비교한다는 문제가 있습니다.

```C++
template <typename T>
void foo(T arg1, T arg2)
{
    if (arg1 == arg2)
    {
        ...
    }
}

foo("hi", "guy");
```

### 7.4.1 Special Implementations for String Literals and Raw Arrays

전달된 인수가 포인터인지 배열인지에 따라 서로 다른 코드를 사용하도록 만들어야 할 때가 있습니다.

물론, 전달받은 배열의 타입이 아직 붕괴되지 않은 상태여야 합니다.

두 경우를 구분하려면 전달된 인수가 배열인지 확인해야 합니다. 여기에는 두 가지 방법이 있습니다.

- 배열일 때만 유효한 템플릿 매개 변수를 선언하는 방법

```C++
template <typename T, std::size_t L1, std::size_t L2>
void foo(T(&arg1)[L1], T(&arg2)[L2])
{
    T* pa = arg1;
    T* pb = arg2;
    if (compareArrays(pa, L1, pb, L2))
    {
        ...
    }
}
```

여기서 ```arg1```과 ```arg2```은 같은 타입 ```T```를 갖지만 서로 다른 크기 ```L1```과 ```L2```를 갖는 배열입니다.

그러나 다양한 형태의 배열을 지원해야 하기 때문에 여러번 구현해야 할 수도 있습니다.

- 배열을 전달했는지 확인 가능한 타입 특성을 사용하는 방법

```C++
template <typename T,
          typename = std::enable_if_t<std::is_array_v<T>>
void foo(T&& arg1, T&& arg2)
{
    ...
}
```

이렇게 별도로 처리해야 하는 이유 때문에 다른 함수 이름을 사용해 배열만 따로 처리하는게 좋습니다.

이보다는 ```std::vector```나 ```std::array```를 사용하는게 더 좋습니다.

## 7.5 Dealing with Return Values

반환 값도 값을 반환할지 레퍼런스를 반환할지 선택할 수 있습니다.

그러나 레퍼런스를 반환하면 통제불능 상태가 되어 골치 아픈 문제가 생길 수 있습니다.

일반적으로 C++ 프로그래밍에서 레퍼런스를 반환하는 경우는 다음과 같습니다.

- 컨테이너나 문자열의 요소를 반환하는 경우 (예 : ```operator[]``` 또는 ```front()```)

- 클래스 멤버에 쓰기 접근 권한을 부여하는 경우

- 연속해서 호출하기 위해 개체를 반환하는 경우 (예 : 스트림의 ```operator<<```나 ```operator>>```, 클래스 개체의 ```operator=```)

상수 레퍼런스를 반환해 클래스 멤버에 읽기 접근 권한을 부여하는 경우도 많습니다.

위에서 언급한 모든 경우에서 부적절하게 사용할 경우 문제가 발생할 수 있습니다.

```C++
std::string* s = new std::string("whatever");
auto& c = (*s)[0];
delete s;
std::cout << c;
```

```C++
auto s = std::make_shared<std::string>("whatever");
auto& c = (*s)[0];
s.reset();
std::cout << c;
```

따라서 함수 템플릿이 결과를 값으로 반환하는지 확인해야 합니다.

하지만 템플릿 매개 변수 ```T```가 항상 레퍼런스라고 보장할 수 없습니다.

왜냐하면 레퍼런스로 암시적 추론되는 경우가 있기 때문입니다.

```C++
template <typename T>
T retR(T&& p)
{
    return T{...};
}
```

```T```가 값에 의한 호출로부터 추론된 템플릿 매개 변수라고 하더라도 문제는 여전히 남아 있습니다.

템플릿 매개 변수를 레퍼런스로 명시적으로 지정할 경우 레퍼런스 타입이 됩니다.

```C++
template <typename T>
T retV(T p)
{
    return T{...};
}

int x;
retV<int&>(x);
```

이 문제를 해결하는 방법은 두 가지가 있습니다.

- 타입 특성 ```std::remove_reference<>```를 사용해 ```T```를 레퍼런스가 아닌 타입으로 변환하는 방법

```C++
template <typename T>
typename std::remove_reference<T>::type retV(T p)
{
    return T{...};
}
```

```std::decay<>```와 같은 타입 특성도 암시적으로 레퍼런스를 제거하므로 유용합니다.

- (C++14부터) 리턴 타입을 ```auto```로 선언해 컴파일러가 리턴 타입을 추론하도록 만드는 방법

```C++
template <typename T>
auto retV(T p)
{
    return T{...};
}
```

```auto```는 항상 붕괴합니다.

## 7.6 Recommended Template Parameter Declarations

- 인수를 값으로 전달하는 방식

간단한 방식이며 문자열 리터럴과 배열의 타입을 붕괴시키지만, 크기가 큰 개체를 전달하는 경우에는 최적의 성능을 제공하지 않습니다.

호출자는 ```std::cref()```와 ```std::ref()```를 사용해 레퍼런스로 전달할지 결정할 수 있습니다.

하지만 레퍼런스로 전달했을 때 유효한 동작을 하는지 반드시 확인해야 합니다.

- 인수를 레퍼런스로 전달하는 방식

크기가 큰 개체를 전달하는 경우 더 나은 성능을 제공하는 방식입니다. 특히 다음을 전달할 때 더욱 그렇습니다.

- 존재하는 개체(Lvalue)를 Lvalue 레퍼런스로 전달할 경우

- 임시 개체(PRvalue)나 이동 가능하다고 표시한 개체(Xvalue)를 Rvalue 레퍼런스로 전달할 경우

- 앞의 두 개체를 포워딩 레퍼런스로 전달할 경우

위의 모든 경우에서 인수를 붕괴시키지 않으며 문자열 리터럴과 배열을 전달할 때 별도로 고려해야 합니다.

포워딩 레퍼런스의 경우 템플릿 매개 변수가 암시적으로 레퍼런스 타입으로 추론될 수 있다는 점을 인지하고 있어야 합니다.

#### 일반적인 권장 사항

1. 기본적으로 인수를 <b>값으로 전달하는 방식</b>을 사용하세요. 간단하며 문자열 리터럴을 전달해도 대부분 잘 동작합니다. 크기가 작은 개체, 임시 개체 또는 이동 가능한 개체를 전달하는 경우 괜찮은 성능을 가집니다. 호출자는 존재하는 크기가 큰 개체를 전달할 때 비용이 비싼 복사를 피하기 위해 ```std::ref()```와 ```std::cref()```를 사용할 수 있습니다.

2. 만약 괜찮은 이유가 있다면, <b>다른 선택지를 고르세요</b>.

    - 만약 새 개체를 반환하거나 인수를 수정할 수 있는 <b>out</b> 또는 <b>inout</b> 매개 변수가 필요하다면, (포인터를 통해 전달하는 방식을 선호하지 않는 경우) 상수가 아닌 레퍼런스를 전달하세요. 그러나 뜻하지 않게 ```const``` 개체를 받는 경우를 사용하지 못하게 할 지를 고려해야 합니다.

    - 템플릿이 인수를 <b>포워드</b>한다면, 퍼펙트 포워딩을 사용하세요. 즉, 퍼펙트 포워딩하도록 매개 변수를 선언하고 적절한 곳에 ```std::forward<>()```를 사용하세요. 서로 다른 문자열 리터럴과 배열 타입을 "조화롭게"하는 ```std::decay<>```와 ```std::common_type<>```의 사용을 고려해야 합니다.

    - <b>성능</b>이 제일 중요하며 인수를 복사하는데 드는 비용이 비싸다고 생각한다면 상수 레퍼런스를 사용하세요. 물론, 지역 복사본이 필요할 경우 적용해서는 안됩니다.

3. 만약 여러분이 더 잘 알고 있다면, 권장 사항을 따르지 마세요. 그러나 성능에 대해 직관적으로 가정하지 마세요. 전문가라도 실패할 때가 있습니다. 대신, 측정하세요!

#### 지나치게 일반화하지 마세요

실제로 함수 템플릿은 임의의 인수 타입을 갖지 않습니다. 대신, 일부 제약 사항을 적용합니다.

예를 들어, 일부 타입의 벡터만 전달된다는 사실을 알고 있다고 합시다. 이 경우, 함수를 너무 일반화해서 선언하지 않는게 좋습니다.

왜냐하면 예상하지 못한 사이드 이펙트가 발생할 수 있기 때문입니다. 대신, 다음 선언을 사용하세요.

```C++
template <typename T>
void printVector(std::vector<T> const& v)
{
    ...
}
```

### ```std::make_pair()``` 예제

- C++98

```C++
template <typename T1, typename T2>
pair<T1, T2> make_pair(T1 const& a, T2 const& b)
{
    return pair<T1, T2>(a, b);
}
```

- C++03

```C++
template <typename T1, typename T2>
pair<T1, T2> make_pair(T1 a, T2 b)
{
    return pair<T1, T2>(a, b);
}
```

- C++11

```C++
template <typename T1, typename T2>
constexpr pair<typename decay<T1>::type, typename decay<T2>::type>
make_pair(T1&& a, T2&& b)
{
    return pair<typename decay<T1>::type, typename decay<T2>::type>(forward<T1>(a), forward<T2>(b));
}
```