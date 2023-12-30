#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>

// g++ -ansi -std=c++11 connectsync.cpp -o connectsync -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread

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
  for(int i=1; i<=2; i++)
    threads.create_thread(boost::bind(&WorkerThread, io_svc, i));


  boost::asio::ip::tcp::socket sckt(*io_svc);

  try {
    boost::asio::ip::tcp::resolver resolver(*io_svc);
    boost::asio::ip::tcp::resolver::query query("www.packtpub.com", 
                                                boost::lexical_cast<std::string>(80)); // HTTP port - 80, for HTTPS - 443
    boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
    boost::asio::ip::tcp::endpoint endpoint = *iterator;

    // ::resolver is used to get the address of the remote host that we want to connect with.
    // With the ::query class, we pass the Internet address and port as a parameter.
    // The ::query  class is used to describe the ::query that can be passed to a ::resolver.
    // By using the ::iterator class, we will define iterators from the results returned by a ::resolver.
    // After the ::iterator is successfully created, we give it to the ::endpoint type variable. 
    // The ::endpoint will store the list of ip addresses that are generated by the ::resolver.
    // Then, the socket::connect() member function will connect the socket to the ::endpoint.

    global_stream_lock.lock();
    std::cout << "Connecting to: " << endpoint << std::endl;
    global_stream_lock.unlock();

    sckt.connect(endpoint); 
    std::cout << "Connected!\n";
  }
  catch(std::exception &ex) {
    global_stream_lock.lock();
    std::cout << "Message: " << ex.what() << ".\n";
    global_stream_lock.unlock();
  }

  std::cin.get();

  // To release the connection, we have to disable the sending and receiving data process on the socket first
  // by using the shutdown() member function.
  boost::system::error_code ec;
  sckt.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec); // shutdown receive and send (both)
  // We invoke the close() member function to close the socket.
  sckt.close(ec);

  io_svc->stop();

  threads.join_all();

  return 0;
}