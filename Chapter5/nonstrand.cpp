#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>

// g++ -ansi -std=c++11 nonstrand.cpp -o nonstrand -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread

boost::mutex global_stream_lock;

void WorkerThread(boost::shared_ptr<boost::asio::io_service> iosvc, int counter) {
  global_stream_lock.lock();
  std::cout << "Thread " << counter << " Start.\n";
  global_stream_lock.unlock();

  // Thread is blocked for io_svc->post() that we call further
  iosvc->run();

  // Thread is not blocked and just go ahead skippink io_svc->post() that we call further
  // iosvc->poll();

  global_stream_lock.lock();
  std::cout << "Thread " << counter << " End.\n";
global_stream_lock.unlock();
}

void Print(int number) {
  std::cout << "Number: " << number << std::endl;
}

int main(void) {
  boost::shared_ptr<boost::asio::io_service> io_svc(
    new boost::asio::io_service
  );

  boost::shared_ptr<boost::asio::io_service::work> worker(
    new boost::asio::io_service::work(*io_svc)
  );

  global_stream_lock.lock();
  std::cout << "The program will exit once all work has finished.\n";
  global_stream_lock.unlock();

  boost::thread_group threads;
  for(int i=1; i<=5; i++)
    threads.create_thread(boost::bind(&WorkerThread, io_svc, i));

  boost::this_thread::sleep(boost::posix_time::milliseconds(500));

  io_svc->post(boost::bind(&Print, 1));
  io_svc->post(boost::bind(&Print, 2));
  io_svc->post(boost::bind(&Print, 3));
  io_svc->post(boost::bind(&Print, 4));
  io_svc->post(boost::bind(&Print, 5));

  worker.reset();

  threads.join_all();


  return 0;
}

// Output: 
// The program will exit once all work has finished.
// Thread 1 Start.
// Thread 3 Start.
// Thread 2 Start.
// Thread 4 Start.
// Thread 5 Start.
// Number: 2Number: 1
// Number: 4

// Number: Number: 5
// 3
// Thread 3 End.
// Thread 2 End.
// Thread 4 End.
// Thread 5 End.
// Thread 1 End
