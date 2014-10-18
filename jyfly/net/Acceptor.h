#pragma once

#include "Connection.h"

namespace jyfly
{
namespace net
{
	class Acceptor
		: public boost::enable_shared_from_this<Acceptor>
	{
		typedef boost::function<
		void(const boost::system::error_code&, ConnectionPtr)> ConnHandler;

	public:
		Acceptor(boost::asio::io_service& s);
		~Acceptor();

		void setConnHandler(const ConnHandler& ch);
		bool initAddress(const std::string& ip, unsigned short port);
		void accept();

	private:
		void onAccept(const boost::system::error_code& ec, ConnectionPtr pConn);

	private:
		boost::asio::ip::tcp::acceptor m_acceptor;
		ConnHandler m_connHandler;
	};
} //namespace net
} //namespace jyfly



