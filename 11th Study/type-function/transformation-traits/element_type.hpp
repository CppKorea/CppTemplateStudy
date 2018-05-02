//
//  element_type.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef element_type_h
#define element_type_h

template <typename T>
struct element_type {
    using type = typename T::value_type;
};

// for array of unknown bound
template <typename T>
struct element_type<T[]> {
    using type = T;
};

// for array of known bound
template <typename T, std::size_t N>
struct element_type<T[N]> {
    using type = T;
};

template <typename T>
using element_t = typename element_type<T>::type;

#endif /* element_type_h */
