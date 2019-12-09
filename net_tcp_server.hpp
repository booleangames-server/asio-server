#pragma once
#include <boost/asio.hpp>

namespace io_engine {
using namespace boost::asio;

template<typename _T>
class TCPServer
{
public:
	TCPServer(io_service& io_, short port_) :_acceptor(io_, ip::tcp::endpoint(ip::tcp::v4(), port_)) {};
	virtual ~TCPServer() = default;

	void start()
	{
		_acceptor.set_option(ip::tcp::acceptor::reuse_address(true));
		_acceptor.listen();

		asyncAccept();
	}

	void stop()
	{
		_acceptor.close();
	}

public:
	void asyncAccept()
	{
		if (_acceptor.is_open() == false) return;

		ip::tcp::socket socket(_acceptor.get_io_service());
		auto session = std::make_shared<_T>(std::move(socket));
		
		_acceptor.async_accept(session->getSocket(),
			[&server = *this, session](const auto& e_) mutable {
				if (!e_)
				{
					session->onAcceptComplete();
				}

				server.asyncAccept();
			}
		);
	}

private:
	ip::tcp::acceptor _acceptor;
};
}
