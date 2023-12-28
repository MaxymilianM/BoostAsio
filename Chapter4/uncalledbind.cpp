#include <boost/bind/bind.hpp>
#include <iostream>

// g++ -ansi -std=c++11 uncalledbind.cpp -o uncalledbind -I /usr/local/include

void func() {
  std::cout << "Binding Function" << std::endl;
}

int main(void) {
  auto funct = boost::bind(&func);
  funct();
  return 0;
}
