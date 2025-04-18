#pragma once
#include <string>
#include <functional>
#include <regex>
#include "traits/binary_traits.h"
#include "traits/compound_file_binary_traits.h"
#include "traits/editor_traits.h"
#include "word/doc_syntax.h"

namespace filter
{
namespace doc
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
		typedef editor_traits::para_t para_t;
		typedef editor_traits::section_t section_t;
		typedef editor_traits::sections_t sections_t;
		typedef editor_traits::rule_t rule_t;
		typedef editor_traits::rules_t rules_t;

		filter_t() = default;

		std::unique_ptr<consumer_t> open(const std::string& path);
		void save(const std::string& path, std::unique_ptr<consumer_t>& consumer);
	private:
	};
}
}