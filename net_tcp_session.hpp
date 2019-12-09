#pragma once
#include <boost/asio.hpp>
#include <atomic>
#include <boost/circular_buffer.hpp>
#include "buffer_helper.hpp"
#include "mem_pool.hpp"

namespace io_engine {

using namespace boost::asio;

class NetTCPSession : public std::enable_shared_from_this<NetTCPSession>
{
	const static std::size_t buff_size = 65536;
	const static std::size_t buff_count = 16;
public:
	DECLARE_MEMPOOL_NEW_DELETE;

	NetTCPSession(ip::tcp::socket&& s_) noexcept : _socket(std::forward<ip::tcp::socket>(s_))
	{
		_recvBuff.resize(buff_count); 
	}
	NetTCPSession(io_service& io_) noexcept :_socket{io_}
	{
		_recvBuff.resize(buff_count); 
	}
	virtual ~NetTCPSession() {}

	ip::tcp::socket& getSocket() { return _socket; }

public:
	virtual void onAcceptComplete() 
	{
		_connected.store(true);
		postRecv();
	}
	virtual void onConnectComplete(const boost::system::error_code& e_)
	{}
	virtual void onRecvComplete(boost::system::error_code e_, std::size_t l_) 
	{
		if (e_ != boost::asio::error::would_block)
		{
			_connected.store(false);
			onClosed();
			return;
		}

		auto buff = _recvBuff.front();
		_recvBuff.pop_front();

		// parsing buff
		// ...
		// ...

		_recvBuff.push_back(buff);
	}
	virtual void onSendComplete(boost::system::error_code e_, std::size_t l_) 
	{
		_sending.store(false);
		auto buff = _sendBuff.front();
		_sendBuff.pop_front();
	}
	virtual void onClosed() {}
public:
	bool sendBuff(const BufferHelper& buf_)
	{
		if (_connected.load() == false) return false;

		// _sendbuff push
		_sendBuff.push_back(buf_);

		if (_sending.load() == false)
		{
			postSend();
		}
		return true;
	}

	void close()
	{
		_socket.close();
	}

protected:
	void postRecv()
	{
		// recv buffer
		_socket.async_receive(
			boost::asio::buffer(&_recvBuff.front(), _recvBuff.front().size()),
			[session = shared_from_this()](auto e_, auto l_) { session->onRecvComplete(e_, l_); });
	}

	void postSend()
	{
		// send buffer
		_socket.async_send(
			boost::asio::buffer(_sendBuff.front().start(), _sendBuff.front().length()),
			[session = shared_from_this()](auto e_, auto l_) { session->onSendComplete(e_, l_); });
	}

protected:
	ip::tcp::socket _socket;
	std::atomic<bool> _connected{ false };
	// buffer
	boost::circular_buffer<std::array<char, buff_size>> _recvBuff{ buff_count };
	boost::circular_buffer<BufferHelper> _sendBuff{ buff_count };

	// sending status
	std::atomic<bool> _sending{ false };
};


class ConnectTCPSession : public NetTCPSession
{
public:
	ConnectTCPSession(io_service& io_) noexcept : NetTCPSession(io_)
	{}

public:
	virtual bool connect(const char* addr_, const char* port_)
	{
		if (_connected.load() == true) return true;

		ip::tcp::resolver r{ _socket.get_io_service() };
		auto i = r.resolve(ip::tcp::resolver::query(addr_,port_));

		async_connect(_socket, i,
			[this](const boost::system::error_code& e_, ip::tcp::resolver::iterator p_)
			{
				onConnectComplete(e_);
			}
		);

		return true;
	}

public:
	virtual void onConnectComplete(const boost::system::error_code& e_) override
	{
		if (e_)
		{
			return;
		}

		_connected.store(true);
		postRecv();
	}
};

}