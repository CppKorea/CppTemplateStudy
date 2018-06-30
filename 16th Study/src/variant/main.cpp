#include <iostream>
#include "Variant.hpp"

using namespace std;

int main()
{
    Variant<char, short, int, long long> var(3);
    
    cout << var.is<int>() << endl;
    var.visit([](auto const &val)
    {
        cout << val << endl;
    });

    system("pause");
    return 0;
}