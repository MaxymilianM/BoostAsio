#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>

// g++ -ansi -std=c++11 strandwrap.cpp -o strandwrap -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread

boost::mutex global_stream_lock;

void WorkerThread(boost::shared_ptr<boost::asio::io_service> iosvc, int counter) {
  global_stream_lock.lock();
  std::cout << "Thread " << counter << " Start.\n";
  global_stream_lock.unlock();

  iosvc->run();

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

  boost::asio::io_service::strand strand(*io_svc);

  global_stream_lock.lock();
  std::cout << "The program will exit once all work has finished." <<  std::endl;
  global_stream_lock.unlock();

  boost::thread_group threads;
  for(int i=1; i<=5; i++)
    threads.create_thread(boost::bind(&WorkerThread, io_svc, i));

  // boost::asio::strand has wrap() method.
  // It creates a new handler function object that will automatically pass the wrapped handler
  // to the strand object's dispatch function when it is called.
  // The work is guaranteed to be executed serially, 
  // but it is not guaranteed which work's order actually takes place as a result of the built-in handler wrapper.
  boost::this_thread::sleep(boost::posix_time::milliseconds(100));
  io_svc->post(strand.wrap(boost::bind(&Print, 1)));
  io_svc->post(strand.wrap(boost::bind(&Print, 2)));

  boost::this_thread::sleep(boost::posix_time::milliseconds(100));
  io_svc->post(strand.wrap(boost::bind(&Print, 3)));
  io_svc->post(strand.wrap(boost::bind(&Print, 4)));

  boost::this_thread::sleep(boost::posix_time::milliseconds(100));
  io_svc->post(strand.wrap(boost::bind(&Print, 5)));
  io_svc->post(strand.wrap(boost::bind(&Print, 6)));

  worker.reset();

  threads.join_all();

  return 0;
}

// Output:
// The program will exit once all work has finished.
// Thread 1 Start.
// Thread 2 Start.
// Thread 3 Start.
// Thread 4 Start.
// Thread 5 Start.
// Number: 2
// Number: 1

// Number: 3
// Number: 4

// Number: 6
// Number: 5
// Thread 3 End.
// Thread 4 End.
// Thread 5 End.
// Thread 1 End.
// Thread 2 End.
