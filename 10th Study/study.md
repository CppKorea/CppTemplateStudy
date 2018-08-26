# Chapter 18 The Polymorphic Power of Templates

## The Polymorphic Power of Templates

객체들의 타입이 다르면 똑같은 메시지가 전달되더라도 서로 다른 동작을 하는 것 즉 다형성은 객체 지향 기법에서 하나의 코드로 다양한 타입의 객체를 처리하는 기술입니다.

C ++에서 주로 클래스 상속 및 가상 함수를 통해 지원되고 이러한 메커니즘은 런타임에 이루어집니다.
이를 우리는 동적 다형성이라고 부릅니다.

C ++에서 일반적으로 다형성에 대해 이야기할 때는 동적 다형성을 의미합니다.

하지만 템플릿을 사용할 경우 다형성은 컴파일 시간에 처리되고 이를 정적 다형성이라고 부릅니다.

## 18.1 Dynamic Polymorphism

C++은 가상 함수를 통해서 다형성을 지원했습니다.

다형성은 관련 객체 간의 동일한 역할을 하는 부분(함수)를 찾아내어 관련 객체를 나타내는 하나의 베이스 클래스에서 가상 함수 인터페이스로 선언하는 것으로 구성됩니다.


```cpp
#include "coord.hpp"
// common abstract base class GeoObj for geometric objects
class GeoObj 
{
public:
  // draw geometric object:
  virtual void draw() const = 0;
  // return center of gravity of geometric object:
  virtual Coord center_of_gravity() const = 0;
  …
  virtual ~GeoObj() = default;
};

// concrete geometric object class Circle
// - derived from GeoObj
class Circle : public GeoObj 
{
public:
  virtual void draw() const override;
  virtual Coord center_of_gravity() const override;
  …
};

// concrete geometric object class Line
// - derived from GeoObj
class Line : public GeoObj 
{
public:
  virtual void draw() const override;
  virtual Coord center_of_gravity() const override;
  …
};
  …
```

객체가 생성된 이후 가상 함수 디스패치 메커니즘을 사용해서, 베이스 클래스 타입을 통한 참조/포인터를 통해서 파생 된 객체의 상속받은 함수를 호출할 수 있습니다.

```cpp
#include "dynahier.hpp"
#include <vector>

// draw any GeoObj
void myDraw (GeoObj const& obj)
{
  obj.draw(); // call draw() according to type of object
}

// compute distance of center of gravity between two GeoObjs
Coord distance (GeoObj const& x1, GeoObj const& x2)
{
  Coord c = x1.center_of_gravity() - x2.center_of_gravity();
  return c.abs(); // return coordinates as absolute values
}

// draw heterogeneous collection of GeoObjs
void drawElems (std::vector<GeoObj*> const& elems)
{
  for (std::size_type i=0; i<elems.size(); ++i) 
  {
    elems[i]->draw(); // call draw() according to type of element
  }
}

int main()
{
  Line l;
  Circle c, c1, c2;
  myDraw(l); // myDraw(GeoObj&) => Line::draw()
  myDraw(c); // myDraw(GeoObj&) => Circle::draw()
  distance(c1,c2); // distance(GeoObj&,GeoObj&)
  distance(l,c); // distance(GeoObj&,GeoObj&)
  std::vector<GeoObj*> coll; // heterogeneous collection
  coll.push_back(&l); // insert line
  coll.push_back(&c); // insert circle
  drawElems(coll); // draw different kinds of GeoObjs
}
```

다형성 인터페이스 함수는 draw() 및 centre_of_gravity() 입니다.

둘 다 가상 함수이고 mydraw(), distance() 및 drawElems() 함수 호출을 통해서 사용되는 예를 볼 수 있습니다.

함수 인자를 보면 GeoObj가 사용되는 것을 볼 수 있습니다.

컴파일 타임에 draw(), centre_of_gravity() 호출은 어떤 객체의 멤버 함수를 호출해야하는지 알 수 없습니다.
하지만 런타임에는 호출 객체의 가상 함수 테이블을 사용하여 파생 된 객체의 함수가 호출되는 것을 볼 수 있습니다.

Line 객체에 대한 mydraw() 함수에서의 obj.draw() 호출은 Line::draw()를 호출하고 Circle 객체에 대해서는  Circle::draw() 함수를 호출합니다.
마찬가지로 distance() 함수에서는 상속받은 각 하위 객체들의 centre_of_gravity ()가 호출됩니다.

동적 다형성의 가장 강력한 특징은 베이스 객체의 포인터 혹은 참조를 통해서 하위 객체(자식 클래스)의 함수를 호출할 수 있다는 것입니다.

## 18.2 Static Polymorphism

템플릿 또한 다형성을 표현하는데 사용될 수 있습니다.

```cpp
void myDraw (GeoObj const& obj) // GeoObj is abstract base class
{
  obj.draw();
}
```

를 대신해서

```cpp
template<typename GeoObj>
void myDraw (GeoObj const& obj) // GeoObj is template parameter
{
  obj.draw();
}
```
로 작성될 수 있습니다.

myDraw()의 두 구현된 코드를 비교해보면 주요 차이점은 베이스 클래스를 대신해서 GeoObj가 템플릿 매개 변수라는 것입니다.

두 함수에는 근본적인 차이점이 있습니다.

동적 다형성을 사용하면 런타임에 베이스 클래스의 가상 함수 테이블을 통한 myDraw() 함수를 호출할 수 있는 반면 템플릿은 하위 클래스들에 대해서 구체화된 myDraw<Line>(), myDraw<Circle>()를 직접 호출할 수 있다는 것입니다.

```cpp
#include "coord.hpp"
// concrete geometric object class Circle
// - not derived from any class
class Circle 
{
public:
  void draw() const;
  Coord center_of_gravity() const;
…
};

// concrete geometric object class Line
// - not derived from any class
class Line 
{
public:
  void draw() const;
  Coord center_of_gravity() const;
…
};
…
```

```cpp
#include "statichier.hpp"
#include <vector>

// draw any GeoObj
template<typename GeoObj>
void myDraw (GeoObj const& obj)
{
  obj.draw(); // call draw() according to type of object 
}

// compute distance of center of gravity between two GeoObjs
template<typename GeoObj1, typename GeoObj2>
Coord distance (GeoObj1 const& x1, GeoObj2 const& x2)
{
  Coord c = x1.center_of_gravity() - x2.center_of_gravity();
  return c.abs(); // return coordinates as absolute values
}

// draw homogeneous collection of GeoObjs
template<typename GeoObj>
void drawElems (std::vector<GeoObj> const& elems)
{
  for (unsigned i=0; i<elems.size(); ++i) 
  {
    elems[i].draw(); // call draw() according to type of element
  }
}

int main()
{
  Line l;
  Circle c, c1, c2;
  myDraw(l); // myDraw<Line>(GeoObj&) => Line::draw()
  myDraw(c); // myDraw<Circle>(GeoObj&) => Circle::draw()
  distance(c1,c2); //distance<Circle,Circle>(GeoObj1&,GeoObj2&)
  distance(l,c); // distance<Line,Circle>(GeoObj1&,GeoObj2&)
  // std::vector<GeoObj*> coll; //ERROR: no heterogeneous collection
  possible
  std::vector<Line> coll; // OK: homogeneous collection possible
  coll.push_back(l); // insert line
  drawElems(coll); // draw all lines
}
```

distance() 함수는 GeoObj1, GeoObj2 템플릿 매개 변수를 인자로 받습니다.

```cpp
distance(l,c); // distance<Line,Circle>(GeoObj1&,GeoObj2&)
```

템플릿 인자의 타입은 컴파일 시간에 결정됩니다.

템플릿 함수 인자 타입들은 베이스 클래스의 포인터와 레퍼런스 타입으로 제한될 필요가 없고 성능 및 타입에 대한 안정성 측면에서도 상당한 이점을 가질 수 있습니다.

## 18.3 Dynamic versus Static Polymorphism

```Terminology```

- 상속을 통해 구현된 다형성은 제한적이고 동적인 특징을 같습니다
   - 다형성 동작에 참여하는 인터페이스는 베이스 클래스의 디자인에 의해 미리 결정됩니다
   - 인터페이스의 바인딩이 런타임에 완료됩니다.

- 템플릿을 통해 구현 된 다형성은 제약이 적고 정적인 특징을 같습니다.
  - 다형성 동작에 참여하는 인터페이스가 미리 결정되지 않습니다.
  - 인터페이스의 바인딩이 컴파일 타임에 완료됩니다.

동적 다형성은 베이스 클래스 타입이 같아야 하는 등의 제한이 있는 반면 정적 다형성은 이러한 제약이 없습니다.

```Strengths and Weaknesses```

동적 다형성의 장점
- Heterogeneous collections are handled elegantly.
- 코드의 크기가 정적 다형성에 의해 생성되는 코드의 크기보다 작습니다.
- 소스 코드가 빌드된 형태로 제공될 수 있기 때문에 소스를 같이 배포할 필요가 없습니다.(템플릿 라이브러리는 일반적으로 구현 소스 코드를 배포)

정적 다형성의 장점

Collections of built-in types are easily implemented. 
   
베이스 클래스의 인터페이스를 상속받는 형태로 코드가 작성될 필요가 없습니다.

- 타입에 대한 코드가 직접 생성되기 때문에 수행 속도가 빠릅니다. (가상함수 테이블을 통한 호출이 없고, 비 가상함수로 코드가 생성될 경우 컴파일러에 의해서 인라인화 될 수 있습니다.)
- Concrete types that provide only partial interfaces can still be used if only that
part ends up being exercised by the application.

정적 다형성은 컴파일 타임에 바인딩이 처리되기 떄문에 동적 다형성에 비해서 안전합니다.

예를 들면, 템플릿으로 구현된 컨테이너의 인스턴스화가 진행될 때 다른 타입의 오브젝트가 사용되면 컴파일 에러가 발생하여 빌드 타임에 문제를 인지할 수 있습니다.

하지만 기본 클래스에 대한 포인터를 가지고 있는 컨테이너의 경우 의도와는 다르게 완전히 다른 타입의 객체를 가르킬 가능성도 있습니다.

정적 다형성에도 문제가 있습니다. 예를 들어 operator +가 있다고 가정하는 템플릿 코드에서 해당 연산자와는 관련이 없는 타입에 대해서도 컴파일러는  일단 인스턴스화를 시도합니다. (컴파일 에러 발생)

```Combining Both Forms.```

동적 다형성과 정적 다형성은 같이 사용될 수 있습니다.

베이스 클래스로 부터 다양한 종류의 객체(오브젝트)를 파생시킬 수 있고(동적 다형성), 여기에 템플릿을 사용해서 다양한 종류의 객체에 대한 코드를 작성할 수 있습니다.(정적 다형성)

## 18.4 Using Concepts

템플릿을 사용하는 정적 다형성은 대상이 되는 템플릿 코드를 인스턴화하고 인스턴스화된 인터페이스(코드)를 사용해서 바인딩을 수행합니다. 이 말은 곧 동적 다형성에서 작성해야 하는 베이스 클래스가 없다는 것입니다. 

대신 템플릿을 사용해서 인스턴스화된 코드가 유효하다면 코드는 사용될 수 있지만 그렇지 않을 경우에는 이해하기 힘든 컴파일 에러를 보거나 런타임에서 의도하지 않은 실행 결과를 가져올 수도 있습니다.

그래서 C++ 설계자들은  템플릿 매개 변수에 대해서 명시적으로 제약 사항을 걸 수 있는 기능을 연구하고 있습니다. 
이러한 인터페이스를 ```concept```라고 합니다.

```concept```는 템플릿 코드를 인스턴스화하기 위한 템플릿 인수가 가지는 일련의 제약 조건을 가르킵니다.

C++17에는 ```concept```은 포함되지 않았고 일부 컴파일러에서 제한적으로 지원하고 있습니다.

```concept```는 정적 다형성을 위한 일종의 "인터페이스"로 볼 수 있습니다.

```cpp
#include "coord.hpp"
template<typename T>
concept GeoObj = requires(T x) 
{
  { x.draw() } -> void;
  { x.center_of_gravity() } -> Coord;
  …
};
````

```concept``` 사용하여 GeoObj 개념을 정의합니다. 

위의 코드는 draw(), center_of_gravity() 멤버 함수를 호출할 수 있는 타입으로 제한합니다.

위에서 정의한 ```GeoObj concept``` 템플릿 매개변수에 제한을 두는 예제입니다.

```cpp
#include "conceptsreq.hpp"
#include <vector>
// draw any GeoObj
template<typename T>
requires GeoObj<T>
void myDraw (T const& obj)
{
  obj.draw(); // call draw() according to type of object
}

// compute distance of center of gravity between two GeoObjs
template<typename T1, typename T2>
requires GeoObj<T1> && GeoObj<T2>
Coord distance (T1 const& x1, T2 const& x2)
{
  Coord c = x1.center_of_gravity() - x2.center_of_gravity();
  return c.abs(); // return coordinates as absolute values
}

// draw homogeneous collection of GeoObjs
template<typename T>
requires GeoObj<T>
void drawElems (std::vector<T> const& elems)
{
  for (std::size_type i=0; i<elems.size(); ++i) 
  {
    elems[i].draw(); // call draw() according to type of element
  }
}
```

정적인 다형성 행동에 참여할 수있는 타입들과 관련하여 여전히 비 침습적입니다. 

(This approach is still noninvasive with respect to the types that can participate in the (static) polymorphic behavior.)

```cpp
// concrete geometric object class Circle
// - not derived from any class or implementing any interface
class Circle 
{
public:
  void draw() const;
  Coord center_of_gravity() const;
  …
};
```

## 18.5 New Forms of Design Patterns

C++에서 정적 다형성을 사용할 수 있게 되면서 고전적인 디자인 패턴을 구현하는 방법이 생깁니다.

상속을 이용한 Bridge pattern을 대신해서 템플릿을 사용해서 구현이 된다면, 구현 부 타입이 컴파일 시간에 알려짐으로 인해서 타입에 대한 안정성 및 상속 기반의 함수 호출등의 비용을 피할 수 있기 때문에 더 나은 성능을 기대할 수 있습니다

## 18.6 Generic Programming

정적 다형성은 generic programming 개념으로 이어지지만, 아직까지 generic programming 명확한 정의는 없습니다.

Generic programming은  효율적인 알고리즘, 데이터 구조 및 기타 소프트웨어 개념의 추상적인 표현을 다루는 컴퓨터 공학의 하위 분야입니다.

Generic programming은 도메인 개념의 집합을 대표하는 데 초점을 두고 있습니다.
(Generic programming focuses on representing families of domain concepts.)

C++의 관점에서 보면, generic programming은 템플릿을 사용한 프로그래밍으로 정의되기도 합니다. (반면에 객체 지향 프로그래밍은 상속을 통한 가상함수를 사용하는 프로그래밍으로 간주됩니다.)

이런점에서 C++ 템플릿은 generic programming의 한 예로 볼 수 있습니다.

하지만 generic programming은 추가적인 필수 요소가 있다고 생각하는 경우가 많습니다. (practitioners often think of generic programming as having an additional essential ingredient.)

템플릿은 다양한 조합을 가능하게 할 목적으로 설계되어야 합니다.

이 분야에서 가장 중요한 공헌은 STL(Standard Template Library)이며  C++ standard library에 적용되어 통합되었습니다.

STL은 알고리즘이라고 불리는 여러가지 유용한 operations을 제공하고, 컨테이너라고 불리는 선형 데이터 구조를 가지는 여러개의 개체 컬렉션을 제공하는 프레임워크 입니다.

알고리즘과 컨테이너는 모두 템플릿입니다.

하지만 중요한 것은 알고리즘은 컨테이너와는 독립적입니다(멤버함수가 아닙니다).대신 알고리즘은 모든 컨테이너에서 사용할 수 있도록 일반적인 방식으로 작성됩니다.

이렇게 하기 위해서는 STL의 디자이너들은 어떤 종류의 선형 컬렉션에도 적용될 수 있는 반복자라는 추상적인 개념 가져오게 됩니다.

기본적으로 컨테이너 operations의 컬렉션 별 측면(입력 반복자, 출력 반복자, 순방향 반복자, 양방향 반복자, 임의 접근 반복자)들이 반복자의 기능에 포함되어 있습니다.

결과적으로 우리는 컨테이너 내부적으로 값들이 어떻게 저장되는지 모르는 상태에서도 반복자를 통해서 최대값을 구할 수 있는 연산을 수행할 수 있습니다.

```cpp
template<typename Iterator>
Iterator max_element (Iterator beg /*refers to start of collection*/,  Iterator end /*refers to end of collection*/)
{
  // use only certain Iterator operations to traverse all elements
  // of the collection to find the element with the maximum value
  // and return its position as Iterator
…
}
```

선형 컨테이너마다 max_element ()와 같은 유용한 연산을 제공하는 대신, 컨테이너는 포함하고있는 값을 탐색할 수 있는 반복자와 그러한 반복자를 생성하는 멤버 함수를 제공합니다.

```cpp
namespace std 
{
template<typename T, …>
class vector 
{
public:
  using const_iterator = …; // implementation-specific iterator
  … // type for constant vectors
  const_iterator begin() const; // iterator for start of collection
  const_iterator end() const; // iterator for end of collection
…
};

template<typename T, …>
class list 
{
public:
  using const_iterator = …; // implementation-specific iterator
  … // type for constant lists
  const_iterator begin() const; // iterator for start of collection
  const_iterator end() const; // iterator for end of collection
  …
  };
}
```

```cpp
#include <vector>
#include <list>
#include <algorithm>
#include <iostream>
#include "MyClass.hpp"

template<typename T>
void printMax (T const& coll)
{
  // compute position of maximum value
  auto pos = std::max_element(coll.begin(),coll.end());
  // print value of maximum element of coll (if any):
  if (pos != coll.end()) 
  {
    std::cout << *pos << ’\n’;
  }
  else 
  {
    std::cout << "empty" << ’\n’;
  }
}

int main()
{
  std::vector<MyClass> c1;
  std::list<MyClass> c2;
  …
  printMax(c1);
  printMax(c2);
}
```

컨테이너의 시작과 끝을 나타내는 반복자를 인수로 하여 max_element() 함수 호출하여 모든 컨테이너의 최대 값을 구할 수 있습니다.

이러한 반복자를 매개 변수화함으로써 STL은 컨테이너의 operation(함수)가 늘어나는 것을 방지합니다.(알고리즘을 사용하여 대체)
컨테이너마다 operation(함수)을 구현하는 대신 우리는 모든 컨테이너에 사용할 수 있는 알고리즘을 구현할 수 있습니다.

generic glue는  컨테이너에서 제공하고 알고리즘에서 사용하는 반복자입니다.(The generic glue is the iterators, which are provided by the containers and used by the algorithms.)

이는 반복자가 컨테이너에서 제공되고 알고리즘에서 사용되는 특정 인터페이스를 가지고 있기 때문에 가능합니다.

이 인터페이스는 일반적으로 템플릿이 이 프레임 워크(STL)에 맞게 수행해야하는 일련의 제약 조건을 나타내는 concept 입니다.

사실, STL 같은 접근 방식과 같은 기능성은 동적 다형성으로 구현될 수 있다. 

그러나 실제로는 가상 함수 호출 메커니즘에 비해 반복자 개념이 너무 가볍기(수행속도?) 때문에 제한적으로 사용됩니다.

가상 함수를 기반으로 인터페이스 계층을 추가하면 수행 속도가 느려질 가능성이 큽니다.

generic programming은 컴파일 시간에 인터페이스 바인딩을 해결하는 정적 다형성에 의존하기 때문에 실용적입니다.

반면에, 컴파일 시 바인딩을 해결해야 하는 요구 사항은 객체 지향 설계 원칙과 다른면에서 새로운 디자인 원칙을 요구합니다.

## 18.7 Afternotes

컨테이너 타입은 템플릿을 도입하기위한 주된 동기였습니다.

템플릿 이전에는 다형성 계층 구조가 컨테이너에 널리 사용되었습니다. 보편적 인 예로 NIHCL (National Institutes of Health Class Library)이 있는데 이는  Smalltalk의 컨테이너 체계를 크게 개선시켰습니다.

C ++ 표준 라이브러리와 유사하지만, 

NIHCL은 반복자뿐만 아니라 다양한 컨테이너를 지원했습니다. 

그러나 구현 방식은 동적 다형성의 스몰 토크 스타일을 따랐습니다. 

반복자는 추상 기본 클래스인 Collection을 사용하여 여러 유형의 컬렉션에서 동작합니다.

```cpp
…
Iterator i1(c1);
Iterator i2(c2);
…
```

하지만 이 방식은 실행 시간과 메모리 사용량 측면에서 비용이 높았습니다. 

대부분의 operations이 가상 호출을 필요로하기 때문에 실행 시간이 일반적으로 C ++ 표준 라이브러리 보다 좋지 않습니다.

(C ++ 표준 라이브러리에서는 많은 작업이 인라인되고 반복자 및 컨테이너 인터페이스에는 가상 함수가 포함되지 않습니다).

(스몰 토크와 달리) 인터페이스가 한정되어 있기 때문에, 빌트인 타입을 더 큰 다형성 클래스로 래핑해야했기 때문에 코드 크기가 커질 수 있습니다.

C ++ 표준 라이브러리의 STL은 동적 다형성 컨테이너가 포함되지 않지만, 정적 다형성에 기반을 둔 다앙햔  컨테이너 및 반복자가 포함되어 있습니다.

# Chapter 19 Implementing Traits

형질은 템플릿에서 사용되는 매개 변수의 관리를 크게 향상시키는 C ++ 프로그래밍 장치입니다.

## 19.1 An Example: Accumulating a Sequence

### 19.1.1 Fixed Traits

배열을 사용해서 배열 첫 번째 요소의 포인터와 마지막 요소의 포인터를 인자를 받아서 합계를 구하는 코드입니다.

```cpp
#ifndef ACCUM_HPP
#define ACCUM_HPP

template<typename T>
T accum (T const* beg, T const* end)
{
  T total{}; // assume this actually creates a zero value
  while (beg != end) 
  {
    total += *beg;
    ++beg;
  }
  return total;
}
```
Section 5.2 on page 68에서 소개된 값 초기화를 사용합니다. (포인터라면 nullptr, bool 값이라면 false)

첫 번째 형질 템플릿을 적용하기 위해 다음 코드를 확인합니다.

```cpp
#include "accum1.hpp"
#include <iostream>
int main()
{
  // create array of 5 integer values
  int num[] = { 1, 2, 3, 4, 5 };
  // print average value
  std::cout << "the average value of the integer values is " << accum(num, num+5) / 5 << ’\n’;
  // create array of character values
  char name[] = "templates";
  int length = sizeof(name)-1;
  // (try to) print average character value
  std::cout << "the average value of the characters in \"" << name << "\" is " << accum(name, name+length) / length << ’\n’;
}
```

첫 번째로,

accum() 템플릿 함수를 사용해서 5개의 정수 값을 합산합니다.

```cpp
int num[] = { 1, 2, 3, 4, 5 };
…
accum(num0, num+5)
```

그런 다음 평균 값을 구합니다.

두 번째로,

문자열을 사용해서 동일한 작업을 수행합니다.

예상되는 결과는 아마도 a와 z 사이에 있어야 합니다. a~z의 값은 대부분 ASCII 코드로 표현됩니다.

a = 97, z = 122로 표현되기 때문에  97 ~ 122 사이의 결과값을 예상합니다.

실제 출력은 다음과 같습니다.

```
the average value of the integer values is 3
the average value of the characters in "templates" is -5
```

여기서 문제는 char 타입으로 템플릿이 인스턴스화 되었다는 것에 문제가 있습니다.(char가 표현할 수 있는 범위는 -128 ~ 127)

우리는 변수 합계에 사용 된 타입을 설명하는 추가 템플릿 매개 변수 AccT를 도입하여 이를 해결할 수 있습니다.

하지만 이 방법은 템플릿 호출마다 추가적으로 타입을 지정해야 합니다.

따라서 우리는 다음과 같이 코드를 작성할 수 있습니다.

```accum <int> (name, name + 5)```, 사용하는데 큰 불편함은 없지만 피할 수 있는 방법은 있습니다.

accum ()이 호출되는 각 타입 T와 누적 타입간에(total) 연관성을 만드는 것입니다.

이 연관성은 유형 T의 형질로 간주 될 수 있으므로 합계가 계산되는 유형을 T의 특성이라고 합니다. 

이러한 연관성은 타입 T의 특성으로 간주 될 수 있으므로 합계를 계산하는 타입을 T의 형질이라고 합니다.

```cpp
template<typename T>
struct AccumulationTraits;

template<>
struct AccumulationTraits<char> 
{
  using AccT = int;
};

template<>
struct AccumulationTraits<short> 
{
  using AccT = int;
};

template<>
struct AccumulationTraits<int> 
{
  using AccT = long;
};

template<>
struct AccumulationTraits<unsigned int> 
{
  using AccT = unsigned long;
};

template<>
struct AccumulationTraits<float> 
{
  using AccT = double;
};
```

AccumulationTraits 템플릿은  매개 변수 유형의 형질을 보유하기 때문에 형질 템플릿이라고 합니다.

AccumulationTraits 템플릿에 대한 일반적인 정의하지 않습니다. 왜냐하면 타입이 무엇인지 모를 때는 원하는 타입을 선택할 수 있는 좋은 방법이 없기 때문입니다. 

다음과 같이 accum() 템플릿을 다시 작성할 수 있습니다.

```cpp
#ifndef ACCUM_HPP
#define ACCUM_HPP
#include "accumtraits2.hpp"

template<typename T>
auto accum (T const* beg, T const* end)
{
  // return type is traits of the element type
  using AccT = typename AccumulationTraits<T>::AccT;
  AccT total{}; // assume this actually creates a zero value

  while (beg != end) 
  {
    total += *beg;
    ++beg;
  }
  return total;
}
#endif //ACCUM_HPP
```

다시 프로그램을 돌려보면 원하던 결과를 확인할 수 있습니다.

```
the average value of the integer values is 3
the average value of the characters in "templates" is 108
```

새로운 타입들이 accum()에 사용되어도 AccumulationTraits 템플릿의 추가 명시 전문화 통해서  적절한 AccT를 타입을 정의할 수 있습니다.

이는 모든 타입들에 대해서 유효합니다.

### 19.1.2 Value Traits

extra information(??)이 유형에만 한정될 필요가 없음을 이 섹션에서 확인할 수 있습니다.

accum () 템플릿은 반환 값의 기본 생성자를 사용하여 결과 변수를 0과 비슷한 값으로 초기화하려고 합니다.

```cpp
AccT total{}; // assume this actually creates a zero value
…
return total;
```

이 초기화가 accumulation loop를 시작하는데 유효한 결과 값을 가질 수 있다고는 보장할 수 없습니다.

타입 AccT에는 기본 생성자가 없을 수도 있습니다.

AccumulationTraits에 새로운 형질을 추가 할 수 있습니다

```cpp
template<typename T>
struct AccumulationTraits;

template<>
struct AccumulationTraits<char> 
{
  using AccT = int;
  static AccT const zero = 0;
};

template<>
struct AccumulationTraits<short> 
{
  using AccT = int;
  static AccT const zero = 0;
};

template<>
struct AccumulationTraits<int>
{
  using AccT = long;
  static AccT const zero = 0;
};
…
```

이 경우, 새로운 형질은 컴파일 시간에 평가 될 수 있는 상수로 초기화 가능한 요소를 제공합니다.

 accum()을 다음과 재 작성할 수 있습니다.

 ```cpp
#ifndef ACCUM_HPP
#define ACCUM_HPP
#include "accumtraits3.hpp"

template<typename T>
auto accum (T const* beg, T const* end)
{
  // return type is traits of the element type
  using AccT = typename AccumulationTraits<T>::AccT;
  AccT total = AccumulationTraits<T>::zero; // init total by trait value

  while (beg != end) 
  {
    total += *beg;
    ++beg;
  }
  return total;
}
```

 total 변수에 대한 초기화는 간단합니다.

```
AccT total = AccumulationTraits<T>::zero;
```

이 형식의 단점은 클래스 내부의 정적 상수 데이터 멤버를 초기화 할 수 있는 것은 C++에서 정수 또는 열거 형만 가능하다는 것입니다.

```constexpr``` 정적 데이터 멤버는 다른 리터럴 유형뿐만 아니라 부동 소수점 유형도 허용합니다

```cpp
template<>
struct AccumulationTraits<float> 
{
  using Acct = float;
  static constexpr float zero = 0.0f;
};
```

그러나 const와 constexpr 둘 다 nonliteral types에 대해서는 초기화를 할 수 없습니다.

예를 들어, 사용자 정의한 임의 정밀도 BigInt 유형은 리터럴 유형이 아닐 수도 있습니다. 

일반적으로 힙에 할당하기 때문에 리터럴 타입이 될 수 없고 생성자가 constexpr이 아니기 때문입니다.

다음 전문화는 오류 입니다.

```cpp
class BigInt 
{
  BigInt(long long);
…
};
…

template<>
struct AccumulationTraits<BigInt> 
{
  using AccT = BigInt;
  static constexpr BigInt zero = BigInt{0}; // ERROR: not a literal type
};
```

```
1>CppTemplateStudy.cpp
1>c:\users\***\desktop\cpptemplatestudy\cpptemplatestudy\cpptemplatestudy.cpp(32): error C2131: expression did not evaluate to a constant
1>c:\users\***\desktop\cpptemplatestudy\cpptemplatestudy\cpptemplatestudy.cpp(32): note: failure was caused by call of undefined function or one not declared 'constexpr'
1>c:\users\***\desktop\cpptemplatestudy\cpptemplatestudy\cpptemplatestudy.cpp(32): note: see usage of 'BigInt::BigInt'
```


직접적인 대안은 클래스에 value trait를 정의하는 않는 것입니다.

```cpp
template<>
struct AccumulationTraits<BigInt> 
{
  using AccT = BigInt;
  static BigInt const zero; // declaration only
};
```

이니셜라이저는 소스에서 다음과 같이 사용 됩니다.
```
BigInt const AccumulationTraits<BigInt>::zero = BigInt{0}; 
```
이 방법이 동작은 하지만, 단점이 있습니다. 컴파일러는 다른 파일에 정의되어 있는 값을 인식하지 못합니다.

C ++ 17에서는 인라인 변수를 사용하여이 문제를 해결할 수 있습니다.

```cpp
template<>
struct AccumulationTraits<BigInt> 
{
  using AccT = BigInt;
  inline static BigInt const zero = BigInt{0}; // OK since C++17
};
```

C++17 이전에는 정수 값을 산출하지 않는 값 형질에 대해서는 인라인 멤버 함수를 사용하는 것이 대안이었습니다.
다시 그런 함수가 리터럴 타입을 반환하면 constexpr로 선언될 수 있습니다.

예를 들어 다음과 같이 AccumulationTraits을 다시 작성해보면

```cpp
template<typename T>
struct AccumulationTraits;

template<>
struct AccumulationTraits<char> 
{
  using AccT = int;
  static constexpr AccT zero() {
    return 0;
  }
};

template<>
struct AccumulationTraits<short> 
{
  using AccT = int;
  static constexpr AccT zero() {
    return 0;
  }
};

template<>
struct AccumulationTraits<int> 
{
  using AccT = long;
  static constexpr AccT zero() {
    return 0;
  }
};

template<>
struct AccumulationTraits<unsigned int> 
{
  using AccT = unsigned long;
  static constexpr AccT zero() {
    return 0;
  }
};

template<>
struct AccumulationTraits<float> 
{
  using AccT = double;
  static constexpr AccT zero() {
    return 0;
  }
};
…
```

우리가 정의한 타입에 맞게 유형을 확장해보면

```cpp
template<>
struct AccumulationTraits<BigInt> 
{
  using AccT = BigInt;
  static BigInt zero() {
    return BigInt{0};
  }
};
```

응용 프로그램 코드의 경우 유일한 차이점은 함수 호출 구문의 사용입니다.

```
AccT total = AccumulationTraits<T>::zero(); // init total by trait function
```

형질은 추가적인 타입 이상일 수 있습니다.


위에서 다루었던 예제를 보면, accum()이 호출되는 element type 대해 필요한 정보를 제공하는 하나의 메커니즘으로 볼 수 있습니다.

이것이 형질의 개념의 핵심입니다. 형질은 generic computations을 위한 구체적인 요소를 구성 할 수있는 수단을 제공합니다.

### 19.1.3 Parameterized Traits

이전 섹션에서 accum ()에 사용된 형질을 고정된 형질이라고 합니다, 왜냐하면 일단 형질이 고정이 되면 알고리즘에서 대체할 수 없기 때문입니다.

예를 들어 float 값들이 동일한 타입의 변수로 안전하게 합을 구할 수 있다는 것을 알고 있다면 효율성을 확보할 수 있습니다.

(we may happen to know that a set of float values can safely be summed into a variable of the same type, and doing so may buy us some efficiency)

형질 템플릿에 의해 결정된 기본 값을 가진 형질 자체에 AT템플릿 매개 변수를 추가하여 이 문제를 해결할 수 있습니다.

```cpp
template<typename T, typename AT = AccumulationTraits<T>>
auto accum (T const* beg, T const* end)
{
  typename AT::AccT total = AT::zero();
  
  while (beg != end) 
  {
    total += *beg;
    ++beg;
  }
  return total;
}

```

이 방법으로 많은 사용자는 템플릿 인수를 생략할 수 있지만, 예외적인 경우에는 사용자가 accumulation typed에 대해서 대안을 지정할 수 있습니다.

아마도이 템플릿의 대부분의 사용자는 첫 번째 인수에 대해 추론된 모든 유형에 대해 적절한 기본값으로 구성할 수 있기 때문에 두 번째 템플릿 인수를 명시 적으로 제공하지 않아도됩니다.

## 19.2 Traits versus Policies and Policy Classes

지금까지 accumulation과 summation을 동일시 했습니다.

하지만 다른 종류의 accumulations을 사용할 수 있지 않을까 생각할 수 있습니다.

예를 들면 합 대신에 주어진 값들을 곱할 수도 있습니다. 만약 문자열이면 이어 붙일 수도 있습니다.

시퀀스에서 최대 값을 찾는 경우에 accumulation problem를 공식화 할 수 있습니다.

이러한 대안에서 보면  변경해야하는 accum()연산은 ```total + = * beg```입니다.

이 작업을 accumulation process의 정책이라고 할 수 있습니다.

다음은 accum()함수 템플릿에서 이러한 정책을 어떻게 도입 할 수 있는지에 대한 예입니다.

```cpp
template<typename T, typename Policy = SumPolicy, typename Traits = AccumulationTraits<T>>
auto accum (T const* beg, T const* end)
{
  using AccT = typename Traits::AccT;
  AccT total = Traits::zero();
  while (beg != end) 
  {
    Policy::accumulate(total, *beg);
    ++beg;
  }
  return total;
}
```

이 버전의 accum()의 Subtolicy는 정의된 인터페이스를 통해 알고리즘에 대해 하나 이상의 정책을 구현하는 정책 클래스입니다.

```cpp
class SumPolicy 
{
public:
  template<typename T1, typename T2>
  static void accumulate (T1& total, T2 const& value) 
  {
    total += value;
  }
};
```

값을 축적하기 위한 다른 정책을 지정하여 다른 종류의 계산할 수 있습니다.

```cpp
class MultPolicy 
{
public:
  template<typename T1, typename T2>
  static void accumulate (T1& total, T2 const& value) 
  {
    total *= value;
  }
};

int main()
{
  // create array of 5 integer values
  int num[] = { 1, 2, 3, 4, 5 };
  // print product of all values
  std::cout << "the product of the integer values is "
  << accum<int,MultPolicy>(num, num+5)
  << ’\n’;
}
```

하지만 프로그램의 결과는 원하던 결과 값이 아닙니다.

```
the product of the integer values is 0
```

문제는 초기값에 있습니다.(0으로 초기화 된다)

합계에서는 초기값 0이 유효하지만 곱셉에서는 맞지 않습니다.(0을 곱하면 결과는 0)

이는 다양한 형질과 정책이 상호 작용할 수 있다는 것을 보여 주며, 신중하게 템플릿을 설계해야 하는 중요성을 강조합니다.

이 경우, accumulation loop의 초기화가 accumulation 정책의 일부임을 알 수 있습니다.

accumulation 정책은 특성 zero()를 사용하거나 사용하지 않을 수 있습니다. 
모든 것들이 특성과 정책으로 해결되어야 하는 것은 아닙니다. 

예를 들어 C++표준 라이브러리의 std:accumulate()함수는 세번째(함수 호출)인수로 초기 값을 전달받습니다.

### 19.2.1 Traits and Policies: What’s the Difference?

정책이 특성의 특별한 경우라는 사실을 뒷바침하기 위한 사례가 될 수 있습니다.(A reasonable case can be made in support of the fact that policies are just a special case of traits. )

반대로, 특성은 정책을 인코딩한다고 주장 할 수 있습니다.(Conversely, it could be claimed that traits just encode a policy.)

• 특성 n…. 사물의 특성을 나타내는 독특한 특징

• 정책 n .... 유익하고 편리하게 채택 된 모든 행동 과정에 대해서 템플릿 인수와 관련해서 대체로 일종의 동작(행동)을 인코딩하는 클래스로 정책 클래스라는 용어를 사용합니다.(any course of action adopted as advantageous or expedient Based on this, we tend to limit the use of the term policy classes to classes that encode an action of some sort that is largely orthogonal with respect to any other template argument with which it is combined.)

```
정책은 특성과 공통점이 많지만 유형에 덜 중점을두고 행동에 더 중점을 두는 점에서 다릅니다.
```

형질 기술을 도입 한 Nathan Myers는 다음과 같이 더 개방적인 정의를 제안했습니다.

특성 클래스 : 템플릿 매개 변수 대신 사용되는 클래스입니다. 클래스로서 유용한 유형 및 상수를 집계합니다. 템플릿으로 모든 소프트웨어 문제를 해결하는 "간접적 인 수준의 추가 수준"을 제공합니다. (Traits class : A class used in place of template parameters. As a class, it aggregates useful types and constants; as a template, it provides an avenue for that “extra level of indirection” that solves all software problems.)

따라서 일반적으로 우리는 다음과 같이 정리합니다.

```
• 특성은 템플릿 매개 변수의 추가 속성을 나타냅니다.
• 정책은 일반적인 기능 및 유형에 대해 구성 가능한 행동을 나타냅니다. (일반적으로 기본 값 포함 경우도 있음). 
```

두 개념 사이의 가능한 차이에 대해 자세히 설명하기 위해 특성에 대해서 정리합니다.

```
• 특성은 고정 형질 (즉, 템플릿 매개 변수를 통과하지 않고)로서 유용 할 수있다.
• 특성 매개 변수는 일반적으로 매우 자연스러운 기본값을 갖습니다.(무시하거나 재정의 할 수 없음)
• 특성 매개 변수는 하나 이상의 주요 매개 변수에 의존적입니다.
• 특성은 주로 멤버 함수가 아닌 유형과 상수를 결합합니다.
• Traits tend to be collected in traits templates.
```

정책에 대해서 다음과 같이 정리합니다.
```
• 정책 클래스는 템플릿 매개 변수로 전달되지 않으면 사용할 수 없습니다.
• 정책 매개 변수는 기본값이 필요하지 않으며 명시 적으로 지정되는 경우가 많습니다(일반적으로 사용되는 기본 정책으로 구성)
• Policy parameters are mostly orthogonal to other parameters of a template(정책 매개변수는 템플릿 매개변수 타입에 의존적이다?)
• 정책 클래스는 주로 멤버 함수 형태로 제공됩니다.
• 정책은 일반 클래스나 클래스 템플릿으로 표현됩니다.
```

하지만 두 용어 사이에 분명히 불분명한 경계가 있다.

예를 들어, C ++ 표준 라이브러리의 character traits은 문자를 비교, 이동 및 찾는 것과 같은 기능적 동작을 정의합니다. 그리고 이러한 특성을 대체함으로써 대소문자를 구분하지 않는 방식으로 동작하는 문자열 클래스를 정의할 수 있습니다.
따라서 이러한 속성을 특성이라고 하지만 정책과 관련된 일부 속성을 가지고 있습니다.

### 19.2.2 Member Templates versus Template Template Parameters

accumulation policy를 구현하기 위해 SumPolicy와 MultPolicy를 멤버 템플릿이 있는 일반 클래스로 작성하였습니다.

클래스 템플릿을 사용하여 정책 클래스 인터페이스를 디자인 한 다음 템플릿 템플릿 인수로 사용하는 것입니다.

예를 들어 SumPolicy 보면

```cpp
template<typename T1, typename T2>
class SumPolicy 
{
public:
  static void accumulate (T1& total, T2 const& value) 
  {
  total += value;
  }
};
```

그런 다음 Accum의 인터페이스를 템플릿 템플릿 인수로 사용하도록 합니다.

```cpp
template<typename T, template<typename,typename> class Policy = SumPolicy, typename Traits = AccumulationTraits<T>>
auto accum (T const* beg, T const* end)
{
  using AccT = typename Traits::AccT;
  AccT total = Traits::zero();
  while (beg != end) 
  {
    Policy<AccT,T>::accumulate(total, *beg);
    ++beg;
  }
  return total;
}
```
traits 매개 변수에도 동일한 변환을 적용 할 수 있습니다.(AccT 유형을 정책 유형에 명시적으로 전달하는 대신 accumulation trait을 전달하여 정책이 특성 매개 변수에 대한 결과 유형을 결정하도록 할 수도 있습니다.)

템플릿 템플릿 매개 변수를 통해 정책 클래스에 접근하는 가장 큰 이점은 템플릿 매개 변수에 의존하는 유형의 일부 상태 정보 (즉, 정적 데이터 구성원)를 정책 클래스에 쉽게 전달할 할 수 있습니다.

그러나 템플릿 매개 변수 접근 방식의 단점은 인터페이스에 의해 정의된 정확한 템플릿 매개 변수들을 사용하여 정책 클래스를 템플릿으로 작성해야 한다는 점입니다.

이것은 traits 자체의 표현식을 단순한 비 템플릿 클래스보다 어렵고 장황하게 작성될 수도 있습니다.

### 19.2.3 Combining Multiple Policies and/or Traits

특성과 정책이 사용해야 하는 많은 수의 템플릿 매개 변수를 줄이는데 도움이 되지는 않습니다. 
하지만 관리가 가능한 수준까지 그 수를 줄일 수는 있습니다.

여기서 재미있는 질문 하나는 매개 변수들을 순서화하는 것입니다.

간단한 전략은 기본값을 선택해서 사용할 경우가 많기 때문에 기본값에 따라서 매개 변수를 정렬하는 것입니다. 특성 매개 변수가 우리의 코드에서 더 자주 무시되기 때문에 이는 특성 매개 변수가 정책 매개 변수를 따름을 의미합니다

코드에 복잡성(제약)을 주고자 한다면 기본값을 제공해주지 않고 파라미터 인수의 순서에 맞게 지정하는 방식을 제공하면 됩니다.

### 19.2.4 Accumulation with General Iterators

포인터 대신 일반화 된 반복자를 처리하는 기능을 추가하는 accum () 버전을 보도록 하겠습니다.

```cpp
#include <iterator>
template<typename Iter>
auto accum (Iter start, Iter end)
{
  using VT = typename std::iterator_traits<Iter>::value_type;
  VT total{}; // assume this actually creates a zero value
  while (start != end) 
  {
    total += *start;
    ++start;
  }
  return total;
}
```

std :: iterator_traits 구조체는 반복자의 모든 관련 속성을 캡슐화하고 있습니다. 포인터에 대한 부분 전문화가 존재하기 때문에 이러한 특성은 일반 포인터 함께 편리하게 사용됩니다. 표준 라이브러리의 구현은 다음과 같습니다

```cpp
namespace std {
template<typename T>
struct iterator_traits<T*> {
using difference_type = ptrdiff_t;
using value_type = T;
using pointer = T*;
using reference = T&;
using iterator_category = random_access_iterator_tag ;
};
}
```

하지만 그러나 반복자가 참조하는 값의 누적 유형은 없습니다. 따라서 우리는 여전히 우리 고유의 AccumulationTraits을 제공해야 합니다.