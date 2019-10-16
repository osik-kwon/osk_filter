#include <filesystem>
#include <iostream>
#include <locale>

// document filters
#include "hwp/hwp50_filter.h"
#include "hwp/hwp30_filter.h"
#include "hwp/hwpx_filter.h"
#include "hwp/hwpml_filter.h"
#include "txt/txt_filter.h"
#include "locale/charset_encoder.h"
#include "traits/editor_traits.h"

// signature analyzer
#include <signature/signature_analyzer.h>
#include <signature/signature_builder.h>

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
		// TODO: implement
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


// signatures examples

enum document_id_t : uint16_t
{
	txt = 0,
	pdf,
	PKZIP_archive_1,
	hwp30,
	hwp50,
	hwpx,
	hwpml
};

std::unique_ptr< filter::signature::analyzer_t<document_id_t> > build_enum_rules()
{
	using filter::signature::storage_t;
	auto make =	std::make_unique< filter::signature::analyzer_t<document_id_t> >(document_id_t::txt);

	make->deterministic(document_id_t::pdf, "\x25\x50\x44\x46");
	make->deterministic(document_id_t::PKZIP_archive_1, "\x50\x4B\x03\x04");
	make->deterministic(document_id_t::hwp30, "HWP Document File", [](storage_t& storage) {
		return storage.range(0, 30).match("HWP Document File V[1-3]\\.[0-9]{2} \x1a\x1\x2\x3\x4\x5");
		});
	make->deterministic(document_id_t::hwp50, "\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1", [](storage_t& storage) {
		return storage.compound("/FileHeader").range(0, 32).match("HWP Document File.*");
		});
	make->deterministic(document_id_t::hwpx, "\x50\x4B\x03\x04", [](storage_t& storage) {
		return storage.package("META-INF/container.xml").
			sequence(3).element("rootfile").attribute("media-type").equal("application/hwpml-package+xml");
		});

	make->nondeterministic(document_id_t::hwpml, [](storage_t& storage) {
		return storage.sequence(1).element("HWPML").exist();
		});
	return make;
}

void test_signature()
{
	{
		auto rules = filter::signature::builder_t::build_string_rules();
		std::cout << rules->scan("d:/signature/docx.docx") << std::endl;
		std::cout << rules->scan("d:/signature/hwpx.hwpx") << std::endl;
		std::cout << rules->scan("d:/signature/zero.hwp") << std::endl;
		std::cout << rules->scan("d:/signature/txt.txt") << std::endl;
		std::cout << rules->scan("d:/signature/pdf.pdf") << std::endl;
		std::cout << rules->scan("d:/signature/hwp50.hwp") << std::endl;
		std::cout << rules->scan("d:/signature/hml.hml") << std::endl;
		std::cout << rules->scan("d:/signature/hwp30.hwp") << std::endl;
	}
	{
		auto rules = build_enum_rules();
		std::cout << rules->scan("d:/signature/zero.hwp") << std::endl;
		std::cout << rules->scan("d:/signature/txt.txt") << std::endl;
		std::cout << rules->scan("d:/signature/pdf.pdf") << std::endl;
		std::cout << rules->scan("d:/signature/hwp50.hwp") << std::endl;
		std::cout << rules->scan("d:/signature/hwpx.hwpx") << std::endl;
		std::cout << rules->scan("d:/signature/hml.hml") << std::endl;
		std::cout << rules->scan("d:/signature/hwp30.hwp") << std::endl;
	}
}

int main()
{
	try
	{
		test_signature();
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