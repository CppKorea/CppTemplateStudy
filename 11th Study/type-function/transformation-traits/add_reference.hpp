//
//  add_reference.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef add_reference_h
#define add_reference_h

template <typename T>
struct add_lvalue_reference {
    using type = T&;
};

template <>
struct add_lvalue_reference<void> {
    using type = void;
};

template <>
struct add_lvalue_reference<void const> {
    using type = void const;
};

template <>
struct add_lvalue_reference<void const volatile> {
    using type = void const volatile;
};

template <typename T>
using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;


template <typename T>
struct add_rvalue_reference {
    using type = T&&;
};

template <>
struct add_rvalue_reference<void> {
    using type = void;
};

template <>
struct add_rvalue_reference<void const> {
    using type = void const;
};

template <>
struct add_rvalue_reference<void const volatile> {
    using type = void const volatile;
};

template <typename T>
using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

#endif /* add_reference_h */
