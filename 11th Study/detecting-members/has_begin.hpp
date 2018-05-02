//
//  has_begin.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef has_begin_h
#define has_begin_h

#include "..common/integral_constant.hpp"
#include "..common/void_type.hpp"

template <typename, typename = void_t<>>
struct has_begin : false_type {};

template <typename T>
struct has_begin<T, void_t<decltype(std::declval<T>().begin())>> : true_type {};

#endif /* has_begin_h */
