#pragma once
#include <iostream>
#include "define/binary_traits.h"

#define DECLARE_BINARY_SERIALIZER(T) \
friend bufferstream& operator >> (bufferstream& , T& ); \
friend bufferstream& operator << (bufferstream&, const T& );

namespace filter
{
	class binary_io
	{
	public:
		typedef binary_traits::buffer_t buffer_t;
		binary_io() = default;

		// interfaces of input
		static int8_t read_int8(std::iostream& stream);
		static int16_t read_int16(std::iostream& stream);
		static int32_t read_int32(std::iostream& stream);
		static int64_t read_int64(std::iostream& stream);
		static uint8_t read_uint8(std::iostream& stream);
		static uint16_t read_uint16(std::iostream& stream);
		static uint32_t read_uint32(std::iostream& stream);
		static uint64_t read_uint64(std::iostream& stream);
		static float read_float(std::iostream& stream);
		static double read_double(std::iostream& stream);
		static buffer_t read(std::iostream& stream, std::streamsize size);
		static void consume(std::iostream& stream, std::streamsize size);
		static std::string read_string(std::iostream& stream, std::streamsize size);
		static std::u16string read_u16string(std::iostream& stream, std::streamsize size);
		static std::vector<uint8_t> read_u8vector(std::iostream& stream, std::streamsize size);

		// interfaces of output
		static void write_int8(std::iostream& stream, int8_t value);
		static void write_int16(std::iostream& stream, int16_t value);
		static void write_int32(std::iostream& stream, int32_t value);
		static void write_int64(std::iostream& stream, int64_t value);
		static void write_uint8(std::iostream& stream, uint8_t value);
		static void write_uint16(std::iostream& stream, uint16_t value);
		static void write_uint32(std::iostream& stream, uint32_t value);
		static void write_uint64(std::iostream& stream, uint64_t value);
		static void write_float(std::iostream& stream, float value);
		static void write_double(std::iostream& stream, double value);
		static void write(std::iostream& stream, const buffer_t& value);
		static void write_string(std::iostream& stream, const std::string& value);
		static void write_u16string(std::iostream& stream, const std::u16string& value);
		static void write_u8vector(std::iostream& stream, const std::vector<uint8_t>& value);
	private:
	};
}