//
//  integral_constant.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef integral_constant_h
#define integral_constant_h

template <typename T, T val>
struct integral_constant {
    using type = T;
    static constexpr T value = val;
};

template <bool B>
using bool_constant = integral_constant<bool, B>;

using true_type = bool_constant<true>;
using false_type = bool_constant<false>;

#endif /* integral_constant_h */
