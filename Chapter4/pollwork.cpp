#include <boost/asio.hpp>
#include <iostream>

// g++ -ansi -std=c++11 pollwork.cpp -o pollwork -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread

// The poll() function will not block the program while there is more work to do.
// It will execute the current work and then return the value.

int main(void) {
  boost::asio::io_service io_svc;
  boost::asio::io_service::work work(io_svc);

  for(int i=0; i<5; i++) {
    io_svc.poll();
    std::cout << "Line: " << i << std::endl;
  }

  return 0;
}
