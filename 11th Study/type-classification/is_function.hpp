//
//  is_function.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef is_function_h
#define is_function_h

#include <tuple>
#include "../common/integral_constant.hpp"

template <typename T>
struct is_function : false_type {};

template <typename R, typename... Params>
struct is_function<R(Params...)> : true_type {
    using type = R;
    using param_type = std::tuple<Params...>;
    static constexpr bool variadic = false;
};

// c-style variadic parameter
template <typename R, typename... Params>
struct is_function<R(Params..., ...)> : true_type {
    using type = R;
    using params_type = std::tuple<Params...>;
    static constexpr bool variadic = true;
};

template <typename T>
constexpr bool is_function_v = is_function<T>::value;

#endif /* is_function_h */
