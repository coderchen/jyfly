#include "Connector.h"

#include <boost/assert.hpp>
#include <boost/bind.hpp>

namespace jyfly
{
namespace net
{
	Connector::Connector(boost::asio::io_service& s)
		: m_service(s)
		, m_peer()
		, m_connHandler()
	{
	}


	Connector::~Connector()
	{
	}

	void Connector::setConnHandler(const ConnHandler& ch)
	{
		m_connHandler = ch;
	}

	void Connector::initAddress(const std::string& ip, unsigned short port)
	{
		m_peer = boost::asio::ip::tcp::endpoint(
			boost::asio::ip::address::from_string(ip), port);
	}

	void Connector::connect()
	{
		BOOST_ASSERT(m_connHandler);

		ConnectionPtr pConn(new Connection(m_service));
		pConn->socket().async_connect(m_peer,
			boost::bind(&Connector::onConnect, shared_from_this(),
			boost::asio::placeholders::error, pConn));
	}

	void Connector::onConnect(const boost::system::error_code& ec, ConnectionPtr pConn)
	{
		m_connHandler(ec, pConn);
	}
} //namespace net
} //namespace jyfly