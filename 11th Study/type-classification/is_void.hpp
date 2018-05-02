//
//  is_void.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef is_void_h
#define is_void_h

#include "../common/integral_constant.hpp"

template <typename T>
struct is_void : false_type {};

template <>
struct is_void<void> : true_type {};

template <typename T>
constexpr bool is_void_v = is_void<T>::value;

#endif /* is_void_h */
