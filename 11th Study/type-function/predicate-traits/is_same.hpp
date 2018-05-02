//
//  is_same.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef is_same_h
#define is_same_h

#include "../../common/integral_constant.hpp"

template <typename T1, typename T2>
struct is_same : false_type {
};

template <typename T>
struct is_same<T, T> : true_type {
};

template <typename T1, typename T2>
constexpr bool is_same_v = is_same<T1, T2>::value;

#endif /* is_same_h */
