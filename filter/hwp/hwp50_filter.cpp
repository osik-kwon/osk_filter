#include "filter_pch.h"
#include "hwp/hwp50_filter.h"
#include "locale/charset_encoder.h"
#include "io/compound_file_binary.h"
#include "io/zlib.h"

namespace filter
{
namespace hwp50
{
	class search_texts_t
	{
	public:
		typedef editor_traits::para_t para_t;
		typedef editor_traits::section_t section_t;
		typedef editor_traits::rule_t rule_t;
		typedef editor_traits::rule_string rule_string;
		typedef std::pair< std::reference_wrapper<record_t>, section_t > result_t;

		search_texts_t(const rule_t& pattern, char16_t replacement);
		section_t results_to_section() const;
		void search(rule_string& texts, std::reference_wrapper<record_t>& record);
		bool empty() const {
			return results.empty();
		}
		const rule_t& get_pattern() const {
			return pattern;
		}
		char16_t get_replacement() const {
			return replacement;
		}
		const std::vector< result_t >& get_results() const {
			return results;
		}
	private:
		std::vector< result_t > results;
		rule_t pattern;
		char16_t replacement;
	};

	search_texts_t::search_texts_t(const rule_t& pattern, char16_t replacement)
		: pattern(pattern), replacement(replacement)
	{}

	void search_texts_t::search(rule_string& texts, std::reference_wrapper<record_t>& record)
	{
		std::match_results<para_t::iterator> match;

		section_t lists;
		auto begin = texts.begin();
		while (std::regex_search(begin, texts.end(), match, pattern))
		{
			begin += match.position() + match.length();
			lists.push_back(match.str());
		}
		results.emplace_back(std::make_pair(record, lists));
	}

	search_texts_t::section_t search_texts_t::results_to_section() const
	{
		section_t section;
		for (auto& result : results)
		{
			section.reserve(section.size() + result.second.size());
			std::copy(result.second.begin(), result.second.end(), std::back_inserter(section));
		}
		return section;
	}

	class replace_texts_t
	{
	public:
		typedef editor_traits::para_t para_t;
		typedef editor_traits::section_t section_t;
		typedef editor_traits::rule_t rule_t;
		typedef editor_traits::rule_string rule_string;
		replace_texts_t();
		static void replace(rule_string& texts, std::reference_wrapper<record_t>& record, const rule_t& pattern, char16_t replacement);
	};

	replace_texts_t::replace_texts_t()
	{}

	void replace_texts_t::replace(rule_string& texts, std::reference_wrapper<record_t>& record, const rule_t& pattern, char16_t replacement)
	{
		std::match_results<para_t::iterator> results;
		auto begin = texts.begin();
		while (std::regex_search(begin, texts.end(), results, pattern))
		{
			for (auto i = results[0].first; i != results[0].second; ++i)
			{
				*i = replacement;
			}
			begin += results.position() + results.length();
		}
	}

	struct find_and_replace_strategy_t
	{
		typedef editor_traits::rule_string rule_string;
		static void do_nothing(rule_string& texts, std::reference_wrapper<record_t>& record, std::vector<search_texts_t>& rules);
		static void find_only(rule_string& texts, std::reference_wrapper<record_t>& record, std::vector<search_texts_t>& rules);
		static void find_and_replace(rule_string& texts, std::reference_wrapper<record_t>& record, std::vector<search_texts_t>& rules);
	};

	void find_and_replace_strategy_t::do_nothing(rule_string& texts, std::reference_wrapper<record_t>& record, std::vector<search_texts_t>& rules)
	{}

	void find_and_replace_strategy_t::find_only(rule_string& texts, std::reference_wrapper<record_t>& record, std::vector<search_texts_t>& rules)
	{
		for (auto& rule : rules)
			rule.search(texts, record);
	}

	void find_and_replace_strategy_t::find_and_replace(rule_string& texts, std::reference_wrapper<record_t>& record, std::vector<search_texts_t>& rules)
	{
		for (auto& rule : rules)
		{
			rule.search(texts, record);
			if (!rule.empty())
			{
				replace_texts_t::replace(texts, record, rule.get_pattern(), rule.get_replacement());
			}
		}
	}

	class editor_t;
	class extract_texts_t
	{
	public:
		friend class editor_t;
		typedef editor_traits::para_t para_t;
		typedef editor_traits::section_t section_t;
		typedef editor_traits::rule_t rule_t;
		typedef editor_traits::rule_string rule_string;
		typedef std::vector<std::reference_wrapper<record_t>> record_refs_t;
		typedef std::function<void(rule_string&, std::reference_wrapper<record_t>&, std::vector<search_texts_t>&)> ruler_t;
		extract_texts_t();
		~extract_texts_t();
		extract_texts_t& make_rule(const rule_t& pattern, char16_t replacement = u'*');
		void change_rule(ruler_t that);
		const std::vector<search_texts_t>& get_rules() const {
			return rules;
		}
		void run(record_refs_t& record_refs);
	private:
		section_t section;
		std::vector<search_texts_t> rules;
		ruler_t ruler;
	};

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
			for (auto contol = para_texts.controls.begin(); contol != para_texts.controls.end(); ++contol)
			{
				if (contol->type == para_text_t::is_char_control)
				{
					rule_string texts;
					for (auto code : contol->body)
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

					contol->body.clear();
					std::copy(texts.begin(), texts.end(), std::back_inserter(contol->body));
				}
				else if (contol->type == para_text_t::is_inline_control)
				{
					if(!contol->body.empty() && syntax_t::is_tab(contol->body[0]))
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

	editor_t::editor_t() : strategy(nullptr), replacement(0)
	{}

	editor_t& editor_t::extract(records_t&& that)
	{
		records = std::move(that);
		for (auto& record : records)
		{
			if (syntax_t::is_para_text(record.header.tag))
				para_text_record_refs.push_back(record);
		}
		return *this;
	}

	editor_t& editor_t::find(const rules_t& that)
	{
		rules = that;
		return *this;
	}

	editor_t& editor_t::replace(char16_t that)
	{
		replacement = that;
		return *this;
	}

	void editor_t::write_records(std::unique_ptr<consumer_t>& consumer, std::unique_ptr<buffer_t>& src)
	{
		auto write_size = std::accumulate(records.begin(), records.end(), 0, [](size_t size, auto& record) {
			return size + record.size(); });
		buffer_t dest;
		dest.resize(write_size);
		bufferstream write_records_stream(&dest[0], dest.size());
		consumer->write_records(write_records_stream, records);
		std::swap(*src.get(), dest);
	}

	editor_t& editor_t::finalize(std::unique_ptr<consumer_t>& consumer, std::unique_ptr<buffer_t>& src)
	{
		try
		{
			if (src->empty())
				return *this;
			strategy = std::make_unique<extract_texts_t>();
			for (auto& rule : rules)
			{
				strategy->make_rule(rule);
			}
			if (!rules.empty())
				strategy->change_rule(find_and_replace_strategy_t::find_only);
			if (replacement != 0)
				strategy->change_rule(find_and_replace_strategy_t::find_and_replace);

			strategy->run(para_text_record_refs);
			write_records(consumer, src);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return *this;
	}

	editor_t::section_t editor_t::get_extract_result() {
		if (strategy)
			return strategy->section;
		return section_t();
	}

	editor_t::sections_t editor_t::get_find_result()
	{
		if (strategy && !strategy->get_rules().empty())
		{
			sections_t sections;
			auto& rules = strategy->get_rules();
			for (auto& rule : rules)
				sections.emplace_back(rule.results_to_section());
			return sections;
		}
		return sections_t();
	}

	std::unique_ptr<consumer_t> filter_t::open(const std::string& path)
	{
		try
		{
			std::unique_ptr<consumer_t> consumer = std::make_unique<consumer_t>();
			consumer->open(path);
			return consumer;

		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return std::make_unique<consumer_t>();
	}

	void filter_t::save(const std::string& path, std::unique_ptr<consumer_t>& consumer)
	{
		try
		{
			producer_t producer;
			producer.save(path, consumer);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	filter_t::sections_t filter_t::extract_all_texts(std::unique_ptr<consumer_t>& consumer)
	{
		try
		{
			sections_t sections;
			auto& entries = consumer->get_streams();
			for (auto entry = entries.begin(); entry != entries.end(); ++entry)
			{
				if (!consumer->has_paragraph(entry->first))
					continue;
				auto& section = entry->second;
				bufferstream stream(&section->at(0), section->size());
				editor_t editor;
				editor.extract(consumer->read_records(stream))
					.finalize(consumer, section);
				sections.emplace_back(editor.get_extract_result());
			}
			return sections;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return sections_t();
	}

	void filter_t::replace_privacy(const rules_t& rules, char16_t replacement, std::unique_ptr<consumer_t>& consumer)
	{
		try
		{
			auto& entries = consumer->get_streams();
			for (auto entry = entries.begin(); entry != entries.end(); ++entry)
			{
				if (!consumer->has_paragraph(entry->first))
					continue;
				auto& section = entry->second;
				bufferstream stream(&section->at(0), section->size());
				editor_t editor;
				editor.extract( consumer->read_records(stream) )
					.find(rules)
					.replace(replacement)
					.finalize(consumer, section);
			}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}
}
}