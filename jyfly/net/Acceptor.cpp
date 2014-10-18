#include "Acceptor.h"

#include <boost/assert.hpp>
#include <boost/bind.hpp>

namespace jyfly
{
namespace net
{
	Acceptor::Acceptor(boost::asio::io_service& s)
		: m_acceptor(s)
		, m_connHandler()
	{
	}


	Acceptor::~Acceptor()
	{
	}

	void Acceptor::setConnHandler(const ConnHandler& ch)
	{
		m_connHandler = ch;
	}

	bool Acceptor::initAddress(const std::string& ip, unsigned short port)
	{
		boost::system::error_code ec;

		boost::asio::ip::tcp::endpoint ep(
			boost::asio::ip::address::from_string(ip), port);
		m_acceptor.open(ep.protocol(), ec);
		if (ec) return false;

		m_acceptor.set_option(
			boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
		if (ec) return false;

		m_acceptor.bind(ep, ec);
		if (ec) return false;

		m_acceptor.listen(32, ec);
		if (ec) return false;

		return true;
	}

	void Acceptor::accept()
	{
		ConnectionPtr pConn(new Connection(m_acceptor.get_io_service()));
		m_acceptor.async_accept(pConn->socket(),
			boost::bind(&Acceptor::onAccept, shared_from_this(),
			boost::asio::placeholders::error, pConn));
	}

	void Acceptor::onAccept(const boost::system::error_code& ec, ConnectionPtr pConn)
	{
		m_connHandler(ec, pConn);
		accept();
	}
} //namespace net
} //namespace jyfly