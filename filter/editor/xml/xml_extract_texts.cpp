#include "filter_pch.h"
#include "editor/xml/xml_extract_texts.h"
#include "editor/xml/xml_find_and_replace_strategy.h"
#include "locale/charset_encoder.h"
#include "traits/xml_traits.h"

namespace filter
{
namespace xml
{
	extract_texts_t::extract_texts_t(const std::string& text_tag, const std::string& para_tag)
		: text_tag(text_tag), para_tag(para_tag), ruler(&find_and_replace_strategy_t::do_nothing), shared_strings(nullptr), has_shared(false)
	{
		section.resize(1); // IMPORTANT!
	}

	extract_texts_t::extract_texts_t(const std::string& text_tag, const std::string& para_tag, std::map<uint32_t, std::string>* shared)
		: text_tag(text_tag), para_tag(para_tag), ruler(&find_and_replace_strategy_t::do_nothing), shared_strings(shared), has_shared(false)
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

	bool extract_texts_t::for_each(pugi::xml_node& node)
	{	
		end_paragraph_element();
		auto name = std::string(node.name());
		if (name == text_tag)
		{
			if (shared_strings && has_shared)
			{
				uint32_t id = std::atoi(node.first_child().value());
				auto texts = shared_strings->find(id);
				if (texts == shared_strings->end())
					throw std::runtime_error("invalid shared strings");

				section.back() += to_wchar(texts->second);
				section.back() += L"\n";
				ruler(texts->second, node, rules);
				return true;
			}

			std::string text(node.first_child().value());
			if (!text.empty())
			{
				if (lookup_break())
					section.emplace_back(std::move(to_wchar(text)));
				else
				{
					section.back() += to_wchar(text);
				}
				ruler(text, node, rules);
			}
		}
		else if (name == para_tag)
		{
			has_shared = false;
			if (shared_strings && !shared_strings->empty())
			{
				for (auto& attribute : node.attributes())
				{
					std::string name = attribute.name();
					std::string value = attribute.value();
					if (name == "t" && value == "s")
					{
						has_shared = true;
						return true;
					}
				}
			}

			if (para_nodes.find(depth()) != para_nodes.end())
				throw std::runtime_error("invalid para end");
			para_nodes.insert(std::make_pair(depth(), node));
		}
		return true;
	}

	void extract_texts_t::end_paragraph_element()
	{
		if (!para_nodes.empty())
		{
			auto cur = depth();
			//auto upper = para_nodes.upper_bound(cur);
			auto upper = para_nodes.find(cur);
			if (upper == para_nodes.end())
				upper = para_nodes.find(cur);
			while (upper != para_nodes.end())
			{
				if (upper != para_nodes.end())
				{
					upper = para_nodes.erase(upper);
					section.back().push_back(L'\n'); // TODO: normalize
				}
				else
					++upper;
			}
		}
	}

	bool extract_texts_t::lookup_break() const
	{
		if (!section.empty() && !section.back().empty())
		{
			return section.back().back() == L'\n'; // TODO: normalize
		}
		return false;
	}
}
}