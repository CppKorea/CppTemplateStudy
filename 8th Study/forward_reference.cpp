#include <iostream>
#include "print_type.h"

template<typename T> void f(T&& p) {
  T x = p;
  print_type(x);
}

template<typename T> void g(T&& p) {
  std::remove_reference_t<T> x = p;
  print_type(x);
}

int main() {
  int i  = 0;
  f(i);
  g(i);
  
  return 0;
}

