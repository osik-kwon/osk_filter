#include "filter_pch.h"
#include "io/zlib.h"

namespace filter
{
namespace zip
{
	boost::iostreams::zlib_params RFC1521::make_param()
	{
		using namespace boost::iostreams::zlib;
		// NOTE! https://www.ietf.org/rfc/rfc1521.txt
		boost::iostreams::zlib_params params;

		//params.level = default_compression;
		//params.method = deflated;
		params.window_bits = 12;
		//params.mem_level = 9;
		//params.strategy = default_strategy;
		params.noheader = true;
		params.calculate_crc = false;

		return params;
	}
}
}
