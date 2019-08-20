#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <array>
#include <type_traits>
#include <memory>
#include <functional>
#include <bitset>
#include <numeric>
#include <algorithm>
#include <fstream>
#include <regex>

#include <locale>
#include <codecvt>

#include "pole/pole.h"

#include <sstream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>

#include <boost/detail/endian.hpp>
#include <boost/cstdint.hpp>

namespace filter
{
	struct binary_traits
	{
		typedef uint8_t byte_t;
		typedef boost::interprocess::bufferstream   bufferstream;
		typedef std::streamsize streamsize;
		typedef std::vector<char> buffer_t;
	};

	struct cfb_traits
	{
		typedef POLE::Storage storage_t;
		typedef POLE::Stream stream_t;
	};

	struct syntax_traits
	{
		typedef std::wstring ustring;
		typedef ustring text_t;
		typedef std::vector<ustring> texts_t;
	};
}

namespace filter
{
	template <class T>
	void endian_swap(T* bytes)
	{
		if(!bytes)
			throw std::runtime_error("endian_swap fail");
		uint8_t* raw = reinterpret_cast<unsigned char*>(bytes);
		std::reverse(raw, raw + sizeof(T));
	}

	template <typename T>
	void endian_swap_and_push_back(T* bytes, std::vector<char>& buffer)
	{
		static_assert(std::is_fundamental<T>::value, "should use primitive type");
		if (!bytes)
			throw std::runtime_error("endian_swap_and_make_buffer fail");
		uint8_t* raw = reinterpret_cast<unsigned char*>(bytes);
		const size_t size = sizeof(T);
		for (int i = 0; i < size; ++i)
		{
			buffer.push_back(raw[i]);
		}
	}

	/*
	template <typename T>
	static T endian_swap(uint8_t* buffer, std::streamsize offset)
	{
		if (!buffer)
			throw std::runtime_error("endian_swap fail");

		static_assert(std::is_fundamental<T>::value, "should use primitive type");
		const size_t size = sizeof(T);
		T out = 0;
		for (int i = size - 1; i >= 0; --i)
		{
			out |= (buffer[offset + i] << (i * 8));
		}
		return out;
	}
	*/
#if defined(BOOST_BIG_ENDIAN)
#define FILTER_ENDIAN_SWAP(bytes) endian_swap(bytes)
#elif defined(BOOST_LITTLE_ENDIAN)
#define FILTER_ENDIAN_SWAP(bytes)
#else
# error "boost Don't know what endianness this system is"
#endif 
}

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
			if(size < 1)
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

namespace filter
{
	class hwp_zip
	{
	public:
		hwp_zip(){}
		typedef binary_traits::buffer_t buffer_t;

		static buffer_t compress(const buffer_t& data) {
			return compress(data.data(), data.size());
		}

		static buffer_t decompress(const buffer_t& data) {
			return decompress(data.data(), data.size());
		}

		static buffer_t compress_noexcept(const buffer_t& data) {
			try	{
				return compress(data.data(), data.size());
			}
			catch (const std::exception&)
			{}
			return buffer_t();
		}

		static buffer_t decompress_noexcept(const buffer_t& data) {
			try {
				return decompress(data.data(), data.size());
			}
			catch (const std::exception&)
			{}
			return buffer_t();
		}

		static buffer_t compress(const char* data, std::size_t size)
		{
			boost::iostreams::array_source src(reinterpret_cast<const char*>(data), size);
			boost::iostreams::filtering_istreambuf iin;
			iin.push(boost::iostreams::zlib_compressor(make_param()));
			iin.push(src);

			buffer_t out;
			out.assign(std::istreambuf_iterator<char>(&iin), {});
			return out;
		}

		static buffer_t decompress(const char* data, std::size_t size)
		{
			boost::iostreams::array_source src(reinterpret_cast<const char*>(data), size);
			boost::iostreams::filtering_istreambuf iin;
			iin.push(boost::iostreams::zlib_decompressor(make_param()));
			iin.push(src);

			buffer_t out;
			out.assign(std::istreambuf_iterator<char>(&iin), {});
			return out;
		}

		static std::string compress_string(const std::string& data)
		{
			std::stringstream compressed;
			std::stringstream original;
			original << data;

			boost::iostreams::filtering_streambuf<boost::iostreams::input> out;
			out.push(boost::iostreams::zlib_compressor(make_param()));
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
			in.push(boost::iostreams::zlib_decompressor(make_param()));
			in.push(compressed_encoded);
			boost::iostreams::copy(in, decompressed);
			return decompressed.str();
		}
	private:
		static boost::iostreams::zlib_params make_param()
		{
			// NOTE! https://www.ietf.org/rfc/rfc1521.txt
			boost::iostreams::zlib_params params;
			params.window_bits = 15;
			params.noheader = true;
			params.calculate_crc = false;
			return params;
		}
	};

	class cfb_t
	{
	public:
		typedef cfb_traits::storage_t storage_t;
		typedef cfb_traits::stream_t stream_t;
		typedef binary_traits::byte_t byte_t;
		typedef binary_traits::buffer_t buffer_t;
		cfb_t()
		{}

		static buffer_t extract_stream(std::unique_ptr<storage_t>& storage, const std::string& name)
		{
			std::unique_ptr<stream_t> cfb_stream = std::make_unique<stream_t>(storage.get(), name);
			if (cfb_stream->fail())
				throw std::runtime_error("stream create error");
			buffer_t buffer;
			buffer.resize( static_cast<std::size_t>(cfb_stream->size()) );
			if (cfb_stream->read(reinterpret_cast<byte_t*>(&buffer[0]), cfb_stream->size()) != cfb_stream->size())
				throw std::runtime_error("stream read error");
			return buffer;
		}

		static void make_stream(std::unique_ptr<storage_t>& storage, const std::string& name, const buffer_t& buffer)
		{
			auto stream = std::make_unique<stream_t>(storage.get(), name, true, buffer.size());
			if (stream->fail())
				throw std::runtime_error("stream open error");
			stream->seek(0);
			stream->write(reinterpret_cast<byte_t*>(
				const_cast<char*>(&buffer[0])
				), buffer.size());
			stream->flush();
		}

		static void copy_streams(std::unique_ptr<storage_t>& import_storage, std::unique_ptr<storage_t>& export_storage,
			const std::vector<std::string>& all_streams_except_sections)
		{
			for (auto& stream : all_streams_except_sections)
			{
				make_stream(export_storage, stream, extract_stream(import_storage, stream));
			}
		}

		static std::vector<std::string> make_all_streams_except(std::unique_ptr<storage_t>& import_storage,
			const std::vector<std::string>& others)
		{
			auto all_streams_others = make_full_entries(import_storage, "/");
			for (const auto& other : others)
			{
				auto found_other = std::find_if(all_streams_others.begin(),
					all_streams_others.end(), [&other](const std::string& name) {
					return name == other; });
				if (found_other != all_streams_others.end())
					all_streams_others.erase(found_other);
			}
			return all_streams_others;
		}

		static std::unique_ptr<storage_t> make_read_only_storage(const std::string& path) {
			return make_storage(path, false, false);
		}

		static std::unique_ptr<storage_t> make_writable_storage(const std::string& path) {
			return make_storage(path, true, true);
		}

		static std::unique_ptr<storage_t> make_storage(const std::string& path, bool write_access, bool create)
		{
			auto original_storage = std::make_unique<storage_t>(path.c_str());
			original_storage->open(write_access, create);
			if (original_storage->result() != POLE::Storage::Ok)
				throw std::runtime_error("storage open error");
			return original_storage;
		}

		static std::vector<std::string> make_full_entries(std::unique_ptr<storage_t>& storage, const std::string root)
		{
			std::vector<std::string> full_entries;
			auto streams = storage->GetAllStreams(root);
			std::copy(streams.begin(), streams.end(), std::back_inserter(full_entries));
			return full_entries;
		}
	private:
	};

	class hwp_50_filter_t
	{
	public:
		typedef cfb_traits::storage_t storage_t;
		typedef cfb_traits::stream_t stream_t;
		typedef binary_traits::byte_t byte_t;
		typedef binary_traits::buffer_t buffer_t;
		typedef binary_traits::bufferstream bufferstream;
		typedef binary_traits::streamsize streamsize;
		typedef syntax_traits::texts_t texts_t;
		typedef syntax_traits::text_t text_t;
		hwp_50_filter_t() {}

		struct file_header_t
		{
			file_header_t() : version(0), kogl(0)
			{
				std::fill(std::begin(reserved), std::end(reserved), 0);
			}

			bool is_compressed() const {
				return options[0];
			}

			void set_compressed(bool compress) {
				options[0] = compress;
			}

			static const size_t signature_size = 32;
			std::string signature;
			uint32_t version;
			std::bitset<32> options;
			std::bitset<32> extended_options;
			binary_traits::byte_t kogl;
			binary_traits::byte_t reserved[207];
		};

		struct header_t
		{
			typedef uint32_t tag_t;
			typedef uint32_t size_t;
			typedef uint32_t level_t;
			header_t() : tag(0), level(0), body_size(0)
			{}

			std::size_t size() const {
				return sizeof(uint32_t);
			}

			tag_t tag;
			level_t level;
			size_t body_size;
		};

		struct record_t
		{
			record_t()
			{}

			std::size_t size() const {
				return header.size() + body.size();
			}

			header_t header;
			buffer_t body;
		};

		struct para_text_t
		{
			enum control_is_t : uint16_t {
				is_char_control = 0,
				is_extend_control,
				is_inline_control
			};

			struct control_t
			{
				typedef uint16_t value_type;
				control_t() : type(is_char_control)
				{}

				std::size_t size() const {
					return body.size() * sizeof(value_type);
				}

				control_is_t type;
				std::vector<value_type> body;
			};

			para_text_t()
			{}

			std::size_t size() const
			{
				return std::accumulate(controls.begin(), controls.end(), 0, [](std::size_t size, auto& control) {
					return size + control.size();
				});
			}

			std::vector<control_t> controls;
		};

		struct syntax_t
		{
			typedef uint16_t control_t;
			enum tag_t : header_t::tag_t
			{
				// TODO: implement
				HWPTAG_BEGIN = 16,
				// docinfo
				
				// body text
				HWPTAG_PARA_TEXT = HWPTAG_BEGIN + 51
			};

			static bool is_carriage_return(control_t code)
			{
				return (code == 13);
			}

			static bool is_para_text(control_t code)
			{
				return (code == HWPTAG_PARA_TEXT);
			}

			static bool is_char_control(control_t code)
			{
				return !is_extend_control(code) && !is_inline_control(code);
			}

			static bool is_extend_control(control_t code)
			{
				return (
					(code >= 1 && code <= 3) || (code >= 11 && code <= 12) ||
					(code >= 14 && code <= 18) || (code >= 21 && code <= 23)
					);
			}

			static bool is_inline_control(control_t code)
			{
				return ( (code >= 4 && code <= 9) || (code >= 19 && code <= 20) );
			}

			static std::string section_root()
			{
				return std::string("/BodyText/");
			}
		};

		bool replace_privacy(const std::string& import_path, const std::string& export_path, const std::wregex& pattern, char16_t replace_dest)
		{
			try
			{
				std::unique_ptr<storage_t> import_storage = cfb_t::make_read_only_storage(import_path);
				std::unique_ptr<storage_t> export_storage = cfb_t::make_writable_storage(export_path);
				auto section_streams = cfb_t::make_full_entries(import_storage, syntax_t::section_root());
				auto all_streams_except_sections = cfb_t::make_all_streams_except(import_storage, section_streams);
				auto import_header = read_file_header(import_storage);
				cfb_t::copy_streams(import_storage, export_storage, all_streams_except_sections);
				for (auto& section_stream : section_streams)
				{
					auto section = cfb_t::extract_stream(import_storage, section_stream);
					if (import_header.is_compressed())
						section = hwp_zip::decompress_noexcept(section);

					bufferstream records_stream(&section[0], section.size());
					auto records = read_records(records_stream);
					std::vector<std::reference_wrapper<record_t>> para_text_record;
					std::for_each(records.begin(), records.end(), [&para_text_record](record_t& record) {
						if (syntax_t::is_para_text(record.header.tag))
							para_text_record.push_back(record);
					});

					for (auto record : para_text_record)
					{
						bufferstream para_text_stream(&record.get().body[0], record.get().header.body_size);
						auto para_texts = read_para_text(para_text_stream, record.get().header.body_size);
						for (auto& para_text : para_texts.controls)
						{
							if (para_text.type == para_text_t::is_char_control)
							{
								std::wstring texts;
								std::copy(para_text.body.begin(), para_text.body.end(), std::back_inserter(texts));
								std::match_results<std::wstring::iterator> results;
								auto begin = texts.begin();
								while (std::regex_search(begin, texts.end(), results, pattern))
								{
									for (auto i = results[0].first; i != results[0].second; ++i)
									{
										*i = replace_dest;
									}
									begin += results.position() + results.length();
								}

								para_text.body.clear();
								std::copy(texts.begin(), texts.end(), std::back_inserter(para_text.body));
							}
						}
						record.get().header.body_size = para_texts.size();
						buffer_t write_record_buffer;
						write_record_buffer.resize(record.get().header.body_size);
						bufferstream para_text_export_stream(&write_record_buffer[0], write_record_buffer.size());
						write_para_text(para_text_export_stream, para_texts);
						record.get().body = std::move(write_record_buffer);
					}

					auto write_size = std::accumulate(records.begin(), records.end(), 0, [](std::size_t size, auto& record) {
						return size + record.size(); });

					buffer_t write_buffer;
					write_buffer.resize(write_size);
					bufferstream write_records_stream(&write_buffer[0], write_buffer.size());
					write_records(write_records_stream, records);

					if (!write_buffer.empty())
					{
						if (import_header.is_compressed())
							write_buffer = hwp_zip::compress_noexcept(write_buffer);
						cfb_t::make_stream(export_storage, section_stream, write_buffer);
					}
				}
				export_storage->close();
				return true;
			}
			catch (const std::exception& e)
			{
				std::cout << e.what() << std::endl;
			}
			catch (...)
			{
			}
			return false;
		}

		bool replace_all_texts(const std::string& import_path, const std::string& export_path, wchar_t replace_dest)
		{
			try
			{
				std::unique_ptr<storage_t> import_storage = cfb_t::make_read_only_storage(import_path);
				std::unique_ptr<storage_t> export_storage = cfb_t::make_writable_storage(export_path);		
				auto section_streams = cfb_t::make_full_entries(import_storage, syntax_t::section_root());
				auto all_streams_except_sections = cfb_t::make_all_streams_except(import_storage, section_streams);
				auto import_header = read_file_header(import_storage);
				cfb_t::copy_streams(import_storage, export_storage, all_streams_except_sections);
				for (auto& section_stream : section_streams)
				{
					auto section = cfb_t::extract_stream(import_storage, section_stream);					
					if (import_header.is_compressed())
						section = hwp_zip::decompress_noexcept(section);

					bufferstream records_stream(&section[0], section.size());
					auto records = read_records(records_stream);
					std::vector<std::reference_wrapper<record_t>> para_text_record;
					std::for_each(records.begin(), records.end(), [&para_text_record](record_t& record) {
						if (syntax_t::is_para_text(record.header.tag))
							para_text_record.push_back(record);
					});

					for (auto record : para_text_record)
					{
						bufferstream para_text_stream(&record.get().body[0], record.get().header.body_size);
						auto para_texts = read_para_text(para_text_stream, record.get().header.body_size);
						for (auto& para_text : para_texts.controls)
						{
							if (para_text.type == para_text_t::is_char_control)
							{
								std::for_each(para_text.body.begin(), para_text.body.end(), [&replace_dest](auto& code) {
									typedef std::remove_reference< std::remove_cv<decltype(code)>::type >::type code_t;
									if (!syntax_t::is_carriage_return(code))
										code = static_cast<code_t>(replace_dest);
								});
							}
						}
						record.get().header.body_size = para_texts.size();
						buffer_t write_record_buffer;
						write_record_buffer.resize(record.get().header.body_size);
						bufferstream para_text_export_stream(&write_record_buffer[0], write_record_buffer.size());
						write_para_text(para_text_export_stream, para_texts);
						record.get().body = std::move(write_record_buffer);
					}

					auto write_size = std::accumulate(records.begin(), records.end(), 0, [](std::size_t size, auto& record) {
						return size + record.size(); });

					buffer_t write_buffer;
					write_buffer.resize(write_size);
					bufferstream write_records_stream(&write_buffer[0], write_buffer.size());
					write_records(write_records_stream, records);

					if (!write_buffer.empty())
					{
						if (import_header.is_compressed())
							write_buffer = hwp_zip::compress_noexcept(write_buffer);
						cfb_t::make_stream(export_storage, section_stream, write_buffer);
					}
				}
				export_storage->close();
				return true;
			}
			catch (const std::exception& e)
			{
				std::cout << e.what() << std::endl;
			}
			catch (...)
			{
			}
			return false;
		}

		std::vector<texts_t> extract_all_texts(const std::string& import_path)
		{
			try
			{
				std::vector<texts_t> sections;
				std::unique_ptr<storage_t> import_storage = cfb_t::make_read_only_storage(import_path);
				auto section_streams = cfb_t::make_full_entries(import_storage, syntax_t::section_root());
				auto header = read_file_header(import_storage);

				for (auto& section_stream : section_streams)
				{
					auto section = cfb_t::extract_stream(import_storage, section_stream);
					if (header.is_compressed())
					{
						section = hwp_zip::decompress(section);
					}

					bufferstream stream(&section[0], section.size());
					auto records = read_records(stream);
					auto section_texts = extract_texts(records);
					sections.push_back(std::move(section_texts));
				}
				return sections;
			}
			catch (const std::exception& e)
			{
				std::cout << e.what() << std::endl;
			}
			catch (...)
			{
			}
			return std::vector<texts_t>();
		}

		bool decompress_save(const std::string& import_path, const std::string& export_path)
		{
			try
			{
				std::unique_ptr<storage_t> import_storage = cfb_t::make_read_only_storage(import_path);
				std::unique_ptr<storage_t> export_storage = cfb_t::make_writable_storage(export_path);

				auto all_streams = cfb_t::make_full_entries(import_storage, "/");
				auto file_header_name = std::find_if(all_streams.begin(), all_streams.end(), [](const std::string& name) {
					return name == "/FileHeader"; });
				if(file_header_name != all_streams.end())
					all_streams.erase(file_header_name);

				auto import_header = read_file_header(import_storage);
				auto export_header = import_header;
				export_header.set_compressed(false);
				write_file_header(export_storage, export_header);
				for (auto& stream : all_streams)
				{
					auto plain = cfb_t::extract_stream(import_storage, stream);
					if (import_header.is_compressed())
						plain = hwp_zip::decompress_noexcept(plain);
					if(!plain.empty())
						cfb_t::make_stream(export_storage, stream, plain);
				}
				export_storage->close();
				return true;
			}
			catch (const std::exception& e)
			{
				std::cout << e.what() << std::endl;
			}
			catch (...)
			{}
			return false;
		}
	private:
		file_header_t read_file_header(std::unique_ptr<storage_t>& storage)
		{
			buffer_t buffer = cfb_t::extract_stream(storage, "/FileHeader");
			bufferstream stream(&buffer[0], buffer.size());
			file_header_t file_header;
			file_header.signature = binary_stream_t::read_string(stream, file_header.signature_size);
			file_header.version = binary_stream_t::read_uint32(stream);
			file_header.options = binary_stream_t::read_uint32(stream);
			file_header.extended_options = binary_stream_t::read_uint32(stream);
			file_header.kogl = binary_stream_t::read_uint8(stream);
			return file_header;
		}

		void write_file_header(std::unique_ptr<storage_t>& storage, const file_header_t& file_header)
		{
			buffer_t buffer;
			buffer.resize(256);
			bufferstream stream(&buffer[0], buffer.size());
			binary_stream_t::write_string(stream, file_header.signature);
			binary_stream_t::write_uint32(stream, file_header.version);
			binary_stream_t::write_uint32(stream, file_header.options.to_ulong() );
			binary_stream_t::write_uint32(stream, file_header.extended_options.to_ulong());
			binary_stream_t::write_uint8(stream, file_header.kogl);
			cfb_t::make_stream(storage, "/FileHeader", buffer);
		}

		header_t read_header(bufferstream& stream)
		{
			header_t header;
			uint32_t plain = binary_stream_t::read_uint32(stream);
			header.tag = plain & 0x3FF;
			header.level = (plain >> 10) & 0x3FF;
			header.body_size = (plain >> 20) & 0xFFF;

			if (header.body_size == 0xFFF)
			{
				header.body_size = binary_stream_t::read_uint32(stream);
			}
			return header;
		}

		void write_header(bufferstream& stream, const header_t& header)
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
		}

		buffer_t read_body(bufferstream& stream, streamsize size)
		{
			return binary_stream_t::read(stream, size);
		}

		void write_body(bufferstream& stream, const buffer_t& buffer)
		{
			return binary_stream_t::write(stream, buffer);
		}

		std::vector<record_t> read_records(bufferstream& stream)
		{
			std::vector<record_t> records;
			stream.seekg(0);
			try
			{		
				do
				{
					record_t record;
					record.header = read_header(stream);
					record.body = read_body(stream, record.header.body_size);
					records.push_back(std::move(record));
				} while (!stream.eof());
			}
			catch (const std::exception&)
			{
				if (stream.eof())
					return records;
				throw std::runtime_error("read records error");
			}
			return records;
		}

		void write_records(bufferstream& stream, const std::vector<record_t>& records)
		{
			for (auto& record : records)
			{
				write_header(stream, record.header);
				write_body(stream, record.body);
			}
		}

		para_text_t read_para_text(bufferstream& stream, streamsize size)
		{
			const streamsize size_of_control = sizeof(syntax_t::control_t);
			const streamsize size_of_inline_control = 7 * sizeof(syntax_t::control_t); // TODO: abstract 7
			para_text_t para_text;
			for (streamsize offset = 0; offset < size; offset += size_of_control)
			{
				syntax_t::control_t code = binary_stream_t::read_uint16(stream);			
				if (syntax_t::is_extend_control(code))
				{
					para_text_t::control_t control;
					control.body.push_back(code);
					control.type = para_text_t::control_is_t::is_extend_control;
					for (size_t i = 0; i < size_of_inline_control; i += size_of_control)
						control.body.push_back( binary_stream_t::read_uint16(stream) );
					offset += size_of_inline_control;
					para_text.controls.push_back(std::move(control));
				}
				else if (syntax_t::is_inline_control(code))
				{
					para_text_t::control_t control;
					control.body.push_back(code);
					control.type = para_text_t::control_is_t::is_inline_control;
					for (size_t i = 0; i < size_of_inline_control; i += size_of_control)
						control.body.push_back(binary_stream_t::read_uint16(stream));
					offset += size_of_inline_control;
					para_text.controls.push_back(std::move(control));
				}
				else // char control
				{
					if ( !para_text.controls.empty() &&
						para_text.controls.back().type == para_text_t::control_is_t::is_char_control )
					{
						para_text.controls.back().body.push_back(code);
					}
					else
					{
						para_text_t::control_t control;
						control.type = para_text_t::control_is_t::is_char_control;
						control.body.push_back(code);
						para_text.controls.push_back(std::move(control));
					}
				}
			}
			return para_text;
		}

		void write_para_text(bufferstream& stream, const para_text_t& records)
		{
			for (auto& control : records.controls)
			{
				for (auto& code : control.body)
				{
					binary_stream_t::write_uint16(stream, code);
				}
			}
		}

		text_t extract_para_text(bufferstream& stream, streamsize size)
		{
			text_t texts;
			const streamsize size_of_control = sizeof(syntax_t::control_t);
			const streamsize size_of_inline_control = 7 * sizeof(syntax_t::control_t); // TODO: abstract 7

			for (streamsize offset = 0; offset < size; offset += size_of_control)
			{
				syntax_t::control_t code = binary_stream_t::read_uint16(stream);
				if (syntax_t::is_char_control(code))
				{
					if (syntax_t::is_carriage_return(code))
						texts.push_back(L'\n'); // TODO: normalize
					else
						texts.push_back(static_cast<text_t::value_type>(code));
				}
				else
				{
					// TODO: implement tab
					auto inline_contol = binary_stream_t::read(stream, size_of_inline_control);
					offset += size_of_inline_control;
				}
			}
			return texts;
		}

		texts_t extract_texts(std::vector<record_t>& records)
		{
			texts_t texts;
			for (auto& record : records)
			{
				if (syntax_t::is_para_text(record.header.tag))
				{
					bufferstream stream(&record.body[0], record.header.body_size);
					auto text = extract_para_text(stream, record.header.body_size);
					if (!text.empty())
						texts.push_back(text);
				}
			}
			return texts;
		}
	};
}


std::u16string u8_to_u16(const std::string& source)
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
	return convert.from_bytes(source);
}

std::string u16_to_u8(const std::u16string& source)
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
	return convert.to_bytes(source);
}

void test_decompress_save()
{
	filter::hwp_50_filter_t filter;
	filter.decompress_save("d:/filter/sample_compress.hwp", "d:/filter/sample_compress.hwp.hwp");
	filter.decompress_save("d:/filter/text.hwp", "d:/filter/text.hwp.hwp");
	filter.decompress_save("d:/filter/sample2.hwp", "d:/filter/sample2.hwp.hwp");
}

void test_extract_all_texts()
{
	filter::hwp_50_filter_t filter;
	auto sections = filter.extract_all_texts(u16_to_u8(u"d:/filter/[여성부]김현진_우리아이지키기_종합대책(08.5.27).hwp"));
	//auto sections = filter.extract_all_texts(u16_to_u8(u"d:/filter/[국립공원관리공단]조선희_북한산독립유공자묘역정비[이미지].hwp"));
	//auto sections = filter.extract_all_texts("d:/filter/sample2.hwp");
	//auto sections = filter.extract_all_texts("d:/filter/sample_compress.hwp");
	//auto sections = filter.extract_all_texts("d:/filter/text.hwp");
	setlocale(LC_ALL, "korean");
	for (auto& section : sections)
	{
		for (auto& para : section)
		{
			if (!para.empty() && para.size() != 1)
			{
				std::wcout << para;
				if (std::wcout.bad())
					std::wcout.clear();
			}
		}
	}
}

void test_replace_all_texts()
{
	filter::hwp_50_filter_t filter;
	filter.replace_all_texts("d:/filter/sample_compress.hwp", "d:/filter/sample_compress.hwp.hwp", L'*');
	filter.replace_all_texts("d:/filter/text.hwp", "d:/filter/text.hwp.hwp", L'*');
	filter.replace_all_texts("d:/filter/sample2.hwp", "d:/filter/sample2.hwp.hwp", L'*');
}


void test_replace_privacy()
{
	std::wregex resident_registration_number(L"(?:[0-9]{2}(?:0[1-9]|1[0-2])(?:0[1-9]|[1,2][0-9]|3[0,1]))-[1-4][0-9]{6}");
	filter::hwp_50_filter_t filter;
	filter.replace_privacy(u16_to_u8(u"d:/filter/privacy.hwp"), u16_to_u8(u"d:/filter/privacy.hwp.hwp"), resident_registration_number, u'*');
	filter.replace_privacy(u16_to_u8(u"d:/filter/개인정보.hwp"), u16_to_u8(u"d:/filter/개인정보.hwp.hwp"), resident_registration_number, u'*');
}

int main()
{
	//test_decompress_save();
	test_extract_all_texts();
	//test_replace_all_texts();
	//test_replace_privacy();
	return 0;
}