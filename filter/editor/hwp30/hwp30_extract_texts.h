#pragma once
#include <functional>
#include <map>
#include "traits/editor_traits.h"
#include "editor/hwp30/hwp30_search_texts.h"

namespace filter
{
namespace hwp30
{
	class extractor_t
	{
	public:
		typedef editor_traits::para_t para_t;
		typedef editor_traits::section_t section_t;
		typedef editor_traits::rule_t rule_t;
		typedef editor_traits::rule_string rule_string;
		typedef std::vector< std::reference_wrapper<hchar_t> > para_ref_t;
		typedef std::vector<para_ref_t> para_list_ref_t;
		extractor_t() = default;
		static void extract_texts(const para_list_ref_t& para_list_ref, section_t& para_texts);
		static void extract_para_list_ref(const paragraph_list_t& section, para_list_ref_t& para_list_ref);
		static void extract_control_ref(const std::unique_ptr<control_code_t>& control,
			para_ref_t& para_ref, para_list_ref_t& para_list_ref);
	private:
		static void extract_drawing_object_control_ref(const std::unique_ptr<control_code_t>& control,
			para_ref_t& para_ref, para_list_ref_t& para_list_ref);
		static void extract_text_control_ref(const std::unique_ptr<control_code_t>& control, para_ref_t& para_ref);
		static void extract_caption_ref(const std::unique_ptr<control_code_t>& control,
			para_ref_t& para_ref, para_list_ref_t& para_list_ref);
		static void extract_para_list_ref(const std::unique_ptr<control_code_t>& control,
			para_ref_t& para_ref, para_list_ref_t& para_list_ref);
	};

	class editor_t;
	class extract_texts_t
	{
	public:
		friend class editor_t;
		typedef editor_traits::para_t para_t;
		typedef editor_traits::section_t section_t;
		typedef editor_traits::rule_t rule_t;
		typedef editor_traits::rule_string rule_string;
		typedef std::vector< std::reference_wrapper<hchar_t> > para_ref_t;
		typedef std::vector<para_ref_t> para_list_ref_t;
		typedef std::function<void(rule_string&, para_list_ref_t&, std::vector<search_texts_t>&)> ruler_t;
		extract_texts_t();
		~extract_texts_t();
		extract_texts_t& make_rule(const rule_t& pattern, char16_t replacement = u'*');
		void change_rule(ruler_t that);
		const std::vector<search_texts_t>& get_rules() const {
			return rules;
		}
		void run(para_list_ref_t& para_list_ref);
	private:
		section_t section;
		std::vector<search_texts_t> rules;
		ruler_t ruler;
	};
}
}