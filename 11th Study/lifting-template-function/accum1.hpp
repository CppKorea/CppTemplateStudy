//
//  accum1.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef accum1_h
#define accum1_h

template <typename>
struct accum_traits;

template <>
struct accum_traits<char> {
    using type = int;
    static constexpr type identity() {
        return 0;
    }
};

template <typename T>
auto accum(T const* beg, T const* end)
{
    using accum_type = typename accum_traits<T>::type;
    
    accum_type total = accum_traits<T>::identity();
    while (beg != end) {
        total = total + *beg;
        ++beg;
    }
    return total;
}

#endif /* accum1_h */
