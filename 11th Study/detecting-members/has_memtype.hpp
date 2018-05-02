//
//  has_memtype.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef has_memtype_h
#define has_memtype_h

#include "../common/integral_constant.hpp"
#include "../common/void_type.hpp"

#define DEFINE_HAS_TYPE(MemType) \
template <typename, typename = void_t<>> \
struct has_##MemType : false_type {}; \
template <typename T> \
struct has_##MemType<T, void_t<typename T::##MemType>> : true_type {};

#endif /* has_memtype_h */
