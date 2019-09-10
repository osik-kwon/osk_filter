#pragma once
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include "traits/xml_traits.h"

namespace filter
{
namespace hml
{
	class filter_t
	{
	public:
		typedef xml_traits::xml_document_t xml_document_t;
		typedef xml_traits::path_t path_t;

		typedef std::wstring para_t;
		typedef std::vector<para_t> section_t;
		typedef std::vector<section_t> sections_t;
		filter_t();
		~filter_t();

		std::unique_ptr<xml_document_t> open(const std::string& path);
		void save(const std::string& path, std::unique_ptr<xml_document_t>& document);

		sections_t extract_all_texts(std::unique_ptr<xml_document_t>& document);
		void replace_privacy(const std::wregex& pattern, char16_t replace_dest, std::unique_ptr<xml_document_t>& document);
	private:
	};
}
}