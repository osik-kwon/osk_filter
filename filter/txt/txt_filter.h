#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "traits/editor_traits.h"

namespace filter
{
namespace txt
{
	class consumer_t
	{
	public:
		typedef std::string para_t;
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
		producer_t();
		void save(const std::string& path, std::unique_ptr<consumer_t>& consumer, std::string charset = "");
	};

	class filter_t
	{
	public:
		typedef editor_traits::para_t para_t;
		typedef editor_traits::section_t section_t;
		typedef editor_traits::sections_t sections_t;
		typedef editor_traits::rule_t rule_t;
		typedef editor_traits::rules_t rules_t;

		filter_t();
		std::string detect_charset(const std::string& path);
		std::unique_ptr<consumer_t> open(const std::string& path);
		void save(const std::string& path, std::unique_ptr<consumer_t>& consumer, std::string dest_charset = "");
		sections_t extract_all_texts(std::unique_ptr<consumer_t>& consumer);
		sections_t search_privacy(const rules_t& rules, std::unique_ptr<consumer_t>& consumer);
		void replace_privacy(const rules_t& rules, char16_t replacement, std::unique_ptr<consumer_t>& consumer);
	private:
	};
}
}