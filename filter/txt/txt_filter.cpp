#include "filter_pch.h"
#include "txt/txt_filter.h"
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
	consumer_t::consumer_t()
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
			return ::filter::charset_detecter::detect(buffer);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return std::string();
	}

	void consumer_t::open(const std::string& path)
	{
		try
		{
			charset = consumer_t::detect_charset(path);
			std::ifstream file(to_fstream_path(path), std::ios::binary);
			if (file.fail())
				throw std::runtime_error("file I/O error");

			boost::iostreams::filtering_istream filtering_stream;
			filtering_stream.push(boost::iostreams::newline_filter(boost::iostreams::newline::posix));
			filtering_stream.push(file);

			para_t line;
			while (!filtering_stream.eof())
			{
				std::getline(filtering_stream, line);
				document.push_back(boost::locale::conv::to_utf<char>(line, charset));
			}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}	
	}

	producer_t::producer_t()
	{}

	void producer_t::save(const std::string& path, std::unique_ptr<consumer_t>& consumer, std::string dest_charset)
	{
		try
		{
			if(consumer->get_document().size() == 0)
				throw std::runtime_error("txt document is empty");
			std::string charset = dest_charset.empty() ? consumer->get_charset() : dest_charset;
			const std::string newline = boost::locale::conv::from_utf<char>("\n", charset);
			std::ofstream file(to_fstream_path(path), std::ios::binary | std::ios::out);
			auto& document = consumer->get_document();
			int last_para_id = document.size() - 1;
			for (int i = 0; i < last_para_id; ++i)
			{
				file << boost::locale::conv::from_utf<char>(document[i], charset);
				file << newline;
			}
			file << boost::locale::conv::from_utf<char>(document[last_para_id], charset);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	filter_t::filter_t()
	{}

	std::string filter_t::detect_charset(const std::string& path)
	{
		return consumer_t::detect_charset(path);
	}

	std::unique_ptr<consumer_t> filter_t::open(const std::string& path)
	{
		try
		{
			std::unique_ptr<consumer_t> consumer = std::make_unique<consumer_t>();
			consumer->open(path);
			return consumer;

		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return std::make_unique<consumer_t>();
	}

	void filter_t::save(const std::string& path, std::unique_ptr<consumer_t>& consumer, std::string dest_charset)
	{
		try
		{
			producer_t producer;
			producer.save(path, consumer, dest_charset);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	filter_t::sections_t filter_t::extract_all_texts(std::unique_ptr<consumer_t>& consumer)
	{
		sections_t sections;
		// TODO: implement
		return sections;
	}

	filter_t::sections_t filter_t::search_privacy(const rules_t& rules, std::unique_ptr<consumer_t>& consumer)
	{
		sections_t sections;
		// TODO: implement
		return sections;
	}

	void filter_t::replace_privacy(const rules_t& rules, char16_t replacement, std::unique_ptr<consumer_t>& consumer)
	{
		// TODO: implement
	}
}
}