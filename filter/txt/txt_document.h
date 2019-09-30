#pragma once
#include <memory>
#include <string>
#include <vector>
#include <boost/optional.hpp>

namespace filter
{
namespace txt
{
	class consumer_t
	{
	public:
		typedef std::wstring::value_type char_t;
		typedef std::wstring para_t;
		typedef std::vector< para_t > document_t;
		enum byte_order_t : int
		{
			unknown = 0, utf8_bom, little_endian, big_endian
		};
		consumer_t();
		void open(const std::string& path);
		static std::string detect_charset(const std::string& path);
		document_t& get_document() {
			return document;
		}
		const std::string& get_charset() const {
			return charset;
		}
		int get_newline_type() const {
			return newline_type;
		}
		byte_order_t get_byte_order() const {
			return byte_order;
		}
	private:
		void open_non_international(const std::string& path);
		void open_international(const std::string& path);
		void open_utf32(const std::string& path);
		std::u32string open_utf32_stream(const std::string& path);

		document_t document;
		std::string charset;
		int newline_type;
		byte_order_t byte_order;
	};

	struct custom_params_t
	{
		typedef consumer_t::byte_order_t byte_order_t;
		custom_params_t() = default;
		boost::optional<std::string> charset;
		boost::optional<int> newline_type;
		boost::optional<byte_order_t> byte_order;
	};

	class producer_t
	{
	public:
		typedef consumer_t::char_t char_t;
		typedef consumer_t::byte_order_t byte_order_t;
		producer_t();
		void save(const std::string& path, std::unique_ptr<consumer_t>& consumer, custom_params_t params);
	private:
		static const int original_newline = -1;
		void save_non_international(const std::string& path, std::unique_ptr<consumer_t>& consumer, custom_params_t params);
		void save_international(const std::string& path, std::unique_ptr<consumer_t>& consumer, custom_params_t params);
		void save_utf32(const std::string& path, std::unique_ptr<consumer_t>& consumer, custom_params_t params);

		std::string make_newline(std::unique_ptr<consumer_t>& consumer, int custom_type = original_newline) const;
		std::wstring make_wnewline(std::unique_ptr<consumer_t>& consumer, int custom_type = original_newline) const;
		std::u32string make_u32newline(std::unique_ptr<consumer_t>& consumer, int custom_type = original_newline) const;
	};
}
}