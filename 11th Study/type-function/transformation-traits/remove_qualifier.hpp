//
//  remove_qualifier.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef remove_qualifier_h
#define remove_qualifier_h

template <typename T>
struct remove_const {
    using type = T;
};

template <typename T>
struct remove_const<T const> {
    using type = T;
};

template <typename T>
using remove_const_t = typename remove_const<T>::type;

template <typename T>
struct remove_volatile {
    using type = T;
};

template <typename T>
struct remove_volatile<T volatile> {
    using type = T;
};

template <typename T>
using remove_volatile_t = typename remove_const<T>::type;

template <typename T>
struct remove_cv : remove_const< remove_volatile_t<T>> {
};

template <typename T>
using remove_cv_t = typename remove_cv<T>::type;

#endif /* remove_qualifier_h */
