
#include <cstdio>
#include "io_engine.hpp"
#include "job_helper.hpp"
#include "buffer_helper.hpp"
#include "net_tcp_server.hpp"
#include "net_tcp_session.hpp"
#include "net_udp_server.hpp"
#include "net_udp_session.hpp"
#include "mem_pool.hpp"

#include "log.hpp"
#include "console_log_writer.hpp"
#include "file_log_writer.hpp"

#include <iostream>

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/circular_buffer.hpp>

#include "unit_helper.hpp"

int main()
{
	std::cout << "hello from asio_dev!" << std::endl;

	io_engine::BufferPool<io_engine::buffer>::getInstance();
	io_engine::MemPool<io_engine::MemHeader>::getInstance();
	utility::Log::Logger::getInstance().addWriter(new utility::Log::ConsoleLogWriter());
	utility::Log::Logger::getInstance().addWriter(new utility::Log::FileLogWriter("test", "/home/ubuntu/projects/log"));
	utility::Log::Logger::getInstance().start();
	io_engine::IOEngine engine{0};

	engine.start();
	io_engine::TCPServer<io_engine::NetTCPSession> server{ engine.getIO(), 10101 };

	server.start();

	auto uid = content::unit::genUUId(10, 3, 111, time(0));
	auto serverNo = content::unit::getServerNo(uid);
	auto serverType = content::unit::getServerType(uid);
	auto seq = content::unit::getSeq(uid);
	auto time1 = content::unit::getTime(uid);

	std::cout << "genUUID " << "uid " << (int64_t)uid << std::endl;
	LOG(utility::Log::info, "genUUID", "uid", (int64_t)uid, "no", (int)serverNo, "type", (int)serverType, "seq", (int)seq, "time", (int64_t)time1);

	io_engine::UDPServer<io_engine::NetUDPSession> udpServer{ engine.getIO(), 10100 };

	udpServer.start();

	engine.asyncJob(
		io_engine::makeGoobJob(
			[a = 10, b = 20]()
			{
				auto mem = io_engine::MemPool<io_engine::MemHeader>::getInstance().alloc(1024 * 64);

				io_engine::BufferHelper buf(1024);
				buf.write(10);
				buf.write(20);
				buf.write(30);
				buf.write(40);
				buf.write(50);

				LOG(utility::Log::info, "async job", "a", 10, "b", 20, "C", 100);
				io_engine::MemPool<io_engine::MemHeader>::getInstance().free(mem);
			}
		) );

	auto Afunc = [](const int& a, const int& b) { return a + b; };
	engine.asyncJob(
		io_engine::makeGoobJob(
			[Afunc](int a, int b)
			{
				io_engine::BufferHelper buf(1024);
				buf.write(10);
				buf.write(20);
				buf.write(30);
				buf.write(40);
				buf.write(50);

				LOG(utility::Log::info, "function call", "Afunc(a, b)", Afunc(a, b));
			}, 
		1234, 12345)
	);
	/*
	engine.asyncJob([]() { std::cout << "exec async job1 -" << std::this_thread::get_id() << std::endl; });
	engine.asyncJob([]() { std::cout << "exec async job2 -" << std::this_thread::get_id() << std::endl; });
	engine.asyncJob([]() { std::cout << "exec async job3 -" << std::this_thread::get_id() << std::endl; });
	engine.asyncJob([]() { std::cout << "exec async job4 -" << std::this_thread::get_id() << std::endl; });
	
	engine.syncJob([]() { std::cout << "exec sync job1-" << std::this_thread::get_id() << std::endl; });
	engine.syncJob([]() { std::cout << "exec sync job2-" << std::this_thread::get_id() << std::endl; });
	engine.syncJob([]() { std::cout << "exec sync job3-" << std::this_thread::get_id() << std::endl; });

	*/

	{
		LOG(utility::Log::info, "async timer start wait .... ");
		getchar();
		engine.asyncTimer(1000 * 10,
			io_engine::makeGoobJob(
				[](auto& a, auto& b)
				{
					io_engine::BufferHelper buf(1024);
					buf.write(10);
					buf.write(20);
					buf.write(30);
					buf.write(40);
					buf.write(50);

					LOG(utility::Log::info, "timer", "job", 1);
				}, 10, 20
			)
		);

		engine.asyncTimer(1000 * 15,
			io_engine::makeGoobJob(
				[](auto& a, auto& b)
				{
					io_engine::BufferHelper buf(1024);
					buf.write(10);
					buf.write(20);
					buf.write(30);
					buf.write(40);
					buf.write(50);

					LOG(utility::Log::info, "timer", "job", 2);
				}, 10, 20
			)
		);

		engine.asyncTimer(1000 * 12,
			io_engine::makeGoobJob(
				[](auto& a, auto& b)
				{
					io_engine::BufferHelper buf(1024);
					buf.write(10);
					buf.write(20);
					buf.write(30);
					buf.write(40);
					buf.write(50);

					LOG(utility::Log::info, "timer", "job", 3);
				}, 10, 20
			)
		);

		engine.asyncTimer(1000 * 11,
			io_engine::makeGoobJob(
				[](auto& a, auto& b)
				{
					io_engine::BufferHelper buf(1024);
					buf.write(10);
					buf.write(20);
					buf.write(30);
					buf.write(40);
					buf.write(50);

					LOG(utility::Log::info, "timer", "job", 4);
				}, 10, 20
			)
		);

		engine.asyncTimer(1000 * 12,
			io_engine::makeGoobJob(
				[](auto& a, auto& b)
				{
					io_engine::BufferHelper buf(1024);
					buf.write(10);
					buf.write(20);
					buf.write(30);
					buf.write(40);
					buf.write(50);

					LOG(utility::Log::info, "timer", "job", 5);
				}, 10, 20
			)
		);

		engine.asyncTimer(1000 * 13,
			io_engine::makeGoobJob(
				[](auto& a, auto& b)
				{
					io_engine::BufferHelper buf(1024);
					buf.write(10);
					buf.write(20);
					buf.write(30);
					buf.write(40);
					buf.write(50);

					LOG(utility::Log::info, "timer", "job", 6);
				}, 10, 20
			)
		);

	}

	//std::cout << "tcp connection start wait .... " << std::endl;

	LOG(utility::Log::info, "tcp connection start wait .... ");
	getchar();

	if(false)
	{
		auto con = std::make_shared<io_engine::ConnectTCPSession>(engine.getIO());
		con->connect("127.0.0.1", "10101");

		io_engine::BufferHelper buf(1024);
		buf.write(10);
		buf.write(20);
		buf.write(30);
		buf.write(40);
		buf.write(50);
		con->sendBuff(buf);

		LOG(utility::Log::info, "tcp close start wait .... ");
		getchar();
		con->close();
	}

	if (true)
	{
		auto udpCon = std::make_shared<io_engine::ConnectUDPSession>(engine.getIO());

		boost::asio::ip::udp::resolver resolver(engine.getIO());
		boost::asio::ip::udp::endpoint endpoint = *resolver.resolve({ boost::asio::ip::udp::v4(), 10100 });
		udpCon->setEndPoint(endpoint);
		udpCon->recvFrom();

		io_engine::BufferHelper buf(1024);
		buf.write(10);
		buf.write(20);
		buf.write(30);
		buf.write(40);
		buf.write(50);

		udpCon->sendTo(buf);
	}

	if (true)
	{
		auto udpCon = std::make_shared<io_engine::ConnectUDPSession>(engine.getIO());

		boost::asio::ip::udp::resolver resolver(engine.getIO());
		boost::asio::ip::udp::endpoint endpoint = *resolver.resolve({ boost::asio::ip::udp::v4(), 10100 });
		udpCon->setEndPoint(endpoint);
		udpCon->recvFrom();

		io_engine::BufferHelper buf(1024);
		buf.write(1);
		buf.write(2);
		buf.write(3);
		buf.write(4);
		buf.write(5);

		udpCon->sendTo(buf);
	}

	//main thread waiting

	LOG(utility::Log::info, "terminate start wait .... ");
	getchar();
	engine.stop();
	utility::Log::Logger::getInstance().stop();
	

    return 0;
}