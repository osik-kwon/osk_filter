#include "filter_pch.h"
#include "io/binary_iostream.h"
#include "io/endianness.h"

namespace filter
{
	int8_t binary_io::read_int8(std::iostream& stream)
	{
		int8_t buf = 0;
		stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
		if (!stream.good())
			throw std::runtime_error("read_int8 fail");
		FILTER_ENDIAN_SWAP(&buf);
		return buf;
	}

	int16_t binary_io::read_int16(std::iostream& stream)
	{
		int16_t buf = 0;
		stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
		if (!stream.good())
			throw std::runtime_error("read_int16 fail");
		FILTER_ENDIAN_SWAP(&buf);
		return buf;
	}

	int32_t binary_io::read_int32(std::iostream& stream)
	{
		int32_t buf = 0;
		stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
		if (!stream.good())
			throw std::runtime_error("read_int32 fail");
		FILTER_ENDIAN_SWAP(&buf);
		return buf;
	}

	int64_t binary_io::read_int64(std::iostream& stream)
	{
		int64_t buf = 0;
		stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
		if (!stream.good())
			throw std::runtime_error("read_int64 fail");
		FILTER_ENDIAN_SWAP(&buf);
		return buf;
	}

	uint8_t binary_io::read_uint8(std::iostream& stream)
	{
		uint8_t buf = 0;
		stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
		if (!stream.good())
			throw std::runtime_error("read_uint8 fail");
		FILTER_ENDIAN_SWAP(&buf);
		return buf;
	}

	uint16_t binary_io::read_uint16(std::iostream& stream)
	{
		uint16_t buf = 0;
		stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
		if (!stream.good())
			throw std::runtime_error("read_uint16 fail");
		FILTER_ENDIAN_SWAP(&buf);
		return buf;
	}

	uint32_t binary_io::read_uint32(std::iostream& stream)
	{
		uint32_t buf = 0;
		stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
		if (!stream.good())
			throw std::runtime_error("read_uint32 fail");
		FILTER_ENDIAN_SWAP(&buf);
		return buf;
	}

	uint64_t binary_io::read_uint64(std::iostream& stream)
	{
		uint64_t buf = 0;
		stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
		if (!stream.good())
			throw std::runtime_error("read_uint64 fail");
		FILTER_ENDIAN_SWAP(&buf);
		return buf;
	}

	float binary_io::read_float(std::iostream& stream)
	{
		static_assert(sizeof(float) == sizeof(uint32_t), "todo implement");
		uint32_t buf = read_uint32(stream);
		return *reinterpret_cast<float*>(&buf);
	}

	double binary_io::read_double(std::iostream& stream)
	{
		static_assert(sizeof(double) == sizeof(uint64_t), "todo implement");
		uint64_t buf = read_uint64(stream);
		return *reinterpret_cast<double*>(&buf);
	}

	binary_io::buffer_t binary_io::read(std::iostream& stream, std::streamsize size)
	{
		if (size < 1)
			throw std::runtime_error("read stream fail");
		buffer_t buf;
		buf.resize(static_cast<size_t>(size));
		stream.read(reinterpret_cast<char*>(&buf[0]), buf.size());
		if (!stream.good())
			throw std::runtime_error("read stream fail");
		FILTER_ENDIAN_SWAP(&buf[0]);
		return buf;
	}

	void binary_io::consume(std::iostream& stream, std::streamsize size)
	{
		if (size < 1)
			throw std::runtime_error("consume fail");
		buffer_t buf;
		buf.resize(static_cast<size_t>(size));
		stream.read(reinterpret_cast<char*>(&buf[0]), buf.size());
		if (!stream.good())
			throw std::runtime_error("consume fail");
		FILTER_ENDIAN_SWAP(&buf[0]);
	}

	std::string binary_io::read_string(std::iostream& stream, std::streamsize size)
	{
		if (size < 1)
			throw std::runtime_error("read_string fail");
		std::string buf;
		buf.resize(static_cast<size_t>(size));
		stream.read(reinterpret_cast<char*>(&buf[0]), buf.size());
		if (!stream.good())
			throw std::runtime_error("read_string fail");
		FILTER_ENDIAN_SWAP(&buf[0]);
		return buf;
	}

	void binary_io::write_int8(std::iostream& stream, int8_t value)
	{
		FILTER_ENDIAN_SWAP(&value);
		stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
		if (!stream.good())
			throw std::runtime_error("write_int8 fail");
	}

	void binary_io::write_int16(std::iostream& stream, int16_t value)
	{
		FILTER_ENDIAN_SWAP(&value);
		stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
		if (!stream.good())
			throw std::runtime_error("write_int16 fail");
	}

	void binary_io::write_int32(std::iostream& stream, int32_t value)
	{
		FILTER_ENDIAN_SWAP(&value);
		stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
		if (!stream.good())
			throw std::runtime_error("write_int32 fail");
	}

	void binary_io::write_int64(std::iostream& stream, int64_t value)
	{
		FILTER_ENDIAN_SWAP(&value);
		stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
		if (!stream.good())
			throw std::runtime_error("write_int64 fail");
	}

	void binary_io::write_uint8(std::iostream& stream, uint8_t value)
	{
		FILTER_ENDIAN_SWAP(&value);
		stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
		if (!stream.good())
			throw std::runtime_error("write_uint8 fail");
	}

	void binary_io::write_uint16(std::iostream& stream, uint16_t value)
	{
		FILTER_ENDIAN_SWAP(&value);
		stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
		if (!stream.good())
			throw std::runtime_error("write_uint16 fail");
	}

	void binary_io::write_uint32(std::iostream& stream, uint32_t value)
	{
		FILTER_ENDIAN_SWAP(&value);
		stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
		if (!stream.good())
			throw std::runtime_error("write_uint32 fail");
	}

	void binary_io::write_uint64(std::iostream& stream, uint64_t value)
	{
		FILTER_ENDIAN_SWAP(&value);
		stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
		if (!stream.good())
			throw std::runtime_error("write_uint64 fail");
	}

	void binary_io::write_float(std::iostream& stream, float value)
	{
		write_uint32(stream, *reinterpret_cast<uint32_t*>(&value));
	}

	void binary_io::write_double(std::iostream& stream, double value)
	{
		write_uint64(stream, *reinterpret_cast<uint64_t*>(&value));
	}

	void binary_io::write(std::iostream& stream, const buffer_t& value)
	{
		if (value.size() < 1)
			throw std::runtime_error("write stream fail");

		FILTER_ENDIAN_SWAP(const_cast<char*>(&value[0]));
		stream.write(reinterpret_cast<const char*>(&value[0]), value.size());
		if (!stream.good())
		{
			size_t cur = (size_t)stream.tellg();
			throw std::runtime_error("write stream fail");
		}
	}

	void binary_io::write_string(std::iostream& stream, const std::string& value)
	{
		if (value.size() < 1)
			throw std::runtime_error("write_string fail");

		FILTER_ENDIAN_SWAP(const_cast<char*>(&value[0]));
		stream.write(reinterpret_cast<const char*>(&value[0]), value.size());
		if (!stream.good())
			throw std::runtime_error("write_string fail");
	}

	std::u16string binary_io::read_u16string(std::iostream& stream, std::streamsize size)
	{
		if (size < 1)
			throw std::runtime_error("read_u16string fail");
		std::u16string buf;
		buf.reserve((size_t)size);
		for (std::streamsize i = 0; i < size; ++i)
			buf.push_back( read_uint16(stream) );

		if (!stream.good())
			throw std::runtime_error("read_u16string fail");
		return buf;
	}

	void binary_io::write_u16string(std::iostream& stream, const std::u16string& value)
	{
		if (value.size() < 1)
			throw std::runtime_error("write_u16string fail");

		for (auto code : value)
			write_uint16(stream, code);
		if (!stream.good())
			throw std::runtime_error("write_u16string fail");
	}
}