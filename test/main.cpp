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
		std::cout << "[0] open charset is " << filter.detect_charset(open_path) << std::endl;
		std::cout << "[1] open new document"<< std::endl;
		auto src = filter.open(open_path);
		std::cout << "[2] extract all texts from new document" << std::endl;
		std::wcout << filter.extract_all_texts(src);
		std::cout << "[3] save" << std::endl;
		filter.save(save_path, src);
		std::cout << "[4] save charset is" << filter.detect_charset(save_path) << std::endl;
		std::cout << "[5] open saved document" << std::endl;
		auto dest = filter.open(save_path);
		std::cout << "[6] extract all texts from saved document" << std::endl;
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


/*
void test_hwp30()
{
	std::cout << "===== hwp30 test =====" << std::endl;
	typedef filter::hwp30::filter_t filter_t;
	{
		std::cout << "===== hwp30 non ascii path test =====" << std::endl;
		filter_t filter;
		auto src = filter.open(to_utf8(u"d:/filter/hwp30_한글경로.hwp"));
		print(filter.extract_all_texts(src));
		filter.save(to_utf8(u"d:/filter/hwp30_한글경로.export.hwp"), src);
		auto dest = filter.open(to_utf8(u"d:/filter/hwp30_한글경로.export.hwp"));
		print(filter.extract_all_texts(dest));
	}
	{
		std::cout << "===== hwp30 open/save test =====" << std::endl;
		filter_t filter;
		auto src = filter.open(to_utf8(u"d:/filter/hwp30_shape.hwp"));
		print(filter.extract_all_texts(src));
		filter.save(to_utf8(u"d:/filter/hwp30_shape.export.hwp"), src);
		auto dest = filter.open(to_utf8(u"d:/filter/hwp30_shape.export.hwp"));
		print(filter.extract_all_texts(dest));
	}
	{
		std::cout << "===== hwp30 search privacy test =====" << std::endl;
		filter_t filter;
		auto src = filter.open(to_utf8(u"d:/filter/hwp30_privacy.hwp"));
		print(filter.extract_all_texts(src));
		std::wregex resident_registration_number(L"(?:[0-9]{2}(?:0[1-9]|1[0-2])(?:0[1-9]|[1,2][0-9]|3[0,1]))-[1-4][0-9]{6}");
		print(filter.search_privacy({ resident_registration_number }, src));
	}
	{
		std::cout << "===== hwp30 replace privacy test =====" << std::endl;
		std::wregex resident_registration_number(L"(?:[0-9]{2}(?:0[1-9]|1[0-2])(?:0[1-9]|[1,2][0-9]|3[0,1]))-[1-4][0-9]{6}");
		filter_t filter;

		auto src = filter.open(to_utf8(u"d:/filter/hwp30_privacy.hwp"));
		print(filter.extract_all_texts(src));

		filter.replace_privacy({ resident_registration_number }, u'@', src);
		filter.save(to_utf8(u"d:/filter/hwp30_privacy.export.hwp"), src);

		auto dest = filter.open(to_utf8(u"d:/filter/hwp30_privacy.export.hwp"));
		print(filter.extract_all_texts(dest));
	}
}

void test_hwp50()
{
	std::cout << "===== hwp50 test =====" << std::endl;
	typedef filter::hwp50::filter_t filter_t;
	{
		std::cout << "===== hwp50 non ascii path test =====" << std::endl;
		filter_t filter;
		auto src = filter.open(to_utf8(u"d:/filter/hwp50_한글경로.hwp"));
		print(filter.extract_all_texts(src));
		filter.save(to_utf8(u"d:/filter/hwp50_한글경로.export.hwp"), src);
		auto dest = filter.open(to_utf8(u"d:/filter/hwp50_한글경로.export.hwp"));
		print(filter.extract_all_texts(dest));
	}
	{
		std::cout << "===== hwp50 open/save test =====" << std::endl;
		filter_t filter;
		auto src = filter.open(to_utf8(u"d:/filter/hwp50.hwp"));
		print(filter.extract_all_texts(src));
		filter.save(to_utf8(u"d:/filter/hwp50.export.hwp"), src);
		auto dest = filter.open(to_utf8(u"d:/filter/hwp50.export.hwp"));
		print(filter.extract_all_texts(dest));
	}
	{
		std::cout << "===== hwp50 search privacy test =====" << std::endl;
		filter_t filter;
		auto src = filter.open(to_utf8(u"d:/filter/privacy2.hwp"));
		print(filter.extract_all_texts(src));
		std::wregex resident_registration_number(L"(?:[0-9]{2}(?:0[1-9]|1[0-2])(?:0[1-9]|[1,2][0-9]|3[0,1]))-[1-4][0-9]{6}");
		print(filter.search_privacy({ resident_registration_number }, src));
	}
	{
		std::cout << "===== hwp50 replace privacy test =====" << std::endl;
		std::wregex resident_registration_number(L"(?:[0-9]{2}(?:0[1-9]|1[0-2])(?:0[1-9]|[1,2][0-9]|3[0,1]))-[1-4][0-9]{6}");
		filter_t filter;

		auto src = filter.open(to_utf8(u"d:/filter/privacy2.hwp"));
		print(filter.extract_all_texts(src));

		filter.replace_privacy({ resident_registration_number }, u'@', src);
		filter.save(to_utf8(u"d:/filter/privacy2.export.hwp"), src);

		auto dest = filter.open(to_utf8(u"d:/filter/privacy2.export.hwp"));
		print(filter.extract_all_texts(dest));
	}
}

void test_hwp50_distribution()
{
	std::cout << "===== hwp50 distribution test =====" << std::endl;
	typedef filter::hwp50::filter_t filter_t;
	{
		std::cout << "===== hwp50 open/save test =====" << std::endl;
		filter_t filter;
		auto src = filter.open(to_utf8(u"d:/filter/hwp50_dist.hwp"));
		print(filter.extract_all_texts(src));
		filter.save(to_utf8(u"d:/filter/hwp50_dist.export.hwp"), src);
		auto dest = filter.open(to_utf8(u"d:/filter/hwp50_dist.export.hwp"));
		print(filter.extract_all_texts(dest));
	}
	{
		std::cout << "===== hwp50 search privacy test =====" << std::endl;
		filter_t filter;
		auto src = filter.open(to_utf8(u"d:/filter/hwp50_privacy_dist.hwp"));
		print(filter.extract_all_texts(src));
		std::wregex resident_registration_number(L"(?:[0-9]{2}(?:0[1-9]|1[0-2])(?:0[1-9]|[1,2][0-9]|3[0,1]))-[1-4][0-9]{6}");
		print(filter.search_privacy({ resident_registration_number }, src));
	}
	{
		std::cout << "===== hwp50 replace privacy test =====" << std::endl;
		std::wregex resident_registration_number(L"(?:[0-9]{2}(?:0[1-9]|1[0-2])(?:0[1-9]|[1,2][0-9]|3[0,1]))-[1-4][0-9]{6}");
		filter_t filter;

		auto src = filter.open(to_utf8(u"d:/filter/hwp50_privacy_dist.hwp"));
		print(filter.extract_all_texts(src));

		filter.replace_privacy({ resident_registration_number }, u'@', src);
		filter.save(to_utf8(u"d:/filter/hwp50_privacy_dist.export.hwp"), src);

		auto dest = filter.open(to_utf8(u"d:/filter/hwp50_privacy_dist.export.hwp"));
		print(filter.extract_all_texts(dest));
	}
}

void test_hwpml()
{
	std::cout << "===== hwpml test =====" << std::endl;
	typedef filter::hml::filter_t filter_t;
	{
		std::cout << "===== hwpml non ascii path test =====" << std::endl;
		filter_t filter;
		auto src = filter.open(to_utf8(u"d:/filter/hml_한글경로.hml"));
		print(filter.extract_all_texts(src));
		filter.save(to_utf8(u"d:/filter/hml_한글경로.export.hml"), src);
		auto dest = filter.open(to_utf8(u"d:/filter/hml_한글경로.export.hml"));
		print(filter.extract_all_texts(dest));
	}
	{
		std::cout << "===== hwpml open/save test =====" << std::endl;
		filter_t filter;
		auto src = filter.open(to_utf8(u"d:/filter/hml.hml"));
		print(filter.extract_all_texts(src));
		filter.save(to_utf8(u"d:/filter/hml.export.hml"), src);
		auto dest = filter.open(to_utf8(u"d:/filter/hml.export.hml"));
		print(filter.extract_all_texts(dest));
	}
	{
		std::cout << "===== hwpml search privacy test =====" << std::endl;
		filter_t filter;
		auto src = filter.open(to_utf8(u"d:/filter/privacy.hml"));
		print(filter.extract_all_texts(src));
		std::wregex resident_registration_number(L"(?:[0-9]{2}(?:0[1-9]|1[0-2])(?:0[1-9]|[1,2][0-9]|3[0,1]))-[1-4][0-9]{6}");
		print(filter.search_privacy({ resident_registration_number }, src));
	}
	{
		std::cout << "===== hwpml replace privacy test =====" << std::endl;
		std::wregex resident_registration_number(L"(?:[0-9]{2}(?:0[1-9]|1[0-2])(?:0[1-9]|[1,2][0-9]|3[0,1]))-[1-4][0-9]{6}");
		filter_t filter;

		auto src = filter.open(to_utf8(u"d:/filter/privacy.hml"));
		print(filter.extract_all_texts(src));

		filter.replace_privacy({ resident_registration_number }, u'@', src);
		filter.save(to_utf8(u"d:/filter/privacy.export.hml"), src);

		auto dest = filter.open(to_utf8(u"d:/filter/privacy.export.hml"));
		print(filter.extract_all_texts(dest));
	}
}

void test_hwpx()
{
	std::cout << "===== hwpx test =====" << std::endl;
	typedef filter::hwpx::filter_t filter_t;
	{
		std::cout << "===== hwpx non ascii path test =====" << std::endl;
		filter_t filter;
		auto src = filter.open(to_utf8(u"d:/filter/hwpx_한글경로.hwpx"));
		print(filter.extract_all_texts(src));
		filter.save(to_utf8(u"d:/filter/hwpx_한글경로.export.hwpx"), src);
		auto dest = filter.open(to_utf8(u"d:/filter/hwpx_한글경로.export.hwpx"));
		print(filter.extract_all_texts(dest));
	}
	{
		std::cout << "===== hwpx open/save test =====" << std::endl;
		filter_t filter;
		auto src = filter.open(to_utf8(u"d:/filter/hwpx.hwpx"));
		print(filter.extract_all_texts(src));
		filter.save(to_utf8(u"d:/filter/hwpx.export.hwpx"), src);
		auto dest = filter.open(to_utf8(u"d:/filter/hwpx.export.hwpx"));
		print(filter.extract_all_texts(dest));
	}
	{
		std::cout << "===== hwpx search privacy test =====" << std::endl;
		filter_t filter;
		auto src = filter.open(to_utf8(u"d:/filter/privacy.hwpx"));
		print(filter.extract_all_texts(src));
		std::wregex resident_registration_number(L"(?:[0-9]{2}(?:0[1-9]|1[0-2])(?:0[1-9]|[1,2][0-9]|3[0,1]))-[1-4][0-9]{6}");
		print(filter.search_privacy({ resident_registration_number }, src));
	}
	{
		std::cout << "===== hwpx replace privacy test =====" << std::endl;
		std::wregex resident_registration_number(L"(?:[0-9]{2}(?:0[1-9]|1[0-2])(?:0[1-9]|[1,2][0-9]|3[0,1]))-[1-4][0-9]{6}");
		filter_t filter;

		auto src = filter.open(to_utf8(u"d:/filter/privacy.hwpx"));
		print(filter.extract_all_texts(src));

		filter.replace_privacy({ resident_registration_number }, u'@', src);
		filter.save(to_utf8(u"d:/filter/privacy.export.hwpx"), src);

		auto dest = filter.open(to_utf8(u"d:/filter/privacy.export.hwpx"));
		print(filter.extract_all_texts(dest));
	}
}

*/
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

int main()
{
	try
	{
		test_txt();
		//test_hwpml();
		//test_hwpx();
		//test_hwp30();
		//test_hwp50();
		//test_hwp50_distribution();
		
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
	return 0;
}