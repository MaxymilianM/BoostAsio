#include "wrapper.h"
// #include <conio.h> // only for Windows OS
// #include <ncurses.h>
#include <stdio.h>
#include <boost/thread/mutex.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp> 

// g++ -ansi -std=c++11 wrapper.cpp echoserverasync.cpp -o echoserverasync -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread -lncurses -lboost_chrono

boost::mutex global_stream_lock;

class MyConnection : public Connection {
private:
  void OnAccept(const std::string &host, uint16_t port) {
    global_stream_lock.lock();
    std::cout << boost::posix_time::microsec_clock::local_time() << ": " << "[OnAccept] " << host << ":" << port << "\n";
    global_stream_lock.unlock();

    Recv();
  }

  void OnConnect(const std::string & host, uint16_t port) {
    global_stream_lock.lock();
    std::cout << boost::posix_time::microsec_clock::local_time() << ": " << "[OnConnect] " << host << ":" << port << "\n";
    global_stream_lock.unlock();

    Recv();
  }

  void OnSend(const std::vector<uint8_t> & buffer) {
    global_stream_lock.lock();
    std::cout << boost::posix_time::microsec_clock::local_time() << ": " << "[OnSend] " << buffer.size() << " bytes\n";
    for(size_t x=0; x<buffer.size(); x++) {

      std::cout << (char)buffer[x];
      if((x + 1) % 16 == 0)
        std::cout << std::endl;
    }
    std::cout << std::endl;
    global_stream_lock.unlock();
  }

  void OnRecv(std::vector<uint8_t> &buffer) {
    global_stream_lock.lock();
    std::cout << boost::posix_time::microsec_clock::local_time() << ": " << "[OnRecv] " << buffer.size() << " bytes\n";
    for(size_t x=0; x<buffer.size(); x++) {

      std::cout << (char)buffer[x];
      if((x + 1) % 16 == 0)
        std::cout << std::endl;
    }
    std::cout << std::endl;
    global_stream_lock.unlock();

    // Start the next receive
    Recv();

    // Echo the data back
    Send(buffer);
  }

  void OnTimer(const boost::posix_time::time_duration &delta) {
    // global_stream_lock.lock();
    // std::cout << "[OnTimer] " << delta << "\n";
    // global_stream_lock.unlock();
  }

  void OnError(const boost::system::error_code &error) {
    global_stream_lock.lock();
    std::cout << boost::posix_time::microsec_clock::local_time() << ": " << "[OnError] " << error << "\n";
    global_stream_lock.unlock();
  }

public:
  MyConnection(boost::shared_ptr<Hive> hive)
    : Connection(hive) {
  }

  ~MyConnection() {
  }
};

class MyAcceptor : public Acceptor {
private:
  bool OnAccept(boost::shared_ptr<Connection> connection, const std::string &host, uint16_t port) {
    global_stream_lock.lock();
    std::cout << boost::posix_time::microsec_clock::local_time() << ": " << "[OnAccept] " << host << ":" << port << "\n";
    global_stream_lock.unlock();

    return true;
  }

  void OnTimer(const boost::posix_time::time_duration &delta) {
    // global_stream_lock.lock();
    // std::cout << "[OnTimer] " << delta << "\n";
    // global_stream_lock.unlock();
  }

  void OnError(const boost::system::error_code &error) {
    global_stream_lock.lock();
    std::cout << boost::posix_time::microsec_clock::local_time() << ": " << "[OnError] " << error << "\n";
    global_stream_lock.unlock();
  }

public:
  MyAcceptor(boost::shared_ptr<Hive> hive)
    : Acceptor(hive) {
  }

  ~MyAcceptor() {
  }
};

int main(void) {
  boost::shared_ptr<Hive> hive(new Hive());
  int number_of_threads = 1000;
  boost::thread_group threads;
  for(int i = 0; i < number_of_threads; i++)
    threads.create_thread(boost::bind(&Hive::Run, hive));

  boost::shared_ptr<MyAcceptor> acceptor(new MyAcceptor(hive));
  acceptor->Listen("127.0.0.1", 4444);

  std::vector< boost::shared_ptr<MyConnection> > connections;
  for(int i = 0; i < number_of_threads; i++)
  {
    connections.emplace_back(new MyConnection(hive));
    acceptor->Accept(connections[i]);
  }

  // Wait for user input to interrapt the program. 
  std::cin.get();

  for(int i = 0; i < number_of_threads; i++)
  {
    connections[i]->Disconnect();;
  }

  hive->Stop();

  threads.join_all();

  return 0;
}
