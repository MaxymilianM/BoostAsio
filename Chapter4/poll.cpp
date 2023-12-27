#include <boost/asio.hpp>
#include <iostream>

// g++ -ansi -std=c++11 poll.cpp -o poll -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread


// The poll() function is used to run ready handlers until there are no more ready handlers remaining
// or until the io_service object has been stopped.
// However, in contrast with the run() function, the poll() function will not block the program.


int main(void) {
  boost::asio::io_service io_svc;

  for(int i=0; i<5; i++) {
    io_svc.poll();
    std::cout << "Line: " << i << std::endl;
  }

  return 0;
}