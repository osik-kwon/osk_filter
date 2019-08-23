#pragma once
#include <string>
#include <vector>
#include <memory>
#include "define/binary_traits.h"

namespace filter
{
namespace hwp30
{
	struct document_t;
	class filter_t
	{
	public:
		typedef binary_traits::byte_t byte_t;
		typedef binary_traits::buffer_t buffer_t;
		typedef binary_traits::bufferstream bufferstream;
		typedef binary_traits::streamsize streamsize;
		typedef std::wstring para_t;
		typedef std::vector<para_t> section_t;
		typedef std::vector<section_t> sections_t;
		filter_t() = default;

		sections_t extract_all_texts(const std::string& import_path);
		void save(const std::string& open_path, const std::string& save_path);
	private:
		buffer_t read_file(const std::string& path);
		std::unique_ptr<document_t> parse(buffer_t& buffer);
		void parse_header(bufferstream& stream, std::unique_ptr<document_t>& document);
		buffer_t extract_body(buffer_t& buffer, bufferstream& stream, std::unique_ptr<document_t>& document);
		void parse_body(bufferstream& stream, std::unique_ptr<document_t>& document);
	};
}
}