#include "filter_pch.h"
#include "hwp/hwp50_syntax.h"

namespace filter
{
namespace hwp50
{
	// serializers
	bufferstream& operator >> (bufferstream& stream, file_header_t& file_header)
	{
		file_header.signature = binary_stream_t::read_string(stream, file_header.signature_size);
		file_header.version = binary_stream_t::read_uint32(stream);
		file_header.options = binary_stream_t::read_uint32(stream);
		file_header.extended_options = binary_stream_t::read_uint32(stream);
		file_header.kogl = binary_stream_t::read_uint8(stream);
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const file_header_t& file_header)
	{
		binary_stream_t::write_string(stream, file_header.signature);
		binary_stream_t::write_uint32(stream, file_header.version);
		binary_stream_t::write_uint32(stream, file_header.options.to_ulong());
		binary_stream_t::write_uint32(stream, file_header.extended_options.to_ulong());
		binary_stream_t::write_uint8(stream, file_header.kogl);
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, header_t& header)
	{
		uint32_t plain = binary_stream_t::read_uint32(stream);
		header.tag = plain & 0x3FF;
		header.level = (plain >> 10) & 0x3FF;
		header.body_size = (plain >> 20) & 0xFFF;

		if (header.body_size == 0xFFF)
		{
			header.body_size = binary_stream_t::read_uint32(stream);
		}
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const header_t& header)
	{
		uint32_t plain = header.tag;
		plain += (header.level << 10);
		if (header.body_size >= 0xFFF)
		{
			plain += (0xFFF << 20);
			binary_stream_t::write_uint32(stream, plain);
			binary_stream_t::write_uint32(stream, header.body_size);
		}
		else
		{
			plain += (header.body_size << 20);
			binary_stream_t::write_uint32(stream, plain);
		}
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, record_t& record)
	{
		stream >> record.header;
		record.body = binary_stream_t::read(stream, record.header.body_size);
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const record_t& record)
	{
		stream << record.header;
		binary_stream_t::write(stream, record.body);
		return stream;
	}
}
}