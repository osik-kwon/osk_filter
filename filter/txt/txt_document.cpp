#include "filter_pch.h"
#include "txt/txt_document.h"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/newline.hpp>
#include <boost/iostreams/device/file.hpp>

#include <boost/iostreams/categories.hpp> 
#include <boost/iostreams/code_converter.hpp>
#include <boost/locale.hpp>

#include "io/file_stream.h"
#include "locale/charset_detecter.h"
#include "locale/charset_encoder.h"

namespace filter
{
namespace txt
{
	consumer_t::consumer_t() : newline_type(boost::iostreams::newline::posix)
	{}

	std::string consumer_t::detect_charset(const std::string& path)
	{
		try
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
			auto charset = ::filter::charset_detecter::detect(buffer);
			if (charset == "ASCII")
				charset = "UTF-8"; // IMPORTANT!
			return charset;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return std::string();
	}

	int consumer_t::detect_newline_type(const std::string& path)
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
			if(!filter)
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
	
	void consumer_t::open(const std::string& path)
	{
		try
		{
			charset = consumer_t::detect_charset(path);
			newline_type = consumer_t::detect_newline_type(path);

			std::ifstream file(to_fstream_path(path), std::ios::binary);					
			boost::iostreams::filtering_istream stream;

			stream.push(boost::iostreams::newline_filter(boost::iostreams::newline::posix));
			stream.push(file);

			std::string line;
			while (!stream.eof())
			{
				std::getline(stream, line);
 				auto para = boost::locale::conv::to_utf<char_t>(line, charset);
				document.emplace_back(std::move(para));
			}

			// TODO: remove BOM
			// TODO: UTF16
			/*if (!document.empty())
			{
				if (!document.front().empty() && document.front()[0] == 0xfeff)
					document.front().erase(document.front().begin());
			}*/
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}	
	}

	producer_t::producer_t()
	{}

	std::string producer_t::make_newline(int type)
	{
		switch (type)
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

	void producer_t::save(const std::string& path, std::unique_ptr<consumer_t>& consumer, std::string dest_charset, int newline_type)
	{
		try
		{
			if(consumer->get_document().size() == 0)
				throw std::runtime_error("txt document is empty");

			std::string charset = dest_charset.empty() ? consumer->get_charset() : dest_charset;
			const std::string newline = boost::locale::conv::from_utf<char>( 
				newline_type > 0 ? make_newline(newline_type) : make_newline(consumer->get_newline_type()),
				charset );

			std::ofstream file(to_fstream_path(path), std::ios::binary | std::ios::out);
			auto& document = consumer->get_document();
			int last_para_id = document.size() - 1;
			for (int i = 0; i < last_para_id; ++i)
			{
				file << boost::locale::conv::from_utf<char_t>(document[i], charset);
				file << newline;
			}
			file << boost::locale::conv::from_utf<char_t>(document[last_para_id], charset);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}
}
}