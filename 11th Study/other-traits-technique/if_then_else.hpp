//
//  if_then_else.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef if_then_else_h
#define if_then_else_h

template <bool Cond, typename TrueType, typename FalseType>
struct if_then_else {
    using type = TrueType;
};

template <typename TrueType, typename FalseType>
struct if_then_else<false, TrueType, FalseType> {
    using type = FalseType;
};

#endif /* if_then_else_h */
