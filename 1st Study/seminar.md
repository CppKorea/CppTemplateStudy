# Templates Study
## 1. The Basics
### 1.1. Macro & Template
템플릿이라는 개념은 매크로를 통해 우연히 발견되었다고 한다.

옛날옛적, 템플릿이 없던 시절의 개발자들은 매크로를 이용해 Generic 한 매크로 함수를 만들어 썼다. 또한 컬렉션 클래스를 만듦에 있어, 중복 코드를 생성치 않기 위하여 매크로를 이용해 Generic 한 클래스들을 만들어 쓰기도 하였다. 이 시절의 매크로 함수와 클래스가 확장되어 만들어진 것이 지금의 템플릿이다.

템플릿의 가장 중요한 특성 중 하나가 바로 compile-time 아닌가? 왜 run-time 이 아닌 compile-time 일까? 매크로의 특성을 생각해보면 뻔하지 않나? 때문에 템플릿을 배울 때, "템플릿은 매크로의 확장판이다" 라는 개념만 기억하고 있으면, 생각보다 템플릿을 쉽고 수월하게 배울 수 있다.

```cpp
#define MIN(x, y) (((a)<(b))?(a):(b))
#define MAX(x, y) (((a)>(b))?(a):(b))

#define collection(T) \
class collection_##T \
{ \
private: \
    T *data_; \
    size_t size_; \
    size_t capacity_; \
    \
public: \
    collection_##T() \
        : data_(nullptr), \
        size_(0), capacity_(0) {}; \
    collection_##T(size_t n, const T &val) \
    { \
        data_ = new T[n]; \
        size_ = capacity_ = n; \
        \
        while (n != 0) \
        { \
            *data_++ = val; \
            --n; \
        } \
    }; \
    \
    T* begin() \
    { \
        return data_; \
    }; \
    T* end() \
    { \
        return data_ + size_; \
    }; \
};
```

### 1.2. STL
Standard Template Library.

C++ Templates 를 대표하는 라이브러리. 아래와 같이 크게 네 파트로 분류됨.
  - containers
  - iterators
  - algorithms
  - functors

STL 이 제공하는 객체들을 둘러보니, 온갖 종류의 Template Features 들이 다채롭게 쓰였떠라. 그렇다면 STL 을 직접 구현해보면 템플릿 도사가 되어있지 않을까?

```cpp
namespace mystl
{
    template <typename... Types>
    class tuple;
    
    template <typename Head>
    class tuple<Head>
    {
    private:
        Head head_;
        
    public:
        tuple() {};
        tuple(const Head &head)
            : head_(head) {};
            
        auto _Get_head() const -> const Head&
        {
            return head_;
        };
    };
    
    template <typename Head, typename... Tail>
    class tuple<Head, Tail...> : public tuple<Head>
    {
    private:
        tuple<Tail...> tail_;
        
    public:
        tuple() : tuple<Head>() {};
        tuple(const Head &head, const tuple<Tail...> &tail)
            : tuple<Head>(head), tail_(tail) {};
            
        auto _Get_tail() const -> const tuple<Tail...>&
        {
            return tail_;
        };
    };
    
    template <size_t N>
    struct _Tuple_getter
    {
        template <typename Tuple>
        static auto apply(const Tuple &t)
        {
            return _Tuple_getter<N-1>::apply(t._Get_tail());
        };
    };
    
    template <>
    struct _Tuple_getter<0>
    {
        template <typename Tuple>
        static auto apply(const Tuple &t)
        {
            return t._Get_head();
        };
    };
    
    template <size_t N, typename Tuple>
    auto get(const Tuple &t)
    {
        return _Tuple_getter<N>::apply(t);
    };
};
```

### 1.3. Template vs. Generic
Template 은 Macro 를 확장한 것이다.
  - Macro 에서 진화한 코드 복제 도구.
  - 새로운 타입을 사용할 때마다, 새로운 코드가 생성됨.
  - 다른 타입의 템플릿 클래스는, 이름만 비슷할 뿐 아주 다른 클래스임.
    > ```cpp
    > vector<int> v1;
    > vector<string> v2;
    > 
    > bool is_same = typeid(v1) == typeid(v2);
    > cout << is_same << endl; // false
    > ```

Generic 은 자동 타입 변환 도구이다.
  - 형 변환을 자동으로 해주는 편의 도구.
  - 새로운 타입을 사용해도, 동일한 코드를 사용함.
  - 다른 타입의 제네릭 클래스는, 실제 같은 타입의 클래스임.
    > ```typescript
    > let v1: std.Vector<number> = new std.Vector();
    > let v2: std.Vector<string> = new std.Vector();
    > 
    > let is_same: boolean = typeof v1 == typeof v2;
    > console.log(is_same); // true
    > ```

Type     | Pros               | Cons
---------|--------------------|--------------------------
Template | 속도가 빠르며, 유연함.  | 프로그램의 크기가 커진다.
Generic  | 프로그램의 크기가 작다.  | 속도가 느리며, 유연하지 못하다.

```cpp
namespace std
{
    template<typename Key, typename T, 
        typename Hash = hash<Key>, 
        typename Pred = equal_to<Key>, 
        typename Alloc = allocator<pair<const Key, T>>
    > class unordered_map
    {
    public:
        template <typename InputIterator>
        void assign(InputIterator first, InputIterator last);
    };
};
```

```ts
namespace std
{
    class HashMap<Key, T>
    {
        public constructor
        (
            hash_fn: (key: Key) => number = std.hash, 
            equal_fn: (x: Key, y: Key) => boolean = std.equal_to
        );

        public assign<Key, T, IForwardIterator<IPair<Key, T>>>
            (first: IForwardIterator, last: IForwardIterator): void;
    }

    interface IForwardIterator<T>
    {
        readonly value: T;
        next(): IForwardIterator<T>;
        equals(obj: IForwardIterator<T>): boolean;
    }

    interface IPair<First, Second>
    {
        first: First;
        second: Second;
    }
    class Entry<Key, T> implements IPair<Key, T>, IComparable<Entry<Key, T>>
    {
        public readonly first: Key;
        public second: T;

        public less(obj: Entry<Key, T>): boolean;
        public equals(obj: Entry<Key, T>): boolean;
        public hashCode(): number;
    }
}
```


## 2. Function Templates
### 2.1. Template Parameters
```cpp
template <typename T>
T max(T x, T y)
{
    return x > y ? x : y;
}
```

### 2.2. Argument Deduction
```cpp
template <typename X, typename Y>
X max(X x, Y y)
{
    return x > y ? x : y;
};

template <typename X, typename Y, typename Ret>
Ret max(X x, Y y)
{
    return x > y ? x : y;
};
```

```cpp
template <typename X, typename Y>
auto max(X x, Y y);

template <typename X, typename Y>
auto max(X x, Y y) -> decltype(a > b ? a : b);

template <typename X, typename Y>
auto max(X x, Y y) -> std::decay<decltype(a > b ? a : b)>::type;

template <typename X, typename Y>
auto max(X x, Y y) -> std::common_type_t<X, Y>;

template <typename X, typename Y, 
          typename Ret = std::decay<decltype(a > b ? a : b)>::type>
auto max(X x, Y y) -> Ret;
```

```cpp
std::decay<int>::type; // int
std::decay<int&>::type; // int
std::decay<int&&>::type; // int
std::decay<const int>::type; // int
std::decay<const int&>::type; // int
std::decay<const int&&>::type; // int
```

### 2.3. Overloading Function Templates
```cpp
template<>
short max(short x, short y)
{
    return (x + (~y + 1) >> 15 == 0) ? x : y;
};

template<>
int max(int x, int y)
{
    return (x + (~y + 1) >> 31 == 0) ? x : y;
};

template <typename X, typename Y, typename Z>
auto max(X x, Y y, Z z)
{
    return max(x, max(y, z));
};
```

```cpp
namespace mystl
{
    template <typename T>
    std::string to_string(T);

    template<> std::string to_string(short val);
    template<> std::string to_string(int val);
    template<> std::string to_string(double val);
};
```



## 3. Class Templates
### 3.1. Implementation
클래스에 Template Argument 를 사용, 클래스 내 멤버에 Generic 한 타입을 사용할 수 있다.
  - Variables
  - Methods
  - Type Definitions

클래스 템플릿의 가장 대표적인 사례는 Container 이다. STL Containers 의 선언부를 살펴보자.

```cpp
namespace std
{
    //----
    // LINEAR CONTAINERS
    //----
    template <typename T, typename Alloc = allocator<T>>
    class vector;

    template <typename Alloc = allocator<bool>>
    class vector<bool>; // OVERLOADED TEMPLATE CLASS

    template <typename T, typename Alloc = allocator<T>>
    class list;

    //----
    // ASSOCIATIVE CONTAINERS
    //----
    template
    <
        typename Key,
        typename T,
        typename Comp = less<Key>,
        typename Alloc = allocator<pair<const Key, T>>
    > class map; // TREE-MAP

    template
    <
        typename Key, 
        typename T, 
        typename Hash = hash<Key>, 
        typename Pred = equal_to<Key>, 
        typename Alloc = allocator<pair<const Key, T>>
    > class unordered_map; // HASH-MAP
};
```

```cpp
namespace mystl
{
    template <typename T>
    class stack
    {
    private:
        std::vector<T> elems_;

    public:
        bool empty() const
        {
            return elems_.empty();
        };
        const T& top() const
        {
            return elems_.back();
        };

        void push(const T &elem)
        {
            elems_.push_back(elem);
        };
        void pop()
        {
            elems_.pop_back();
        };
    };
};
```

### 3.2. Speciailizations
```cpp
namespace mystl
{
    template <>
    class stack<std::string>
    {
    private:
        std::deque<std::string> elems_;

    public:
        bool empty() const;
        const std::string& top() const;

        void push(const std::string &elem);
        void pop();
    };
}
```

```cpp
namespace std
{
    template <typename T, typename Alloc = allocator<T>>
    class deque
    {
    private:
        vector<vector<T>> data_;
        size_t size_;
        size_t capacity_;

    public:
        void pop_front();
        void push_front(const T &elem);
    };
};
```

```cpp
namespace std
{
    // BOOL SPECIALIZATION
    template <typename Alloc = allocator<bool>>
    class vector<bool, Alloc>;
};
```

### 3.3. Utilities
#### 3.3.1. Default Argument
```cpp
namespace mystl
{
    template <typename T, typename Container = std::vector<T>>
    class stack
    {
    private:
        Container elems_;

    public:
        bool empty() const;
        const T& top() const;

        void push(const T &elem);
        void pop();
    };
};
```

#### 3.3.2. Overloadings
```cpp
namespace mystl
{
    template <typename... Types>
    class tuple;
    
    template <typename Head>
    class tuple<Head>
    {
    private:
        Head head_;
        
    public:
        tuple() {};
        tuple(const Head &head) 
            : head_(head) {};

        auto _Get_head() const -> const Head& 
        {
            return head_;
        };
    };
    
    template <typename Head, typename... Tail>
    class tuple<Head, Tail...> : public tuple<Head>
    {
    private:
        tuple<Tail...> tail_;
        
    public:
        tuple() : tuple<Head>() {};
        tuple(const Head &head, const tuple<Tail...> &tail)
            : tuple<Head>(head), tail_(tail) {};
            
        auto _Get_tail() const -> const tuple<Tail...>&
        {
            return tail_;
        };
    };
    
    template <size_t N>
    class _Tuple_getter
    {
        template <typename Tuple>
        static auto apply(const Tuple &t)
        {
            return _Tuple_getter<N-1>::apply(t._Get_tail());
        };
    };
    
    template <>
    class _Tuple_getter<0>
    {
        template <typename Tuple>
        static auto apply(const Tuple &t)
        {
            return t._Get_head();
        };
    };
    
    template <size_t N, typename Tuple>
    auto get(const Tuple &t)
    {
        return _Tuple_getter<N>::apply(t);
    };
};
```