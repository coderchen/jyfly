#pragma once

#include <deque>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/pool/object_pool.hpp>
#include "Buffer.h"

namespace jyfly
{
	namespace net
	{
		class Connection
			: public boost::enable_shared_from_this<Connection>
		{
			typedef boost::function<void()> ReadHandler;
			typedef boost::function<void()> WriteHandler;
			typedef boost::function<void()> CloseHandler;

			typedef Buffer<4096> _Buffer;
			typedef boost::object_pool<_Buffer> BufferPool;
			typedef std::deque<_Buffer*> BufferQueue;

		public:
			Connection(boost::asio::io_service& s);
			~Connection();

			void setReadHandler(const ReadHandler& rh);
			void setWriteHandler(const WriteHandler& wh);
			void setCloseHandler(const CloseHandler& ch);

			std::size_t inputLength() const;
			std::size_t outputLength() const;

			void copy(char* dst, std::size_t len);
			void read(char* dst, std::size_t len);
			void read(std::size_t len);
			void send(const char* src, std::size_t len);

			void start();
			void close();

			boost::asio::ip::tcp::socket& socket();

		private:
			void readFromNet();
			void sendToNet();
			void onRead(const boost::system::error_code& ec, std::size_t bytes, _Buffer* b);
			void onSend(const boost::system::error_code& ec, std::size_t bytes, _Buffer* b);

		private:
			_Buffer* alloc();
			void release(_Buffer* b);

		private:
			boost::asio::ip::tcp::socket m_socket;
			ReadHandler m_readHandler;
			WriteHandler m_writeHandler;
			CloseHandler m_closeHandler;
			bool m_bRecving;
			bool m_bSending;
			std::size_t m_inputLength;
			std::size_t m_outputLength;
			BufferQueue m_inputQueue;
			BufferQueue m_outputQueue;
		};

		typedef boost::shared_ptr<Connection> ConnectionPtr;
	} //namespace net
} // namespace jyfly