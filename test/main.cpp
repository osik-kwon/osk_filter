#include <filesystem>
#include <iostream>
#include <locale>

#include "hwp/hwp50_filter.h"
#include "hwp/hwp30_filter.h"
#include "hwp/hwpx_filter.h"
#include "hwp/hwpml_filter.h"
#include "txt/txt_filter.h"
#include "locale/charset_encoder.h"
#include "traits/editor_traits.h"

std::wostream& operator << (std::wostream& stream, const filter::editor_traits::sections_t& sections)
{
	stream.imbue(std::locale(""));
	for (auto& section : sections)
	{
		for (auto& para : section)
		{
			if (!para.empty() && para.size() != 1)
			{
				stream << para;
				if (stream.bad())
					stream.clear();
			}
		}
	}
	return stream;
}

class privacy_rules_t
{
public:
	typedef filter::editor_traits::rule_t rule_t;
	typedef filter::editor_traits::rules_t rules_t;
	privacy_rules_t() = default;
	static rules_t make_default()
	{
		rules_t rules;
		std::wregex resident_registration_number(L"(?:[0-9]{2}(?:0[1-9]|1[0-2])(?:0[1-9]|[1,2][0-9]|3[0,1]))-[1-4][0-9]{6}");
		rules.emplace_back(std::move(resident_registration_number));
		return rules;
	}

	static char16_t make_replacement(){
		return u'*';
	}
};

namespace filesystem = std::experimental::filesystem;

template <class filter_t>
class format_tester_t
{
public:
	typedef filter::editor_traits::sections_t sections_t;
	typedef filter::editor_traits::para_t para_t;
	typedef filter::editor_traits::rule_t rule_t;
	typedef filter::editor_traits::rules_t rules_t;

	format_tester_t(const std::string& open, const std::string& save) : open_extension(open), save_extension(save)
	{
		std::cout << "********************" << open_extension << " format test ********************" << std::endl;
		rules = privacy_rules_t::make_default();
		replacement = privacy_rules_t::make_replacement();
	}

	format_tester_t& build_open_root(const std::wstring& path)
	{
		open_root = to_utf8(path);
		return *this;
	}

	format_tester_t& build_save_root(const std::wstring& path)
	{
		save_root = to_utf8(path);
		return *this;
	}

	void open_and_save(const std::wstring& open_path) {
		open_and_save(to_utf8(open_path));
	}

	void open_and_save(const std::string& open)
	{
		// TODO: duplication policy
		auto open_path = open_root + open;
		auto save_path = save_root + open + "." + save_extension + "." + open_extension;
		if(!filesystem::exists(save_root))
			filesystem::create_directory(save_root);

		std::cout << "==========" << open_extension <<" open/save test ==========" << std::endl;
		std::wcout << "open path is " << to_wchar(open_path) << std::endl;
		std::wcout << "save path is " << to_wchar(save_path) << std::endl;
		filter_t filter;
		//std::cout << "[0] open charset is " << filter.detect_charset(open_path) << std::endl;
		std::cout << "[1] open new document"<< std::endl;
		auto src = filter.open(open_path);
		std::cout << "[2] extract all texts from new document" << std::endl;
		std::wcout << filter.extract_all_texts(src);
		std::cout << "[3] save" << std::endl;
		filter.save(save_path, src);
		//std::cout << "[4] save charset is" << filter.detect_charset(save_path) << std::endl;
		std::cout << "[4] open saved document" << std::endl;
		auto dest = filter.open(save_path);
		std::cout << "[5] extract all texts from saved document" << std::endl;
		std::wcout << filter.extract_all_texts(dest);
	}

	void search_privacy(const std::wstring& open_path) {
		search_privacy(to_utf8(open_path));
	}

	void search_privacy(const std::string& open)
	{
		// TODO: duplication policy
		auto open_path = open_root + open;
		std::cout << "==========" << open_extension << " search privacy test ==========" << std::endl;
		std::wcout << "open path is " << to_wchar(open_path) << std::endl;

		filter_t filter;
		std::cout << "[1] open new document" << std::endl;
		auto src = filter.open(open_path);
		std::cout << "[2] extract all texts from new document" << std::endl;
		std::wcout << filter.extract_all_texts(src);
		std::cout << "[3] search privacy from new document" << std::endl;
		std::wcout << filter.search_privacy(rules, src);
	}

	void replace_privacy(const std::wstring& open_path) {
		replace_privacy(to_utf8(open_path));
	}

	void replace_privacy(const std::string& open)
	{
		// TODO: duplication policy
		auto open_path = open_root + open;
		auto save_path = save_root + open + "." + save_extension + "." + open_extension;
		if (!filesystem::exists(save_root))
			filesystem::create_directory(save_root);

		std::cout << "==========" << open_extension << " replace privacy test ==========" << std::endl;
		std::wcout << "open path is " << to_wchar(open_path) << std::endl;
		std::wcout << "save path is " << to_wchar(save_path) << std::endl;

		filter_t filter;
		std::cout << "[1] open new document" << std::endl;
		auto src = filter.open(open_path);
		std::cout << "[2] extract all texts from new document" << std::endl;
		std::wcout << filter.extract_all_texts(src);

		std::cout << "[3] replace privacy from new document" << std::endl;
		std::wcout << "replacement char is " << to_wchar(std::u16string{ replacement }) << std::endl;
		filter.replace_privacy(rules, replacement, src);
		std::cout << "[4] save" << std::endl;
		filter.save(save_path, src);
		std::cout << "[5] open saved document" << std::endl;
		auto dest = filter.open(save_path);
		std::cout << "[6] extract all texts from saved document" << std::endl;
		std::wcout << filter.extract_all_texts(dest);
	}
private:
	std::string open_extension;
	std::string save_extension;
	std::string open_root;
	std::string save_root;
	rules_t rules;
	char16_t replacement;
};


class console_tester_t
{
public:
	typedef filter::editor_traits::sections_t sections_t;
	typedef filter::editor_traits::para_t para_t;
	console_tester_t() = default;

	template <class format_tester>
	static void open_and_save(format_tester& tester, std::vector<std::wstring>&& pathes)
	{
		for (auto& path : pathes)
			tester.open_and_save(path);
	}

	template <class format_tester>
	static void search_and_replace_privacy(format_tester& tester, std::vector<std::wstring>&& pathes)
	{
		for (auto& path : pathes)
		{
			tester.search_privacy(path);
			tester.replace_privacy(path);
		}
	}
private:
};

void test_txt()
{
	format_tester_t<filter::txt::filter_t> tester("txt", "export");
	tester.build_open_root(L"./sample/txt/")
		.build_save_root(L"./sample/txt/save/");

	console_tester_t::open_and_save(tester, {
		L"empty.txt",
		L"ascii.txt",
		L"utf8.txt",
		L"utf8_bom.txt",
		L"utf8_LF.txt",
		L"utf8_CR.txt",
		L"utf8_LFCR.txt",
		L"utf16_le.txt",
		L"utf16_be.txt",
		L"utf32_le.txt",
		L"utf32_be.txt",
		L"euckr.txt",
		L"iso-8859-8.txt",
		L"euckr_한글경로.txt"
		});

	console_tester_t::search_and_replace_privacy(tester, {
		L"privacy.txt"
		});
}

void test_hwpml()
{
	format_tester_t<filter::hml::filter_t> tester("hml", "export");
	tester.build_open_root(L"./sample/hml/")
		.build_save_root(L"./sample/hml/save/");

	console_tester_t::open_and_save(tester, {
		L"hml.hml",
		L"hml_한글경로.hml"
		});

	console_tester_t::search_and_replace_privacy(tester, {
		L"privacy.hml"
		});
}

void test_hwpx()
{
	format_tester_t<filter::hwpx::filter_t> tester("hwpx", "export");
	tester.build_open_root(L"./sample/hwpx/")
		.build_save_root(L"./sample/hwpx/save/");

	console_tester_t::open_and_save(tester, {
		L"hwpx.hwpx",
		L"hwpx_한글경로.hwpx"
		});

	console_tester_t::search_and_replace_privacy(tester, {
		L"privacy.hwpx"
		});
}

void test_hwp30()
{
	format_tester_t<filter::hwp30::filter_t> tester("hwp", "export");
	tester.build_open_root(L"./sample/hwp30/")
		.build_save_root(L"./sample/hwp30/save/");

	console_tester_t::open_and_save(tester, {
		L"hwp30.hwp",
		L"hwp30_한글경로.hwp",
		L"hwp30_shape.hwp"
		});

	console_tester_t::search_and_replace_privacy(tester, {
		L"hwp30_privacy.hwp"
		});
}

void test_hwp50()
{
	format_tester_t<filter::hwp50::filter_t> tester("hwp", "export");
	tester.build_open_root(L"./sample/hwp50/")
		.build_save_root(L"./sample/hwp50/save/");

	console_tester_t::open_and_save(tester, {
		L"hwp50.hwp",
		L"hwp50_한글경로.hwp",
		L"hwp50_nocomp.hwp",
		L"hwp50_dist.hwp"
		});

	console_tester_t::search_and_replace_privacy(tester, {
		L"hwp50_privacy.hwp",
		L"hwp50_privacy_dist.hwp"
		});
}

#include <fstream>
#include <algorithm>
#include <io/binary_iostream.h>
#include <io/file_stream.h>

#include <traits/xml_traits.h>
#include <xml/parser>
#include <xml/serializer>

#include <functional>
#include <map>
#include <set>
#include <algorithm>

#include <trie/trie.h>

namespace filter
{
namespace signature
{
	class range_t
	{
	public:
		range_t();
		range_t(const std::string& buffer, std::size_t begin, std::size_t end);
		bool match(const std::string& rule) const;
		bool equal(const std::string& dest) const;
	private:
		std::size_t begin;
		std::size_t end;
		std::string data;
	};

	range_t::range_t() : begin(0), end(0)
	{}

	range_t::range_t(const std::string& buffer, std::size_t begin, std::size_t end) :
		begin(begin), end(end), data(buffer.begin() + begin, buffer.begin() + end)
	{}

	bool range_t::match(const std::string& rule) const
	{
		if (data.size() < (begin + end))
			throw std::runtime_error("buffer overrun error");
		std::regex matcher(rule);
		return std::regex_match(data.begin(), data.begin() + end, matcher);
	}

	bool range_t::equal(const std::string& dest) const
	{
		return data == dest;
	}


	struct element_t
	{
		element_t(std::unique_ptr<xml::parser>& parser) : parser(parser)
		{}
		bool equal(const std::string& dest) const
		{
			return parser->qname() == dest;
		}
		bool exist() const
		{
			return !parser->qname().empty();
		}
		std::unique_ptr<xml::parser>& parser;
	};

	struct attribute_t
	{
		attribute_t(std::unique_ptr<xml::parser>& parser, const std::string& name) : parser(parser), name(name)
		{}
		bool equal(const std::string& dest) const
		{
			auto& attribute_map = parser->attribute_map();
			if (attribute_map.find(name) == attribute_map.end())
				throw std::logic_error("attribute is not exist"); // TODO: custom exception
			return attribute_map.find(name)->second.value == dest;
		}
		bool exist() const
		{
			auto& attribute_map = parser->attribute_map();
			return attribute_map.find(name) != attribute_map.end();
		}
		std::unique_ptr<xml::parser>& parser;
		const std::string& name;
	};

	class sequence_t
	{
	public:
		sequence_t();
		sequence_t(const std::string& path, size_t nth_element);
		element_t element(const std::string& name);
		attribute_t attribute(const std::string& name);
	private:
		void load(const std::string& path);
		void visit(size_t nth_element);

		std::ifstream file;
		std::unique_ptr<xml::parser> parser;
	};

	sequence_t::sequence_t()
	{}

	sequence_t::sequence_t(const std::string& path, size_t nth_element)
	{
		load(path);
		visit(nth_element);
	}

	void sequence_t::load(const std::string& path)
	{
		file.exceptions(std::ifstream::badbit | std::ifstream::failbit);
		file.open(to_fstream_path(path), std::ios::binary);	
		parser = std::make_unique<xml::parser>(file, path);
	}

	void sequence_t::visit(size_t nth_element)
	{
		size_t id = 0;
		for (auto event(parser->next()); event != xml::parser::eof; event = parser->next())
		{
			if (event == xml::parser::start_element)
			{
				++id;
				if (id == nth_element)
					return;
			}
		}
	}

	element_t sequence_t::element(const std::string& name)
	{
		return element_t(parser);
	}

	attribute_t sequence_t::attribute(const std::string& name)
	{
		return attribute_t(parser, name);
	}

	class storage_t
	{
	public:
		storage_t(const std::string& path);
		std::string read_header(size_t minimal_length);
		range_t& range(size_t begin, size_t end);
		sequence_t& sequence(size_t nth_element);
		const std::string& get_header() const {
			return header;
		}
	private:
		std::string path;
		std::string header;

		range_t range_buffer;
		sequence_t sequence_buffer;
	};

	range_t& storage_t::range(size_t begin, size_t end)
	{
		range_buffer = range_t(header, begin, end);
		return range_buffer;
	}

	sequence_t& storage_t::sequence(size_t nth_element)
	{
		sequence_buffer = sequence_t(path, nth_element);
		return sequence_buffer;
	}

	storage_t::storage_t(const std::string& path) : path(path)
	{
		header = read_header(64);
	}

	std::string storage_t::read_header(size_t minimal_length)
	{
		std::ifstream file(to_fstream_path(path), std::ios::binary);
		file.exceptions(std::ifstream::badbit | std::ifstream::failbit);

		file.unsetf(std::ios::skipws);
		file.seekg(0, std::ios::end);
		std::streampos size = file.tellg();
		file.seekg(0, std::ios::beg);

		size_t file_size = (size_t)size;
		size_t buffer_size = 0;
		if (file_size > minimal_length)
			buffer_size = minimal_length;
		else
			buffer_size = file_size;

		std::string buffer;
		std::copy_n(std::istream_iterator<char>(file), buffer_size, std::back_inserter(buffer));
		return buffer;
	}

	class deterministic_classifier_t
	{
	public:
		typedef std::string name_t;
		typedef int trie_key_t;
		typedef trie_impl trie_t;
		typedef trie_impl::result_t trie_result_t;
		typedef std::function<bool(storage_t&)> algorithm_t;
		typedef std::map<trie_key_t, algorithm_t> algorithms_t;
		deterministic_classifier_t() : unique_key(0)
		{}

		// type interfaces
		void insert_type(trie_key_t trie_key, const name_t& name)
		{
			if (types.find(trie_key) != types.end())
				throw std::logic_error("has already registered format");
			types.insert({ trie_key, name });
		}

		bool exist_type(trie_key_t trie_key) const
		{
			return types.find(trie_key) != types.end();
		}

		name_t find_type(trie_key_t trie_key) const
		{
			if (types.find(trie_key) == types.end())
				throw std::logic_error("trie id is not exist");
			return types.find(trie_key)->second;
		}

		// trie interfaces
		trie_key_t get_unique_key() const {
			return unique_key;
		}

		bool exact_match(const std::string& key) const {
			return binary_classifiers.lookup(key) != -1;
		}

		trie_result_t longest_prefix(const std::string& key) const
		{
			return binary_classifiers.longest_prefix(key);
		}

		trie_key_t insert_key_at_trie(const std::string& key) {
			++unique_key;
			if(binary_classifiers.insert(key, unique_key) == -1)
				throw std::logic_error("trie insert fail");
			return unique_key;
		}

		// algorithm interfaces
		bool has_algorithms(const std::string& key) const {
			return classifiers.find(key) != classifiers.end();
		}

		const algorithms_t& get_algorithms(const std::string& key) const {
			if (classifiers.find(key) == classifiers.end())
				throw std::logic_error("algorithm is not exist");
			return classifiers.find(key)->second;
		}

		void insert_algorithm(trie_key_t id, const std::string& key, algorithm_t algorithm) {
			if (classifiers.find(key) == classifiers.end())
			{
				classifiers.insert({ key, algorithms_t{} });
			}
			classifiers[key].insert({ id, algorithm } );
		}
	private:
		trie_key_t unique_key;
		trie_t binary_classifiers;
		std::map<trie_key_t, name_t> types;
		std::map<std::string, algorithms_t > classifiers;
	};

	class nondeterministic_classifier_t
	{
	public:
		typedef std::string name_t;
		typedef std::function<bool(storage_t&)> algorithm_t;
		nondeterministic_classifier_t()
		{}

		// algorithm interfaces
		bool has_algorithm(const name_t& name) const {
			return classifiers.find(name) != classifiers.end();
		}

		algorithm_t get_algorithm(const name_t& name) const {
			if (classifiers.find(name) == classifiers.end())
				throw std::logic_error("algorithm is not exist");
			return classifiers.find(name)->second;
		}

		name_t classify_all(storage_t& storage)
		{
			for (auto& algorithm : classifiers)
			{
				try
				{
					if (algorithm.second(storage))
						return algorithm.first;
				}
				catch (const std::exception&)
				{}
			}
			return name_t();
		}

		void insert_algorithm(const name_t& name, algorithm_t algorithm) {
			if (classifiers.find(name) != classifiers.end())
				throw std::logic_error("has already inserted algorithm");
			classifiers.insert({ name, algorithm });
		}
	private:
		std::map< name_t, algorithm_t > classifiers;
	};

	class analyzer_t
	{
	public:
		typedef deterministic_classifier_t::name_t name_t;
		typedef deterministic_classifier_t::algorithm_t deterministic_algorithm_t;
		typedef nondeterministic_classifier_t::algorithm_t nondeterministic_algorithm_t;
		analyzer_t();
		void make_rules();
		name_t match(const std::string& path);
	private:
		analyzer_t& deterministic(const name_t& name, const std::string& key)
		{
			if (deterministic_classifiers.exact_match(key))
				throw std::logic_error("has already registered key in trie");
			auto trie_key = deterministic_classifiers.insert_key_at_trie(key);
			deterministic_classifiers.insert_type(trie_key, name);
			return *this;
		}

		analyzer_t& deterministic(const name_t& name, const std::string& key, deterministic_algorithm_t algorithm)
		{			
			deterministic(name, key);
			deterministic_classifiers.insert_algorithm(
				deterministic_classifiers.get_unique_key(), key, algorithm);
			return *this;
		}

		analyzer_t& nondeterministic(const name_t& name, nondeterministic_algorithm_t algorithm)
		{
			nondeterministic_classifiers.insert_algorithm(name, algorithm);
			return *this;
		}

		deterministic_classifier_t deterministic_classifiers;
		nondeterministic_classifier_t nondeterministic_classifiers;
	};

	analyzer_t::analyzer_t()
	{}

	void analyzer_t::make_rules()
	{
		deterministic("pdf", "\x25\x50\x44\x46");
		deterministic("PKZIP archive_1", "\x50\x4B\x03\x04");
		deterministic("hwp30", "HWP Document File", [](storage_t& storage) {
			return storage.range(0, 30).match("HWP Document File V[1-3]\\.[0-9]{2} \x1a\x1\x2\x3\x4\x5");
			});
		nondeterministic("hwpml", [](storage_t& storage) {
			return storage.sequence(1).element("HWPML").exist();
			});
	}

	analyzer_t::name_t analyzer_t::match(const std::string& path)
	{
		try
		{
			// TODO: zero byte
			storage_t storage(path);
			auto result = deterministic_classifiers.longest_prefix(storage.get_header());
			auto id = result.value();
			auto key = result.key();
			if (deterministic_classifiers.has_algorithms(key))
			{
				const auto& algorithms = deterministic_classifiers.get_algorithms(key);
				for (auto& algorithm : algorithms)
				{
					if (algorithm.second(storage))
						return deterministic_classifiers.find_type(algorithm.first);
				}
			}

			if (deterministic_classifiers.exist_type(id))
				return deterministic_classifiers.find_type(id);

			auto name = nondeterministic_classifiers.classify_all(storage);
			if (!name.empty())
				return name;

			// TODO: text

			return name_t();
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return name_t();
	}

	void test_trie()
	{
		filter::signature::analyzer_t analyzer;
		analyzer.make_rules();
		std::cout << analyzer.match("d:/signature/hml.hml") << std::endl;
		std::cout << analyzer.match("d:/signature/docx.docx") << std::endl;
		std::cout << analyzer.match("d:/signature/hwp30.hwp") << std::endl;
	}
}

	void test_binary(const std::string& path)
	{
		std::ifstream file(to_fstream_path(path), std::ios::binary);
		file.exceptions(std::ifstream::badbit | std::ifstream::failbit);

		file.unsetf(std::ios::skipws);
		file.seekg(0, std::ios::end);
		std::streampos size = file.tellg();
		file.seekg(0, std::ios::beg);

		// input params
		const size_t minimal_length = 64;
		size_t file_size = (size_t)size;
		size_t buffer_size = 0;
		if (file_size > minimal_length)
			buffer_size = minimal_length;
		else
			buffer_size = file_size;

		std::string buffer;
		std::copy_n(std::istream_iterator<char>(file), buffer_size, std::back_inserter(buffer));
		file.close();

		// input params
		const size_t begin = 0;
		const size_t end = 30;
		const std::string rule = "HWP Document File V[1-3]\\.[0-9]{2} \x1a\x1\x2\x3\x4\x5";

		std::string header(buffer.begin() + begin, buffer.begin() + end);
		std::regex matcher(rule);
		std::cout << std::regex_match(header, matcher);
	}

	void test_xml(const std::string& path)
	{
		std::ifstream file(to_fstream_path(path), std::ios::binary);
		file.exceptions(std::ifstream::badbit | std::ifstream::failbit);
		try
		{
			xml::parser parser(file, path);
			//p.next_expect(::xml::parser::start_element, "HWPML", ::xml::content::complex);
			
			for (auto event(parser.next()); event != xml::parser::eof; event = parser.next())
			{
				if (event == xml::parser::start_element)
				{
					std::cout << parser.qname() << std::endl;
					for (auto& attr : parser.attribute_map())
						std::cout << attr.first << " : " << attr.second.value << std::endl;
				}
			}
		}
		catch (const std::exception&)
		{
			std::cout << "mismatched" << std::endl;
			return;
		}
		std::cout << "matched" << std::endl;
	}
}


int main()
{
	try
	{
		filter::signature::test_trie();
		//filter::test_xml("d:/signature/hml.hml");
		//filter::test_binary("d:/signature/hwp30/hwp21.hwp");
		//test_txt();
		//test_hwpml();
		//test_hwpx();
		//test_hwp30();
		//test_hwp50();		
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
	return 0;
}