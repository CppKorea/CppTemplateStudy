CMAKE_MINIMUM_REQUIRED (VERSION 3.0)

FIND_PACKAGE(Boost)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
MESSAGE("boost include ${Boost_INCLUDE_DIRS}")

SET(CMAKE_CXX_STANDARD 14)

SET(CMAKE_CXX_FLAGS "-g -Wall -Wextra ${CMAKE_CXX_FLAGS}")

ADD_EXECUTABLE(reference_collapsing reference_collapsing.cpp)
ADD_EXECUTABLE(forward_reference forward_reference.cpp)
ADD_EXECUTABLE(perfect_forward perfect_forward.cpp)
ADD_EXECUTABLE(class_template_argument class_template_argument.cpp)
