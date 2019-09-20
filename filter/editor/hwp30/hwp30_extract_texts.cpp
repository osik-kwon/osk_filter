#include "filter_pch.h"
#include "editor/hwp30/hwp30_extract_texts.h"
#include "editor/hwp30/hwp30_find_and_replace_strategy.h"
#include "locale/charset_encoder.h"

namespace filter
{
namespace hwp30
{
	void extractor_t::extract_texts(const para_list_ref_t& para_list_ref, section_t& para_texts)
	{
		for (auto& para_ref : para_list_ref)
		{
			para_t para_text;
			for (auto code : para_ref)
			{
				auto utf16 = to_utf16(code.get().utf32);
				if (utf16.size() == 1)
				{
					if (utf16[0] == syntax_t::para_break)
						para_text.push_back(L'\n'); // TODO: normalize
					else if (utf16[0] == syntax_t::tab)
						para_text.push_back(L'\t'); // TODO: normalize
					else if (utf16[0] == syntax_t::hypen)
						para_text.push_back(L'-'); // TODO: normalize
					else if (utf16[0] == syntax_t::hypen)
						para_text.push_back(L' '); // TODO: normalize
					else if (utf16[0] == syntax_t::fixed_space)
						para_text.push_back(L' '); // TODO: normalize
					else
						para_text.push_back(utf16[0]);
				}
			}
			if (!para_text.empty())
			{
				if (para_text.back() != L'\n') // TODO: normalize
					para_text.push_back(L'\n'); // TODO: normalize
				para_texts.push_back(std::move(para_text));
			}
		}
	}

	void extractor_t::extract_text_control_ref(const std::unique_ptr<control_code_t>& control, para_ref_t& para_ref)
	{
		switch (control->get_code())
		{
		case syntax_t::tab:
		{
			tab_control_t* tab = dynamic_cast<tab_control_t*>(control.get());
			if (tab)
				para_ref.push_back(dynamic_cast<hchar_t&>(tab->code));
		}
		break;
		case syntax_t::hypen:
		{
			hypen_t* hypen = dynamic_cast<hypen_t*>(control.get());
			if (hypen)
				para_ref.push_back(dynamic_cast<hchar_t&>(hypen->code));
		}
		break;
		case syntax_t::blank:
		{
			blank_t* blank = dynamic_cast<blank_t*>(control.get());
			if (blank)
				para_ref.push_back(dynamic_cast<hchar_t&>(blank->code));
		}
		break;
		case syntax_t::fixed_space:
		{
			fixed_space_t* fixed_space = dynamic_cast<fixed_space_t*>(control.get());
			if (fixed_space)
				para_ref.push_back(dynamic_cast<hchar_t&>(fixed_space->code));
		}
		break;
		default:
		{
			hchar_t* code = dynamic_cast<hchar_t*>(control.get());
			if (code)
				para_ref.push_back(dynamic_cast<hchar_t&>(*control.get()));
		}
		break;
		}
	}

	void extractor_t::extract_drawing_object_control_ref(const std::unique_ptr<control_code_t>& control, para_ref_t& para_ref, para_list_ref_t& para_list_ref)
	{
		if (control->get_code() != syntax_t::drawing_object)
			throw std::runtime_error("invalid syntax : drawing object should follow picture");

		drawing_object_t* drawing_object = dynamic_cast<drawing_object_t*>(control.get());
		if (!drawing_object)
			throw std::runtime_error("invalid syntax : drawing object should follow picture");

		if (!para_ref.empty())
			para_list_ref.push_back(std::move(para_ref));
		para_ref = para_ref_t();

		for (auto& object : drawing_object->drawing_object.objects)
		{
			para_ref_t control_para_ref;
			if (object->get_para_lists())
			{
				auto& para_lists = *object->get_para_lists();
				for (auto& para_list : para_lists)
					extract_para_list_ref(para_list, para_list_ref);
			}
			if (!control_para_ref.empty())
				para_list_ref.push_back(std::move(control_para_ref));
		}
	}

	void extractor_t::extract_caption_ref(const std::unique_ptr<control_code_t>& control, para_ref_t& para_ref, para_list_ref_t& para_list_ref)
	{
		para_ref_t caption_para_ref;
		if (control->get_caption())
			extract_para_list_ref(*control->get_caption(), para_list_ref);
		if (!caption_para_ref.empty())
			para_list_ref.push_back(std::move(caption_para_ref));
	}

	void extractor_t::extract_para_list_ref(const std::unique_ptr<control_code_t>& control,
		para_ref_t& para_ref, para_list_ref_t& para_list_ref)
	{
		if (!para_ref.empty())
			para_list_ref.push_back(std::move(para_ref));
		para_ref = para_ref_t();

		para_ref_t control_para_ref;
		if (control->get_para_lists())
		{
			auto& para_lists = *control->get_para_lists();
			for (auto& para_list : para_lists)
				extract_para_list_ref(para_list, para_list_ref);
		}
		if (!control_para_ref.empty())
			para_list_ref.push_back(std::move(control_para_ref));
	}

	void extractor_t::extract_control_ref(const std::unique_ptr<control_code_t>& control, para_ref_t& para_ref, para_list_ref_t& para_list_ref)
	{
		if (!control->is_control_code())
			extract_text_control_ref(control, para_ref);
		else if (control->has_drawing_object())
			extract_drawing_object_control_ref(control, para_ref, para_list_ref);
		else if (control->has_para_list())
			extract_para_list_ref(control, para_ref, para_list_ref);

		if (control->has_caption())
			extract_caption_ref(control, para_ref, para_list_ref);
	}

	void extractor_t::extract_para_list_ref(const paragraph_list_t& section, para_list_ref_t& para_list_ref)
	{
		for (auto& para : section.para_list)
		{
			para_ref_t para_ref;
			for (auto& control : para.controls)
			{
				extract_control_ref(control, para_ref, para_list_ref);
			}

			if (!para_ref.empty())
				para_list_ref.push_back(std::move(para_ref));
		}
	}
	
	extract_texts_t::extract_texts_t() : ruler(&find_and_replace_strategy_t::do_nothing)
	{
		section.resize(1); // IMPORTANT!
	}

	extract_texts_t::~extract_texts_t()
	{}

	extract_texts_t& extract_texts_t::make_rule(const rule_t& pattern, char16_t replacement)
	{
		rules.emplace_back(search_texts_t(pattern, replacement));
		return *this;
	}

	void extract_texts_t::change_rule(ruler_t that) {
		ruler = that;
	}

	void extract_texts_t::run(para_list_ref_t& para_list_ref)
	{
		for (auto& para_ref : para_list_ref)
		{
			rule_string para_text;
			for (auto code : para_ref)
			{
				auto utf16 = to_utf16(code.get().utf32);
				if (utf16.size() == 1)
				{
					if (utf16[0] == syntax_t::para_break)
						para_text.push_back(L'\n'); // TODO: normalize
					else if (utf16[0] == syntax_t::tab)
						para_text.push_back(L'\t'); // TODO: normalize
					else if (utf16[0] == syntax_t::hypen)
						para_text.push_back(L'-'); // TODO: normalize
					else if (utf16[0] == syntax_t::hypen)
						para_text.push_back(L' '); // TODO: normalize
					else if (utf16[0] == syntax_t::fixed_space)
						para_text.push_back(L' '); // TODO: normalize
					else
						para_text.push_back(utf16[0]);
				}
			}
			ruler(para_text, para_ref, rules);
		}
		extractor_t::extract_texts(para_list_ref, section);
	}
}
}