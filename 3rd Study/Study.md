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
}
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
}
```

템플릿 타입 ```T```의 ```const_iterator``` 타입에 접근하려면 ```typename```을 붙여 한정해야 합니다.

```C++
typename T::const_iterator pos;
```

## 5.2 Zero Initialization

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