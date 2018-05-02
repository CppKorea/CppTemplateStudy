//
//  has_member_first.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef has_member_first_h
#define has_member_first_h

#include "../common/integral_constant.hpp"
#include "../common/void_type.hpp"

template <typename, typename = void_t<>>
struct has_member_first : false_type {};

template <typename T>
struct has_member_first<T, void_t<decltype(&T::first)>> : true_type {};

#endif /* has_member_first_h */
