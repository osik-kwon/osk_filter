#ifndef filter_zlib_h
#define filter_zlib_h
#endif
#include <cstdint>
#include <stdexcept>
#include <vector>

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

	template <typename T>
	void endian_swap_and_push_back(T* bytes, std::vector<char>& buffer)
	{
		static_assert(std::is_fundamental<T>::value, "should use primitive type");
		if (!bytes)
			throw std::runtime_error("endian_swap_and_make_buffer fail");
		uint8_t* raw = reinterpret_cast<unsigned char*>(bytes);
		const size_t size = sizeof(T);
		for (int i = 0; i < size; ++i)
		{
			buffer.push_back(raw[i]);
		}
	}

#if defined(BOOST_BIG_ENDIAN)
#define FILTER_ENDIAN_SWAP(bytes) endian_swap(bytes)
#elif defined(BOOST_LITTLE_ENDIAN)
#define FILTER_ENDIAN_SWAP(bytes)
#else
# error "boost Don't know what endianness this system is"
#endif 
}