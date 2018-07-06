#include <iostream>
#include <string>
#include <locale>
#include <vector>
#include <cassert>
#include <list>

using namespace std::literals;
using namespace std;

// Declaration of the concept "EqualityComparable",
// which is satisfied by any type T such that for values a and b of type T
// the expression a == b compiles and its result is convertible to bool
template <typename T>
concept bool EqualityComparable = requires(T a, T b)
{
    { a == b } -> bool;
};

// Forces you to implement to_string method
template <typename T>
concept bool Stringable = requires(T a)
{
    { a.to_string() }->string;
};

// Has a to_string function which returns a string
template <typename T>
concept bool HasStringFunc = requires(T a)
{
    { to_string(a) }->string;
};

struct Person
{
    double height;
    int weight;

    Person(double a, int b)
    {
        height = a;
        weight = b;
    }

    string to_string()
    {
        return "Weight: " + std::to_string(height) + " Height: " + std::to_string(weight);
    }
};

namespace std
{
    string to_string(list<int> l)
    {
        // Better ways to do this but just as example
        string s = "";

        for (int a : l)
        {
            s += (" " + to_string(a) + " ");
        }

        return s;
    }
}

string to_string(std::vector<int> v)
{
    string s = "";

    for (int a : v)
    {
        s += (" " + to_string(a) + " ");
    }

    return s;
}

void pretty_print(Stringable a)
{
    // We know that if the compiler made it this far we are good to go.
    cout << a.to_string() << endl;
}

void pretty_print(HasStringFunc a)
{
    cout << to_string(a) << endl;
}

// declaration of a constrained function template
// template<typename T>
// void f(T&&) requires EqualityComparable<T>; // long form of the same

int main()
{
    assert(__cpp_concepts >= 201500); // check compiled with -fconcepts
    assert(__cplusplus >= 201500);    // check compiled with --std=c++1z

    std::list<int> l{ 1, 2, 3 };
    Person jonathan(5.7, 130);
    std::vector<int> v{ 34, 23, 34, 56, 78 };

    // we can make it possible to pretty print a
    // vector we just need to implement -> string to_string(vector<int> a);
    // If we want to pretty print
    pretty_print(jonathan); // uses concept overload pretty_print(Stringable a)
    pretty_print(3);        // uses concept overload pretty_print(HasStringFunc a) 
    pretty_print(l);

    pretty_print(v);
    // This will result in an error, first you get the old template garbage you are used to
    // but you can ignore that because at the end highlighted in blue you will see
    // main.cpp:41:6: note:   constraints not satisfied
    // void pretty_print(Stringable a)
    // ^~~~~~~~~~~~
    // main.cpp:18:14: note: within ¡®template<class T> concept const bool Stringable<T> [with T = std::vector<std::__cxx11::basic_string<char> >]¡¯
    // concept bool Stringable = requires(T a)
}