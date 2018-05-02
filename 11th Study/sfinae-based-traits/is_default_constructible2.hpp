//
//  is_default_constructible2.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef is_default_constructible2_h
#define is_default_constructible2_h

#include "../common/integral_constant.hpp"

template <typename...> using void_t = void;

template <typename, typename = void_t<>>
struct is_default_constructible : std::false_type {};

template <typename T>
struct is_default_constructible<T, void_t<decltype(T())>> : std::true_type {};

#endif /* is_default_constructible2_h */
