#include <iostream>

#include "print_type.h"
int main() {
  using RCI = int const&;
  RCI volatile&& r = 42; // OK: r has type int const&
  using RRI = int&&;
  RRI const&& rr = 42; // OK: rr has type int&&

  print_type(r);
  print_type(rr);

  RCI const r1 = 42;
  print_type(r1); // int const&

  int i = 0;
  RRI const& r2 = i;
  print_type(r2); // int&

  return 0;
};
