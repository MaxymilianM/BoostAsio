#include "wrapper.h"
// #include <conio.h> // only for Windows OS
#include <ncurses.h>
#include <stdio.h>
#include <boost/thread/mutex.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>

// g++ -ansi -std=c++11 wrapper.cpp echoclient.cpp -o echoclient -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread -lncurses -lboost_chrono

boost::mutex global_stream_lock;

class MyConnection : public Connection {
private:
  void OnAccept(const std::string &host, uint16_t port) {
    global_stream_lock.lock();
    std::cout << boost::posix_time::microsec_clock::local_time() << ": " << "[OnAccept] " << host << ":" << port << "\n";
    global_stream_lock.unlock();

    // Start the next receive
    Recv();
  }

  void OnConnect(const std::string &host, uint16_t port) {
    global_stream_lock.lock();
    std::cout << boost::posix_time::microsec_clock::local_time() << ": " << "[OnConnect] " << host << ":" << port << "\n";
    global_stream_lock.unlock();

    // Start the next receive
    Recv();

    std::string str = "GET / HTTP/1.0\r\n\r\n";

    std::vector<uint8_t> request;
    std::copy(str.begin(), str.end(), std::back_inserter(request));
    Send(request);
  }

  void OnSend(const std::vector<uint8_t> &buffer) {
    global_stream_lock.lock();
    std::cout << boost::posix_time::microsec_clock::local_time() << ": " << "[OnSend] " << buffer.size() << " bytes\n";
    for(size_t x=0; x<buffer.size(); x++) {

      std::cout << (char)buffer[x];
      if((x + 1) % 16 == 0)
        std::cout << "\n";
    }
    std::cout << "\n";
    global_stream_lock.unlock();
  }

  void OnRecv(std::vector<uint8_t> &buffer) {
    global_stream_lock.lock();
    std::cout << boost::posix_time::microsec_clock::local_time() << ": " << "[OnRecv] " << buffer.size() << " bytes\n";
    for(size_t x=0; x<buffer.size(); x++) {

      std::cout << (char)buffer[x];
      if((x + 1) % 16 == 0)
        std::cout << "\n";
    }
    std::cout << "\n";
    global_stream_lock.unlock();

    // Start the next receive
    Recv();
  }

  void OnTimer(const boost::posix_time::time_duration &delta) {
    // global_stream_lock.lock();
    // std::cout << "[OnTimer] " << delta << std::endl;
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

int main(void) {
  boost::shared_ptr<Hive> hive(new Hive());

  boost::shared_ptr<MyConnection> connection(new MyConnection(hive));
  connection->Connect("127.0.0.1", 4444);

  while (true){
    if (getch())
    {
        hive->Poll();
    }
    boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
  }

  hive->Stop();

  return 0;
}
