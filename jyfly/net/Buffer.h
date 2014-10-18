#pragma once

#include <boost/assert.hpp>

namespace jyfly
{
namespace net
{
	template<std::size_t CAPACITY>
	class Buffer
	{
	public:
		Buffer() : m_rdPos(0), m_wrPos(0) { }
		~Buffer() { }

		char* reader() { return m_buf + m_rdPos; }
		char* writer() { return m_buf + m_wrPos; }

		void readBytes(std::size_t len)
		{
			m_rdPos += len;
			BOOST_ASSERT(m_rdPos <= m_wrPos);
		}

		void writeBytes(std::size_t len)
		{
			m_wrPos += len;
			BOOST_ASSERT(m_rdPos <= m_wrPos);
		}

		inline std::size_t length() const 
		{ 
			return m_wrPos - m_rdPos; 
		}

		inline std::size_t space() const 
		{ 
			return CAPACITY - m_wrPos; 
		}

	private:
		std::size_t m_rdPos;
		std::size_t m_wrPos;
		char m_buf[CAPACITY];
	};
} //namespace net
} //namespace jyfly