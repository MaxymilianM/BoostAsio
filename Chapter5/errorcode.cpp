#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>

// g++ -ansi -std=c++11 errorcode.cpp -o errorcode -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread

// std::size_t boost::asio::io_service::run(boost::system::error_code & ec);

boost::mutex global_stream_lock;

void WorkerThread(boost::shared_ptr<boost::asio::io_service> iosvc, int counter) {
  global_stream_lock.lock();
  std::cout << "Thread " << counter << " Start.\n";
  global_stream_lock.unlock();

  // run() has an error code parameter that will be set if an error occurs.
  boost::system::error_code ec;
  iosvc->run(ec);

  // With error variable approach, user exceptions translate to boost::asio exceptions.
  // The error variable "ec" does not interpret the user exception as an error so the exception is not caught by the handler.
  // If the Boost.Asio library needs to throw an error, it will become an exception if there is no error variable,
  // or it will be converted into an error variable.
  // It is better if we keep using the try-catch block to catch any exceptions or errors.

  if(ec) {
    global_stream_lock.lock();
    std::cout << "Message: " << ec << ".\n";
    global_stream_lock.unlock();
  }

  global_stream_lock.lock();
  std::cout << "Thread " << counter << " End.\n";
  global_stream_lock.unlock();
}

void ThrowAnException(boost::shared_ptr<boost::asio::io_service> iosvc) {
  global_stream_lock.lock();
  std::cout << "Throw Exception\n" ;
  global_stream_lock.unlock();

  iosvc->post(boost::bind(&ThrowAnException, iosvc));

  throw(std::runtime_error("The Exception !!!"));
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

  io_svc->post(boost::bind(&ThrowAnException, io_svc));

  threads.join_all();

  return 0;
}

// Output:
// The program will exit once all work has finished.
// Thread 1 Start.
// Thread 3 Start.
// Thread 4 Start.
// Thread 5 Start.
// Throw Exception
// Thread 2 Start.
// Throw Exception
// terminate called after throwing an instance of 'Throw Exception
// terminate called recursively
// std::runtime_error'
// Aborted
