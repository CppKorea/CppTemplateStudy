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

## 8.2 Computing with constexpr

C++11은 ```constexpr```이라는 기능을 도입했는데, 다양한 형태의 컴파일 타임 계산을 간단하게 만들어줍니다. 특히, ```constexpr``` 함수는 적절한 입력이 주어질 경우 컴파일 타임에 계산할 수 있습니다. C++11의 ```constexpr``` 함수는 엄격한 제약 사항들이 있었지만, C++14에서 대부분의 제약 사항들이 사라져 더욱 편하게 사용할 수 있게 되었습니다. 물론, ```constexpr``` 함수는 모든 계산 과정이 컴파일 타임에 가능해야 하고 유효해야 합니다. 현재는 힙 할당이나 예외 발생을 제외합니다.

이전에 다뤘던 숫자가 소수인지 테스트하는 예제를 C++11로 구현한 코드는 다음과 같습니다.

```C++
constexpr bool
doIsPrime(unsigned p, unsigned d)
{
    return d != 2 ? (p % d != 0) && doIsPrime(p, d - 1) : (p % 2 != 0);
}

constexpr bool isPrime(unsigned p)
{
    return p < 4 ? !(p < 2) : doIsPrime(p, p / 2);
}
```

C++11의 엄격한 제약 사항 때문에 선택 메커니즘으로 조건부 연산자만 사용할 수 있습니다. 그리고 요소를 반복하기 위해 재귀를 사용해야 합니다. 하지만 일반적인 C++ 함수 코드이므로 템플릿 인스턴스화에 의존하는 첫번째 버전보다는 코드를 읽기 쉽습니다.

C++14부터는 ```constexpr``` 함수에서 C++에 있는 대부분의 제어문을 사용할 수 있습니다. 따라서 위 코드를 C++14로 구현하면 다음과 같습니다.

```C++
constexpr bool isPrime(unsigned int p)
{
    for (unsigned int d = 2; d <= p / 2; ++d)
    {
        if (p % d == 0)
        {
            return false;
        }
    }

    return p > 1;
}
```

이제 9가 소수인지 확인하고 싶다면

```C++
isPrime(9)
```

를 호출하면 됩니다.

```constexpr```의 진정한 매력은 지금부터입니다. ```constexpr```은 컴파일 타임에 계산할 수 있지만, 반드시 그럴 필요가 없습니다. 컴파일 타임 값(예를 들어, 배열의 길이나 타입이 아닌 템플릿 인수)이 필요한 문맥이라면, 컴파일러는 컴파일 타임에 ```constexpr``` 함수를 호출해 계산하려고 합니다. 이 때 계산이 불가능하다면, 오류를 발생시킵니다. 다른 문맥이라면 컴파일 타임에 계산하거나 계산하지 않고, 설상 계산이 불가능하더라도 오류를 발생시키지 않고 대신 런타임에 호출합니다.

예를 들어,

```C++
constexpr bool b1 = isPrime(9);
```

는 컴파일 타임에 값을 계산합니다.

변수가 전역 범위나 네임스페이스 안에서 정의된 경우도 마찬가지로 컴파일 타임에 값을 계산합니다.

```C++
const bool b2 = isPrime(9);
```

만약 변수가 블록 범위에 정의된 경우라면, 컴파일러는 컴파일 타임에 계산할지 런타임에 계산할지 결정할 수 있습니다.

예를 들어,

```C++
bool fiftySevenIsPrime()
{
    return isPrime(57);
}
```

는 컴파일러가 컴파일 타임에 계산할 수도 있고 계산하지 않을 수도 있습니다.

반면,

```C++
int x;
...
std::cout << isPrime(x);
```

는 런타임에 계산하는 코드를 생성할 것입니다.