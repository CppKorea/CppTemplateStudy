//
//  is_reference.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef is_reference_h
#define is_reference_h

#include "../common/integral_constant.hpp"
#include "../other-traits-technique/if_then_else.hpp"

template <typename T>
struct is_lvalue_reference : false_type {};

template <typename T>
struct is_lvalue_reference<T&> : true_type {
    using base_type = T;
};

template <typename T>
using is_lvalue_reference_t = typename is_lvalue_reference<T>::type;

template <typename T>
constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value;

template <typename T>
struct is_rvalue_reference : false_type {};

template <typename T>
struct is_rvalue_reference<T&&> : true_type {
    using base_type = T;
};

template <typename T>
using is_rvalue_reference_t = typename is_rvalue_reference<T>::type;
template <typename T>
constexpr bool is_rvalue_reference_v = is_rvalue_reference<T>::value;

template <typename T>
struct is_reference : if_then_else<is_lvalue_reference_v<T>, is_lvalue_reference_t<T>, is_rvalue_reference_t<T>> {};

template <typename T>
constexpr bool is_reference_v = is_reference<T>::value;

#endif /* is_reference_h */
