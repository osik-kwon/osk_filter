#include "filter_pch.h"
#include "io/zlib.h"
#include <boost/interprocess/streams/bufferstream.hpp>

#include <detail/serialization/zstream.hpp>
#include <detail/serialization/miniz.hpp>
#include <array>
#include <algorithm>
#include <memory>

namespace filter
{
namespace zip
{
    static const std::size_t buffer_size = 4096;

    class zip_streambuf_decompress : public std::streambuf
    {
        std::istream& istream;

        z_stream strm;
        std::array<char, buffer_size> in;
        std::array<char, buffer_size> out;
        std::size_t total_compressed;
        std::size_t total_read;
        std::size_t total_uncompressed;
        bool valid;
        bool compressed_data;

        static const unsigned short DEFLATE = 8;
        static const unsigned short UNCOMPRESSED = 0;

    public:
        zip_streambuf_decompress(std::istream& stream, std::size_t total_compressed)
            : istream(stream), total_read(0), total_compressed(total_compressed), total_uncompressed(0), valid(true)
        {
            in.fill(0);
            out.fill(0);

            strm.zalloc = nullptr;
            strm.zfree = nullptr;
            strm.opaque = nullptr;
            strm.avail_in = 0;
            strm.next_in = nullptr;

            setg(in.data(), in.data(), in.data());
            setp(nullptr, nullptr);

            compressed_data = true;

            // initialize the inflate
            if (compressed_data && valid)
            {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
                int result = inflateInit2(&strm, -MAX_WBITS);
#pragma clang diagnostic pop

                if (result != Z_OK)
                {
                    throw std::runtime_error("couldn't inflate ZIP, possibly corrupted");
                }
            }
        }

        virtual ~zip_streambuf_decompress()
        {
            if (compressed_data && valid)
            {
                inflateEnd(&strm);
            }
        }

        int process()
        {
            if (!valid)
                return -1;
            strm.avail_out = buffer_size - 4;
            strm.next_out = reinterpret_cast<Bytef*>(out.data() + 4);

            while (strm.avail_out != 0)
            {
                if (strm.avail_in == 0)
                {
                    // buffer empty, read some more from file
                    istream.read(in.data(),
                        static_cast<std::streamsize>(std::min(buffer_size, total_compressed - total_read)));
                    strm.avail_in = static_cast<unsigned int>(istream.gcount());
                    total_read += strm.avail_in;
                    strm.next_in = reinterpret_cast<Bytef*>(in.data());
                }

                const auto ret = inflate(&strm, Z_NO_FLUSH); // decompress

                if (ret == Z_STREAM_ERROR || ret == Z_NEED_DICT || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR)
                {
                    throw std::runtime_error("couldn't inflate ZIP, possibly corrupted");
                }

                if (ret == Z_STREAM_END) break;
            }

            auto unzip_count = buffer_size - strm.avail_out - 4;
            total_uncompressed += unzip_count;
            return static_cast<int>(unzip_count);
        }

        virtual int underflow()
        {
            if (gptr() && (gptr() < egptr()))
                return traits_type::to_int_type(*gptr()); // if we already have data just use it
            auto put_back_count = gptr() - eback();
            if (put_back_count > 4) put_back_count = 4;
            std::memmove(
                out.data() + (4 - put_back_count), gptr() - put_back_count, static_cast<std::size_t>(put_back_count));
            int num = process();
            setg(out.data() + 4 - put_back_count, out.data() + 4, out.data() + 4 + num);
            if (num <= 0) return EOF;
            return traits_type::to_int_type(*gptr());
        }

        virtual int overflow(int c = EOF);
    };

    int zip_streambuf_decompress::overflow(int)
    {
        throw std::runtime_error("writing to read-only buffer");
    }

    std::vector<char> to_vector(std::istream& in_stream)
    {
        if (!in_stream)
        {
            throw std::runtime_error("bad stream");
        }

        return std::vector<char>(
            std::istreambuf_iterator<char>(in_stream),
            std::istreambuf_iterator<char>());
    }

    std::vector<char> decompress_impl(const char* data, size_t size)
    {
        boost::interprocess::basic_bufferbuf<char> src(const_cast<char*>(data), size);
        std::iostream src_stream(&src);
        zip_streambuf_decompress buffer(src_stream, size);
 
        std::istream stream(&buffer);
        return to_vector(stream);
    }

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
