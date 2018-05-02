//
//  is_convertible.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef is_convertible_h
#define is_convertible_h

#include "../common/integral_constant.hpp"
#include "../type-classification/is_void.hpp"
#include "../type-classification/is_array.hpp"
#include "../type-classification/is_function.hpp"

template <typename FROM, typename TO, bool = is_void_v<TO> || is_array_v<TO> || is_function_v<TO>>
struct is_convertible_helper {
private:
    static void aux(TO);
    
    template <typename F, typename = decltype(aux(std::declval<F>()))>
    static true_type test(void*);
    
    template <typename>
    static false_type test(...);
    
public:
    using type = decltype(test<FROM>(nullptr));
};

template <typename FROM, typename TO>
struct is_convertible_helper<FROM, TO, false> {
    using type = false_type;
};

template <typename FROM, typename TO>
struct is_convertible : is_convertible_helper<FROM, TO>::type {};

template <typename FROM, typename TO>
using is_convertible_t = typename is_convertible<FROM, TO>::type;

template <typename FROM, typename TO>
constexpr bool is_convertible_v = is_convertible<FROM, TO>::value;

#endif /* is_convertible_h */
