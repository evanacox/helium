#include "helium/vector.h"
#include "helium_as.hh"
#include <iostream>

struct Point {
  double x, y;

  Point(double x, double y) {
    this->x = x;
    this->y = y;
  }

  void print() { std::cout << "Point { x: " << x << ", y: " << y << " }\n"; }
};

int main() {
  he_vector vec;
  he_vector_init(&vec, sizeof(Point));

  for (auto i = 0; i < 20; ++i) {
    he_vector_push_rval(&vec, Point);
  }

  auto array = (int *)vec.array;

  for (auto i = 0; i < 20; ++i) {
    std::cout << "vec[" << i << "]: " << array[i] << "\n";
  }

  he_vector_destroy(&vec);
}
