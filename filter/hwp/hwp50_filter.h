#pragma once
#include <string>
#include <functional>
#include <regex>
#include "traits/binary_traits.h"
#include "traits/compound_file_binary_traits.h"
#include "hwp/hwp50_syntax.h"

namespace filter
{
namespace hwp50
{
	class filter_t
	{
	public:
		typedef binary_traits::byte_t byte_t;
		typedef binary_traits::buffer_t buffer_t;
		typedef binary_traits::bufferstream bufferstream;
		typedef binary_traits::streamsize streamsize;
		typedef cfb_traits::storage_t storage_t;
		typedef cfb_traits::stream_t stream_t;
		typedef std::wstring para_t;
		typedef std::vector<para_t> section_t;
		typedef std::vector<section_t> sections_t;
		filter_t() = default;

		bool replace_privacy(const std::string& import_path, const std::string& export_path, const std::wregex& pattern, char16_t replace_dest);
		sections_t extract_all_texts(const std::string& import_path);
		bool decompress_save(const std::string& import_path, const std::string& export_path);
	private:
		file_header_t read_file_header(std::unique_ptr<storage_t>& storage);
		void write_file_header(std::unique_ptr<storage_t>& storage, const file_header_t& file_header);
		std::vector<record_t> read_records(bufferstream& stream);
		void write_records(bufferstream& stream, const std::vector<record_t>& records);

		para_t extract_para_text(bufferstream& stream, streamsize size);
		section_t extract_section_text(std::vector<record_t>& records);
	};
}
}