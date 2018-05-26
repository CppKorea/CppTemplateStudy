# Chapter 22

18챕터에서 간단히 Static 다형성과 동적 다형성에 대해 설명 하였다.

Static 한 다형성은 일반 코드와 동일한 성능이 나오는 반면에 런타임에 사용할수 있는 타입이 정해져 있다.

동적 다형성은 컴파일 된 시점에서 알려지지 않은 타입도 다형성을 유지할수 있지만 이는 기본 클래스가 있어야 하여

유연성이 떨어지게 됩니다.

## 22.1 함수 객체와 포인터 그리고 std::function<>

함수 객체를 통해 사용자가 다양하게 작업을 할 수 있다.

예로 다음 0 부터 N까지 반복 하면서 각 값에 지정된 함수 객체 f를 지정해 주고 있다.

```
### bridge/forupto1.cpp
`
#include <vector>
#include <iostream>
template<typename F>
void forUpTo(int n, F f)
{
	for (int i = 0; i != n; ++i)
	{
		f(i); // call passed function f for i
	}
}
void printInt(int i)
{
	std::cout << i << ’ ’;
}
int main()
{
	std::vector<int> values;
	// insert values from 0 to 4:
	forUpTo(5,
		[&values](int i) {
		values.push_back(i);
	});
	// print elements:
	forUpTo(5,
		printInt); // prints 0 1 2 3 4
	std::cout << ’\n’;
}
```

이 예제에서 함수 f는 매우작지만 템플릿이 커지면 커질수록 이는 코드 사이즈가 증가되는 문제가 있다.

코드 사이즈를 제한하는 방법으로는 아래와 같은 방법으로 non-템플릿을 사용하면 된다.
```
void forUpTo(int n, void(*f)(int))
{
	for (int i = 0; i != n; ++i)
	{
		f(i); // call passed function f for i
	}
}
```

하지만 이 함수는 아래와 같은 printInt()함수인 경우 정상적으로 출력이 되지만

람다인경우에는! 에러출력!

```
forUpTo(5,printInt); //OK: prints 0 1 2 3 4
forUpTo(5,[&values](int i) { //ERROR: lambda not convertible to a
function pointer
values.push_back(i);
});
```

이 대안으로 std::function<>객체를 사용하면 됩니다.

```
void forUpTo(int n, std::function<void(int)> f)
{
for (int i = 0; i != n; ++i)
{
f(i) // call passed function f for i
}
}
```

std::function<> 템플릿에 인수는 함수 포인터의 매개 변수와 결과 타입을 지정해줘야 합니다. 

이떄 forUpTo() 함수는 정적 다형성과 비슷한 특성을 제공한다.

이떄 특성은 함수포인터,람다,class의 oepratior()을 넣어서 작업을 할수 있다.

이 기술을 `type erasure` 라고 하며 Static 다형성과 동적 다형성간에 연결 고리를 만들어 냅니다. 



## 22.2 일반화된 함수 포인터

std::function은 기존의 함수 포인터와 동일한 연산을 지원한다.

* 호출자가 이 함수를 몰라도 이 함수를 호출할수 있다.

* 이 함수는 복사,대입,이동연산자를 지원합니다.

* 초기화나 다른 함수를 대입할수 있다.

* 함수가 bind이 되어 있지 않을때 null상태가 됩니다.

하지만 C++포인터와 다르게 std::function은 oepratior()나 람다, 함수객체를 저장할수 있다.

앞으로 아래와같이 std::function과 비슷한 함수 FunctionPtr 만들거다.
```
#include "functionptr.hpp"
#include <vector>
#include <iostream>
void forUpTo(int n, FunctionPtr<void(int)> f)
{
	for (int i = 0; i != n; ++i)
	{
		f(i); // call passed function f for i
	}
}
void printInt(int i)
{
	std::cout << i << ’ ’;
}
int main()
{
	std::vector<int> values;
	// insert values from 0 to 4:
	forUpTo(5, [&values](int i) {
		values.push_back(i);
	});
	// print elements:
	forUpTo(5,
		printInt); // prints 0 1 2 3 4
	std::cout << ’\n’;
}
```
FunctionPtr 인터페이스는 상당히 직관적이며 생성자, 복사, 이동, 소멸자, 초기화 대입연산자을 제공한다.

이 인터페이스에서 가장 재미있는점은 결과타입과 인수를 분리하여 특수화를 하는 방법이다.

FunctionPtr구현은 아래와 같다.

```
template<typename Signature>
class FunctionPtr;


template<typename R, typename … Args>
class FunctionPtr<R(Args…)>
{
private:
	FunctorBridge<R, Args…>* bridge;
public:
	// constructors:
	FunctionPtr() : bridge(nullptr) {
	}
	FunctionPtr(FunctionPtr const& other); // see functionptrcpinv.
		FunctionPtr(FunctionPtr& other)
		: FunctionPtr(static_cast<FunctionPtr const&>(other)) {
	}
	FunctionPtr(FunctionPtr&& other) : bridge(other.bridge) {
		other.bridge = nullptr;
	}
	//construction from arbitrary function objects:
	template<typename F> FunctionPtr(F&& f);
	FunctionPtr& operator=(FunctionPtr const& other) {
		FunctionPtr tmp(other);
		swap(*this, tmp);
		return *this;
	}
	FunctionPtr& operator=(FunctionPtr&& other) {
		delete bridge;
		bridge = other.bridge;
		other.bridge = nullptr;
		return *this;
	}
	//construction and assignment from arbitrary function objects:
	template<typename F> FunctionPtr& operator=(F&& f) {
		FunctionPtr tmp(std::forward<F>(f));
		swap(*this, tmp);
		return *this;
	}
	// destructor:
	~FunctionPtr() {
		delete bridge;
	}
	friend void swap(FunctionPtr& fp1, FunctionPtr& fp2) {
		std::swap(fp1.bridge, fp2.bridge);
	}
	explicit operator bool() const {
		return bridge == nullptr;
	}
	// invocation:
	R operator()(Args… args) const; // see functionptr-cpinv.hpp
};
```
여기 bridge라는 함수 객체를 조작해주고 관리하고 FunctionPtr는 포인터를 관리만 한다.

아직 구현되지 않는 함수는 다음 하위섹션에서 설명하겠다.

## 22.3 Bridge 인터페이스

dynamic버전의 FunctorBridge의 추상클래스는 아래와 같이 구현되어 있다.
```
template<typename R, typename… Args>
class FunctorBridge
{
public:
	virtual ~FunctorBridge() {
	}
	virtual FunctorBridge* clone() const = 0;
	virtual R invoke(Args… args) const = 0;
};
```
FunctorBridge 함수는 가상함수를 통해 함수객체를 저장하고 조작할 필요이 필요하다.

clone의 연산은 함수 복사를 수행하고 invoke는 함수는 함수 객체를 호출한다. 

FunctionPtr의 복사 생성자와 () 연사자는 아래와 같이 구현했다.

```
FunctionPtr<R(Args…)>::FunctionPtr(FunctionPtr const& other)
	: bridge(nullptr)
{
	if (other.bridge) {
		bridge = other.bridge->clone();
	}
}
template<typename R, typename… Args>
R FunctionPtr<R(Args…)>::operator()(Args… args) const
{
	return bridge->invoke(std::forward<Args>(args)…);
}
```
### 타입 제거

FunctorBridge 클래스는 추상클라스여서 자식 클래스가 가상 함수를 만들어야한다.

상속을 통한 구현은 구현하고 싶은 만큼의 수만큼 파생 클래스가 필요로합니다.

상수 클래스를 매개 변수화 함으로 써 구현이 쉽게 가능하다.

```
template<typename Functor, typename R, typename… Args>
class SpecificFunctorBridge : public FunctorBridge<R, Args…> {
	Functor functor;
public:
	template<typename FunctorFwd>
	SpecificFunctorBridge(FunctorFwd&& functor)
		: functor(std::forward<FunctorFwd>(functor)) {
	}
	virtual SpecificFunctorBridge* clone() const override {
		return new SpecificFunctorBridge(functor);
	}
	virtual R invoke(Args… args) const override {
		return functor(std::forward<Args>(args)…);
	}
};
```
SpecificFunctorBridge은 함수 객체를 저장 복사 호출할수 있다. 또한 소멸자도 호출이 가능하다.

FunctionPtr를 초기화 하기 위해서는 아래와 같이 객체가 필요합니다.

```
template<typename R, typename… Args>
template<typename F>
FunctionPtr<R(Args…)>::FunctionPtr(F&& f)
	: bridge(nullptr)
{
	using Functor = std::decay_t<F>;
	using Bridge = SpecificFunctorBridge<Functor, R, Args…>;
	bridge = new Bridge(std::forward<F>(f));
}
```

FunctionPtr 생성자의 탬플릿 F가 있지만 SpecificFunctorBridge에서 특수화 한다.

새로 만들어진 Bridge인스턴스가 데이터 멤버로 활당이 되면 

Bridge 에서 FunctorBridge<R, Args…> *.로 컨버팅이 될떄 특정 타입 F의 데이터가 손실된다.

이렇게 동적 , 정적 다형성 사이에 Bridge 기술을 종종 사용된다.


### 22.5 선택적인 Bridging 인터페이스

FunctionPtr을 함수 포인터대신 사용할수 있지만 함수포인터가 제공하는 equals 연산을 지원 안해준다.


```
virtual bool equals(FunctorBridge const* fb) const = 0;

```
동일한 함수 객체를 비교하는건 다음과 같다.
```
virtual bool equals(FunctorBridge<R, Args…> const* fb) const
override {
	if (auto specFb = dynamic_cast<SpecificFunctorBridge const*>
		(fb)) {
			return functor == specFb->functor;
		}
	//functors with different types are never equal:
	return false;
}
```
마지막으로 operator==을 구현해보자
```
friend bool operator==(FunctionPtr const& f1, FunctionPtr const& f2) {
	if (!f1 || !f2) {
		return !f1 && !f2;
	}
	return f1.bridge->equals(f2.bridge);
} 
friend bool operator!=(FunctionPtr const& f1, FunctionPtr const& f2) {
	return !(f1 == f2);
}
```
구현은 되었으나 이방법의 구현은 하나의 문제점이 있다.

FunctionPtr에 적합한 연산자가 아닌 객체를 활당하면 이 프로그램은 컴파일시 실패하게 된다.

예로 FunctionPtrs에 ==연산자가 없는 함수 객체를 활당하게 되면 초기화 연산을 화면서 컴파일이 실패한다.

대표적인 예로 std::vector이 있다.

이 문제는 operator ==가 타입을 지우기 때문에 발생하는 문제이다.

FunctionPtrs이 할당되거나 초기화 되면 함수 타입을 잃어 버리기때문에 활당하기전에 타입을 캡처해놓아야한다.

이 정보는 operator ==도 있는지 확인 해야한다.


이를 SFINAE를 이용하여 operator==을 호출하기 전에 사용할수 있는지 알수 있다.
```

#include <utility> // for declval()
#include <type_traits> // for true_type and false_type
template<typename T>
class IsEqualityComparable
{
private:
	// test convertibility of == and ! == to bool:
	static void* conv(bool); // to check convertibility to bool
	template<typename U>
	static std::true_type test(decltype(conv(std::declval<U
		const&>() ==
		std::declval<U
		const&>())),
		decltype(conv(!(std::declval<U
			const&>() ==
			std::declval<U
			const&>())))
	);
	// fallback:
	template<typename U>
	static std::false_type test(…);
public:
	static constexpr bool value = decltype(test<T>(nullptr,
		nullptr))::value;
};
```
IsEqualityComparable 섹션 19.4.1에  소개된 expressiontesting traits 기법을 사용했다

test를 오버로딩 하고 하나는 decletype으로 래핑을 했고 하나는 임의의 인수를 사용했다.

첫 test()함수는 두개의 객체가 ==연산자를 사용할수 있다면 그 결과가 bool로 암시적으로 변환이 된다.

그리고 적절한 경우가 없는경우 예외를 던지게 할수 있다.

```
#include <exception>
#include "isequalitycomparable.hpp"
template<typename T, bool EqComparable = IsEqualityComparable<T>::value>
struct TryEquals
{
	static bool equals(T const& x1, T const& x2) {
		return x1 == x2;
	}
};
class NotEqualityComparable : public std::exception
{ };
template<typename T>
struct TryEquals<T, false>
{
	static bool equals(T const& x1, T const& x2) {
		throw NotEqualityComparable();
	}
};
```

마지막으로 FunctionPtr에서 TryEquals을 사용해서 operator==을 사용하는지 알수 있습니다.

```
virtual bool equals(FunctorBridge<R, Args…> const* fb) const
override {
	if (auto specFb = dynamic_cast<SpecificFunctorBridge const*>
		(fb)) {
		return TryEquals<Functor>::equals(functor, specFb->functor);
	}
	//functors with different types are never equal:
	return false;
}
```
### 22.6 Performance Considerations

Type erasure를 통해 정적다형성과 동적다형성의 장점을 둘다 취할수 있습니다.

하지만 Type erasure을 사용하면 정적 함수 다형성과 비슷한 코드를 만들어준다.

이유는 가상함수를 통해 dispach를 하기 때문이다.

따라서 static 다형성의 이점인 inline이 손실된다.이러한 성능 저하는 응용프로그램마다 다르지만

가상 함수의 호출 비용과 호출되는 작업의 양도 고려해야한다.

왜냐하면 정적 다형성보다 매우 느려질수 있다. 

만약에 데이터베이스나 컨테이너 정렬,인터페이스 update를 하게되면 이 비용은 측정이 불가능하다.


<hr/>


## 메타...프로그래밍..?

 메타프로그래밍은 프로그램 프로그래밍이라 하면 다른말로는 시스템 프로그래밍을 통해 원하는 구현 코드를 생성하는 거이다.

### 23.1 모던한 C++ 프로그래밍

메타프로그래밍 기술은 시간의 지남에 따라 발전하였다. 이거에 대해 한번 논의해보자

### 23.1.1 Value 메타프로그래밍

처음에는 재귀적 템플릿 인스턴스화를 사용하여 컴파일 타임에 제곱 근을 계산했다.

하지만 C++14부터 컴파일 타입에 제곱근를 구하는 방법으로 아래와 같이 쓸수 있다.

```
template<typename T>
constexpr T sqrt(T x)
{
	// handle cases where x and its square root are equal as a
	special case to simplify
		// the iteration criterion for larger x:
		if (x <= 1) {
			return x;
		}
	// repeatedly determine in which half of a [lo, hi] interval
	the square root of x is located,
		// until the interval is reduced to just one value:
		T lo = 0, hi = x;
	for (;;) {
		auto mid = (hi + lo) / 2, midSquared = mid * mid;
		if (lo + 1 >= hi || midSquared == x) {
			// mid must be the square root:
			return mid;
		}
		//continue with the higher/lower half-interval:
		if (midSquared < x) {
			lo = mid;
		}
		else {
			hi = mid;
		}
	}
}
```
이 알고리즘은 반으로 쪼갠후에  답이 나올때까지 반복 수행한다. 

이 함수는 런타임도 돌수 있다.
```
static_assert(sqrt(25) == 5, ""); // declares array of elements(compile time)
static_assert(sqrt(40) == 6, ""); // declares array of elements(compile time)
std::array<int, sqrt(40) + 1> arr; //declares array of elements(compile time)

long long l = 53478;
std::cout << sqrt(l) << ’\n’; //prints 231 (evaluatedat run time)
```
이 함수는 런타임과 컴파일 타임때 모두 돌수 있어 이식성이 매우 좋다

또한 흑마법(tempalte magic)이 전혀 보이지 않고 일반적인 C++코드여서 읽기도 편하다.

### 23.1.2 Type Metaprogramming

19장에서 타입 특징을 특정하는 법에 대해 설명하였습니다. 

템플릿 기반의 메타프로그래밍은 재귀형태를 통해 훨씬 더 복잡한 타입들 도 수행이 가능합니다.
```
// primary template: in general we yield the given type:
template<typename T>
struct RemoveAllExtentsT {
	using Type = T;
};
// partial specializations for array types (with and without
bounds):
template<typename T, std::size_t SZ>
struct RemoveAllExtentsT<T[SZ]> {
	using Type = typename RemoveAllExtentsT<T>::Type;
};
template<typename T>
struct RemoveAllExtentsT<T[]> {
	using Type = typename RemoveAllExtentsT<T>::Type;
};
template<typename T>
using RemoveAllExtents = typename RemoveAllExtentsT<T>::Type;
```
RemoveAllExtents 메타함수는 배열형태를 제거해준다. 

이 함수는 다음과 같이 사용이 가능하다.
```
RemoveAllExtents<int[]> // yields int
RemoveAllExtents<int[5][10]> // yields int
RemoveAllExtents<int[][10]> // yields int
RemoveAllExtents<int(*)[5]> // yields int(*)[5]
```

이 함수는 부분 특수화를 통해 재귀적으로 호출해서 배열 레이어를 제거해준다.

아직까지는 배열 int[15]와같은 스칼라값만 가능하다.

메타프로그래밍도 같이 컨테이너 타입이 추가될때마다 위와같은 규칙들이 추가될거이다.

하지만 최신 C++에는 이러한 컨테이너도 처리할수 있도록 Typelist<…>를 만들었다. 이에 관한 설명은 24장에 있다.

### 23.1.3 Hybrid Metaprogramming

value와 type 메타프로그래밍은 컴파일 타임떄 계산이 가능하지만 타입과 상수로 되어있는 곳이면  런타임 코드에 

메타 프로그래밍을 할수 있다. 이를  Hybrid Metaprogramming 라고 한다.

이 원리를 설명하기 위해 두개의 std::array를 이용해 내적을 계산하는 예제로 설명한다.
```
namespace std {
	template<typename T, size_t N> struct array;
}

template<typename T, std::size_t N>
auto dotProduct(std::array<T, N> const& x, std::array<T, N>
	const& y)
{
	T result{};
	for (std::size_t k = 0; k<N; ++k) {
		result += x[k] * y[k];
	}
	return result;
}
```
흔하게 쓰는 반복문은 컴파일라가 오베헤드를 유발하게 하는 코드를 생성하지만.
```
result += x[0]*y[0];
result += x[1]*y[1];
result += x[2]*y[2];
result += x[3]*y[3];
```
최신 컴파일러는 위와같이 반복문을 최적화 합니다.

하지만 이 반복을 제거 하기위해 다음과 같이 만들어 봅니다.

```
template<typename T, std::size_t N>
struct DotProductT {
	static inline T result(T* a, T* b) {
		return *a * *b + DotProduct<T, N - 1>::result(a + 1, b + 1);
	}
};
// partial specialization as end criteria
template<typename T>
struct DotProductT<T, 0> {
	static inline T result(T*, T*) {
		return T{};
	}
};
template<typename T, std::size_t N>
auto dotProduct(std::array<T, N> const& x,
	std::array<T, N> const& y)
{
	return DotProductT<T, N>::result(x.begin(),
		y.begin());
}
```

내적을 구하는 구현부의 코드를 다음과 정의해 두었습니다.

이 방식은 재귀호출방식이며 재귀호출을 종료하기 위해 부분 특수화를 적용했다..

중요한점은 컴파일러가 정적 멤버의 결과를 모두 inline해야한다. 다행스럽게

대부분의 컴파일러는 최적화를 진행할때 자동적으로 inline 된다.

이 코드는 런타임의 계산하고 컴파일 타임을 혼합하는 형태를 가지게 됩니다.

이를 Hybrid Metaprogramming라고 한다.

가장 Hybrid Metaprogramming을 잘쓴것은 예는 C++ standard 라이브러리에 있는 tuple이다.

```
std::tuple<int, std::string, bool> tVal{42, "Answer", true};
```

이는 25장에 자세히 다룰것입니다.

또한 std::variant은 26장에...

### 23.1.4 Hybrid Metaprogramming for Unit Types


Hybrid Metaprogramming의 좋은점은 결과의 타입을 다른 타입의 결과를 계산할수 있다.

런타임에 값이 계산되지만 결과의 단위는 컴파일타임에 결정이 된다.

간단한 예제로 값은 유지하면서 결과의 단위는 추적해 갈수 있다.

예로 밀리초는 1/1000으로 1분의 시간이 주어지면 60/1로 나타낼수 있다.

```
template<unsigned N, unsigned D = 1>
struct Ratio {
	static constexpr unsigned num = N; // numerator
	static constexpr unsigned den = D; // denominator
	using Type = Ratio<num, den>;
};
```

이제 이 두 단위를 컴파일 시간에 계산을 정의할수 있다.

```
template<typename R1, typename R2>
struct RatioAddImpl
{
private:
	static constexpr unsigned den = R1::den * R2::den;
	static constexpr unsigned num = R1::num * R2::den + R2::num *R1::den;
public:
	typedef Ratio<num, den> Type;
};
// using declaration for convenient usage:
template<typename R1, typename R2>
using RatioAdd = typename RatioAddImpl<R1, R2>::Type;
```

이제 컴파일 타임에 두 단위의 합계를 계산할수 있다.

```
using R1 = Ratio<1,1000>;
using R2 = Ratio<2,3>;
using RS = RatioAdd<R1,R2>; //RS has type Ratio<2003,2000>
std::cout << RS::num << ’/’ << RS::den << ’\n’; //prints 2003/3000
using RA = RatioAdd<Ratio<2,3>,Ratio<5,7>>; //RA has type Ratio<29,21>
std::cout << RA::num << ’/’ << RA::den << ’\n’; //prints 29/21
```

이제 Ratio<>을 통해서 임의의 타입과 값을 매개변수로 정의할수 있게되었다.

```
template<typename T, typename U = Ratio<1>>
class Duration {
public:
	using ValueType = T;
	using UnitType = typename U::Type;
private:
	ValueType val;
public:
	constexpr Duration(ValueType v = 0)
		: val(v) {
	}
	constexpr ValueType value() const {
		return val;
	}
};
```

다음은 Durations의 operator+에 대한 정의입니다. 

```
template<typename T1, typename U1, typename T2, typename U2>
auto constexpr operator+(Duration<T1, U1> const& lhs,
	Duration<T2, U2> const& rhs)
{
	// resulting type is a unit with 1 a nominator and
	// the resulting denominator of adding both unit type fractions
	using VT = Ratio<1, RatioAdd<U1, U2>::den>;
	// resulting value is the sum of both values
	// converted to the resulting unit type:
	auto val = lhs.value() * VT::den / U1::den * U1::num +
		rhs.value() * VT::den / U2::den * U2::num;
	return Duration<decltype(val), VT>(val);
}
```

+operator에서 중요한점은 U1과 U2가 다른 단위일수 있고. 그 duration의 단위를 


we use these unit types to compute the resulting duration to have a unit type that is the corresponding unit fraction 

이제 다음과 같은 코드를 컴파일 할수 있다.

```
int x = 42;
int y = 77;
auto a = Duration<int, Ratio<1, 1000>>(x); // x milliseconds
auto b = Duration<int, Ratio<2, 3>>(y); // y 2/3 seconds
auto c = a + b; //computes resulting unit type 1/3000 seconds
				//and generates run-time code for c = a*3 + b * 2000
```

여기서 가장 중요한점은 변수 c의 값을 컴파일시 단위를 1/3000으로 계산하고 런타임에 결과값을 계산한다는 점이다.

vaule의 타입이 템플릿 매개변수가 되어 int이외에 값을 이질적인 (double)값을 계산 할수도 있다.


```
auto d = Duration<double, Ratio<1, 3>>(7.5); // 7.5 1/3 seconds
auto e = Duration<int, Ratio<1>>(4); // 4 seconds
auto f = d + e; //computes resulting unit type 1/3 seconds
				// and generates code for f = d + e*3
```

또한 operator+에 constexpr이 붙어 컴파일러는 컴파일 시간에 값을 알면 컴파일 타임에 값을 계산할수 있게된다.

이 기술은 std::chrono에서 사용하고 있다.

### 23.2 The Dimensions of Reflective Metaprogramming

value 메타 프로그래밍은 재귀 템플릿에도 동작이 가능하다.

C++11에는 constexpr함수가 도입되기 전에는 선택이였다.

다음은 그 예이다. 

```
// primary template to compute sqrt(N)
template<int N, int LO = 1, int HI = N>
struct Sqrt {
	// compute the midpoint, rounded up
	static constexpr auto mid = (LO + HI + 1) / 2;
	// search a not too large value in a halved interval
	static constexpr auto value = (N<mid*mid) ? Sqrt<N, LO, mid -
		1>::value
		:
		Sqrt<N, mid, HI>::value;
};
// partial specialization for the case when LO equals HI
template<int N, int M>
struct Sqrt<N, M, M> {
	static constexpr auto value = M;
};
```

이 제곱근을 구하는 메타프로그래밍은 23.1.1에 있는 제곱근 구하는 방식하고 동일한 알고리즘이다.

하지만 메타함수은 Nontype 템플릿 인자 대신에 한수 인자를 사용하고 지역변수로 재구성된다.
However, the input to the metafunction is
a nontype template argument instead of a function argument, and the “local variables” tracking the bounds to the interval are also recast as nontype template arguments

이 방식은 constexpr보다는 친숙하지 않은 방법이지만 그럼에도 컴파일러가 리소스를 어떻게 사용하는지 나중에 

분석할수 있다.

하여튼 메타프로그래밍은 계산을 주로하는 엔진에서는 많은 잠재성을 가지고 있지만.

하나의 방법(dimension)만 고려대는것만 아니라 총 3개 가지를 고려해야한다.

* Computation
* Generation
* Reflection

Reflection은 프로그래밍 특징을 측정하는 기능이고 Generation은 프로그램의 대한 추가 코드를 생성하는 기능을 말한다.

이 두가지를 Computation이라고 한다. 재귀적으로 인스턴스화 시키고 constexpr을 계산한다.

## 23.3 The Cost of Recursive Instantiation


```
// primary template to compute sqrt(N)
template<int N, int LO = 1, int HI = N>
struct Sqrt {
	// compute the midpoint, rounded up
	static constexpr auto mid = (LO + HI + 1) / 2;
	// search a not too large value in a halved interval
	static constexpr auto value = (N<mid*mid) ? Sqrt<N, LO, mid -
		1>::value
		:
		Sqrt<N, mid, HI>::value;
};
// partial specialization for the case when LO equals HI
template<int N, int M>
struct Sqrt<N, M, M> {
	static constexpr auto value = M;
};
```

책 537쪽에 소개된 Sqrt<> 템플릿을 분석해보자.

primary template은 재귀적으로 템플릿 인자 N과 LO ,HI  매개변수 변수로 재귀 호출을 하여 계산한다.

하나의 인수가 호출이되면 제곱근은 최대값인 1 된다.

이진 탐색 기법으로 재귀를 진행한다. 

이 템플릿 인스턴스화 비용은 매우 비쌉니다.

sqrt템플릿을 사용하여 세부사항을 살펴 보자.

```
#include <iostream>
#include "sqrt1.hpp"
int main()
{
	std::cout << "Sqrt<16>::value = " << Sqrt<16>::value << ’\n’;
	std::cout << "Sqrt<25>::value = " << Sqrt<25>::value << ’\n’;
	std::cout << "Sqrt<42>::value = " << Sqrt<42>::value << ’\n’;
	std::cout << "Sqrt<1>::value = " << Sqrt<1>::value << ’\n’;
}
```
아래의 표현식은 
```
Sqrt<16>::value

Sqrt<16,1,16>::value

mid = (1+16+1)/2 = 9

value = (16<9*9) ? Sqrt<16,1,8>::value: Sqrt<16,9,16>::value 
= (16<81) ? Sqrt<16,1,8>::value : Sqrt<16,9,16>::value 
= Sqrt<16,1,8>::value

->

mid = (1+8+1)/2
= 5
value = (16<5*5) ? Sqrt<16,1,4>::value
: Sqrt<16,5,8>::value
= (16<25) ? Sqrt<16,1,4>::value
: Sqrt<16,5,8>::value
= Sqrt<16,1,4>::value

->

mid = (1 + 4 + 1) / 2 = 3
value = (16<3 * 3) ? Sqrt<16, 1, 2>::value
	: Sqrt<16, 3, 4>::value
	= (16<9) ? Sqrt<16, 1, 2>::value
	: Sqrt<16, 3, 4>::value
	= Sqrt<16, 3, 4>::value
```

이런식으로 확장이 되서 결국 value = 4가 됩니다.

### 23.3.1 Tracking All Instantiations

컴팡일러가 표현식을 평가하게되면 위에 16의 제곱근을 인스턴스화 하게됩니다.

```
(16<=8*8) ? Sqrt<16,1,8>::value
: Sqrt<16,9,16>::value
```
it instantiates not only the templates in the positive branch but also those in the
negative branch (Sqrt<16,9,16>).


class의 operator::를 사용했기떄문에 클래스의 모든 요소가 전부 인스턴스화 된다.

예로  Sqrt<16,9,16>을 인스턴스화 하게되면  Sqrt<16,9,12> , Sqrt<16,13,16> 전부 인스턴스화 하게된다.

다른말로하면 N의 제곱근을 구할려면 N의 두배를 인스턴스화가 필요하다는 거다.

다행히 아래는 인스턴스를 줄이는 기술이 하나를 적용시킨 소스이다.

```

#include "ifthenelse.hpp"
// primary template for main recursive step
template<int N, int LO = 1, int HI = N>
struct Sqrt {
	// compute the midpoint, rounded up
	static constexpr auto mid = (LO + HI + 1) / 2;
	// search a not too large value in a halved interval
	using SubT = IfThenElse<(N<mid*mid),
		Sqrt<N, LO, mid - 1>,
		Sqrt<N, mid, HI>>;
	static constexpr auto value = SubT::value;
};
// partial specialization for end of recursion criterion
template<int N, int S>
struct Sqrt<N, S, S> {
	static constexpr auto value = S;
};
```
```
#ifndef IFTHENELSE_HPP
#define IFTHENELSE_HPP
// primary template: yield the second argument by default and rely on
// a partial specialization to yield the third argument
// if COND is false
template<bool COND, typename TrueType, typename FalseType>
struct IfThenElseT {
	using Type = TrueType;
};
// partial specialization: false yields third argument
template<typename TrueType, typename FalseType>
struct IfThenElseT<false, TrueType, FalseType> {
	using Type = FalseType;
};
template<bool COND, typename TrueType, typename FalseType>
using IfThenElse = typename IfThenElseT<COND, TrueType,
	FalseType>::Type;
#endif //IFTHENELSE_HPP
```
IfThenElse이 가장 중요한 템플릿인데 이 템플릿은 19.7.1절에 소개했다.

IfThenElse템플릿은 bool을 주어 두 타입을 선택하는 템플릿 이다.

이 값이 참이면 첫 타입이 선택이 되고 그러지 않으면 두번째 타입이 선택이 된다.

그 결과 선택된 context가 인스턴스화가 된다.


```
using SubT = IfThenElse<(N<mid*mid),
	Sqrt<N, LO, mid - 1>,
```

이 결과 log2(N) 만큼 인스턴스화가 됩니다.

### 23.4 Computational Completeness

sqrt<> 템플릿 메타프로그램은 다음과 같은 특징이 있다.

* State variables: The template parameters
* Loop constructs: Through recursion
* Execution path selection: By using conditional expressions or specializations
* Integer arithmetic

만약에 재귀에 제한이 없다면 수많은 변수들이 만들수 있고 무엇이든 계산이 가능하다.

하지만 템플릿을 사용하기 때문에 편하지는 않다.

또한 컴파일자체에 리소스를 소모하기때문에 속도를 늦추기도 한다.

c++ 표준에서는 최소 1024레벨의 재귀 인스턴스화가 허용되도록 한다. 

이 정도 작업량은 메타프로그래밍에서 충분하게 작용한다.

하지만 템플릿으로 구현되지 않는 경우가 있다.

 In particular, they can sometimes be hidden in the innards of more conventional templates to squeeze more performance out of critical algorithm implementations.

### 23.5 Recursive Instantiation versus Recursive Template Arguments

다음 재귀 템플릿을 봐라

```
template<typename T, typename U>
struct Doublify {
};\

struct Trouble {
	using LongType = Doublify<typename Trouble<N - 1>::LongType,
		typename Trouble<N - 1>::LongType>;
};
template<>
struct Trouble<0> {
	using LongType = double;
};
Trouble<10>::LongType ouch;
```


Trouble<10>::LongType을 사용하게 되면 Trouble<8>,Trouble<7>의 재귀 인스턴스화를 만들어 Trouble을 더 복잡하게 인스턴스하게 된다.

| Type Alias | Underlying Type | 
| :--------: | :--------: |
| Trouble<0>::LongType | double | 
| Trouble<1>::LongType | Doublify<double,double> |
| Trouble<2>::LongType | Doublify<Doublify<double,double>,Doublify<double,double>> |

테이블에서 볼수 있듯이 타입의 복잡성은 기하 급후적으로 증가되고 C++컴파일러를 괴롭게 만드는 요인이 된다.

여기서 또 문제되는 거는 컴파일러가 타입을 훼손하게 된다는 점이다.

이 훼손된 타입은 어떻게든 이 템플릿을 인코딩하며 초기 C++에서는 탬플릿 ID에 길이에 비례하는 인코딩을 한다.

Trouble<10>::LongType 같은 경우 컴파일러가 1000자 캐릭터를 사용한다.

최신 C++구현에는 압축기술이 좋아져 타입의 이름이 상당히 줄여들었다.

또한  low-level 코드가 필요없으면 훼손된 템플릿을 만들지 않는다.

때문에 템플릿 인스턴스를 중접보다는 재귀적으로 인스턴스하는게 좋다.



##  Enumeration Values versus Static Constants

초기 c++에는 클래스안에 열거값을 넣어 메타프로그래밍을 구현하는게 유일하였다.

아래가 그 예이다.
```
meta / pow3enum.hpp
// primary template to compute 3 to the Nth
template<int N>
struct Pow3 {
	enum { value = 3 * Pow3<N - 1>::value };
};
// full specialization to end the recursion
template<>
struct Pow3<0> {
	enum { value = 1 };
};
```

C++ 98에는 클래스의 정적 상수 초기화라는 개념을 도입하여 다음과 같이 작성할수 있었다.

// primary template to compute 3 to the Nth
template<int N
	struct Pow3 {
	static int const value = 3 * Pow3<N - 1>::value;
};
// full specialization to end the recursion
template<>
struct Pow3<0> {
	static int const value = 1;
};

하지만 이거의 단점이 있다 정적변수는 기본적으로 Lvalue이기 때문에 다음과 같이 선언하게 되면

```
void foo(int const&);
```

메타프로그래밍 결과를 다음과 같이 전달한다.

```
foo(Pow3<7>::value);
```

컴파일러는 Pow3<7>::value의 주소를 넘겨줄떄 강제로 static한 멤버변수를 정의하고 활당한다.

그 결과 순수하게 컴파일타임에 계산하지 않게된다.

하지만 열거형은 lvalue가 아니다. 그래서 reference로 전달할떄 상수로 넘어가고 static한 메모리를 사용하지 않았다.

그래서 이책의 초판에는 열거형을 사용했습니다.

C++ 11에서는 constexpr static data가 도입이 되서 더이상 정적데이터 타입은 정수형에 제한되지 않게 되었다.

주소 문제는 아직 해결하지 못했지만 이제는 메타프로그래밍을 구하는데 일반적인 방법이 되었습니다.

C++17에서는 inline static data member라는 개념을 추가해서 메모리를 활당하는 해결할수 있게 되었다.

