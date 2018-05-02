//
//  decay.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef decay_h
#define decay_h

#include "remove_qualifier.hpp"

template <typename T>
struct decay : remove_cv<T> {
};

// for array of unknown bound
template <typename T>
struct decay<T[]> {
    using type = T*;
};

// for array of known bound
template <typename T, std::size_t N>
struct decay<T[N]> {
    using type = T*;
};

template <typename R, typename... Args>
struct decay<R(Args...)> {
    using type = R(*)(Args...);
};

// c-style variadic resolution
template <typename R, typename... Args>
struct decay<R(Args..., ...)> {
    using type = R(*)(Args..., ...);
};

template <typename T>
using decay_t = typename decay<T>::type;

#endif /* decay_h */
