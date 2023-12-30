#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>

// g++ -ansi -std=c++11 serverasync.cpp -o serverasync -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread

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

void OnAccept(const boost::system::error_code &ec) {
  if(ec) {
    global_stream_lock.lock();
    std::cout << "OnAccept Error: " << ec << ".\n";
    global_stream_lock.unlock();
  }
  else {
    global_stream_lock.lock();
    std::cout << "Accepted!" << ".\n";
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

  boost::shared_ptr<boost::asio::io_service::strand> strand(
    new boost::asio::io_service::strand(*io_svc)
  );

  global_stream_lock.lock();
  std::cout << "Press ENTER to exit!\n";
  global_stream_lock.unlock();

  boost::thread_group threads;
  for(int i=1; i<=2; i++)
    threads.create_thread(boost::bind(&WorkerThread, io_svc, i));

  boost::shared_ptr< boost::asio::ip::tcp::acceptor > acceptor(
    new boost::asio::ip::tcp::acceptor(*io_svc)
  );

  // However, because we only have one socket object and invoke the async_accept() function just once,
  // the program will accept one connection only.
  boost::shared_ptr<boost::asio::ip::tcp::socket> sckt(
    new boost::asio::ip::tcp::socket(*io_svc)
  );

  try {
    boost::asio::ip::tcp::resolver resolver(*io_svc);
    boost::asio::ip::tcp::resolver::query query(
      "127.0.0.1", 
      boost::lexical_cast<std::string>(4444)
    );
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
    acceptor->open(endpoint.protocol());
    acceptor->set_option(
      boost::asio::ip::tcp::acceptor::reuse_address(false));
    acceptor->bind(endpoint);
    acceptor->listen(boost::asio::socket_base::max_connections);
    acceptor->async_accept(*sckt, boost::bind(OnAccept, std::placeholders::_1));

    // The program calls the open() function to open the acceptor by using the protocol that is retrieved from the endpoint variable.
    // Then, by using the set_option function, we set an option on the acceptor to not reuse the address.
    // The acceptor is also bound to the endpoint using the bind() function.
    // After that, we invoke the listen() function to put the acceptor into the state where it will listen for new connections.
    // Finally, the acceptor will accept new connections by using the async_accept() function, which will start an asynchronous accept.

    global_stream_lock.lock();
    std::cout << "Listening on: " << endpoint << std::endl;
    global_stream_lock.unlock();
  }
  catch(std::exception &ex) {
    global_stream_lock.lock();
    std::cout << "Message: " << ex.what() << ".\n";
    global_stream_lock.unlock();
  }

  std::cin.get();

  boost::system::error_code ec;
  acceptor->close(ec);

  sckt->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
  sckt->close(ec);

  io_svc->stop();

  threads.join_all();

  return 0;
}
