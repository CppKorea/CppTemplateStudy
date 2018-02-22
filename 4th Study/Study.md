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