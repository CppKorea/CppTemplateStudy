//
//  plus_result.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef plus_result_h
#define plus_result_h

#include "../../common/integral_constant.hpp"
#include "../../common/void_type.hpp"

template <typename, typename, typename = void_t<>>
struct has_plus : false_type {};

template <typename T1, typename T2>
struct has_plus<T1, T2, void_t<decltype(std::declval<T1>() + std::declval<T2>())>> : true_type {};

template <typename T1, typename T2>
static constexpr bool has_plus_v = has_plus<T1, T2>::value;

template <typename T1, typename T2, bool = has_plus_v<T1, T2>>
struct plus_result {
    using type = decltype(std::declval<T1>() + std::declval<T2>());
};

template <typename T1, typename T2>
struct plus_result<T1, T2, false> {
};

#endif /* plus_result_h */
