//
//  is_pointer_to_member.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef is_pointer_to_member_h
#define is_pointer_to_member_h

#include "../common/integral_constant.hpp"

template <typename T>
struct is_pointer_to_member : false_type {
};

template <typename T, typename C>
struct is_pointer_to_member<T C::*> : true_type {
    using member_type = T;
    using class_type = C;
};

template <typename T>
constexpr bool is_pointer_to_member_v = is_pointer_to_member<T>::value;

#endif /* is_pointer_to_member_h */
