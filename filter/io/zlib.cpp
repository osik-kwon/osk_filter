#include "io/zlib.h"

namespace filter
{
namespace zip
{
	boost::iostreams::zlib_params RFC1521::make_param()
	{
		// NOTE! https://www.ietf.org/rfc/rfc1521.txt
		boost::iostreams::zlib_params params;
		params.window_bits = 15;
		params.noheader = true;
		params.calculate_crc = false;
		return params;
	}
}
}
