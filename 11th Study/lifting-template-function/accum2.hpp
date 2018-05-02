//
//  accum2.h
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef accum2_h
#define accum2_h

#include "accum_policy.hpp"

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

#endif /* accum2_h */
