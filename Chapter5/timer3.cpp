#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>

// g++ -ansi -std=c++11 timer3.cpp -o timer3 -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread

boost::mutex global_stream_lock;

void WorkerThread(boost::shared_ptr<boost::asio::io_service> iosvc, int counter) {
  global_stream_lock.lock();
  std::cout << "Thread " << counter << " Start.\n";
  global_stream_lock.unlock();

  while( true ) {
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

// From the output, we can see that the first five work objects are executed first
// because they have to be serially executed
// and afterwards, the TimerHandler() functions are executed.
// The work objects have to be completed first before the timer thread is executed.
void TimerHandler(
  const boost::system::error_code &ec,
  boost::shared_ptr<boost::asio::deadline_timer> tmr,
  boost::shared_ptr<boost::asio::io_service::strand> strand
)
{
  if(ec) {
    global_stream_lock.lock();
    std::cout << "Error Message: " << ec << ".\n";
    global_stream_lock.unlock();
  }
  else {
    global_stream_lock.lock();
    std::cout << "You see this every three seconds.\n";
    global_stream_lock.unlock();

    tmr->expires_from_now( boost::posix_time::seconds(1));
    tmr->async_wait(
      strand->wrap(boost::bind(&TimerHandler, std::placeholders::_1, tmr, strand))
    );
  }
}

void Print(int number) {
  std::cout << "Number: " << number << std::endl;
  boost::this_thread::sleep( boost::posix_time::milliseconds(500));
}

int main(void) {
  boost::shared_ptr<boost::asio::io_service> io_svc(
    new boost::asio::io_service
  );

  boost::shared_ptr<boost::asio::io_service::work> worker(
    new boost::asio::io_service::work(*io_svc)
  );
  boost::shared_ptr<boost::asio::io_service::strand> strand(
    new boost::asio::io_service::strand(*io_svc)
  );

  global_stream_lock.lock();
  std::cout << "Press ENTER to exit!\n";
  global_stream_lock.unlock();

  boost::thread_group threads;
  for(int i=1; i<=5; i++)
    threads.create_thread(boost::bind(&WorkerThread, io_svc, i));

  boost::this_thread::sleep(boost::posix_time::seconds(1));

  strand->post(boost::bind(&Print, 1));
  strand->post(boost::bind(&Print, 2));
  strand->post(boost::bind(&Print, 3));
  strand->post(boost::bind(&Print, 4));
  strand->post(boost::bind(&Print, 5));

  boost::shared_ptr<boost::asio::deadline_timer> timer(
    new boost::asio::deadline_timer(*io_svc)
  );

  timer->expires_from_now( boost::posix_time::seconds(1));
  // std::placeholders::_1 will be replaced with the first argument given from async_wait().
  timer->async_wait( 
    strand->wrap(boost::bind(&TimerHandler, std::placeholders::_1, timer, strand))
  );

  std::cin.get();

  io_svc->stop();

  threads.join_all();

  return 0;
}

// Output:
// Press ENTER to exit!
// Thread 1 Start.
// Thread 2 Start.
// Thread 3 Start.
// Thread 4 Start.
// Thread 5 Start.
// Number: 1
// Number: 2
// Number: 3
// Number: 4
// Number: 5
// You see this every three seconds.
// You see this every three seconds.
// You see this every three seconds.

// Thread 5 End.
// Thread 1 End.
// Thread 2 End.
// Thread 3 End.
// Thread 4 End.
