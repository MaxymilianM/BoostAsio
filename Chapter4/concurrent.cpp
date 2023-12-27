#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <iostream>

// g++ -ansi -std=c++11 concurrent.cpp -o concurrent -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lboost_chrono -lpthread

void Print1() {
  for (int i=0; i<5; i++) {
    boost::this_thread::sleep_for(boost::chrono::milliseconds{500});
    std::cout << "[Print1] Line: " << i << '\n';
  }
}

void Print2() {
  for (int i=0; i<5; i++) {
    boost::this_thread::sleep_for(boost::chrono::milliseconds{500});
    std::cout << "[Print2] Line: " << i << '\n';
  }
}

int main(void) {
  boost::thread_group threads;
  threads.create_thread(Print1);
  threads.create_thread(Print2);
  threads.join_all();
}
