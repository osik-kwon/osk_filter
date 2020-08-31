#pragma once
#include <sstream>

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>

#include "traits/binary_traits.h"

namespace filter
{
namespace zip
{
	std::vector<char> decompress_impl(const char* data, size_t size);

	struct RFC1521
	{
		static boost::iostreams::zlib_params make_param();
	};

	template <class algorithm_t>
	class zip_impl
	{
	public:
		zip_impl() = default;
		typedef binary_traits::buffer_t buffer_t;

		static buffer_t compress(const buffer_t& data) {
			return compress(data.data(), data.size());
		}

		static buffer_t decompress(const buffer_t& data) {
			return decompress(data.data(), data.size());
		}

		static buffer_t compress_noexcept(const buffer_t& data) {
			try {
				return compress(data.data(), data.size());
			}
			catch (const std::exception&)
			{
			}
			return buffer_t();
		}

		static buffer_t decompress_noexcept(const buffer_t& data) {
			try {
				return decompress(data.data(), data.size());
			}
			catch (const std::exception&)
			{
			}
			return buffer_t();
		}

		static buffer_t compress_noexcept(const char* data, size_t size) {
			try {
				return compress(data, size);
			}
			catch (const std::exception&)
			{
			}
			return buffer_t();
		}

		static buffer_t decompress_noexcept(const char* data, size_t size) {
			try {
				return decompress(data, size);
			}
			catch (const std::exception&)
			{
			}
			return buffer_t();
		}

		static buffer_t compress(const char* data, size_t size)
		{
			boost::iostreams::array_source src(reinterpret_cast<const char*>(data), size);
			boost::iostreams::filtering_istreambuf iin;
			iin.push(boost::iostreams::zlib_compressor(algorithm_t::make_param()));
			iin.push(src);

			buffer_t out;
			out.assign(std::istreambuf_iterator<char>(&iin), {});
			return out;
		}

		static buffer_t decompress(const char* data, size_t size)
		{
			return decompress_impl(data, size);
			/*
			boost::iostreams::array_source src(reinterpret_cast<const char*>(data), size);
			boost::iostreams::filtering_istreambuf iin;
			iin.push(boost::iostreams::zlib_decompressor(algorithm_t::make_param()));
			iin.push(src);

			buffer_t out;
			out.assign(std::istreambuf_iterator<char>(&iin), {});
			return out;
			*/
		}

		static std::string compress_string(const std::string& data)
		{
			std::stringstream compressed;
			std::stringstream original;
			original << data;

			boost::iostreams::filtering_streambuf<boost::iostreams::input> out;
			out.push(boost::iostreams::zlib_compressor(algorithm_t::make_param()));
			out.push(original);
			boost::iostreams::copy(out, compressed);

			return compressed.str();
		}

		static std::string decompress_string(const std::string& data)
		{
			std::stringstream compressed_encoded;
			std::stringstream decompressed;
			compressed_encoded << data;

			boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
			in.push(boost::iostreams::zlib_decompressor(algorithm_t::make_param()));
			in.push(compressed_encoded);
			boost::iostreams::copy(in, decompressed);
			return decompressed.str();
		}
	private:
	};
}

	typedef ::filter::zip::zip_impl<::filter::zip::RFC1521> hwp_zip;
}