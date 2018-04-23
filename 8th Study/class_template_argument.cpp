template<typename T>
class S {
public:
  S(T b) : a(b) {
  }
private:
  T a;
};

int main() {
  // S x(1); // Error before c++17: 
             // the class template parameter T was not deduced 
             // from the constructor call argument 1
  S<int> x(1);
  return 0;
}
