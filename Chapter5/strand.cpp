#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>

// g++ -ansi -std=c++11 strand.cpp -o strand -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread

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

// We avoid lock() in the Print() function and it still runs properly due to the io_service::strand
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

  // Strand is a class in the io_service object that provides handler execution serialization (ordered).
  // It can be used to ensure the work we have will be executed serially (ordered).
  boost::asio::io_service::strand strand(*io_svc);

  global_stream_lock.lock();
  std::cout << "The program will exit once all work has finished.\n";
  global_stream_lock.unlock();

  boost::thread_group threads;
  for(int i=1; i<=5; i++)
    threads.create_thread(boost::bind(&WorkerThread, io_svc, i));

  boost::this_thread::sleep(boost::posix_time::milliseconds(500));

  // Then, we use the strand object to give the work to the io_service object.
  // By using this method, we will ensure that the order of the work is exactly the same as what we have stated in the code
  strand.post(boost::bind(&Print, 1));
  strand.post(boost::bind(&Print, 2));
  strand.post(boost::bind(&Print, 3));
  strand.post(boost::bind(&Print, 4));
  strand.post(boost::bind(&Print, 5));

  worker.reset();

  threads.join_all();

  return 0;
}
