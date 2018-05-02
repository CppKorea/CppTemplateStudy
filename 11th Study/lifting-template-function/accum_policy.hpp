//
//  accum_policy.hpp
//  cpptemples-ch19
//
//  Created by yws6909 on 02/05/2018.
//  Copyright © 2018 yws6909. All rights reserved.
//

#ifndef accum_policy_h
#define accum_policy_h

template <typename>
struct sum_policy;

template <>
struct sum_policy<char> {
public:
    using value_type = char;
    using accum_type = int;
    static constexpr accum_type identity() {
        return 0;
    }
    static constexpr accum_type accum(accum_type total, value_type value) {
        return total + value;
    }
};


template <typename>
struct mult_policy;

template <>
struct mult_policy<char> {
public:
    using value_type = char;
    using accum_type = int;
    static constexpr accum_type identity() {
        return 1;
    }
    static constexpr accum_type accum(accum_type total, value_type value) {
        return total * value;
    }
};

#endif /* accum_policy_h */
