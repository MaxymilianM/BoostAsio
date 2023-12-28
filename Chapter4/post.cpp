#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>

// g++ -ansi -std=c++11 post.cpp -o post -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread

boost::mutex global_stream_lock;

void WorkerThread(boost::shared_ptr<boost::asio::io_service> iosvc, int counter) {
  global_stream_lock.lock();
  std::cout << counter << ".\n";
  global_stream_lock.unlock();
  
  // Program waits here.
  iosvc->run();

  global_stream_lock.lock();
  std::cout << "End.\n";
  global_stream_lock.unlock();
}

// Function to calculate the n factorial recursively.
size_t fac(size_t n) {
  if ( n <= 1 ) {
    return n;
  }
  // Time delay to slow down the process in order to see the work of our worker threads:
  boost::this_thread::sleep(
    boost::posix_time::milliseconds(1000)
  );
  return n * fac(n - 1);
}

void CalculateFactorial(size_t n) {
  global_stream_lock.lock();
  std::cout << "Calculating " << n << "! factorial" << std::endl;
  global_stream_lock.unlock();

  size_t f = fac(n);

  global_stream_lock.lock();
  std::cout << n << "! = " << f << std::endl;
  global_stream_lock.unlock();
}

int main(void) {
  boost::shared_ptr<boost::asio::io_service> io_svc(
    new boost::asio::io_service
  );

  boost::shared_ptr<boost::asio::io_service::work> worker(
    new boost::asio::io_service::work(*io_svc)
  );

  global_stream_lock.lock();
  std::cout << "The program will exit once all work has finished." << std::endl;
  global_stream_lock.unlock();

  boost::thread_group threads;
  for(int i=1; i<=5; i++)
    threads.create_thread(boost::bind(&WorkerThread, io_svc, i));

  std::cout << "All threads were created." << std::endl;

  // The program runs the thread from the pool of threads, and after it finishes one thread,
  // it calls the post() function from the io_service object until all three post() functions
  // and all five threads have been called.
  // Then, it calculates the factorial for each three n number.
  io_svc->post(boost::bind(CalculateFactorial, 5));
  io_svc->post(boost::bind(CalculateFactorial, 6));
  io_svc->post(boost::bind(CalculateFactorial, 7));
  std::cout << "Post done." << std::endl;

  // After program reset shared pointer,
  // Destructor of the work class notified that the work has been finished.
  worker.reset();
  std::cout << "worker.reset()" << std::endl;

  // Joins all the threads via the threads.join_all() function and wait for result.
  threads.join_all();

  return 0;
}

// Output:
// The program will exit once all work has finished.
// 1.
// 2.
// 3.
// 4.
// All threads were created.
// Calculating 5! factorial
// Post done.
// worker.reset()
// Calculating 6! factorial
// Calculating 7! factorial
// 5.
// 5! = 120
// 6! = 720
// 7! = 5040
// End.
// End.
// End.
// End.
// End.