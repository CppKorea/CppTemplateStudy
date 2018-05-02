//
//  size_type.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef sizeof_h
#define sizeof_h

template <typename T>
struct type_size {
    static constexpr std::size_t value = sizeof(T);
};

#endif /* sizeof_h */
