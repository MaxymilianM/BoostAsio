#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <iostream>

// g++ -ansi -std=c++11 multithreads.cpp -o multithreads -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread

boost::asio::io_service io_svc;
int a = 0;


// In every thread that is created, the program will invoke the run() function to run the work of the io_service object.
// Calling the run() function once is insufficient because all nonworkers will be invoked after the run() object finishes all its work.
void WorkerThread() {
  std::cout << ++a << ".\n";
  io_svc.run();
  std::cout << "End.\n";
}

int main(void) {
  boost::shared_ptr<boost::asio::io_service::work> worker(
    new boost::asio::io_service::work(io_svc)
  );

  std::cout << "Press ENTER key to exit!" << std::endl;

  // Create 5 threads and run them one by one
  boost::thread_group threads;
  for(int i=0; i<5; i++)
    threads.create_thread(WorkerThread);

  // Wait for user input Enter
  std::cin.get();

  // Notify the io_service object that all the work should be stopped,
  // so we unblocked all io_svc.run()
  io_svc.stop();
  std::cout << "io_svc.stop().\n";

  // Continue with all the unfinished threads, 
  // and the program will wait until all the processes in all the threads are completed.
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
