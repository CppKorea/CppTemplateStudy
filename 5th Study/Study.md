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

## 8.3 Execution Path Selection with Partial Specialization

```isPrime()```처럼 컴파일 타임에 테스트하는 함수를 응용해 예제를 만들 수 있는데, 부분 특수화를 사용해 컴파일 타임에 서로 다른 함수 구현 중 하나를 선택할 수 있게 만들 수 있습니다.

예를 들어, 템플릿 인수가 소수인지에 따라 서로 다른 구현 중 하나를 선택할 수 있게 만들 수 있습니다.

```C++
template <int SZ, bool isPrime(SZ)>
struct Helper;

template <int SZ>
struct Helper<SZ, false>
{
    ...
};

template <typename T, std::size_t SZ>
long foo(std::array<T, SZ> const& coll)
{
    Helper<SZ> h;
}
```

위 코드는 ```std::array<>```의 크기가 소수인가 아닌가에 따라 서로 다른 클래스 ```Helper<>```를 사용합니다.

위 코드에서는 서로 다른 두 가지 경우의 수를 고려해 2개의 부분 특수화를 사용했습니다. 대신, 경우의 수 중 한 가지를 기본 템플릿으로 사용하고 다른 특별한 경우에는 부분 특수화를 사용하도록 변경할 수 있습니다.

```C++
template <int SZ, bool = isPrime(SZ)>
struct Helper
{
    ...
};

template <int SZ>
struct Helper<SZ, true>
{
    ...
};
```

함수 템플릿은 부분 특수화를 지원하지 않기 때문에, 특정 제약 사항에 따라 함수 구현을 변경하는 다른 메커니즘을 사용해야 합니다.

- 클래스를 정적 함수와 함께 사용하세요.
- ```std::enable_if```를 사용하세요.
- SFINAE 원칙을 사용하세요.
- 컴파일 타임 ```if```를 사용하세요

## 8.4 SFINAE (Substitution Failure Is Not An Error)

보통 C++에서는 다양한 인수 타입을 처리하기 위해 함수를 오버로드합니다. 컴파일러가 오버로드된 함수를 호출하는 코드를 볼 때, 후보를 하나하나씩 고려해야 합니다. 이 때 컴파일러는 호출하는 코드의 인수를 확인한 뒤, 가장 잘 일치하는 후보를 선택합니다.

호출 가능한 후보 집합 중 함수 템플릿이 있다면, 컴파일러는 먼저 해당 템플릿에서 사용해야 하는 템플릿 인수를 결정한 다음 함수의 매개 변수 목록과 리턴 타입에서 해당 인수를 치환한 뒤 얼마나 잘 일치하는지 확인합니다.

그러나 치환 과정에 문제가 있을 수 있습니다. 왜냐하면 의미가 없는 구조를 만들 수 있기 때문입니다. 언어 규칙은 이러한 무의미한 치환으로 인해 오류를 발생시키는 대신, 치환 문제가 있는 후보를 단순히 무시한다고 규정합니다.

우리는 이 원칙을 <b>SFINAE(Substitution Failure Is Not An Error, 추론 실패는 오류가 아님)</b>라고 합니다.

참고로 여기에서 설명하는 치환 과정은 사용자의 요구에 따라 인스턴스를 생성하는 과정과 다릅니다. 필요하지 않은 잠재적 인스턴스화에 대해서도 치환 과정을 수행할 수 있습니다. 따라서 컴파일러는 실제로 불필요한 것인지 여부를 확인할 수 있습니다. 이 때, 함수의 본문이 아닌 선언부를 바로 치환합니다.

다음 예제를 고려해 봅시다.

```C++
template <typename T, unsigned N>
std::size_t len(T(&)[N])
{
    return N;
}

template <typename T>
typename T::size_type len(T const& t)
{
    return t.size();
}
```

여기서 하나의 제너릭 인수를 받는 함수 템플릿 ```len()```을 두 개 정의합니다.

1. 첫번째 함수 템플릿은 매개 변수를 ```T(&)[N]```으로 선언합니다. 이는 매개 변수가 타입 ```T```의 요소 ```N```개를 갖는 배열을 의미합니다.

2. 두번째 함수 템플릿은 매개 변수를 ```T```로 선언합니다. 매개 변수에 아무런 제약 사항이 없지만, 리턴 타입이 ```T::size_type```입니다. 이는 전달된 인수 타입이 ```size_type``` 멤버를 갖고 있어야 한다는 것을 의미합니다.

배열이나 문자열 리터럴을 전달하면, 배열을 받는 함수 템플릿만 일치합니다.

```C++
int a[10];
std::cout << len(a);
std::cout << len("tmp");
```

함수 서명에 따르면, 두번째 함수 템플릿도 ```T```를 ```int[10]```과 ```char const[4]```로 치환하면 일치하지만 리턴 타입 ```T::size_type```으로 인해 잠재적인 오류가 발생할 수 있습니다. 따라서 두번째 템플릿을 무시합니다.

```std::vector<>```를 전달하면, 두번째 함수 템플릿만 일치합니다.

```C++
std::vector<int> v;
std::cout << len(v);
```

포인터를 전달하면, 두 템플릿 모두 일치하지 않습니다. 그 결과, 컴파일러는 일치하는 ```len()``` 함수를 찾지 못했다고 오류 메시지를 출력할 것입니다.

```C++
int* p;
std::cout << len(p);
```

```size_type``` 멤버를 갖지만 ```size()``` 멤버 함수가 없는 타입의 개체를 전달하는 경우는 동작이 조금 다릅니다. 대표적인 예로 ```std::allocator<>```가 있습니다.

```C++
std::allocator<int> x;
std::cout << len(x);
```

컴파일러는 이러한 타입의 객체를 전달하면 두번째 함수 템플릿을 일치하는 함수 템플릿으로 찾습니다. 따라서 일치하는 ```len()``` 함수가 없다는 오류 대신 ```std::allocator<int>```에 ```size()```를 호출할 수 없다는 컴파일 타임 오류 메시지를 출력합니다. 이번에는 두번째 함수 템플릿을 무시하지 않습니다.

리턴 타입을 치환할 때 후보를 무시하면, 컴파일러가 덜 일치하는 매개 변수를 갖는 다른 후보를 선택할 수도 있습니다.

예를 들어,

```C++
template <typename T, unsigned N>
std::size_t len(T(&)[N])
{
    return N;
}

template <typename T>
typename T::size_type len(T const& t)
{
    return t.size();
}

std::size_t len(...)
{
    return 0;
}
```

는 항상 일치하지만 최악의 일치를 갖는 일반 ```len()``` 함수도 제공합니다.

따라서 배열과 벡터의 경우 일치하는 항목이 두 개 있는데, 특정 항목이 더 잘 일치합니다. 포인터의 경우에는 마지막 함수만 일치하므로 컴파일러가 이 호출에 대해  ```len()```이 없다고 불평하지 않습니다. 그러나 할당자의 경우에는 두번째와 세번째 함수 템플릿이 일치하고 두번째 함수 템플릿이 더 잘 일치합니다. 그럼에도 불구하고, ```size()``` 멤버 함수를 호출 할 수 없다는 오류가 발생합니다.

```C++
int a[10];
std::cout << len(a);
std::cout << len("tmp");

std::vector<int> v;
std::cout << len(v);

int* p;
std::cout << len(p);

std::allocator<int> x;
std::cout << len(x);
```

#### SFINAE와 오버로드 해결

시간이 지남에 따라 SFINAE 원칙이 너무 중요해져서, 템플릿 설계자들 사이에서 이 약어를 동사로 부르기 시작했습니다. SFINAE 메커니즘을 적용해 특정 제약 조건에 대해 함수 템플릿을 무시하도록 만들려면, 템플릿 코드가 이러한 제약 조건에 대해 유효하지 않은 코드를 생성하도록 만들어 SFINAE 메커니즘을 적용해야 합니다. 우리는 이를 "함수를 SFINAE한다."라고 말합니다. 그리고 C++ 표준에서 함수 템플릿이 "오버로드 해결에 참여하지 않을 것"이라는 문장을 읽을 때마다 SFINAE가 특정 경우에 해당 함수 템플릿을 "SFINAE하는 데 사용한다"라고 받아들입니다.

예를 들어, ```std::thread``` 클래스는 생성자를 선언합니다.

```C++
namespace std
{
    class thread
    {
    public:
        ...
        template <typename F, typename... Args>
        explicit thread(F&& f, Args&&... args);
    ...
    };
}
```

여기서 주의할 점은 ```decay_t<F>```가 ```std::thread```와 동일한 타입인 경우, 이 생성자는 오버로드 해결에 참여하지 않는다는 점입니다.

즉, ```std::thread```를 첫번째이자 유일한 인수로 호출하면 템플릿 생성자가 무시됩니다. 그 이유는 다른 경우에는 이와 같은 멤버 템플릿이 미리 정의된 복사 또는 이동 생성자보다 더 잘 일치할 수 있기 때문입니다. 스레드를 호출할 때 생성자 템플릿을 SFINAE하면, 다른 스레드에서 스레드를 생성할 때 항상 미리 정의된 복사 또는 이동 생성자를 사용하도록 합니다.

이 기술을 사례별로 다루는건 힘들 수 있습니다. 다행히도 표준 라이브러리는 템플릿을 더 쉽게 사용하지 못하게 하는 도구를 제공합니다. 이러한 기능 중 가장 잘 알려진 기능으로 ```std::enable_if<>```가 있습니다. 이 타입 특성을 사용하면 템플릿을 사용하지 못하게 하는 조건이 포함된 구문으로 대체할 수 있습니다.

결과적으로, ```std::thread```의 실제 선언은 다음과 같습니다.

```C++
namespace std
{
    class thread
    {
    public:
        ...
        template <typename F, typename... Args,
                  typename = std::enable_if<!std::is_same_v<std::decay_t<F>>
        explicit thread(F&& f, Args&&... args);
    ...
    };
}
```