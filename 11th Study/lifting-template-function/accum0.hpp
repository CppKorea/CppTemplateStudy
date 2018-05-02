//
//  accum0.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef accum0_h
#define accum0_h

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

#endif /* accum0_h */
