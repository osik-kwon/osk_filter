#pragma once
#include <cstdint>
#include <stdexcept>
#include <boost/detail/endian.hpp>

namespace filter
{
	template <class T>
	void endian_swap(T* bytes)
	{
		if (!bytes)
			throw std::runtime_error("endian_swap fail");
		uint8_t* raw = reinterpret_cast<unsigned char*>(bytes);
		std::reverse(raw, raw + sizeof(T));
	}

#if defined(BOOST_BIG_ENDIAN)
#define FILTER_ENDIAN_SWAP(bytes) endian_swap(bytes)
#elif defined(BOOST_LITTLE_ENDIAN)
#define FILTER_ENDIAN_SWAP(bytes)
#else
# error "boost Don't know what endianness this system is"
#endif 
}