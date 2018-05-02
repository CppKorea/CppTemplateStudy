//
//  add_qualifier.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef add_qualifier_h
#define add_qualifier_h

template <typename T>
struct add_const {
    using type = T const;
};

template <typename T>
struct add_const<T const> {
    using type = T const;
};

template <typename T>
using add_const_t = typename add_const<T>::type;

template <typename T>
struct add_volatile {
    using type = T volatile;
};

template <typename T>
struct add_volatile<T volatile> {
    using type = T volatile;
};

template <typename T>
using add_volatile_t = typename add_const<T>::type;

template <typename T>
struct add_cv : add_const< add_volatile_t<T>> {
};

template <typename T>
using add_cv_t = typename add_cv<T>::type;

#endif /* add_qualifier_h */
