//
//  has_size.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef has_size_h
#define has_size_h

#include "../common/integral_constant.hpp"
#include "../common/void_type.hpp"
#include "../type-function/transformation-traits/remove_reference.hpp"

template <typename, typename = void_t<>>
struct has_size_type : false_type {};

template <typename T>
struct has_size_type<T, void_t<typename remove_reference_t<T>::size_type>> : true_type {};

#endif /* has_size_h */
