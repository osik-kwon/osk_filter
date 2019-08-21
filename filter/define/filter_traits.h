#pragma once
#include <cstdint>
#include <vector>
#include <string>

#include <boost/interprocess/streams/bufferstream.hpp>

#include "pole/pole.h"

namespace filter
{
	struct binary_traits
	{
		typedef uint8_t byte_t;
		typedef boost::interprocess::bufferstream   bufferstream;
		typedef std::streamsize streamsize;
		typedef std::vector<char> buffer_t;
	};

	struct cfb_traits
	{
		typedef POLE::Storage storage_t;
		typedef POLE::Stream stream_t;
	};

	struct syntax_traits
	{
		typedef std::wstring ustring;
		typedef ustring text_t;
		typedef std::vector<ustring> texts_t;
	};
}