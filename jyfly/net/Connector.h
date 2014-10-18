#pragma once

#include "Connection.h"

namespace jyfly
{
namespace net
{
	class Connector
		: public boost::enable_shared_from_this<Connector>
	{
		typedef boost::function<
		void(const boost::system::error_code&, ConnectionPtr)> ConnHandler;

	public:
		Connector(boost::asio::io_service& s);
		~Connector();

		void setConnHandler(const ConnHandler& ch);
		void initAddress(const std::string& ip, unsigned short port);
		void connect();

	private:
		void onConnect(const boost::system::error_code& ec, ConnectionPtr pConn);

	private:
		boost::asio::io_service& m_service;
		boost::asio::ip::tcp::endpoint m_peer;
		ConnHandler m_connHandler;
	};
} //namespace net
} //namespace jyfly