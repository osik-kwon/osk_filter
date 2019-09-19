#include "filter_pch.h"
#include "editor/hwp50/hwp50_extract_texts.h"
#include "editor/hwp50/hwp50_find_and_replace_strategy.h"

namespace filter
{
namespace hwp50
{
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

	void extract_texts_t::run(record_refs_t& record_refs)
	{
		for (auto record : record_refs)
		{
			bufferstream para_text_stream(&record.get().body[0], record.get().header.body_size);
			para_text_t para_texts(record.get().header.body_size);
			para_text_stream >> para_texts;
			for (auto control = para_texts.controls.begin(); control != para_texts.controls.end(); ++control)
			{
				if (control->type == para_text_t::is_char_control)
				{
					rule_string texts;
					for (auto code : control->body)
					{
						texts.push_back(static_cast<para_t::value_type>(code));
						if (syntax_t::is_carriage_return(code))
						{
							section.back().push_back(L'\n'); // TODO: normalize
							section.emplace_back(para_t());
						}
						else
						{
							section.back() += static_cast<para_t::value_type>(code);
						}
					}

					ruler(texts, record, rules);

					control->body.clear();
					std::copy(texts.begin(), texts.end(), std::back_inserter(control->body));
				}
				else if (control->type == para_text_t::is_inline_control)
				{
					if (!control->body.empty() && syntax_t::is_tab(control->body[0]))
						section.back().push_back(L'\t'); // TODO: normalize
				}
			}
			record.get().header.body_size = para_texts.size();
			buffer_t dest;
			dest.resize(record.get().header.body_size);
			bufferstream para_text_export_stream(&dest[0], dest.size());
			para_text_export_stream << para_texts;
			record.get().body = std::move(dest);
		}
	}
}
}