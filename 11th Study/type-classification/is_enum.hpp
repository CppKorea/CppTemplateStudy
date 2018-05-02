//
//  is_enum.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef is_enum_h
#define is_enum_h

#include "is_funda.hpp"
#include "is_pointer.hpp"
#include "is_reference.hpp"
#include "is_array.hpp"
#include "is_pointer_to_member.hpp"
#include "is_function.hpp"
#include "is_class.hpp"

template <typename T>
struct is_enum {
    static constexpr bool value =
    !is_funda_v<T> &&
    !is_pointer_v<T> &&
    !is_reference_v<T> &&
    !is_array_v<T> &&
    !is_pointer_to_member_v<T> &&
    !is_function_v<T> &&
    !is_class_v<T>;
};

#endif /* is_enum_h */
