//
//  is_valid.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef sfinae_helper_h
#define sfinae_helper_h

#include "../common/integral_constant.hpp"

template <typename F, typename... Args,
typename = decltype(std::declval<F>()(std::declval<Args&&>()...))>
true_type is_valid_impl(void*);

template <typename F, typename... Args>
false_type is_valid_impl(...);

inline constexpr
auto is_valid = [](auto f) {
    return [](auto&&... args) {
        return decltype(is_valid_impl<decltype(f), decltype(args)&&...>(nullptr)){};
    };
};

template <typename T>
struct type_wrapper {
    using type = T;
};

template <typename T>
constexpr auto type_instance = type_wrapper<T>{};

template <typename T>
T unwrap_type(type_wrapper<T>);

#endif /* is_valid_h */
