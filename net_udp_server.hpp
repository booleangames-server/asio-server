#pragma once
#include <boost/asio.hpp>

namespace io_engine {
using namespace boost::asio;

template<typename _T>
class UDPServer
{
	const static std::size_t buff_size = 65536;
	const static std::size_t buff_count = 16;

public:
	UDPServer(io_service& io_, short port_) :_socket(io_, ip::udp::endpoint(ip::udp::v4(), port_)) {};
	virtual ~UDPServer() = default;

	void start()
	{
		startRecv();
	}

	void stop()
	{
		_socket.close();
	}

public:
	void startRecv()
	{
		if (_socket.is_open() == false) return;

		auto session = std::make_shared<_T>(_socket.get_io_service());
		_socket.async_receive_from(
			boost::asio::buffer(&session->getRecvBuffer(), session->getRecvBuffer().size()),
			session->getEndPoint(),
			[&server = *this, session](const auto e_, const auto l_) mutable {
				if (!e_ && l_ > 0)
				{
					session->onRecvComplete(e_, l_);
				}

				server.startRecv();
			}
		);


		/*
		std::array<char, 1024> buf;
		_socket.async_receive_from(
			boost::asio::buffer(&buf, buf.size()),
			_endPoint,
			[&server = *this](const auto e_, const auto l_) mutable {
				if (!e_ && l_ > 0)
				{
					server.send();
				}
				else
				{
					server.startRecv();
				}
			}
		);
		*/
	}

	void send()
	{
		std::array<char, 1024> buf;
		_socket.async_send_to(
			boost::asio::buffer(&buf, buf.size()), 
			_endPoint,
			[&server = *this](const auto e_, const auto l_) mutable {
				server.startRecv();
			}
		);
	}

private:
	ip::udp::socket _socket{};
	ip::udp::endpoint _endPoint{};
};



}