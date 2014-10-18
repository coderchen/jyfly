#include "Connection.h"

#include <cstring>
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/assert.hpp>

namespace jyfly
{
	namespace net
	{
		Connection::Connection(boost::asio::io_service& s)
			: m_socket(s)
			, m_readHandler()
			, m_writeHandler()
			, m_closeHandler()
			, m_bRecving(false)
			, m_bSending(false)
			, m_inputLength(0)
			, m_outputLength(0)
			, m_inputQueue()
			, m_outputQueue()
		{
		}


		Connection::~Connection()
		{
			for (BufferQueue::iterator it = m_inputQueue.begin();
				it != m_inputQueue.end(); ++it)
			{
				this->release(*it);
			}

			for (BufferQueue::iterator it = m_outputQueue.begin();
				it != m_outputQueue.end(); ++it)
			{
				this->release(*it);
			}
		}

		void Connection::setReadHandler(const ReadHandler& rh)
		{
			m_readHandler = rh;
		}

		void Connection::setWriteHandler(const WriteHandler& wh)
		{
			m_writeHandler = wh;
		}

		void Connection::setCloseHandler(const CloseHandler& ch)
		{
			m_closeHandler = ch;
		}

		std::size_t Connection::inputLength() const
		{
			return m_inputLength;
		}

		std::size_t Connection::outputLength() const
		{
			return m_outputLength;
		}

		void Connection::copy(char* dst, std::size_t len)
		{
			BOOST_ASSERT(dst != nullptr);
			BOOST_ASSERT(len <= inputLength());

			std::size_t pos = 0;
			while (len)
			{
				_Buffer* b = m_inputQueue[pos];
				std::size_t copyLen = std::min(len, b->length());
				memcpy(dst, b->reader(), copyLen);
				len -= copyLen;
				dst += copyLen;
				if (copyLen == b->length())
				{
					++pos;
				}
			}
		}

		void Connection::read(char* dst, std::size_t len)
		{
			BOOST_ASSERT(dst != nullptr);
			BOOST_ASSERT(len <= inputLength());

			while (len)
			{
				BOOST_ASSERT(!m_inputQueue.empty());
				_Buffer* b = m_inputQueue.front();
				m_inputQueue.pop_front();
				m_inputLength -= b->length();
				std::size_t copyLen = std::min(len, b->length());
				memcpy(dst, b->reader(), copyLen);
				len -= copyLen;
				dst += copyLen;
				b->readBytes(copyLen);
				if (b->length() == 0)
				{
					release(b);
				}
				else
				{
					BOOST_ASSERT(len == 0);
					m_inputQueue.push_front(b);
					m_inputLength += b->length();
				}
			}
		}

		void Connection::read(std::size_t len)
		{
			BOOST_ASSERT(len <= inputLength());

			while (len)
			{
				BOOST_ASSERT(!m_inputQueue.empty());
				_Buffer* b = m_inputQueue.front();
				m_inputQueue.pop_front();
				m_inputLength -= b->length();
				std::size_t readLen = std::min(len, b->length());
				len -= readLen;
				b->readBytes(readLen);
				if (b->length() == 0)
				{
					release(b);
				}
				else
				{
					BOOST_ASSERT(len == 0);
					m_inputQueue.push_front(b);
					m_inputLength += b->length();
				}
			}
		}

		void Connection::send(const char* src, std::size_t len)
		{
			BOOST_ASSERT(src != nullptr);

			while (len)
			{
				_Buffer* b = nullptr;
				if (!m_outputQueue.empty() && m_outputQueue.back()->space() > 0)
				{
					b = m_outputQueue.back();
					m_outputLength -= b->length();
				}
				else
				{
					b = alloc();
				}

				std::size_t copyLen = std::min(len, b->space());
				memcpy(b->writer(), src, copyLen);
				len -= copyLen;
				src += copyLen;
				b->writeBytes(copyLen);
				m_outputQueue.push_back(b);
				m_outputLength += b->length();
			}

			sendToNet();
		}

		void Connection::start()
		{
			readFromNet();
		}

		void Connection::close()
		{
			if (m_socket.is_open())
			{
				m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
				m_socket.close();
			}
		}

		void Connection::readFromNet()
		{
			if (m_bRecving) return;
			m_bRecving = true;

			_Buffer* b = nullptr;
			if (!m_inputQueue.empty() && m_inputQueue.back()->space() > 0)
			{
				b = m_inputQueue.back();
				m_inputQueue.pop_back();
				m_inputLength -= b->length();
			}
			else
			{
				b = alloc();
			}

			m_socket.async_read_some(
				boost::asio::buffer(b->writer(), b->space()),
				boost::bind(&Connection::onRead, shared_from_this(),
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, b));
		}

		void Connection::sendToNet()
		{
			if (m_bSending) return;
			if (outputLength() == 0) return;
			BOOST_ASSERT(!m_outputQueue.empty());
			m_bSending = true;

			_Buffer* b = m_outputQueue.front();
			m_outputQueue.pop_front();
			m_outputLength -= b->length();

			m_socket.async_write_some(
				boost::asio::buffer(b->reader(), b->length()),
				boost::bind(&Connection::onSend, shared_from_this(),
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, b));
		}

		void Connection::onRead(const boost::system::error_code& ec, std::size_t bytes, _Buffer* b)
		{
			BOOST_ASSERT(m_bRecving);
			m_bRecving = false;

			if (ec)
			{
				release(b);
				close();
				m_closeHandler();
			}
			else
			{
				b->writeBytes(bytes);
				m_inputQueue.push_back(b);
				m_inputLength += b->length();
				m_readHandler();
				readFromNet();
			}
		}

		void Connection::onSend(const boost::system::error_code& ec, std::size_t bytes, _Buffer* b)
		{
			BOOST_ASSERT(m_bSending);
			m_bSending = false;

			if (ec)
			{
				release(b);
				close();
				m_closeHandler();
			}
			else
			{
				b->readBytes(bytes);
				if (b->length() == 0)
				{
					release(b);
				}
				else
				{
					m_outputQueue.push_front(b);
					m_outputLength += b->length();
				}

				m_writeHandler();
				sendToNet();
			}
		}

		boost::asio::ip::tcp::socket& Connection::socket()
		{
			return m_socket;
		}
	}
}

