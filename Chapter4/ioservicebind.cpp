#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>

// g++ -ansi -std=c++11 ioservicebind.cpp -o ioservicebind -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread

void WorkerThread(boost::shared_ptr<boost::asio::io_service> iosvc, int counter) {
  std::cout << counter << ".\n";
  iosvc->run();
  std::cout << "End.\n";
}

int main(void) {
  boost::shared_ptr<boost::asio::io_service> io_svc(
    new boost::asio::io_service
  );

  boost::shared_ptr<boost::asio::io_service::work> worker(
    new boost::asio::io_service::work(*io_svc)
  );

  std::cout << "Press ENTER key to exit!" << std::endl;

  boost::thread_group threads;
  for(int i=1; i<=5; i++)
    threads.create_thread(boost::bind(&WorkerThread, io_svc, i));

  std::cin.get();

  io_svc->stop();
  std::cout << "io_svc.stop().\n";

  threads.join_all();
  std::cout << "threads.join_all().\n";

  return 0;
}

// Output:
// <Random order from 1 to 5>

// io_svc.stop().
// End.
// End.
// End.
// End.
// End.
// threads.join_all()