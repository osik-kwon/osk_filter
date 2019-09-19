#pragma once
#include <string>
#include <memory>
#include "hwp/hwp30_syntax.h"
#include "traits/editor_traits.h"
#include "traits/binary_traits.h"

namespace filter
{
namespace hwp30
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
		typedef std::vector< std::reference_wrapper<hchar_t> > para_ref_t;
		typedef std::vector<para_ref_t> para_list_ref_t;
		editor_t();
		editor_t& extract(std::unique_ptr<consumer_t>& consumer);
		editor_t& find(const rules_t& rules);
		editor_t& replace(char16_t replacement = u'*');
		editor_t& finalize();

		section_t get_extract_result();
		sections_t get_find_result();
	private:
		std::unique_ptr<extract_texts_t> strategy;
		rules_t rules;
		char16_t replacement;
		para_list_ref_t para_list_ref;
	};
}
}