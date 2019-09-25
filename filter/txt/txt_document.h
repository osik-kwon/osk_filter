#pragma once
#include <memory>
#include <string>
#include <vector>

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
		consumer_t();
		void open(const std::string& path);
		static std::string detect_charset(const std::string& path);
		document_t& get_document() {
			return document;
		}
		const std::string& get_charset() const {
			return charset;
		}
	private:
		document_t document;
		std::string charset;
	};

	class producer_t
	{
	public:
		typedef consumer_t::char_t char_t;
		producer_t();
		void save(const std::string& path, std::unique_ptr<consumer_t>& consumer, std::string charset = "");
	};
}
}