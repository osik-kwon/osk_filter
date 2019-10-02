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

namespace filter
{
	class linear_match_t
	{
	public:
		linear_match_t(const std::string rule, std::size_t begin, std::size_t end);
		bool match(const std::string& data) const;
	private:
		const std::regex matcher;
		const std::size_t begin;
		const std::size_t end;
	};

	linear_match_t::linear_match_t(const std::string rule, std::size_t begin, std::size_t end) :
		matcher(rule), begin(begin), end(end)
	{}

	bool linear_match_t::match(const std::string& data) const
	{
		if (data.size() < (begin + end))
			throw std::runtime_error("buffer overrun error");
		return std::regex_match(data.begin(), data.begin() + end, matcher);
	}

	class xml_match_t
	{
	public:
		xml_match_t(){}
	private:
	};

	class signature_t
	{
	public:
		signature_t(){}
	private:
		std::string read(const std::string& path, size_t minimal_length);
	};

	std::string signature_t::read(const std::string& path, size_t minimal_length)
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
		using namespace ::xml;
		std::ifstream file(to_fstream_path(path), std::ios::binary);
		file.exceptions(std::ifstream::badbit | std::ifstream::failbit);
		try
		{
			//::xml::parser parser(file, path);
			//parser.next_expect(::xml::parser::start_element, "HWPML", ::xml::content::complex);

			parser p(file,
				path,
				parser::receive_default |
				parser::receive_attributes_event |
				parser::receive_namespace_decls);

			serializer s(std::cout, "out", 0);

			for (parser::event_type e(p.next()); e != parser::eof; e = p.next())
			{
				switch (e)
				{
				case parser::start_element:
				{
					s.start_element(p.qname());
					break;
				}
				case parser::end_element:
				{
					s.end_element();
					break;
				}
				case parser::start_namespace_decl:
				{
					s.namespace_decl(p.namespace_(), p.prefix());
					break;
				}
				case parser::end_namespace_decl:
				{
					// There is nothing in XML that indicates the end of namespace
					// declaration since it is scope-based.
					//
					break;
				}
				case parser::start_attribute:
				{
					s.start_attribute(p.qname());
					break;
				}
				case parser::end_attribute:
				{
					s.end_attribute();
					break;
				}
				case parser::characters:
				{
					s.characters(p.value());
					break;
				}
				case parser::eof:
				{
					// Handled in the for loop.
					//
					break;
				}
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
		filter::test_xml("d:/signature/hml.hml");
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