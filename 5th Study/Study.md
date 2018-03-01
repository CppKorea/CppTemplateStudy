# Chapter 8: Compile-Time Programming

- C++98 이전에는 템플릿을 통해 반복문과 실행 경로 선택 사용을 포함해 컴파일 타임에 계산하는 기능을 제공했습니다.

- 부분 특수화를 통해 특정 제약 사항이나 요구 사항에 따라 컴파일 타임에 서로 다른 클래스 템플릿 구현 중 하나를 선택할 수 있습니다.

- SFINAE 원리를 통해 서로 다른 타입이나 제약 사항에 따라 서로 다른 함수 템플릿 구현 중 하나를 선택할 수 있게 해줍니다.

- C++11과 C++14에서는 ```constexpr``` 기능을 통해 컴파일 타임 계산을 더 쉽게 사용할 수 있습니다.

- C++17에서는 "컴파일 타임 ```if```"를 도입해 컴파일 타임 조건 또는 제약 사항에 따라 문장을 제거할 수 있습니다. 템플릿 외부에서도 동작합니다.

## 8.1 Template Metaprogramming

템플릿은 컴파일 타임에 인스턴스화됩니다. C++ 템플릿의 일부 기능을 인스턴스화 과정과 결합하면 C++ 언어 자체에서 일종의 원시 재귀적인 "프로그래밍 언어"를 만들 수 있습니다. 이러한 이유로 템플릿을 사용해 "프로그래밍을 계산할 수 있습니다".

다음 코드는 컴파일 타임에 주어진 숫자가 소수인지 알려줍니다.

```C++
template <unsigned p, unsigned d>
struct DoIsPrime
{
    static constexpr bool value = (p%d != 0) && DoIsPrime<p,d-
1>::value;
};

template <unsigned p>
struct DoIsPrime<p, 2>
{
    static constexpr bool value = (p%2 != 0);
};

template <unsigned p>
struct IsPrime
{
    static constexpr bool value = DoIsPrime<p, p / 2>::value;
};

template <>
struct IsPrime<0> { static constexpr bool value = false; };
template <>
struct IsPrime<1> { static constexpr bool value = false; };
template <>
struct IsPrime<2> { static constexpr bool value = true; };
template <>
struct IsPrime<3> { static constexpr bool value = true; };
```

```IsPrime<>``` 템플릿은 전달한 템플릿 매개 변수 ```p```가 소수인지의 여부를 멤버 ```value```에 반환합니다. 이를 위해 ```DoIsPrime<>```를 표현식을 재귀적으로 확장하며 인스턴스화합니다.

예를 들어, 표현식

```C++
IsPrime<9>::value
```

는

```C++
DoIsPrime<9, 4>::value
```
로 확장하고

```C++
9 % 4 != 0 && DoIsPrime<9, 3>::value
```

```C++
9 % 4 != 0 && 9 % 3 != 0 && DoIsPrime<9, 2>::value
```

로 확장하고

```C++
9 % 4 != 0 && 9 % 3 != 0 && 9 % 2 != 0
```

으로 확장합니다.

9 % 3은 0이기 때문에 위 표현식의 결과는 ```false```가 됩니다.

템플릿 문법은 매우 복잡하지만, C++98부터 C++17까지 문제없이 사용할 수 있는 코드입니다. 그리고 몇몇 라이브러리에서 유용하다고 입증되었습니다.