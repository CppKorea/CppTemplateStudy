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

## 5.4 Templates for Raw Arrays and String Literals

## 5.5 Member Templates

### 5.5.1 The ```.template``` Construct

### 5.5.2 Generic Lambdas and Member Templates

## 5.6 Variable Templates

## 5.7 Template Template Parameters

# Chapter 6: Move Semantics and ```enable_if<>```

## 6.1 Perfect Forwarding

## 6.2 Special Member Function Templates

## 6.3 Disable Templates with ```enable_if<>```

## 6.4 Using ```enable_if<>```

## 6.5 Using Concepts to Simplify ```enable_if<>``` Expressions