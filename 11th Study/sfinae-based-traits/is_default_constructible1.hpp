//
//  is_default_constructible1.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef is_default_constructible1_h
#define is_default_constructible1_h

#include "../common/integral_constant.hpp"

template <typename T>
struct is_default_constructible_helper {
private:
    template <typename U, typename = decltype(U())>
    static true_type test(void*);
    
    template <typename>
    static false_type test(...);
    
public:
    using type = decltype(test<T>(nullptr));
};

template <typename T>
struct is_default_constructible : is_default_constructible_helper<T>::type {};

#endif /* is_default_constructible1_h */
