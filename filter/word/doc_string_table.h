#pragma once
#include <string>
#include <bitset>
#include <memory>
#include "traits/binary_traits.h"
#include "traits/compound_file_binary_traits.h"
#include "io/binary_iostream.h"

namespace filter
{
namespace doc
{
	typedef binary_traits::byte_t byte_t;
	typedef binary_traits::buffer_t buffer_t;
	typedef binary_traits::bufferstream bufferstream;
	typedef binary_traits::streamsize streamsize;

	// serializers
	//bufferstream& operator >> (bufferstream& stream, FontFamilyName& data);
	//bufferstream& operator << (bufferstream& stream, const FontFamilyName& data);
}
}