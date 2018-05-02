//
//  accum3.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef accum3_h
#define accum3_h

#include "accum_policy.hpp"

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

#endif /* accum3_h */
