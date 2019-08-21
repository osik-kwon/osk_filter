#pragma once
#include <iostream>

#include "define/filter_traits.h"
#include "io/endianness.h"

namespace filter
{
	class binary_stream_t
	{
	public:
		typedef binary_traits::buffer_t buffer_t;
		binary_stream_t() {}

		static int8_t read_int8(std::iostream& stream)
		{
			int8_t buf = 0;
			stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
			if (!stream.good())
				throw std::runtime_error("read stream fail");
			FILTER_ENDIAN_SWAP(&buf);
			return buf;
		}

		static int16_t read_int16(std::iostream& stream)
		{
			int16_t buf = 0;
			stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
			if (!stream.good())
				throw std::runtime_error("read stream fail");
			FILTER_ENDIAN_SWAP(&buf);
			return buf;
		}

		static int32_t read_int32(std::iostream& stream)
		{
			int32_t buf = 0;
			stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
			if (!stream.good())
				throw std::runtime_error("read stream fail");
			FILTER_ENDIAN_SWAP(&buf);
			return buf;
		}

		static int64_t read_int64(std::iostream& stream)
		{
			int64_t buf = 0;
			stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
			if (!stream.good())
				throw std::runtime_error("read stream fail");
			FILTER_ENDIAN_SWAP(&buf);
			return buf;
		}

		static uint8_t read_uint8(std::iostream& stream)
		{
			uint8_t buf = 0;
			stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
			if (!stream.good())
				throw std::runtime_error("read stream fail");
			FILTER_ENDIAN_SWAP(&buf);
			return buf;
		}

		static uint16_t read_uint16(std::iostream& stream)
		{
			uint16_t buf = 0;
			stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
			if (!stream.good())
				throw std::runtime_error("read stream fail");
			FILTER_ENDIAN_SWAP(&buf);
			return buf;
		}

		static uint32_t read_uint32(std::iostream& stream)
		{
			uint32_t buf = 0;
			stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
			if (!stream.good())
				throw std::runtime_error("read stream fail");
			FILTER_ENDIAN_SWAP(&buf);
			return buf;
		}

		static uint64_t read_uint64(std::iostream& stream)
		{
			uint64_t buf = 0;
			stream.read(reinterpret_cast<char*>(&buf), sizeof(buf));
			if (!stream.good())
				throw std::runtime_error("read stream fail");
			FILTER_ENDIAN_SWAP(&buf);
			return buf;
		}

		static float read_float(std::iostream& stream)
		{
			static_assert(sizeof(float) == sizeof(uint32_t), "todo implement");
			uint32_t buf = read_uint32(stream);
			return *reinterpret_cast<float*>(&buf);
		}

		static double read_double(std::iostream& stream)
		{
			static_assert(sizeof(double) == sizeof(uint64_t), "todo implement");
			uint64_t buf = read_uint64(stream);
			return *reinterpret_cast<double*>(&buf);
		}

		static buffer_t read(std::iostream& stream, std::streamsize size)
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

		static void consume(std::iostream& stream, std::streamsize size)
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

		static std::string read_string(std::iostream& stream, std::streamsize size)
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

		static void write_int8(std::iostream& stream, int8_t value)
		{
			FILTER_ENDIAN_SWAP(&value);
			stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
			if (!stream.good())
				throw std::runtime_error("write stream fail");
		}

		static void write_int16(std::iostream& stream, int16_t value)
		{
			FILTER_ENDIAN_SWAP(&value);
			stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
			if (!stream.good())
				throw std::runtime_error("write stream fail");
		}

		static void write_int32(std::iostream& stream, int32_t value)
		{
			FILTER_ENDIAN_SWAP(&value);
			stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
			if (!stream.good())
				throw std::runtime_error("write stream fail");
		}

		static void write_int64(std::iostream& stream, int64_t value)
		{
			FILTER_ENDIAN_SWAP(&value);
			stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
			if (!stream.good())
				throw std::runtime_error("write stream fail");
		}

		static void write_uint8(std::iostream& stream, uint8_t value)
		{
			FILTER_ENDIAN_SWAP(&value);
			stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
			if (!stream.good())
				throw std::runtime_error("write stream fail");
		}

		static void write_uint16(std::iostream& stream, uint16_t value)
		{
			FILTER_ENDIAN_SWAP(&value);
			stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
			if (!stream.good())
				throw std::runtime_error("write stream fail");
		}

		static void write_uint32(std::iostream& stream, uint32_t value)
		{
			FILTER_ENDIAN_SWAP(&value);
			stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
			if (!stream.good())
				throw std::runtime_error("write stream fail");
		}

		static void write_uint64(std::iostream& stream, uint64_t value)
		{
			FILTER_ENDIAN_SWAP(&value);
			stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
			if (!stream.good())
				throw std::runtime_error("write stream fail");
		}

		static void write_float(std::iostream& stream, float value)
		{
			write_uint32(stream, *reinterpret_cast<uint32_t*>(&value));
		}

		static void write_double(std::iostream& stream, double value)
		{
			write_uint64(stream, *reinterpret_cast<uint64_t*>(&value));
		}

		static void write(std::iostream& stream, const buffer_t& value)
		{
			if (value.size() < 1)
				throw std::runtime_error("write stream fail");

			FILTER_ENDIAN_SWAP(const_cast<char*>(&value[0]));
			stream.write(reinterpret_cast<const char*>(&value[0]), value.size());
			if (!stream.good())
				throw std::runtime_error("write stream fail");
		}

		static void write_string(std::iostream& stream, const std::string& value)
		{
			if (value.size() < 1)
				throw std::runtime_error("write stream fail");

			FILTER_ENDIAN_SWAP(const_cast<char*>(&value[0]));
			stream.write(reinterpret_cast<const char*>(&value[0]), value.size());
			if (!stream.good())
				throw std::runtime_error("write stream fail");
		}
	private:
	};
}