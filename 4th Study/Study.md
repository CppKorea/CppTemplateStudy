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