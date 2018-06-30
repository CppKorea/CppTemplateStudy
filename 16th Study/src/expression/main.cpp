#include <iostream>

#include "SArray.hpp"
#include "Array.hpp"

using namespace std;

void s_array()
{
    Array<int> x(5);
    Array<int> y(5);

    for (size_t i = 0; i < 5; ++i)
    {
        x[i] = (int)i;
        y[i] = (int)-i;
    }

    auto op = A_Mult<int, A_Scalar<int>, SArray<int>>
    (
        A_Scalar<int>(4),
        x.data()
    );
    auto arr = Array<int, A_Mult<int, A_Scalar<int>, SArray<int>>>(op);

    // auto ret = 2*x + x*y; //--> ??
    auto ret = x + x*y;
    for (size_t i = 0; i < ret.size(); ++i)
        // cout << ret[i] << endl; //--> ???
        cout << ret.data()[i] << endl;
}

int main()
{
    s_array();

    system("pause");
    return 0;
}