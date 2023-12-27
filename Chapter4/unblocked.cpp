#include <boost/asio.hpp>
#include <iostream>

// g++ -ansi -std=c++11 unblocked.cpp -o unblocked -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread

int main(void) {
  boost::asio::io_service io_svc;

  io_svc.run();

  std::cout << "We will see this line in console window." << std::endl;

  return 0;
}
