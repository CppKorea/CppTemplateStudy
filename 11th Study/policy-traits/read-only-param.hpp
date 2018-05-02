//
//  read-only-param.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef read_only_param_h
#define read_only_param_h

#include "../other-traits/technique/if_then_else.hpp"

template <typename T>
struct read_only_param {
    using type = typename if_then_else<sizeof(T)<=2*sizeof(void*), T, T const&>::type;
};

#endif /* read_only_param_h */
