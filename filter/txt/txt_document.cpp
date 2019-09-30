#include "filter_pch.h"
#include "txt/txt_document.h"
#include <locale>
#include <codecvt>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/newline.hpp>
#include <boost/iostreams/device/file.hpp>

#include <boost/iostreams/categories.hpp> 
#include <boost/iostreams/code_converter.hpp>
#include <boost/locale.hpp>

#include "traits/binary_traits.h"
#include "io/file_stream.h"
#include "io/newline_filter.h"
#include "locale/charset_detecter.h"
#include "locale/charset_encoder.h"

namespace filter
{
namespace txt
{
	class locale_t
	{
	public:
		locale_t() = default;

		static bool is_international(const std::string& charset)
		{
			return charset == "UTF-8" || charset == "UTF-16" || charset == "UTF-32";
		}

		static bool is_utf32(const std::string& charset)
		{
			return charset == "UTF-32";
		}

		template <class stream_t, std::codecvt_mode option>
		static void imbue(const std::string& charset, stream_t& stream)
		{
			if (charset == "UTF-8")
				stream.imbue(std::locale(stream.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, option>));
			else if (charset == "UTF-16")
				stream.imbue(std::locale(stream.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, option>));
			else if (charset == "UTF-32")
				stream.imbue(std::locale(stream.getloc(), new std::codecvt_utf16<char32_t, 0x10ffff, option>));
		}

		static std::wstring non_international_to_wstring(std::string& src, const std::string& charset);
	private:
	};


	std::wstring locale_t::non_international_to_wstring(std::string& src, const std::string& charset)
	{
		return boost::locale::conv::to_utf<wchar_t>(src, charset);
	}

	class detecter_t
	{
	public:
		typedef consumer_t::byte_order_t byte_order_t;
		detecter_t() = default;
		static std::string detect_charset(const std::string& path);
		static int detect_newline_type(const std::string& path);
		static int detect_newline_type_from_string(const std::string& u8string);
		static int detect_wnewline_type(const std::string& path, const std::string& charset);
		static byte_order_t detect_byte_order(const std::string& path, const std::string& charset);
	private:
		static std::string read_buffer(const std::string& path);
	};

	std::string detecter_t::read_buffer(const std::string& path)
	{
		std::ifstream file(to_fstream_path(path), std::ios::binary);
		if (file.fail())
			throw std::runtime_error("file I/O error");
		file.unsetf(std::ios::skipws);
		file.seekg(0, std::ios::end);
		std::streampos size = file.tellg();
		file.seekg(0, std::ios::beg);

		const size_t minimal_length = 255;
		size_t file_size = (size_t)size;
		size_t buffer_size = 0;
		if (file_size > minimal_length)
			buffer_size = minimal_length;
		else
			buffer_size = file_size;

		std::string buffer(buffer_size, ' ');
		file.read(&buffer[0], buffer_size);
		file.close();
		return buffer;
	}

	std::string detecter_t::detect_charset(const std::string& path)
	{
		try
		{
			auto buffer = read_buffer(path);
			auto charset = ::filter::charset_detecter::detect(buffer);
			if (charset.empty() || charset == "ASCII")
				charset = "UTF-8"; // IMPORTANT!
			return charset;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return std::string();
	}

	detecter_t::byte_order_t detecter_t::detect_byte_order(const std::string& path, const std::string& charset)
	{
		if (!locale_t::is_international(charset))
			return byte_order_t::unknown;

		auto buffer = read_buffer(path);
		auto length = buffer.size();
		if (length < 3)
			return byte_order_t::unknown;
		auto first = buffer[0];
		auto second = buffer[1];
		auto third = buffer[2];
		if (first == '\xEF' && second == '\xBB' && third == '\xBF')
			return byte_order_t::utf8_bom; // EF BB BF: UTF-8 encoded BOM
		if (first == '\xFE' && second == '\xFF')
			return byte_order_t::big_endian; // FE FF: UTF-16, big endian BOM
		if (first == '\xFF' && second == '\xFE')
		{
			if (length > 3 && third == '\x00' && buffer[3] == '\x00')
				return byte_order_t::little_endian; // FF FE 00 00: UTF-32 (LE).
			else
				return byte_order_t::little_endian; // FF FE: UTF-16, little endian BOM.
		}
		if (first == '\x00')
		{
			if (length > 3 && second == '\x00' && third == '\xFE' && buffer[3] == '\xFF')
				return byte_order_t::big_endian; // 00 00 FE FF: UTF-32 (BE).
		}
		return byte_order_t::unknown;
	}

	int detecter_t::detect_newline_type(const std::string& path)
	{
		try
		{
			std::ifstream file(to_fstream_path(path), std::ios::binary);
			if (file.fail())
				throw std::runtime_error("file I/O error");

			boost::iostreams::filtering_istream stream;
			stream.push(boost::iostreams::newline_checker());
			stream.push(file);

			std::string para;
			std::getline(stream, para);
			auto filter = stream.filters().component<boost::iostreams::newline_checker>(0);
			if (!filter)
				throw std::runtime_error("boost::iostreams::newline_checker is null");

			int newline_type = boost::iostreams::newline::posix;
			if (filter->is_dos())
				newline_type = boost::iostreams::newline::dos;
			else if (filter->is_mac())
				newline_type = boost::iostreams::newline::mac;
			else if (filter->is_posix())
				newline_type = boost::iostreams::newline::posix;
			return newline_type;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return boost::iostreams::newline::posix;
	}

	int detecter_t::detect_newline_type_from_string(const std::string& u8string)
	{
		try
		{
			boost::iostreams::array_source src(reinterpret_cast<const char*>(u8string.data()), u8string.size());
			boost::iostreams::filtering_istream stream;
			stream.push(boost::iostreams::newline_checker());
			stream.push(src);

			std::string para;
			std::getline(stream, para);
			auto filter = stream.filters().component<boost::iostreams::newline_checker>(0);
			if (!filter)
				throw std::runtime_error("boost::iostreams::newline_checker is null");

			int newline_type = boost::iostreams::newline::posix;
			if (filter->is_dos())
				newline_type = boost::iostreams::newline::dos;
			else if (filter->is_mac())
				newline_type = boost::iostreams::newline::mac;
			else if (filter->is_posix())
				newline_type = boost::iostreams::newline::posix;
			return newline_type;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return boost::iostreams::newline::posix;
	}

	int detecter_t::detect_wnewline_type(const std::string& path, const std::string& charset)
	{
		try
		{
			std::wifstream file(to_fstream_path(path), std::ios::binary);
			if (file.fail())
				throw std::runtime_error("file I/O error");
			locale_t::imbue<std::wifstream, std::consume_header>(charset, file);

			boost::iostreams::filtering_wistream stream;
			stream.push(boost::iostreams::wnewline_checker());
			stream.push(file);

			std::wstring para;
			std::getline(stream, para);
			auto filter = stream.filters().component<boost::iostreams::wnewline_checker>(0);
			if (!filter)
				throw std::runtime_error("boost::iostreams::wnewline_checker is null");

			int newline_type = boost::iostreams::newline::posix;
			if (filter->is_dos())
				newline_type = boost::iostreams::newline::dos;
			else if (filter->is_mac())
				newline_type = boost::iostreams::newline::mac;
			else if (filter->is_posix())
				newline_type = boost::iostreams::newline::posix;
			return newline_type;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return boost::iostreams::newline::posix;
	}

	consumer_t::consumer_t() : newline_type(boost::iostreams::newline::posix), byte_order(byte_order_t::unknown)
	{}

	std::string consumer_t::detect_charset(const std::string& path) {
		return detecter_t::detect_charset(path);
	}
	
	void consumer_t::open_non_international(const std::string& path)
	{
		try
		{
			newline_type = detecter_t::detect_newline_type(path);

			std::ifstream file(to_fstream_path(path), std::ios::binary);
			if (file.fail())
				throw std::runtime_error("file I/O error");

			boost::iostreams::filtering_istream stream;
			stream.push(boost::iostreams::newline_filter(boost::iostreams::newline::posix));
			stream.push(file);

			std::string line;
			while (!stream.eof())
			{
				std::getline(stream, line);
				auto para = locale_t::non_international_to_wstring(line, charset);
				document.emplace_back(std::move(para));
			}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	void consumer_t::open_international(const std::string& path)
	{
		try
		{
			newline_type = detecter_t::detect_wnewline_type(path, charset);
			byte_order = detecter_t::detect_byte_order(path, charset);

			std::wifstream file(to_fstream_path(path), std::ios::binary);
			if (file.fail())
				throw std::runtime_error("file I/O error");

			locale_t::imbue<std::wifstream, std::consume_header>(charset, file);
			boost::iostreams::filtering_wistream stream;
			stream.push(boost::iostreams::wnewline_filter(boost::iostreams::newline::posix));
			stream.push(file);

			while (!stream.eof())
			{
				std::wstring para;
				std::getline(stream, para);
				document.emplace_back(std::move(para));
			}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	void consumer_t::open_utf32(const std::string& path)
	{
		try
		{
			byte_order = detecter_t::detect_byte_order(path, charset);
			std::ifstream file(to_fstream_path(path), std::ios::binary);
			if (file.fail())
				throw std::runtime_error("file I/O error");

			std::string buffer{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
			if (buffer.length() % 4 != 0)
				throw std::logic_error("size in bytes must be a multiple of 4");

			buffer = buffer.substr(4);
			const size_t count = buffer.length() / 4;
			auto u32buffer = std::u32string(count, 0);
			if (byte_order == byte_order_t::big_endian)
			{
				for (size_t i = 0; i < count; ++i)
				{
					uint32_t first = static_cast<uint8_t>(buffer[i * 4 + 3]);
					uint32_t second = static_cast<uint8_t>(buffer[i * 4 + 2]) << 8;
					uint32_t third = static_cast<uint8_t>(buffer[i * 4 + 1]) << 16;
					uint32_t fourth = static_cast<uint8_t>(buffer[i * 4 + 0]) << 24;
					u32buffer[i] = static_cast<char32_t>(first | second | third | fourth);
				}
			}
			else if(byte_order == byte_order_t::little_endian)
			{
				for (size_t i = 0; i < count; ++i)
				{
					uint32_t first = static_cast<uint8_t>(buffer[i * 4 + 0]);
					uint32_t second = static_cast<uint8_t>(buffer[i * 4 + 1]) << 8;
					uint32_t third = static_cast<uint8_t>(buffer[i * 4 + 2]) << 16;
					uint32_t fourth = static_cast<uint8_t>(buffer[i * 4 + 3]) << 24;
					u32buffer[i] = static_cast<char32_t>(first | second | third | fourth);
				}
			}
			auto u8buffer = to_utf8(u32buffer);
			newline_type = detecter_t::detect_newline_type_from_string(u8buffer);
			// TODO:
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	void consumer_t::open(const std::string& path)
	{
		try
		{
			charset = detecter_t::detect_charset(path);	
			if (locale_t::is_international(charset))
			{
				if (locale_t::is_utf32(charset))
					open_utf32(path);
				else
					open_international(path);
			}
			else
			{
				open_non_international(path);
			}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	producer_t::producer_t()
	{}

	std::string producer_t::make_newline(std::unique_ptr<consumer_t>& consumer, int custom_type) const
	{
		custom_type = custom_type > 0 ? custom_type : consumer->get_newline_type();
		switch (custom_type)
		{
		case boost::iostreams::newline::posix:
			return std::string("\n");
		case boost::iostreams::newline::mac:
			return std::string("\r");
		case boost::iostreams::newline::dos:
			return std::string("\r\n");
		default:
			throw std::runtime_error("invalid newline type");
		}
	}

	std::wstring producer_t::make_wnewline(std::unique_ptr<consumer_t>& consumer, int custom_type) const
	{
		auto newline = make_newline(consumer, custom_type);
		std::wstring wnewline;
		std::copy(newline.begin(), newline.end(), std::back_inserter(wnewline));
		return wnewline;
	}

	void producer_t::save_international(const std::string& path, std::unique_ptr<consumer_t>& consumer, custom_params_t params)
	{
		try
		{
			if (consumer->get_document().size() == 0)
				throw std::runtime_error("txt document is empty");			
			std::wofstream file(to_fstream_path(path), std::ios::binary | std::ios::out);
			if (file.fail())
				throw std::runtime_error("file I/O error");

			if (*params.byte_order == byte_order_t::utf8_bom)
			{
				const auto option = (std::codecvt_mode)(std::generate_header);
				locale_t::imbue<std::wofstream, option >(*params.charset, file);
			}
			else if (*params.byte_order == byte_order_t::little_endian)
			{
				const auto option = (std::codecvt_mode)(std::generate_header | std::little_endian);
				locale_t::imbue<std::wofstream, option >(*params.charset, file);
			}
			else if(*params.byte_order == byte_order_t::big_endian)
				locale_t::imbue<std::wofstream, std::generate_header>(*params.charset, file);
			else
				locale_t::imbue<std::wofstream, (std::codecvt_mode)(0)>(*params.charset, file);

			boost::iostreams::filtering_wostream stream;
			stream.push(boost::iostreams::wnewline_filter(*params.newline_type ));
			stream.push(file);

			auto newline = make_wnewline(consumer, *params.newline_type);
			auto& document = consumer->get_document();
			int last_para_id = document.size() - 1;
			for (int i = 0; i < last_para_id; ++i)
			{
				stream << document[i];
				stream << newline;
			}
			stream << document[last_para_id];
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	void producer_t::save_non_international(const std::string& path, std::unique_ptr<consumer_t>& consumer, custom_params_t params)
	{
		try
		{
			if (consumer->get_document().size() == 0)
				throw std::runtime_error("txt document is empty");

			std::ofstream file(to_fstream_path(path), std::ios::binary | std::ios::out);
			auto newline = make_newline(consumer, *params.newline_type);
			auto& document = consumer->get_document();
			int last_para_id = document.size() - 1;
			for (int i = 0; i < last_para_id; ++i)
			{
				file << boost::locale::conv::from_utf<char_t>(document[i], *params.charset);
				file << newline;
			}
			file << boost::locale::conv::from_utf<char_t>(document[last_para_id], *params.charset);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	void producer_t::save(const std::string& path, std::unique_ptr<consumer_t>& consumer, custom_params_t params)
	{
		try
		{
			if (consumer->get_document().size() == 0)
				throw std::runtime_error("txt document is empty");
			params.charset = params.charset ?
				params.charset : consumer->get_charset();
			params.newline_type = params.newline_type ?
				params.newline_type : consumer->get_newline_type();
			params.byte_order = params.byte_order ?
				params.byte_order : consumer->get_byte_order();

			if (locale_t::is_international(*params.charset))
				save_international(path, consumer, params);
			else
				save_non_international(path, consumer, params);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}
}
}