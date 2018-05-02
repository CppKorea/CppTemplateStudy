C++ Templates
===
## Chapter 19; Implementation Traits
###### Created by Wonsang You ( [@yws6909](https://github.com/yws6909) ) using [Marp](https://yhatt.github.io/marp/)

---

# Index
- Traits and Policy
  - Benefit of Templates
  - Lifting Template Function (Traits/Policy) 
- Type Functions
  - Element Types
  - Transformation Traits (Add/Remove Qualifier)
  - Predicate Traits
  - Result Type Traits
- SFINAE-Based Traits
  - SFINAE Out Function Overloads
  - SFINAE Out Partial Specializations
  - Using Generic Lambdas for SFINAE
  - SFINAE-Friendly Traits
- ConvertibleT
- Detecting Members
  - (Member Types, Arbitrary Member Types, Nontype Members, Member Functions, Other Expression) 
- Other Traits Techniques
  - If-Then-Else
  - Alias Templates and Traits
- Type Classification
  - (fundamental, compound, array, pointer-to-member, identifying funciton types, class type, enum type)
- Policy Traits
  - Read-Only Parameter Types
- In the Standard Library

---

# 19.1-2 Traits and Policy

## Benefit of Templates
### 동적 다형성의 장점
- 합/부분집합 관계를 기반으로한 일정한 처리 방식
- 동적인 바인딩과 분리된 컴파일

### 정적 다형성의 장점:
- 인터페이스의 형식과 기능을 기반으로 한 일정한 처리방식
- 정적인 타입 검사
- 컴파일시의 계산 진행이나 정적 바인딩은 동적 바인딩에서 얻을 수 없는 최적화와 효율성
- 독립적이면서도 자유롭게 연결되고 조합될 수 있다

### 어느것이 좋은가?
- 어떤 장점을 우선시 하는지는 선택.
- 정적 다형성의 장점 + 동적 다형성의 장점 = 더 큰 장점

---

## Traits versus Policy
### 특질 클래스 (traits class)
- 타입에 대한 고유한 타입 및 상수를 집계한다.
- type alias 와 정적 함수, 상수만을 포함하며, 상태 변경이 불가능하다.
- 인스턴스가 만들어지지 않는다.
- 대체로 전역적으로 사용되는 클래스이다.
- 정책 클래스에 수집되는 경향을 보인다.
- 메타프로그래밍(컴파일 타임 계산)에서 주가 된다.

### 정책 클래스 (policy class)
- 구성가능한 동작과, 관련된 타입 및 함수를 집계한다.
- 템플릿 매개변수로 전달되지 않으면 아무런 역할을 하지 않는다.
- 추가/제거가 가능한 기능의 일부이다.
- 독립적으로 인스턴스가 만들어지지 않으며 멤버로만 만들어진다.
- 표현식 프로그래밍(지연평가)에서 주가 된다.

---

## Lifting Template Function

모든 타입에서 사용할 수 있는 일반화된 누적연산을 만들어보자.

```cpp
template <typename T>
T accum (T const* beg, T const* end) 
{
    T total {};
    while (beg != end) {
        total = total + *beg;
        ++beg;
    }
    return total;
}
```

* 문제점 : T 타입끼리의 + 연산으로 T 타입을 만들것인가? (T = char, std::string_view 라면?)

* 개선책 : 리턴타입을 T 타입에 따라서 결정짓게 하자.

---

```cpp
template <typename>
struct accum_traits;

template <>
struct accum_traits<char> {
    using type = int;
    static constexpr type identity() {
        return 0;
    }
};

template <typename T>
auto accum(T const* beg, T const* end)
{
    using accum_type = typename accum_traits<T>::type;
    
    accum_type total = accum_traits<T>::identity();
    while (beg != end) {
        total = total + *beg;
        ++beg;
    }
    return total;
}
```

* 누적 연산은 + 연산만 되는 것이 아니다. 다른 연산도 지원해보자!

---

#### * sum_policy / mult_policy
```cpp
template <typename>
struct sum_policy;

template <>
struct sum_policy<char> {
public:
    using value_type = char;
    using accum_type = int;
    static constexpr accum_type identity() {
        return 0;
    }
    static constexpr accum_type accum(accum_type total, value_type value) {
        return total + value;
    }
};


template <typename>
struct mult_policy;

template <>
struct mult_policy<char> {
public:
    using value_type = char;
    using accum_type = int;
    static constexpr accum_type identity() {
        return 1;
    }
    static constexpr accum_type accum(accum_type total, value_type value) {
        return total * value;
    }
};
```
#### * accum (with policy) / main
```cpp
template <typename T, template <typename> class Policy = sum_policy>
auto accum(T const* beg, T const* end)
{
    using accum_type = typename Policy<T>::accum_type;
    accum_type total = Policy<T>::identity();
    while (beg != end) {
        total = Policy<T>::accum(total, *beg);
        ++beg;
    }
    return total;
}

int main(int argc, const char* argv[]) {
    char str[] = "hello template";
    
    std::cout << accum<char>(str, str + std::strlen(str));
    
    std::cout << accum<char, mult_policy>(str, str + std::strlen(str));

    return 0;
}
```

#### * accum (support iterator)
```cpp
template <typename Iter, template <typename> class Policy = sum_policy>
auto accum(Iter beg, Iter end)
{
    using value_type = typename std::iterator_traits<Iter>::value_type;
    
    auto total = Policy<value_type>::identity();
    while (beg != end) {
        total = Policy<value_type>::accum(total, *beg);
        ++beg;
    }
    return total;
}
```

#### * iterator_traits

```cpp
namespace std {
    template <typename T>
    struct iterator_traits<T*> {
        using difference_type   = ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;
        using reference         = T&;
        using iterator_category = random_access_iterator_tag;
    };
}
```

---

# 19.3 Type Function

- 타입함수 : 타입을 인자로 하여 타입이나 상수를 생성하는 함수
(상수를 생성하는 유용한 내장함수: sizeof (meta function))

---

#### * sizeof 를 이용한 인터페이스
```cpp
template <typename T>
struct type_size {
    static constexpr std::size_t value = sizeof(T);
};

int main(int argc, const char* argv[]) {
    std::cout << type_size<int>::value;
    return 0;
}
```

---

## Element Types
#### * element type 을 추출
```cpp
template <typename T>
struct element_type {
    using type = typename T::value_type;
};

// for array of unknown bound
template <typename T>
struct element_type<T[]> {
    using type = T;
};

// for array of known bound
template <typename T, std::size_t N>
struct element_type<T[N]> {
    using type = T;
};

template <typename T>
using element_t = typename element_type<T>::type;
```

---

## Transformation Traits

#### * remove reference
```cpp
template <typename T>
struct remove_reference {
    using type = T;
};

template <typename T>
struct remove_reference<T&> {
    using type = T;
};

template <typename T>
struct remove_reference<T&&> {
    using type = T;
};

template <typename T>
using remove_reference_t = typename  remove_reference<T>::type;
```

---

#### * adding reference (alias template)
```cpp
template <typename T>
using add_lvalue_reference_t = T&;

template <typename T>
using add_rvalue_reference_t = T&&;
```
(발표일 기준(C++17)으로 alias template 은 특수화가 어렵다.)

#### * adding reference (traits class)
---
lvalue reference
```cpp
template <typename T>
struct add_lvalue_reference {
    using type = T&;
};

template <>
struct add_lvalue_reference<void> {
    using type = void;
};

template <>
struct add_lvalue_reference<void const> {
    using type = void const;
};

template <>
struct add_lvalue_reference<void const volatile> {
    using type = void const volatile;
};

template <typename T>
using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;
```

rvalue reference
```cpp
template <typename T>
struct add_rvalue_reference {
    using type = T&&;
};

template <>
struct add_rvalue_reference<void> {
    using type = void;
};

template <>
struct add_rvalue_reference<void const> {
    using type = void const;
};

template <>
struct add_rvalue_reference<void const volatile> {
    using type = void const volatile;
};

template <typename T>
using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;
```
---

#### * remove qualifier
remove const
```cpp
template <typename T>
struct remove_const {
    using type = T;
};

template <typename T>
struct remove_const<T const> {
    using type = T;
};

template <typename T>
using remove_const_t = typename remove_const<T>::type;
```

remove volatile
```cpp
template <typename T>
struct remove_volatile {
    using type = T;
};

template <typename T>
struct remove_volatile<T volatile> {
    using type = T;
};

template <typename T>
using remove_volatile_t = typename remove_const<T>::type;
```

---

#### * remove const volatile
```cpp
template <typename T>
struct remove_cv : remove_const< remove_volatile_t<T>> {
};

template <typename T>
using remove_cv_t = typename remove_cv<T>::type;
```

---

#### * decay
reference 는 타입을 엄밀하게 보존한다.
decay 시에는 const, volatile 가 붕괴된다.

```cpp
template <typename T>
struct decay : remove_cv<T> {
};

// for array of unknown bound
template <typename T>
struct decay<T[]> {
    using type = T*;
};

// for array of known bound
template <typename T, std::size_t N>
struct decay<T[N]> {
    using type = T*;
};

template <typename R, typename... Args>
struct decay<R(Args...)> {
    using type = R(*)(Args...);
};

// c-style variadic resolution
template <typename R, typename... Args>
struct decay<R(Args..., ...)> {
    using type = R(*)(Args..., ...);
};

template <typename T>
using decay_t = typename decay<T>::type;
```

---

## Predicate Traits

#### * integral_constant
```cpp
template <typename T, T val>
struct integral_constant {
    using type = T;
    static constexpr T value = val;
};

template <bool B>
using bool_constant = integral_constant<bool, B>;

using true_type = bool_constant<true>;
using false_type = bool_constant<false>;
```

---

#### * is_same
```cpp
template <typename T1, typename T2>
struct is_same : false_type {
};

template <typename T>
struct is_same<T, T> : true_type {
};

template <typename T1, typename T2>
constexpr bool is_same_v = is_same<T1, T2>::value;
```

#### * true_type, false_type dispatching
```cpp

template <typename T>
void fooImpl(T, true_type) 
{
    std::cout << "fooImpl(T, true) for int called";
}

template <typename T>
void fooImpl(T, false_type) 
{
    std::cout << "fooImpl(T, false) for other type called";
}

template <typename T>
void foo(T t)
{
    fooImpl(T, is_same<T, int>{});
}

int main(int argc, const char* argv[]) {
    foo(42); // fooImpl(T, true) for int called
    foo(7.7); // fooImpl(T, false) for other type called
    return 0;
}
```

---

## Result Type Traits

#### * T1 + T2 = ?  
```cpp
template <typename T1, typename T2>
array<?> operator+ (array<T1> const&, array<T2> const&);
```

#### * plus_result traits
```cpp
template <typename T1, typename T2>
struct plus_result {
    using type = decltype(T1() + T2());
};

template <typename T1, typename T2>
using plus_result_t = typename plus_result<T1, T2>::type;
```

보완할점 :
- decltype 은 레퍼런스를 비롯하여 한정자를 포함합니다.
- 위의 array  컨테이너의 경우는 레퍼런스 타입을 처리하도록 설계되있지 않습니다.
- 어떤 + 연산에서는 const 를 리턴하는 경우도 있을 수 있습니다.
- T1 과 T2 의 기본생성자를 필요로 합니다. 
- (array 클래스 자체는 요소타입의 값 초기화를 필요로 하지 않을 수 있으므로 추가적이며 불필요한 제한 사항입니다.)

보완방법 :
- std::declval 를 사용하면 좋습니다. std::declval은 다음과 같이 정의됩니다.

#### * std::declval implementation
```cpp
namespace std {
    template <typename T>
    add_rvalue_reference_t<T> declval() noexcept;
}
```
- 평가가 필요하지 않는 (정의를 보지 않는) decltype, sizeof 와 같은 컨텍스트에서 사용되기때문에 고의로 정의를 사용하지 않았습니다.
- 참조가능 타입의 경우 리턴타입은 항상 rvalue 또는 lvalue 참조
- (참조를 리턴하지 않고 T 타입을 직접적으로 리턴해도 테스트해본 일부 케이스에 대해서 정상적으로 추론. 왜 참조를 리턴하는지 이유를 아시는 분은 알려주시면 감사하겠습니다..)
- noexcept 예외 스펙은 declval 자체가 예외를 발생시키는 것으로 간주하지 않는다는 것을 문서화 (noexcept 컨텍스트에서 유용)

#### * plus_result (보완버전)
```cpp
template <typename T1, typename T2>
struct plus_result {
    using type = decltype(declval<T1>() + declval<T2>());
};

template <typename T1, typename T2>
using plus_result_t = typename plus_result<T1, T2>::type;
```

```cpp
template <typename T1, typename T2>
auto operator+(array<T1> const&, array<T2> const&) {
    using result_type = remove_cv_t<plus_result_t<T1, T2>>
    
    array<result_type> ret;
    ...
    
    return ret;
}
```

---

## 19.4 SFINAE-Based Traits
### SFINAE (대체 실패는 오류가 아니다; 128 페이지의 8.4 절 및 284 페이지의 15.7 절 참조)
- 템플릿 인자 추론 중 유효하지 않은 타입 및 표현식이 형성 될 때 발생할 수있는 잠재적 오류를 끕니다.
- 이를 단순 추론 실패로 변경하여 오버로드 결정이 다른 후보를 선택하도록 허용합니다.

### SFINAE 활용성
- SFINAE 는 원래 함수 템플릿 오버로드로 인해 가짜 오류가 발생하는 것을 방지하기 위해 개발되었다.
- 특정 타입이나 표현식이 유효한지 여부를 결정할 수 있는 뛰어난 컴파일 타임 기술을 제공합니다. 

예를 들어 타입에 특정 멤버가 있는지, 특정 연산을 지원하는지 또는 클래스인지 여부를 결정하는 특성을 작성할 수 있습니다. SFINAE 기반 특성의 두 가지 주요 접근 방식은 함수 오버로드를 SFINAE로 처리하거나 부분 특수화를 SFINAE로 처리하는 것입니다.

---

#### * SFINAE Out Function Overloads
decltype 은 평가되지 않는 구문(정의가 필요하지 않음)을 요구한다.
실제 호출가능한가의 유무보다, 선언이 되었는가를 확인한다.
```cpp
template <typename T>
struct is_default_constructible_helper {
private:
    template <typename U, typename = decltype(U())>
    static true_type test(void*);
    
    template <typename>
    static false_type test(...);
    
public:
    using type = decltype(test<T>(nullptr));
};

template <typename T>
struct is_default_constructible : is_default_constructible_helper<T>::type {};
```

---

#### * SFINAE Out Partial Specializations
void_t 을 도입함으로써 템플릿 매개변수의 개수는 동일하면서 SFINAE 조건을 자유롭게 설정할 수 있다.

```cpp
template <typename...> using void_t = void;

template <typename, typename = void_t<>>
struct is_default_constructible : std::false_type {};

template <typename T>
struct is_default_constructible<T, void_t<decltype(T())>> : std::true_type {};
```
---

#### * Using Generic Lambdas for SFINAE
위에서 사용한 함수 오버로딩을 이용한 SFINAE 시에 사용한 코드 사용구 사용을 간편하게 축소한 방법 (C++17)
타입 랩핑을 통해서 템플릿 파라미터에서 오류검사가 이루어지도록 유도함.

```cpp
// F 함수에 대해 인자를 
template <typename F, typename... Args,
typename = decltype(std::declval<F>()(std::declval<Args&&>()...))>
true_type is_valid_impl(void*);

template <typename F, typename... Args>
false_type is_valid_impl(...);

inline constexpr
auto is_valid = [](auto f) {
    return [](auto&&... args) {
        return decltype(is_valid_impl<decltype(f), decltype(args)&&...>(nullptr)){};
    };
};

template <typename T>
struct type_wrapper {
    using type = T;
};

template <typename T>
constexpr auto type_instance = type_wrapper<T>{};

template <typename T>
T unwrap_type(type_wrapper<T>);
```

#### * is_default_constructible, has_first
```cpp
// decltype 내에서 사용한 표현식은 모두 정상적으로 선언이 되어있음.
// decltype 에 대한 () 기본생성자는 간접적인 표현식이므로 이 라인에서는 문제가 되지 않음.
// (정확한 이유를 아시는 분은 수정또는 알려주시면 감사하겠습니다 (_ _ )
constexpr auto is_default_constructible = is_valid([](auto x)->decltype( (void)decltype(unwrap_type(x)) () ) {});

constexpr auto has_first = is_valid([](auto x)->decltype((void)unwrap_type(x).first) {});
```

---

## SFINAE-Friendly Traits

#### * 위에서 작성한 plus_result
사실 이 코드는 문제점이 있습니다.
```cpp
template <typename T1, typename T2>
struct plus_result {
    using type = decltype(std::declval<T1>() + std::declval<T2>());
};

template <typename T1, typename T2>
using plus_result_t = plus_result<T1, T2>::type;
```

#### * arr + arr
```cpp
template <typename T>
class arr {};

class A {}; class B {}; class C{};

// return type auto(C++14)를 사용하면 선언부에 return type이 대체(추론)되기 전에 선언부 검사를 진행하기때문에 문제가 없다.
// auto (function..)-> decltype(expression) (C++11) 과 일반적으로 선언에 타입이 직접 명시되는 문제가 됩니다.
template <typename T1, typename T2>
plus_result_t<T1, T2> operator+ (arr<T1> const&, arr<T2> const&) {
    return plus_result_t<T1, T2>{};
}

arr<C> operator+ (arr<A> const&, arr<B> const&) {
    return arr<C>{};
}

int main(int argc, const char* argv[]) {
    arr<A> A = arr<A>{};
    arr<B> B = arr<B>{};
    auto C = operator+(A, B);
    std::cout << std::is_same_v<decltype(C), arr<C>>;
    return 0;
}
```

결과가 어떻게 나올까요?
=> 컴파일 에러!!

1. 컴파일러가 operator +의 첫 번째 (템플릿) 선언에서 공제 및 대체를 수행하지 않고 operator +의 두 번째 선언이 더 나은 일치라고 판단 할 수 있으면이 코드를 수락합니다.

2. 그러나 함수 템플릿 '후보'를 추론하고 대체하는 동안 클래스 템플릿의 정의를 인스턴스화하는 동안 발생하는 모든 작업은 해당 함수 템플릿 대체에 대한 직접적인 컨텍스트의 일부가 아니며 SFINAE는 유효하지 않은 타입을 만들지 않습니다.

3. 함수 템플릿 후보를 그냥 버리는 대신 PlusResultT <> 안에 A와 B 타입의 두 요소에 대해 operator +를 호출하려고하기 때문에 오류가 바로 발생합니다. (최종적으로 resolution 이 되지 않더라도)

---

#### 그러면 어떻게 해야할까?
plus_type 이 있는지를 SFINAE 로 확인해서 탈락시켜야합니다. 이를 위해서 has_plus 을 만들어보겠습니다.

#### * has_plus
```cpp
template <typename, typename, typename = void_t<>>
struct has_plus : false_type {};

template <typename T1, typename T2>
struct has_plus<T1, T2, void_t<decltype(std::declval<T1>() + std::declval<T2>())>> : true_type {};

template <typename T1, typename T2>
static constexpr bool has_plus_v = has_plus<T1, T2>::value;
```

#### * SFINAE 가 적용된 plus_result
컴파일 해보면 SFINAE 가 적용되어 비 템플릿 함수가 호출된다. (해결)
```cpp
template <typename T1, typename T2, bool = has_plus_v<T1, T2>>
struct plus_result {
    using type = decltype(std::declval<T1>() + std::declval<T2>());
};

template <typename T1, typename T2>
struct plus_result<T1, T2, false> {
};
```

---

# 19.5 IsConvertibleT

#### * is_convertible
위에서 보인 SFINAE'd Out Partial Specialization 을 이용합니다.
```cpp
tempalte <typename FROM, typename TO>
struct is_convertible_helper {
private:
    static void aux(TO);
    
    template <typename F, typename = decltype(aux(std::declval<F>()))>
    static true_type test(void*);
    
    template <typename>
    static false_type test(...);

public:
    using type = decltype(test<FROM>(nullptr));
};

template <typename FROM, typename TO>
struct is_convertible : is_convertible_helper<FROM, TO>::type {
};

template <typename FROM, typename TO>
using is_convertible_t = is_convertible<FROM, TO>::type;

template <typename FROM, typename TO>
constexpr bool is_convertible_v = is_convertible::value;
```

완벽해보이지만 수정해야할 부분이 약간 있습니다.

1. 배열 타입의 변환은 항상 false 를 반환해야하지만 포인터 타입으로 붕괴되는 경우 일부 옳지 않은 결과가 나올 수 있습니다.
2. 함수 타입에 대한 변환은 항상 false 를 반환해야합니다. 여기에서도 마찬가지로 붕괴되는 경우 옳지 않은 결과가 나옵니다.
3. void 타입으로 변환하면 항상 true 가 나옵니다. 매개변수 타입이 void가 될 수 없기때문입니다. 

뒤에서 Type Classification을 한후에 이를 수정해보겠습니다. 

# 19.6 Detecting Members

#### Detecting Member Types

```cpp
template <typename, typename = void_t<>>
struct has_size_type : false_type {};

template <typename T>
struct has_size_type<T, void_t<typename T::size_type>> : true_type {};
```

#### Dealing with Reference Types
```cpp
struct CXR {
    using size_type = char&;
};
std::cout << has_size_type<CXR>::value; // true

std::cout << has_size_type<CXR&>::value; // false
```
이처럼 참조 타입에 대한 멤버타입 탐지는 실패합니다. 
이를 보완하기 위해서 다음과 같이 사용합니다. 

```cpp
template <typename, typename = void_t<>>
struct has_size_type : false_type {};

template <typename T>
struct has_size_type<T, void_t<typename remove_reference_t<T>::size_type>> : true_type {};
```

---

#### Detecting Arbitrary Member Types

```cpp
#define DEFINE_HAS_TYPE(MemType) \
    template <typename, typename = std::void_t<>> \
    struct has_##MemType : std::false_type {}; \
    template <typename T> \
    struct has_##MemType<T, std::void_t<typename T::##MemType>> : std::true_type {};
```

---

#### Detecting Nontype Members

```cpp
template <typename, typename = void_t<>>
struct has_member_first : false_type {};

template <typename T>
struct has_member_first<T, void_t<decltype(&T::first)>> : true_type {};
```

해당 함수가 유효하려면 다음 조건이 충족되어야 합니다.

- T의 멤버를 모호하지 않게 식별해야합니다. (오버로드된 멤버함수, 동일한 이름의 여러 상속된 멤버이름)
- 멤버는 접근 가능해야합니다.
- 멤버는 타입이 아니며 열거형이 아닌 멤버여야합니다. (그렇지 않으면 접두사 &는 유효하지 않습니다.)
- T::Member가 정적 데이터 멤버일경우 해당 타입에 &T::Member 를 유효하지 않게 만드는 연산자 &를 제공하면 안됩니다. 

---

#### Detecting Member Functions

SFINAE 원칙은 함수 템플릿 선언에 잘못된 타입 및 표현식을 작성하려는 시도를 모두 막아주며 오버로딩 기법을 통해서 임의 표현식이 올바른 형식인지 여부를 테스트 할 수 있습니다.

```cpp
template <typename, typename = void_t<>>
struct has_begin : false_type {};

template <typename T>
struct has_begin<T, void_t<decltype(std::declval<T>().begin())>> : true_type {};
```

---

#### Detecting Other Expressions

위의 기술을 사용하여 다른 종류의 표현을 만들거나 여러 표현을 결합 할 수도 있습니다. (생략)

---

#### Using Generic Lambdas to Detect Members

19.4 에서 소개한 is_valid (C++17 이 요구되는 소형화된 SFINAE 구현코드)에 전달하는 제네릭 람다함수의 매개변수가 참조타입이기 때문에 멤버 타입을 찾지 못합니다. 이때문에 항상 대체에 실패하게 되므로 이를 수정해줄 필요가 있습니다. 

```cpp
// 참조타입의 멤버조회가 항상 대체 실패로 인해 false_type 리턴
constexpr auto has_size_type = is_valid([](auto&& x) -> std::void_t<decltype<unwrap_type(x)::size_type>{}

// 붕괴 타입 함수를 이용하여 참조 및 한정자 제거 
constexpr auto has_size_type = is_valid([](auto&& x) -> std::void_t<std::decay_t<unwarp_type_value(x)>::size_type>{};

```
# 19.7 Other Traits Techniques

#### * If Then Else
```cpp
template <bool Cond, typename TrueType, typename FalseType>
struct if_then_else {
    using type = TrueType;
};

template <typename TrueType, typename FalseType>
struct if_then_else<false, TrueType, FalseType> {
    using type = FalseType;
};
```

---

#### * Detecting Nonthrowing Operations

이 부분은 이해가 부족하여 준비하지 못했습니다. 조금 더 살펴본뒤 내용을 채워넣겠습니다.

---

#### * variable template / alias template
```cpp
template <typename T1, typename T2>
constexpr bool is_same_v = is_same<T1, T2>::value;

template <typename T1, typename T2>
using is_same_t = is_same<T1, T2>::type;
```
---

# 19.8 Type Classification

#### Determining Fundamental Types
```cpp
template <typename T>
struct is_funda : false_type {};

template <>
struct is_funda<char> : true_type {};
template <>
struct is_funda<int> : true_type {};
template <>
struct is_funda<double> : true_type {};

// expand other types... using macro...

template <typename T>
constexpr bool is_funda_v = is_funda<T>::value;
```

#### Determining Compound Types
(위에서 element_type 추출한것과 동일한 방법)
```cpp
template <typename T>
struct is_pointer : false_type {};

template <typename T>
struct is_pointer<T*> : true_type {
    using base_type = T;
};

template <typename T>
constexpr bool is_pointer_v = is_pointer<T>::value;
```

#### Determining References
```cpp
template <typename T>
struct is_lvalue_reference : false_type {};

template <typename T>
struct is_lvalue_reference<T&> : true_type {
    using base_type = T;
};

template <typename T>
using is_lvalue_reference_t = typename is_lvalue_reference<T>::type;

template <typename T>
constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value;

template <typename T>
struct is_rvalue_reference : false_type {};

template <typename T>
struct is_rvalue_reference<T&&> : true_type {
    using base_type = T;
};

template <typename T>
using is_rvalue_reference_t = typename is_rvalue_reference<T>::type;
template <typename T>
constexpr bool is_rvalue_reference_v = is_rvalue_reference<T>::value;

template <typename T>
struct is_reference : if_then_else<is_lvalue_reference_v<T>, is_lvalue_reference_t<T>, is_rvalue_reference_t<T>> {};

template <typename T>
constexpr bool is_reference_v = is_reference<T>::value;
```

#### Determining Arrays

```cpp
template <typename>
struct is_array : false_type {};

template <typename T, std::size_t N>
struct is_array<T[N]> : true_type {
    using base_type = T;
    static constexpr std::size_t size = N;
};
template <typename T>
struct is_array<T[]> : true_type {
    using base_type = T;
    static constexpr std::size_t size = 0;
};

template <typename T>
constexpr bool is_array_v = is_array<T>::value;
```

#### Determining Pointers to Members

```cpp
template <typename T>
struct is_pointer_to_member : false_type {
};

template <typename T, typename C>
struct is_pointer_to_member<T C::*> : true_type {
    using member_type = T;
    using class_type = C;
};

template <typename T>
constexpr bool is_pointer_to_member_v = is_pointer_to_member<T>::value;
```

---

#### Identifying Function Types
다양한 한정자를 가진 함수 타입을 인식하려면 한정자의 모든 조합을 포괄하는 많은 수의 부분 특수화를 도입해야합니다.
(const, volatile, reference, pointer, c-style variadic)

```cpp
template <typename T>
struct is_function : false_type {};

template <typename R, typename... Params>
struct is_function<R(Params...)> : true_type {
    using type = R;
    using param_type = std::tuple<Params...>;
    static constexpr bool variadic = false;
};

// c-style variadic parameter
template <typename R, typename... Params>
struct is_function<R(Params..., ...)> : true_type {
    using type = R;
    using params_type = std::tuple<Params...>;
    static constexpr bool variadic = true;
};

template <typename T>
constexpr bool is_function_v = is_function<T>::value;
```
---

#### Determining Class Types

```cpp
template <typename T, typename = void_t<>>
struct is_class : false_type {};

template <typename T>
struct is_class<T, void_t<int T::*>> : true_type {};

template <typename T>
constexpr bool is_class_v = is_class<T>::value;
```

---

#### Determining Enumeration Types
다른 범주에 속하지 않는 타입이 열거형이기때문에 이 조건을 사용해서 만들어주면 됩니다.

```cpp
template <typename T>
struct is_enum {
    static constexpr bool value =
    !is_funda_v<T> &&
    !is_pointer_v<T> &&
    !is_reference_v<T> &&
    !is_array_v<T> &&
    !is_pointer_to_member_v<T> &&
    !is_function_v<T> &&
    !is_class_v<T>;
};
```

---

## +) is_convertible
#### * 19.5 에서 Type Classification 끝난후로 수정을 미룬 is_convertible
```cpp
tempalte <typename FROM, typename TO>
struct is_convertible_helper {
private:
    static void aux(TO);
    
    template <typename F, typename = decltype(aux(std::declval<F>()))>
    static true_type test(void*);
    
    template <typename>
    static false_type test(...);

public:
    using type = decltype(test<FROM>(nullptr));
};

template <typename FROM, typename TO>
struct is_convertible : is_convertible_helper<FROM, TO>::type {
};

template <typename FROM, typename TO>
using is_convertible_t = is_convertible<FROM, TO>::type;

template <typename FROM, typename TO>
constexpr bool is_convertible_v = is_convertible::value;
```
개선할 부분 :
1. 배열 타입의 변환은 항상 false 를 반환해야하지만 포인터 타입으로 붕괴되는 경우 일부 옳지 않은 결과가 나올 수 있습니다.
2. 함수 타입에 대한 변환은 항상 false 를 반환해야합니다. 여기에서도 마찬가지로 붕괴되는 경우 옳지 않은 결과가 나옵니다.
3. void 타입으로 변환하면 항상 true 가 나옵니다. 매개변수 타입이 void가 될 수 없기때문입니다. 

#### * 수정된 is_convertible
```cpp
template <typename FROM, typename TO, bool = is_void_v<TO> || is_array_v<TO> || is_function_v<TO>>
struct is_convertible_helper {
private:
    static void aux(TO);
    
    template <typename F, typename = decltype(aux(std::declval<F>()))>
    static true_type test(void*);
    
    template <typename>
    static false_type test(...);
    
public:
    using type = decltype(test<FROM>(nullptr));
};

template <typename FROM, typename TO>
struct is_convertible_helper<FROM, TO, false> {
    using type = false_type;
};

template <typename FROM, typename TO>
struct is_convertible : is_convertible_helper<FROM, TO>::type {};

template <typename FROM, typename TO>
using is_convertible_t = typename is_convertible<FROM, TO>::type;

template <typename FROM, typename TO>
constexpr bool is_convertible_v = is_convertible<FROM, TO>::value;
```


# 19.9 Policy Traits

지금까지 trait 템플릿 예제는 템플릿 매개변수의 속성을 결정하는데 사용되었습니다. 템플릿 매개변수가 나타내는 타입, 해당 타입의 값에 적용되는 연산자의 결과 타입등이 사용되었는데 이를 속성 특성이라고 부릅니다.

대조적으로 일부 특성은 일부 타입을 어떻게 처리해야하는지 정의합니다. 우리는 그것을 정책 특성이라고 부릅니다.

#### Read-Only Parameter Types
```cpp
template <typename T>
struct read_only_param {
    using type = typename if_then_else<sizeof(T)<=2*sizeof(void*), T, T const&>::type;
};
```

sizeof 가 작은 값을 반환하는 컨테이너 타입에는 비싼 복사 생성자가 필요할 수 있으므로 다음과 같이 특수화가 필요할 수 있습니다. 

```cpp
template <typename T>
struct read_only_param {
    using type = Array<T> const&;
};
```

여기서 불편한 점은 함수의 매개변수로서 사용할때 함수 선언이 복잡해지며, 함수의 타입 추론과 함께 사용될 수 없다는 것입니다. 이를 보완하기 위해서 인라인 래퍼 함수 템플릿을 사용하는 방법이 있습니다.

#### Read-Only Wrapper
```cpp
template <typename T1, typename T2>
void foo_core (typename read_only_param_t<T1> p1, typename read_only_param_t<T2> p2) {
    ...
}

template <typename T1, typename T2>
void foo (T1 && p1, T2 && p2)
{
    foo_core<T1, T2>(std::forward<T1>(p1), std::forward<T2>(p2));
}

int main (int argc, const char* argv[])
{
    int a = 1;
    double b = 4.4f;
    foo(a, b); 
    // read_only_param_t<int>, read_only_param_t<double>
    return 0;
}
```

# In the Standard Library
C++11 에서 type trait 은 STL의 본질적인 부분이 되었습니다. STL 에는 이장에서 논의된 타입함수와 타입특성을 대부분 포함합니다. 또한 컴파일러는 컴파일 시간을 단축하기 위해 언어로 구현된 솔루션이 있더라도 특성을 지원하기 시작했습니다. 이러한 이유로 타입 특성이 필요하며 사용 가능한 경우 C++ 표준 라이브러리의 특성을 사용하는 것이 좋습니다. 