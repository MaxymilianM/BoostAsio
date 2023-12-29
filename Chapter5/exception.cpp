#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>

// g++ -ansi -std=c++11 exception.cpp -o exception -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread

boost::mutex global_stream_lock;

void WorkerThread(boost::shared_ptr<boost::asio::io_service> iosvc, int counter) {
  global_stream_lock.lock();
  std::cout << "Thread " << counter << " Start.\n";
  global_stream_lock.unlock();

  try {
    iosvc->run();
    // After run() program waites io_svc->post() that will throw an exception

    global_stream_lock.lock();
    std::cout << "Thread " << counter << " End.\n";
    global_stream_lock.unlock();
  }
  catch(std::exception & ex) {
    global_stream_lock.lock();
    std::cout << "Message: " << ex.what() << ".\n";
    global_stream_lock.unlock();
  }
}

void ThrowAnException(boost::shared_ptr<boost::asio::io_service> iosvc, int counter) {
  global_stream_lock.lock();
  std::cout << "Throw Exception " << counter << "\n" ;
  global_stream_lock.unlock();

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
  for(int i=1; i<=2; i++)
    threads.create_thread(boost::bind(&WorkerThread, io_svc, i));

  io_svc->post(boost::bind(&ThrowAnException, io_svc, 1));
  io_svc->post(boost::bind(&ThrowAnException, io_svc, 2));
  io_svc->post(boost::bind(&ThrowAnException, io_svc, 3));
  io_svc->post(boost::bind(&ThrowAnException, io_svc, 4));
  io_svc->post(boost::bind(&ThrowAnException, io_svc, 5));

  // We have 5 post work for the io_service object,
  // but the exception handling only handle two exceptions because once the thread has finished,
  // the join_all() function in the thread will finish the thread and exit the program. 
  threads.join_all();

  return 0;
}

// Output:
// The program will exit once all work has finished.
// Thread 1 Start.
// Throw Exception 1
// Thread 2 Start.
// Throw Exception 2
// Message: The Exception !!!.
// Message: The Exception !!!.
