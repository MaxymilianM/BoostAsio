#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>

// g++ -ansi -std=c++11 timer.cpp -o timer -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread

boost::mutex global_stream_lock;

void WorkerThread(boost::shared_ptr<boost::asio::io_service> iosvc, int counter) {
  global_stream_lock.lock();
  std::cout << "Thread " << counter << " Start.\n";
  global_stream_lock.unlock();

  while(true) {
    try {
      boost::system::error_code ec;
      iosvc->run(ec);
      if(ec) {
        global_stream_lock.lock();
        std::cout << "Message: " << ec << ".\n";
        global_stream_lock.unlock();
      }
      break;
    }
    catch(std::exception &ex) {
      global_stream_lock.lock();
      std::cout << "Message: " << ex.what() << ".\n";
      global_stream_lock.unlock();
    }
  }

  global_stream_lock.lock();
  std::cout << "Thread " << counter << " End.\n";
  global_stream_lock.unlock();
}


void TimerHandler(const boost::system::error_code & ec) {
  if(ec) {
    global_stream_lock.lock();
    std::cout << "Error Message: " << ec << ".\n";
    global_stream_lock.unlock();
  }
  else {
    // After the timer has expired, the TimerHandler function will be invoked
    // and since there is no error, the program will execute the code inside the else block.
    global_stream_lock.lock();
    std::cout << "You see this line because you have waited for 10 seconds.\n";
    std::cout << "Now press ENTER to exit.\n";
    global_stream_lock.unlock();
  }
}

int main(void) {
  boost::shared_ptr<boost::asio::io_service> io_svc(
    new boost::asio::io_service
  );

  boost::shared_ptr<boost::asio::io_service::work> worker(
    new boost::asio::io_service::work(*io_svc)
  );

  global_stream_lock.lock();
  std::cout << "Wait for ten seconds to see what happen, ";
  std::cout << "otherwise press ENTER to exit!\n";
  global_stream_lock.unlock();

  boost::thread_group threads;
  for(int i=1; i<=5; i++)
    threads.create_thread(boost::bind(&WorkerThread, io_svc, i));

  // deadline_timer is a class in the Boost C++ library that provides the ability to conduct a blocking
  // or asynchronous wait for a timer until it expires, known as the deadline timer.
  // A deadline timer indicates one of two states: expired or not expired.
  boost::asio::deadline_timer timer(*io_svc);
  timer.expires_from_now(boost::posix_time::seconds(10));
  timer.async_wait(TimerHandler);

  // Before the program calls the TimerHandler function,
  // it has to wait for 10 seconds because we use the expires_from_now function from the timer object.
  // The async_wait() function will wait until the timer has expired:
 
  std::cin.get();

  // If it is system failure - we have to invoke the stop() function in the io_service class
  // to ensure the work object has been destroyed in order for the program to be able to exit.
  io_svc->stop();

  threads.join_all();

  return 0;
}
