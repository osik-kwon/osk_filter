#pragma once
#include <string>
#include <vector>
#include <memory>
#include "traits/binary_traits.h"
#include "traits/editor_traits.h"
#include "hwp/hwp30_syntax.h"

namespace filter
{
namespace hwp30
{
	class filter_t
	{
	public:
		typedef binary_traits::byte_t byte_t;
		typedef binary_traits::buffer_t buffer_t;
		typedef binary_traits::bufferstream bufferstream;
		typedef binary_traits::streamsize streamsize;
		typedef editor_traits::para_t para_t;
		typedef editor_traits::section_t section_t;
		typedef editor_traits::sections_t sections_t;
		typedef editor_traits::rule_t rule_t;
		typedef editor_traits::rules_t rules_t;
		filter_t() = default;

		sections_t extract_all_texts(const std::string& import_path);
		std::unique_ptr<document_t> open(const std::string& open_path);
		bool save(const std::unique_ptr<document_t>& document, const std::string& save_path);
	private:
		buffer_t read_file(const std::string& path);
		std::unique_ptr<document_t> parse(buffer_t& buffer);
		buffer_t extract_body(buffer_t& buffer, bufferstream& stream, std::unique_ptr<document_t>& document);
	};
}
}