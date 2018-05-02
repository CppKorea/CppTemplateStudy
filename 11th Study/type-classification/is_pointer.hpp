//
//  is_pointer.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef is_pointer_h
#define is_pointer_h

#include "../common/integral_constant.hpp"

template <typename T>
struct is_pointer : false_type {};

template <typename T>
struct is_pointer<T*> : true_type {
    using base_type = T;
};

template <typename T>
constexpr bool is_pointer_v = is_pointer<T>::value;

#endif /* is_pointer_h */
