#pragma once
#include <boost/asio.hpp>
#include <atomic>
#include <boost/circular_buffer.hpp>
#include "buffer_helper.hpp"
#include "mem_pool.hpp"

namespace io_engine {

using namespace boost::asio;
class NetUDPSession : public std::enable_shared_from_this<NetUDPSession>
{
	const static std::size_t buff_size = 65536;
	const static std::size_t buff_count = 16;
public:
	DECLARE_MEMPOOL_NEW_DELETE;

	NetUDPSession(ip::udp::socket&& s_) noexcept : _socket(std::forward<ip::udp::socket>(s_))
	{
		_recvBuff.resize(buff_count); 
	}
	NetUDPSession(io_service& io_) noexcept :_socket{io_, ip::udp::endpoint(ip::udp::v4(), 0)}
	{
		_recvBuff.resize(buff_count); 
	}
	virtual ~NetUDPSession() {}

	ip::udp::socket& getSocket() { return _socket; }

	ip::udp::endpoint& getEndPoint() { return _remoteEndPoint; }

	std::array<char, buff_size>& getRecvBuffer()
	{
		return _recvBuff.front();
	}

public:
	void recvFrom()
	{
		_socket.async_receive_from(
			boost::asio::buffer(&_recvBuff.front(), _recvBuff.front().size()),
			_remoteEndPoint,
			[session = shared_from_this()](auto e_, auto l_) { session->onRecvComplete(e_, l_); });
	}

	void sendTo(BufferHelper& buf_)
	{
		_sendBuff.push_back(buf_);

		_socket.async_send_to(
			boost::asio::buffer(buf_.start(), buf_.length()),
			getEndPoint(),
			[session = shared_from_this()](auto e_, auto l_) { session->onSendComplete(e_, l_); }
		);
	}

	virtual void onRecvComplete(boost::system::error_code e_, std::size_t l_) 
	{
		//if (e_ != boost::asio::error::would_block)
		if (e_)
		{
			return;
		}

		auto buff = _recvBuff.front();
		/*
		// parsing buff
		if (auto result = _parser.Pasing(buff, l_); result == false)
		{
			// not yet full packet try again
			_socket.async_receive_from(
				boost::asio::buffer(&buff[l_], buff.size() - l_),
				_remoteEndPoint,
				[session = shared_from_this()](auto e_, auto l_) { session->onRecvComplete(e_, l_); });

			return;
		}
		else
		{
			auto message = *result;
		}
		*/
		_recvBuff.pop_front();
		_recvBuff.push_back(buff);

		recvFrom();
	}
	virtual void onSendComplete(boost::system::error_code e_, std::size_t l_) 
	{
		auto buff = _sendBuff.front();
		_sendBuff.pop_front();
	}
	virtual void onClosed() {}
public:	

	void close()
	{
		_socket.close();
	}


protected:
	ip::udp::socket _socket;
	ip::udp::endpoint _remoteEndPoint;

	// buffer
	boost::circular_buffer<std::array<char, buff_size>> _recvBuff{ buff_count };
	boost::circular_buffer<BufferHelper> _sendBuff{ buff_count };
};


class ConnectUDPSession : public NetUDPSession
{
public:
	ConnectUDPSession(io_service& io_) noexcept : NetUDPSession(io_)
	{}

	void setEndPoint(ip::udp::endpoint endpoint_)
	{
		_remoteEndPoint = endpoint_;
	}
public:

};

}


