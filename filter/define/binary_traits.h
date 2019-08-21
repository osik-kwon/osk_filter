#pragma once
#include <cstdint>
#include <vector>
#include <boost/interprocess/streams/bufferstream.hpp>

namespace filter
{
	struct binary_traits
	{
		typedef uint8_t byte_t;
		typedef boost::interprocess::bufferstream   bufferstream;
		typedef std::streamsize streamsize;
		typedef std::vector<char> buffer_t;
	};
}