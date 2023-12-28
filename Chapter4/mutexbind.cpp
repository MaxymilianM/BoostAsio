#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>

// g++ -ansi -std=c++11 mutexbind.cpp -o mutexbind -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread

// One thing to remember is that we should not call the lock() function recursively
// because if the lock() function is not unlocked by the unlock() function,
// then thread deadlock will occur and it will freeze the application.
// So, we have to be careful when using the lock() and unlock() functions.

// We instantiate the new mutex object, global_stream_lock.
// With this object, we can call the lock() and unlock() functions.
boost::mutex global_stream_lock;

void WorkerThread(boost::shared_ptr<boost::asio::io_service> iosvc, int counter) {
  // The lock() function will block other threads that access the same function to wait for the current thread to be finished.
  global_stream_lock.lock();
  std::cout << counter << ".\n";
  global_stream_lock.unlock();

  iosvc->run();

  global_stream_lock.lock();
  std::cout << "End.\n";
  global_stream_lock.unlock();
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
// 1.
// 2.
// 3.
// 4.
// 5.

// io_svc.stop().
// End.
// End.
// End.
// End.
// End.
// threads.join_all()