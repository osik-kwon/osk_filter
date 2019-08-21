#include "io/binary_iostream.h"
#include "io/endianness.h"

namespace filter
{
	int8_t binary_stream_t::read_int8(std::iostream& stream)
	{
		int8_t buf = 0;
		stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
		if (!stream.good())
			throw std::runtime_error("read stream fail");
		FILTER_ENDIAN_SWAP(&buf);
		return buf;
	}

	int16_t binary_stream_t::read_int16(std::iostream& stream)
	{
		int16_t buf = 0;
		stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
		if (!stream.good())
			throw std::runtime_error("read stream fail");
		FILTER_ENDIAN_SWAP(&buf);
		return buf;
	}

	int32_t binary_stream_t::read_int32(std::iostream& stream)
	{
		int32_t buf = 0;
		stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
		if (!stream.good())
			throw std::runtime_error("read stream fail");
		FILTER_ENDIAN_SWAP(&buf);
		return buf;
	}

	int64_t binary_stream_t::read_int64(std::iostream& stream)
	{
		int64_t buf = 0;
		stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
		if (!stream.good())
			throw std::runtime_error("read stream fail");
		FILTER_ENDIAN_SWAP(&buf);
		return buf;
	}

	uint8_t binary_stream_t::read_uint8(std::iostream& stream)
	{
		uint8_t buf = 0;
		stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
		if (!stream.good())
			throw std::runtime_error("read stream fail");
		FILTER_ENDIAN_SWAP(&buf);
		return buf;
	}

	uint16_t binary_stream_t::read_uint16(std::iostream& stream)
	{
		uint16_t buf = 0;
		stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
		if (!stream.good())
			throw std::runtime_error("read stream fail");
		FILTER_ENDIAN_SWAP(&buf);
		return buf;
	}

	uint32_t binary_stream_t::read_uint32(std::iostream& stream)
	{
		uint32_t buf = 0;
		stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
		if (!stream.good())
			throw std::runtime_error("read stream fail");
		FILTER_ENDIAN_SWAP(&buf);
		return buf;
	}

	uint64_t binary_stream_t::read_uint64(std::iostream& stream)
	{
		uint64_t buf = 0;
		stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
		if (!stream.good())
			throw std::runtime_error("read stream fail");
		FILTER_ENDIAN_SWAP(&buf);
		return buf;
	}

	float binary_stream_t::read_float(std::iostream& stream)
	{
		static_assert(sizeof(float) == sizeof(uint32_t), "todo implement");
		uint32_t buf = read_uint32(stream);
		return *reinterpret_cast<float*>(&buf);
	}

	double binary_stream_t::read_double(std::iostream& stream)
	{
		static_assert(sizeof(double) == sizeof(uint64_t), "todo implement");
		uint64_t buf = read_uint64(stream);
		return *reinterpret_cast<double*>(&buf);
	}

	binary_stream_t::buffer_t binary_stream_t::read(std::iostream& stream, std::streamsize size)
	{
		if (size < 1)
			throw std::runtime_error("read stream fail");
		buffer_t buf;
		buf.resize(static_cast<std::size_t>(size));
		stream.read(reinterpret_cast<char*>(&buf[0]), buf.size());
		if (!stream.good())
			throw std::runtime_error("read stream fail");
		FILTER_ENDIAN_SWAP(&buf[0]);
		return buf;
	}

	void binary_stream_t::consume(std::iostream& stream, std::streamsize size)
	{
		if (size < 1)
			throw std::runtime_error("read stream fail");
		buffer_t buf;
		buf.resize(static_cast<std::size_t>(size));
		stream.read(reinterpret_cast<char*>(&buf[0]), buf.size());
		if (!stream.good())
			throw std::runtime_error("read stream fail");
		FILTER_ENDIAN_SWAP(&buf[0]);
	}

	std::string binary_stream_t::read_string(std::iostream& stream, std::streamsize size)
	{
		if (size < 1)
			throw std::runtime_error("read stream fail");
		std::string buf;
		buf.resize(static_cast<std::size_t>(size));
		stream.read(reinterpret_cast<char*>(&buf[0]), buf.size());
		if (!stream.good())
			throw std::runtime_error("read stream fail");
		FILTER_ENDIAN_SWAP(&buf[0]);
		return buf;
	}

	void binary_stream_t::write_int8(std::iostream& stream, int8_t value)
	{
		FILTER_ENDIAN_SWAP(&value);
		stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
		if (!stream.good())
			throw std::runtime_error("write stream fail");
	}

	void binary_stream_t::write_int16(std::iostream& stream, int16_t value)
	{
		FILTER_ENDIAN_SWAP(&value);
		stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
		if (!stream.good())
			throw std::runtime_error("write stream fail");
	}

	void binary_stream_t::write_int32(std::iostream& stream, int32_t value)
	{
		FILTER_ENDIAN_SWAP(&value);
		stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
		if (!stream.good())
			throw std::runtime_error("write stream fail");
	}

	void binary_stream_t::write_int64(std::iostream& stream, int64_t value)
	{
		FILTER_ENDIAN_SWAP(&value);
		stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
		if (!stream.good())
			throw std::runtime_error("write stream fail");
	}

	void binary_stream_t::write_uint8(std::iostream& stream, uint8_t value)
	{
		FILTER_ENDIAN_SWAP(&value);
		stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
		if (!stream.good())
			throw std::runtime_error("write stream fail");
	}

	void binary_stream_t::write_uint16(std::iostream& stream, uint16_t value)
	{
		FILTER_ENDIAN_SWAP(&value);
		stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
		if (!stream.good())
			throw std::runtime_error("write stream fail");
	}

	void binary_stream_t::write_uint32(std::iostream& stream, uint32_t value)
	{
		FILTER_ENDIAN_SWAP(&value);
		stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
		if (!stream.good())
			throw std::runtime_error("write stream fail");
	}

	void binary_stream_t::write_uint64(std::iostream& stream, uint64_t value)
	{
		FILTER_ENDIAN_SWAP(&value);
		stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
		if (!stream.good())
			throw std::runtime_error("write stream fail");
	}

	void binary_stream_t::write_float(std::iostream& stream, float value)
	{
		write_uint32(stream, *reinterpret_cast<uint32_t*>(&value));
	}

	void binary_stream_t::write_double(std::iostream& stream, double value)
	{
		write_uint64(stream, *reinterpret_cast<uint64_t*>(&value));
	}

	void binary_stream_t::write(std::iostream& stream, const buffer_t& value)
	{
		if (value.size() < 1)
			throw std::runtime_error("write stream fail");

		FILTER_ENDIAN_SWAP(const_cast<char*>(&value[0]));
		stream.write(reinterpret_cast<const char*>(&value[0]), value.size());
		if (!stream.good())
			throw std::runtime_error("write stream fail");
	}

	void binary_stream_t::write_string(std::iostream& stream, const std::string& value)
	{
		if (value.size() < 1)
			throw std::runtime_error("write stream fail");

		FILTER_ENDIAN_SWAP(const_cast<char*>(&value[0]));
		stream.write(reinterpret_cast<const char*>(&value[0]), value.size());
		if (!stream.good())
			throw std::runtime_error("write stream fail");
	}
}