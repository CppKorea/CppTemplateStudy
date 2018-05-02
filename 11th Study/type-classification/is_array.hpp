//
//  is_array.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef is_array_h
#define is_array_h

#include "../common/integral_constant.hpp"

template <typename>
struct is_array : false_type {};

template <typename T, std::size_t N>
struct is_array<T[N]> : true_type {
    using base_type = T;
    static constexpr std::size_t size = N;
};
template <typename T>
struct is_array<T[]> : true_type {
    using base_type = T;
    static constexpr std::size_t size = 0;
};

template <typename T>
constexpr bool is_array_v = is_array<T>::value;

#endif /* is_array_h */
