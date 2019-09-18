#pragma once
#include <string>
#include <memory>
#include "hwp/hwp50_syntax.h"
#include "traits/editor_traits.h"
#include "traits/binary_traits.h"

namespace filter
{
namespace hwp50
{
	class extract_texts_t;
	class editor_t
	{
	public:
		typedef editor_traits::para_t para_t;
		typedef editor_traits::section_t section_t;
		typedef editor_traits::sections_t sections_t;
		typedef editor_traits::rule_t rule_t;
		typedef editor_traits::rules_t rules_t;
		typedef binary_traits::byte_t byte_t;
		typedef binary_traits::buffer_t buffer_t;
		typedef binary_traits::bufferstream bufferstream;
		typedef binary_traits::streamsize streamsize;
		typedef std::vector<record_t> records_t;
		typedef std::vector<std::reference_wrapper<record_t>> record_refs_t;

		editor_t();
		editor_t& extract(records_t&& that);
		editor_t& find(const rules_t& rules);
		editor_t& replace(char16_t replacement = u'*');
		editor_t& finalize(std::unique_ptr<consumer_t>& consumer, std::unique_ptr<buffer_t>& src);

		section_t get_extract_result();
		sections_t get_find_result();
	private:
		void write_records(std::unique_ptr<consumer_t>& consumer, std::unique_ptr<buffer_t>& src);
		std::unique_ptr<extract_texts_t> strategy;
		rules_t rules;
		char16_t replacement;
		records_t records;
		record_refs_t para_text_record_refs;
	};
}
}