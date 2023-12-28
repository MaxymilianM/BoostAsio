#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>

// g++ -ansi -std=c++11 dispatch.cpp -o dispatch -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread

boost::mutex global_stream_lock;

void WorkerThread(boost::shared_ptr<boost::asio::io_service> iosvc) {
  global_stream_lock.lock();
  std::cout << "Thread Start.\n";
  global_stream_lock.unlock();

  iosvc->run();

  global_stream_lock.lock();
  std::cout << "Thread Finish.\n";
  global_stream_lock.unlock();
}

void Dispatch(int i) {
  global_stream_lock.lock();
  std::cout << "dispath() Function for i = " << i <<  std::endl;
  global_stream_lock.unlock();
}

void Post(int i) {
  global_stream_lock.lock();
  std::cout << "post() Function for i = " << i <<  std::endl;
  global_stream_lock.unlock();
}

// We expect to get right ordered output between the dispatch() and post() functions.
// However, the result is different because the dispatch() function is called first 
// and the post() function is called after it.
// Why so?
// This happens because the dispatch() function can be invoked from the current worker thread,
// while the post() function has to wait until the handler of the worker is complete before it can be invoked.
// In other words, the dispatch() function's events can be executed from the current worker thread
// even if there are other pending events queued up,
// while the post() function's events have to wait
// until the handler completes the execution before being allowed to be executed.
void Running(boost::shared_ptr<boost::asio::io_service> iosvc) {
  for( int x = 0; x < 5; ++x ) {
    // The fundamental difference between the dispatch() and the post() functions is that
    // the dispatch() function completes the work right away whenever it can,
    // while the post() function always queues the work.
    iosvc->dispatch(boost::bind(&Dispatch, x));
    std::cout << "dispatch - > post" << std::endl;
    iosvc->post(boost::bind(&Post, x));
    boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
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
  std::cout << "The program will exit automatically once all work has finished." << std::endl;
  global_stream_lock.unlock();

  boost::thread_group threads;

  threads.create_thread(boost::bind(&WorkerThread, io_svc));
  std::cout << "create_thread: WorkerThread()" << std::endl;

  io_svc->post(boost::bind(&Running, io_svc));
  std::cout << "io_svc->post: Running()" << std::endl;

  worker.reset();
  std::cout << "worker.reset()" << std::endl;

  threads.join_all();
  std::cout << "threads.join_all()" << std::endl;

  return 0;
}
// Output:
// The program will exit automatically once all work has finished.
// create_thread: WorkerThread()
// io_svc->post: Running()
// worker.reset()
// Thread Start.
// dispath() Function for i = 0
// dispatch - > post
// dispath() Function for i = 1
// dispatch - > post
// dispath() Function for i = 2
// dispatch - > post
// dispath() Function for i = 3
// dispatch - > post
// dispath() Function for i = 4
// dispatch - > post
// post() Function for i = 0
// post() Function for i = 1
// post() Function for i = 2
// post() Function for i = 3
// post() Function for i = 4
// Thread Finish.
// threads.join_all()
