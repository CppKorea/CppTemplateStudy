//
//  is_funda.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef is_funda_h
#define is_funda_h

#include "../common/integral_constant.hpp"

template <typename T>
struct is_funda : false_type {};

template <>
struct is_funda<char> : true_type {};
template <>
struct is_funda<int> : true_type {};
template <>
struct is_funda<double> : true_type {};

//...

template <typename T>
constexpr bool is_funda_v = is_funda<T>::value;

#endif /* is_funda_h */
