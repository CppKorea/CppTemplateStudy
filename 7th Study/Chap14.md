
18/03/24 진행: [박동하](https://github.com/luncliff) luncliff@gmail.com
 
# Chapter 14: 실체화
> Instantiation

C++ 템플릿의 실체화는 기본적이지만 꽤 복잡미묘합니다.
그 이유 중 하나는, 템플릿을 통해 생성된 개체들이 소스 코드 상의 한 곳에 위치하는게 아니기 때문입니다. 

템플릿의 위치, 템플릿이 사용된 위치, 템플릿 인자가 정의된 위치 모두 고려되어야 합니다.

## 1. 필요에 의한 실체화
> On-Demand Instantiation

템플릿 특수화가 사용되면, C++ 컴파일러는 템플릿 인자들을 치환하여 특수화를 생성합니다. _implicit_ 혹은 _automatic_ 실체화라고도 합니다.

이 말은, C++ 컴파일러가 템플릿의 선언 뿐만 아니라 전체의 정의를 필요로 한다는 말이기도 합니다.

```c++
template <typename T> 
class C; // #1 템플릿 선언

C<int>*p = 0; // #2 포인터. 아직 정의가 필요하지는 않음

template <typename T>
class C{
    public:
        void f();   // #3 멤버 함수 선언
}; // #4 클래스 템플릿 정의 완료

void g(C<int>& c)   // #5 레퍼런스. 
                    // 포인터와 동일하게 클래스 템플릿 선언만 사용
{
    c.f();  // #6 멤버함수 호출. 
            // 이때는 클래스 템플릿 정의를 사용한다
            // C::f()의 정의가 필요 
}

// #6에서 필요로 했던 정의
template <typename T>
void C<T>::f()
{
    // ...
}
```

요컨대, 클래스의 크기가 필요하거나 멤버에 접근하는 경우 전체 정의가 필요합니다. 클래스의 크기에 대해선, 아래와 같은 코드를 예로 들 수 있습니다.
```c++
C<void>* p = new C<void>{};
```
`operator new()`는 메모리 할당을 위해 크기를 알아야 하기 때문에, 템플릿 클래스 `C`의 `void` 특수화에 대한 정의가 필요하게 됩니다.

특히 #6과 같은 경우엔, 함수 `g` 에서 `C<T>::f()`를 볼 수 있고(visible), 에 접근할 수 있는지 확인할 수 있어야 합니다(`private`, `protected`).

## 2. 게으른 실체화
> Lazy Instantiation

이 경우는 템플릿이 아닌 C++ 클래스를 사용하는 것과 다르지 않습니다. 많은 경우 클래스 타입은 _완전해야(complete)_ 하기 때문에, 컴파일러는 클래스 템플릿 정의로부터 완전한 클래스 코드를 생성해냅니다. 

### 2.1 부분/전체 실체화
> Partial and Full Instantiation

모든 경우에 컴파일러가 클래스/함수 전체에 대한 정의를 치환해야 하는 것은 아닙니다. 예를들어,

```c++
template <typename T>
T f(T p){
    return 2*p;
}

decltye(f(2)) x = 2;
```
변수 `x`를 선언하는 부분에서는 `f()`의 전체 정의가 필요한 것은 아닙니다. `f()`의 선언만 치환하게 됩니다. 이를 부분 실체화(partial instantiation)이라고 합니다.

클래스 템플릿도 이와 유사합니다.
```c++
template <typename T>
class Q
{
    using Type = typename T::Type;
};

Q<int>* p = 0; // OK: Q<int>의 몸체는 치환되지 않습니다.
```
이 경우, `int`는 `::Type`을 정의하지 않기 때문에, 템플릿 전체가 치환된다면 에러가 발생할 것입니다. 하지만 이 예제에서는 완전할 필요가 없고, 때문에 이 코드는 허용됩니다.

C++ 에서 "템플릿 실체화"라는 용어가 쓰인다면 보통은 전체 실체화(full instantiation)를 의미합니다.

### 2.2 Instantiated Components

템플릿이 실체화될 때는, 각 멤버의 '선언'이 실체화됩니다. 달리 말해, '정의'는 실체화되지 않습니다.

```c++
template <typename T>
class Safe {
};

template <int N>
class Danger {
    int arr[N]; // N이 음수가 아닌 한 문제 없다...
};

template <typename T, int N>
class Tricky {
    public:
        void noBodyHere(Safe<T> = 3); // OK: 실제로 호출되지만 않는다면...
        
        void inclass(){ // OK: 정의는 나중에 호출할 떄 생성된다.
                        //     그 시점에  N<=0 이면 에러가 된다.
            Danger<N> noBoomYet;
        }

        struct Nested   
        {
            Danger <N> pfew; // OK: Nested가 사용되지 않고 N이 양수라면 문제 없다.
        };

        union{
            Danger<N> anonymous; // OK: Tricky가 N<=0 에서 *실체화*되지 않는 한 문제 없다. 
            int align;
        };

        void unsafe(T (*p)[N]); // OK: Tricky가 N<=0 으로 *실체화*되지 않는 한 문제 없다.

        void error(){
            Danger<-1> boom; // ERROR: 언제나 에러.
        }
}
```

C++ 컴파일러가 템플릿의 문법과 보통의 문맥적 제약을 확인할 떄는, "최선의 경우"를 전제합니다. 예를 들면, `Danger::arr`의 길이는 0 혹은 음수가 될 수 있지만, 그렇지 않을 것이라고 전제합니다.

## 3. C++의 실체화 모델
> The C++ Instantiation Model

템플릿의 실체화 과정은 템플릿 개체로부터 타입, 함수, 또는 변수를 얻어오는 과정이라고 할 수 있습니다. 직관적인 것 같지만, 실제로는 명확하게 정의되어야 하는 부분들이 있죠. 

### 3.1 2단계 탐색
> Two Phase Lookup

13장에서 템플릿 인자에 따라 달라지는 타입들은 템플릿 구문분석 단계에서는 확인할 수 없다는 것을 확인했습니다. 대신, 이 타입들은 실체화 지점에서 다시 한번 탐색하게 됩니다. 템플릿 인자와 무관한 경우라면, 탐색을 일찍 수행해서 템플릿이 처음 확인된 순간에 에러를 찾아냅니다.


 1. 템플릿 구문분석(Parsing)
 1. 템플릿 실체화(Instantiation)


템플릿을 분석할 때는, 템플릿 인자와 무관한 이름들을 먼저 탐색합니다. 이때는 unqualified dependent name들이 탐색됩니다. 탐색에는 Ordinary Lookup 규칙을 사용합니다. 적용할 수 있다면, ADL 역시 적용합니다. (즉, )

템플릿을 실체화 할 때는, 실체화 지점(Point Of Instantiation)에서 탐색을 수행합니다. 이때는 dependent qualified name을 탐색하게 됩니다. 앞서 Parsing 단계에서 찾지 못한 unqualified dependent name에 대해서는 ADL을 수행하게 됩니다.

```c++
namespace N{
    template <typename> void g(){}
    enum E { e };
}

template <typename> void f(){}

template <typename T> void h(T P){
    f<int>(p);  // #1 
    g<int>(p);  // #2 ERROR
}

int main(){
    h(N::e); // 템플릿 함수 호출. T == N::E
}
```
#### Phase 1
`f<int>()`를 분석하는 과정에서 `f()`가 템플릿의 이름인지를 확인합니다. 템플릿인 경우 에러는 발생하지 않습니다. 같은 원리로 `g()`는 템플릿 함수인지 확인할 수 없기 때문에 `g`를 따라오는 `<`는 less-than으로 인식됩니다.

#### Phase 2
사실 `h()`가 1단계에서 에러없이 분석되면, `main`에서의 호출은 ADL로 이어지고, `N::g<T>()`를 탐색하는데 성공하게 됩니다.


### 3.2 실체화 지점
> Points of Instantiation

템플릿의 POI는 인자가 치환되면서 생성된 코드가 삽입될 수 있는 지점을 의미합니다. 다수의 POI가 전부 사용되는 것은 아닙니다. One Definition Rule에 따라서, 컴파일러는 생성된 POI 중 하나를 선택하고, 바로 그 지점에 코드를 생성하게 됩니다.

#### 함수 템플릿
함수 템플릿의 POI는 가장 가까운 네임스페이스 범위 선언 혹은 함수 템플릿을 참조한 정의의 **바로 뒤**에 생성됩니다. 아래 예제코드에서는 #4에 해당합니다.

```c++
class MyInt
{
    public MyInt(int i);
};

MyInt operator -(MyInt const&);

bool operator > (MyInt const&, MyInt const&);

using Int = MyInt;

template <typename T>
void f(T i){
    if(i>0){
        g(-1);
    }
}
// #1
void g(Int)
{
    // #2
    f<Int>(42); // point of call 
    // #3
}
// #4 : 함수 템플릿을 참조한 정의의 바로 뒤
```

#### 클래스 템플릿

클래스 템플릿의 POI는 가장 가까운 네임스페이스 범위 선언 혹은 클래스 템플릿을 참조한 정의의 **바로 앞**에 생성됩니다. 아래 예제 코드에서는 #1에 해당합니다.

```c++
template <typename T>
class S {
    public:
        using I = int;
};

// #1 : 클래스 템플릿을 참조한 정의의 바로 앞
template <typename T>
void f()
{
    S<char>::I var1 = 41; 
    typename S<T>::I var2 = 42;
}

int main()
{
    f<double>();
}
// #2 
//  #2a
//  #2b
```

클래스 템플릿의 경우, 두번째 이후로 생성된 POI는 Primary POI의바로 앞에 배치됩니다. `#2`에 `f<double>()`의 POI가 생성되면, `S<double>`의 POI가 결정되어야 하는데, 이때 `S<double>`의 Primary POI는 `f<double>()`의 POI입니다.

 - `#1`: `S<char>`의 POI
 - `#2`
    - `#2a`: `S<double>`의 POI
    - `#2b`: `f<double>()`의 POI. `S<double>`의 Primary POI


### 3.3 포함 모델
> The Inclusion Model

현재 C++에서는 이 모델만을 사용하고 있는데, 간단히 말해 `#include`를 사용해 템플릿 코드가 모든 translation unit에서 볼 수 있도록 만드는 것입니다.

## 4. 구현 방법들
> Implementation Schemes

Instantiation 전략에 대해서...

### 4.1 탐욕적 전략
> Greedy Instantiation

### 4.2 조회 전략?
> Queried Instantiation

### 4.3 되풀이 전략
> Iterated Instantiation

## 5. 명시적 실체화
> Explicit Instantiation

템플릿 특수화의 POI를 명시적으로 생성하는 것도 가능합니다. 

```c++
template <typename T>
void f(T)
{

}

// 4가지 명시적 실체화
template void f<int>();
template void f<>(float);
template void f(long);
template void f(char);
```
예제의 4가지 모두 유효한 방법입니다. 클래스 템플릿의 멤버들도 이 방법을 사용해서 실체화할 수 있습니다.

```c++
template <typename T>
class S{
    public:
        void f(){
            // ...
        }
};

template void S<int>::f();
template class S<void>;
```
특수화된 클래스 템플릿을 실체화시킬 경우 모든 멤버에 특수화를 적용할 수 있습니다.

명시적으로 실체화된 템플릿은, 명시적으로 특수화되어서는 안됩니다. 그 반대도 마찬가지입니다. 이는 One Definition Rule을 위반하기 때문이죠.

### 5.1 수동 선언
> Manual Instantiation

### 5.2 명시적 실체화의 선언
> Explicit Instantiation Declarations
