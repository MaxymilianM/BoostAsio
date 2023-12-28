#include <boost/bind/bind.hpp>
#include <iostream>

// g++ -ansi -std=c++11 argumentbind.cpp -o argumentbind -I /usr/local/include

void cubevolume(float f) {
  std::cout << "Volume of the cube is " << f * f * f << std::endl;
}

int main(void) {
  boost::bind(&cubevolume, 4.23f)();
  return 0;
}
