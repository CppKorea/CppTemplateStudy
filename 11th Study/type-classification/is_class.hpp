//
//  is_class.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef is_class_h
#define is_class_h

#include "../common/integral_constant.hpp"
#include "../common/void_type.hpp"

template <typename T, typename = void_t<>>
struct is_class : false_type {};

template <typename T>
struct is_class<T, void_t<int T::*>> : true_type {};

template <typename T>
constexpr bool is_class_v = is_class<T>::value;

#endif /* is_class_h */
