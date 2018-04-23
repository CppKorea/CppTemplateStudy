#include <iostream>

class C { 
public:
  C() {}
  int i;
};

void g(C&) { std::cout << "C&" << std::endl; }
void g(C const&) { std::cout << "C const&" << std::endl; }
void g(C&&) { std::cout << "C&&" << std::endl; }

template<typename T> 
void forwardToG(T&& x) {
  g(static_cast<T&&>(x)); // forward x to g()
  //g(x); // forward x to g()
  //g(std::forward<T>(x)); // forward x to g()
} 

int main() {
  C v;
  C const c;
  forwardToG(v); // eventually calls g(C&)
  forwardToG(c); // eventually calls g(C const&)
  forwardToG(C()); // eventually calls g(C&&)
  forwardToG(std::move(v)); // eventually calls g(C&&)

  g(v);
  g(c);
  g(C());
  g(std::move(v));

  return 0;
}

