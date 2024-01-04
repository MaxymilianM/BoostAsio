#include "wrapper.h"
#include <boost/bind/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/interprocess/detail/atomic.hpp>
#include <boost/thread/mutex.hpp>

boost::mutex global_stream_lock2;

void print(const char* str)
{
    global_stream_lock2.lock();
	std::cout << boost::posix_time::microsec_clock::local_time() << ": " << str << std::endl;;
    global_stream_lock2.unlock();
}

void printError(const char* str, const char* name)
{
    global_stream_lock2.lock();
	std::cout << boost::posix_time::microsec_clock::local_time() << ": " << name << "::printError: " << str << std::endl;;
    global_stream_lock2.unlock();
}

// Hive constructor
Hive::Hive()
: m_work_ptr(new boost::asio::io_service::work(m_io_service)), m_shutdown(0)
{
	print(__func__);
}

// Hive destructor
Hive::~Hive()
{
	print(__func__);
}

// Hive::GetService definition
boost::asio::io_service &Hive::GetService()
{
	return m_io_service;
}

// Hive::HasStopped definition
bool Hive::HasStopped()
{
	return (boost::interprocess::ipcdetail::atomic_cas32(&m_shutdown, 1, 1) == 1);
}

// Hive::Poll definition
void Hive::Poll()
{
	print(__func__);
	m_io_service.poll();
}

// Hive::Run definition
void Hive::Run()
{
	print(__func__);
	m_io_service.run();

//   while(true) {
//     try {
//       boost::system::error_code ec;
//       m_io_service.run(ec);
//       if(ec) {
//         global_stream_lock2.lock();
//         std::cout << "Message: " << ec << ".\n";
//         global_stream_lock2.unlock();
//       }
//       break;
//     }
//     catch(std::exception &ex) {
//       global_stream_lock2.lock();
//       std::cout << "Message: " << ex.what() << ".\n";
//       global_stream_lock2.unlock();
//     }
//   }
}

// Hive::Stop definition
void Hive::Stop()
{
	print(__func__);
	if(boost::interprocess::ipcdetail::atomic_cas32(&m_shutdown, 1, 0) == 0)
	{
		m_work_ptr.reset();
		m_io_service.run();
		m_io_service.stop();
	}
}

// Hive::Reset definition
void Hive::Reset()
{
	print(__func__);
	if( boost::interprocess::ipcdetail::atomic_cas32(&m_shutdown, 0, 1) == 1)
	{
		m_io_service.reset();
		m_work_ptr.reset(new boost::asio::io_service::work(m_io_service));
	}
}

// Acceptor constructor 
Acceptor::Acceptor(boost::shared_ptr<Hive> hive)
: m_hive(hive), m_acceptor(hive->GetService()), m_io_strand(hive->GetService()), m_timer(hive->GetService()), m_timer_interval(1000), m_error_state(0)
{
	print(__func__);
}

// Acceptor destructor
Acceptor::~Acceptor()
{
	print(__func__);
}

// Acceptor::StartTimer definition
void Acceptor::StartTimer()
{
	m_last_time = boost::posix_time::microsec_clock::local_time();
	m_timer.expires_from_now(boost::posix_time::milliseconds(m_timer_interval));
	m_timer.async_wait(m_io_strand.wrap(boost::bind(&Acceptor::HandleTimer, shared_from_this(), std::placeholders::_1)));
}

// Acceptor::StartError definition
void Acceptor::StartError( const boost::system::error_code &error)
{
	print(__func__);
	if(boost::interprocess::ipcdetail::atomic_cas32(&m_error_state, 1, 0) == 0)
	{
		boost::system::error_code ec;
		m_acceptor.cancel(ec);
		m_acceptor.close(ec);
		m_timer.cancel(ec);
		OnError(error);
	}
}

// Acceptor::DispatchAccept definition
void Acceptor::DispatchAccept(boost::shared_ptr<Connection> connection)
{
	print(__func__);
	// Prepare Acceptor for any new connection.
	// Once connection request income, using callback HandleAccept.
	m_acceptor.async_accept(connection->GetSocket(), connection->GetStrand().wrap(boost::bind(&Acceptor::HandleAccept, shared_from_this(), std::placeholders::_1, connection)));
}

// Acceptor::HandleTimer definition
void Acceptor::HandleTimer(const boost::system::error_code &error)
{
	if(error || HasError() || m_hive->HasStopped())
		StartError(error);
	else
	{
		OnTimer(boost::posix_time::microsec_clock::local_time() - m_last_time);
		StartTimer();
	}
}

// Acceptor::HandleAccept definition
void Acceptor::HandleAccept(const boost::system::error_code &error, boost::shared_ptr<Connection> connection)
{
	print(__func__);
	if(error || HasError() || m_hive->HasStopped())
		connection->StartError(error);
	else
	{
		if(connection->GetSocket().is_open())
		{
			connection->StartTimer();
			if(OnAccept(connection, connection->GetSocket().remote_endpoint().address().to_string(), connection->GetSocket().remote_endpoint().port()))
			{
				connection->OnAccept(m_acceptor.local_endpoint().address().to_string(), m_acceptor.local_endpoint().port());
			}
		}
		else
			StartError(error);
	}
}

// Acceptor::Stop definition
void Acceptor::Stop()
{
	print(__func__);
	m_io_strand.post(boost::bind(&Acceptor::HandleTimer, shared_from_this(), boost::asio::error::connection_reset));
}

// Acceptor::Accept definition
void Acceptor::Accept(boost::shared_ptr<Connection> connection)
{
	print(__func__);
	m_io_strand.post(boost::bind(&Acceptor::DispatchAccept, shared_from_this(), connection));
}

// Acceptor::Listen definition
void Acceptor::Listen(const std::string &host, const uint16_t & port)
{
	print(__func__);
	try {
		boost::asio::ip::tcp::resolver resolver(m_hive->GetService());
		boost::asio::ip::tcp::resolver::query query(host, boost::lexical_cast<std::string>(port));
		boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
		m_acceptor.open(endpoint.protocol());
		// Set true to avoid Exception "bind: Address already in use"
		m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
		m_acceptor.bind(endpoint);
		m_acceptor.listen(boost::asio::socket_base::max_connections);
		StartTimer();
	}
	catch(std::exception &ex) {
		printError(ex.what(), "Acceptor::Listen");
  	}
}

// Acceptor::GetHive definition
boost::shared_ptr<Hive> Acceptor::GetHive()
{
	return m_hive;
}

// Acceptor::GetAcceptor definition
boost::asio::ip::tcp::acceptor &Acceptor::GetAcceptor()
{
	return m_acceptor;
}

// Acceptor::GetTimerInterval definition
int32_t Acceptor::GetTimerInterval() const
{
	return m_timer_interval;
}

// Acceptor::SetTimerInterval definition
void Acceptor::SetTimerInterval(int32_t timer_interval)
{
	m_timer_interval = timer_interval;
}

// Acceptor::HasError definition
bool Acceptor::HasError()
{
	return (boost::interprocess::ipcdetail::atomic_cas32(&m_error_state, 1, 1) == 1);
}

// Connection constructor
Connection::Connection(boost::shared_ptr<Hive> hive)
: m_hive(hive), m_socket(hive->GetService()), m_io_strand(hive->GetService()), m_timer(hive->GetService()), m_receive_buffer_size(4096), m_timer_interval(1000), m_error_state(0)
{
	print(__func__);
}

// Connection destructor
Connection::~Connection()
{
	print(__func__);
}

// Connection::Bind definition
void Connection::Bind(const std::string &ip, uint16_t port)
{
	print(__func__);
	try {
		boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(ip), port);
		m_socket.open(endpoint.protocol());
		m_socket.set_option(boost::asio::ip::tcp::acceptor::reuse_address(false));
		m_socket.bind(endpoint);
	}
	catch(std::exception &ex) {
		printError(ex.what(), "Connection::Bind");
  	}
}

// Connection::StartSend definition
void Connection::StartSend()
{
	print(__func__);
	// Bind HandleSend callback to the socket through async_write().
	// Once data sent, socket object calls HandleSend callback.
	if(!m_pending_sends.empty())
	{
		boost::asio::async_write(m_socket, boost::asio::buffer(m_pending_sends.front()), m_io_strand.wrap(boost::bind(&Connection::HandleSend, shared_from_this(), boost::asio::placeholders::error, m_pending_sends.begin())));
	}
}

// Connection::StartRecv definition
void Connection::StartRecv(int32_t total_bytes)
{
	print(__func__);
	// Bind HandleRecv callback to the socket through async_read().
	// Once any data income, socket object calls HandleRecv callback.
	if(total_bytes > 0)
	{
		m_recv_buffer.resize(total_bytes);
		boost::asio::async_read(m_socket, boost::asio::buffer(m_recv_buffer), m_io_strand.wrap(boost::bind(&Connection::HandleRecv, shared_from_this(), std::placeholders::_1, std::placeholders::_2)));
	}
	else
	{
		m_recv_buffer.resize(m_receive_buffer_size);
		m_socket.async_read_some(boost::asio::buffer(m_recv_buffer), m_io_strand.wrap(boost::bind(&Connection::HandleRecv, shared_from_this(), std::placeholders::_1, std::placeholders::_2)));
	}
}

// Connection::StartTimer definition
void Connection::StartTimer()
{
	m_last_time = boost::posix_time::microsec_clock::local_time();
	m_timer.expires_from_now(boost::posix_time::milliseconds(m_timer_interval));
	m_timer.async_wait(m_io_strand.wrap(boost::bind(&Connection::DispatchTimer, shared_from_this(), std::placeholders::_1)));
}

// Connection::StartError definition
void Connection::StartError(const boost::system::error_code &error)
{
	print(__func__);
	if(boost::interprocess::ipcdetail::atomic_cas32(&m_error_state, 1, 0) == 0)
	{
		boost::system::error_code ec;
		m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
		m_socket.close(ec);
		m_timer.cancel(ec);
		OnError(error);
	}
}

// Connection::HandleConnect definition
void Connection::HandleConnect(const boost::system::error_code &error)
{
	print(__func__);
	if(error || HasError() || m_hive->HasStopped())
		StartError( error );
	else
	{
		if(m_socket.is_open())
			OnConnect( m_socket.remote_endpoint().address().to_string(), m_socket.remote_endpoint().port() );
		else
			StartError( error );
	}
}

// Connection::HandleSend definition
void Connection::HandleSend(const boost::system::error_code &error, std::list<std::vector<uint8_t> >::iterator itr)
{
	print(__func__);
	// The socket object sent data and calls HandleRecv callback.
	if(error || HasError() || m_hive->HasStopped())
		StartError(error);
	else
	{
		OnSend(*itr);
		m_pending_sends.erase(itr);
		StartSend();
	}
}

// Connection::HandleRecv definition
void Connection::HandleRecv(const boost::system::error_code &error, int32_t actual_bytes)
{
	print(__func__);
	// The socket object received data and calls HandleRecv callback.
	if(error || HasError() || m_hive->HasStopped())
		StartError( error );
	else
	{
		m_recv_buffer.resize(actual_bytes);
		OnRecv(m_recv_buffer);
		m_pending_recvs.pop_front();
		if(!m_pending_recvs.empty())
			StartRecv( m_pending_recvs.front() );
	}
}

// Connection::HandleTimer definition
void Connection::HandleTimer(const boost::system::error_code &error)
{
	if(error || HasError() || m_hive->HasStopped())
		StartError( error );
	else
	{
		OnTimer(boost::posix_time::microsec_clock::local_time() - m_last_time);
		StartTimer();
	}
}

// Connection::DispatchSend definition
void Connection::DispatchSend(std::vector<uint8_t> buffer)
{
	print(__func__);
	// DispatchSend was called from the thread queue.
	bool should_start_send = m_pending_sends.empty();
	m_pending_sends.push_back(buffer);
	if(should_start_send)
		StartSend();
}

// Connection::DispatchRecv definition
void Connection::DispatchRecv(int32_t total_bytes)
{
	print(__func__);
	// DispatchRecv was called from the thread queue.
	bool should_start_receive = m_pending_recvs.empty();
	m_pending_recvs.push_back(total_bytes);
	if(should_start_receive)
		StartRecv(total_bytes);
}

// Connection::DispatchTimer definition
void Connection::DispatchTimer(const boost::system::error_code &error)
{
	m_io_strand.post(boost::bind(&Connection::HandleTimer, shared_from_this(), error));
}

// Connection::Connect definition
void Connection::Connect(const std::string & host, uint16_t port)
{
	print(__func__);
	try {
		boost::system::error_code ec;
		boost::asio::ip::tcp::resolver resolver(m_hive->GetService());
		boost::asio::ip::tcp::resolver::query query(host, boost::lexical_cast<std::string>(port));
		boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
		boost::asio::ip::tcp::endpoint endpoint = *iterator;
		m_socket.async_connect(endpoint, m_io_strand.wrap(boost::bind(&Connection::HandleConnect, shared_from_this(), std::placeholders::_1)));
		StartTimer();
	}
	catch(std::exception &ex) {
		printError(ex.what(), "Connection::Connect");
  	}
}

// Connection::Disconnect definition
void Connection::Disconnect()
{
	print(__func__);
	m_io_strand.post(boost::bind(&Connection::HandleTimer, shared_from_this(), boost::asio::error::connection_reset));
}

// Connection::Recv definition
void Connection::Recv(int32_t total_bytes)
{
	print(__func__);
	// Initiate connection to receive data after connection accepted.
	// Placed DispatchRecv callback in the thread queue.
	m_io_strand.post(boost::bind(&Connection::DispatchRecv, shared_from_this(), total_bytes));
}

// Connection::Send definition
void Connection::Send(const std::vector<uint8_t> &buffer)
{
	print(__func__);
	// Initiate connection to send data after handling received data in OnRecv().
	// Placed DispatchSend callback in the thread queue.
	m_io_strand.post(boost::bind(&Connection::DispatchSend, shared_from_this(), buffer));
}

// Connection::GetSocket definition
boost::asio::ip::tcp::socket &Connection::GetSocket()
{
	return m_socket;
}

// Connection::GetStrand definition
boost::asio::io_service::strand &Connection::GetStrand()
{
	return m_io_strand;
}

// Connection::GetHive definition
boost::shared_ptr<Hive> Connection::GetHive()
{
	return m_hive;
}

// Connection::SetReceiveBufferSize definition
void Connection::SetReceiveBufferSize(int32_t size)
{
	m_receive_buffer_size = size;
}

// Connection::GetReceiveBufferSize definition
int32_t Connection::GetReceiveBufferSize() const
{
	return m_receive_buffer_size;
}

// Connection::GetTimerInterval definition
int32_t Connection::GetTimerInterval() const
{
	return m_timer_interval;
}

// Connection::SetTimerInterval definition
void Connection::SetTimerInterval(int32_t timer_interval)
{
	m_timer_interval = timer_interval;
}

// Connection::HasError definition
bool Connection::HasError()
{
	return (boost::interprocess::ipcdetail::atomic_cas32(&m_error_state, 1, 1) == 1);
}
