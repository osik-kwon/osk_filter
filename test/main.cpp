#include <filesystem>
#include <iostream>
#include <locale>

// document filters
#include "hwp/hwp50_filter.h"
#include "hwp/hwp30_filter.h"
#include "hwp/hwpx_filter.h"
#include "hwp/hwpml_filter.h"
#include "txt/txt_filter.h"
#include "word/doc_filter.h"
#include "word/docx_filter.h"
#include "slide/pptx_filter.h"
#include "sheet/xlsx_filter.h"
#include "locale/charset_encoder.h"
#include "traits/editor_traits.h"

// signature analyzer
#include <signature/signature_analyzer.h>
#include <signature/signature_builder.h>

// NLP
#include <textrank/textrank.h>
#include <chrono>

#include <boost/algorithm/string.hpp> 
#include <boost/algorithm/string/case_conv.hpp>

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
		if(!std::filesystem::exists(save_root))
			std::filesystem::create_directory(save_root);

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
		if (!std::filesystem::exists(save_root))
			std::filesystem::create_directory(save_root);

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

// TODO: remove

void test_doc()
{
	using namespace filter::doc;
	filter_t filter;
	auto src = filter.open(to_utf8(L"d:/sombra/text.doc"));
}

template <typename filter_t>
void exract_text(const std::wstring& src_path, std::wstring& dest)
{
	filter_t filter;
	auto src = filter.open(to_utf8(src_path));
	auto sections = filter.extract_all_texts(src);
	for (auto& section : sections)
	{
		for (auto& para : section)
		{
			if (!para.empty())
			{
				dest += para;
			}
		}
	}
}

std::wstring normalize_texts(const std::wstring& src)
{
	const std::wstring para_delimiter = L"\n";
	std::vector<std::wstring> paragraphes;
	boost::split(paragraphes, src, boost::is_any_of(para_delimiter));
	
	std::wstring dest;
	dest.reserve(paragraphes.size());
	for (auto& paragraph : paragraphes)
	{
		if (paragraph.size() > 0 && paragraph[0] != L'\n')
		{
			dest += paragraph;
			if(paragraph.back() != L'\n')
				dest += L'\n';
		}
	}
	return dest;
}


void test_summary_file(const std::wstring& src_path, const std::wstring& dest_path)
{
	try
	{
		auto rules = filter::signature::builder_t::build_string_rules();
		auto spec = rules->scan(to_utf8(src_path));

		std::wstring input;
		if (spec == "hwp30")
		{
			exract_text<filter::hwp30::filter_t>(src_path, input);
		}
		else if (spec == "hwp50")
		{
			exract_text<filter::hwp50::filter_t>(src_path, input);
		}
		else if (spec == "hwpx")
		{
			exract_text<filter::hwpx::filter_t>(src_path, input);
		}
		else if (spec == "hwpml")
		{
			exract_text<filter::hml::filter_t>(src_path, input);
		}
		else if (spec == "docx")
		{
			exract_text<filter::docx::filter_t>(src_path, input);
		}
		else if (spec == "pptx")
		{
			exract_text<filter::pptx::filter_t>(src_path, input);
		}
		else if (spec == "xlsx")
		{
			exract_text<filter::xlsx::filter_t>(src_path, input);
		}
		else
			return;
		

		nlp::text_ranker text_ranker;
		std::vector<std::string> stop_words_pathes;
		for (auto& path : std::filesystem::directory_iterator("dictionary/stopwords"))
			stop_words_pathes.push_back(std::filesystem::absolute(path).string());
		text_ranker.load_stop_words(stop_words_pathes);

		std::vector< std::pair< std::wstring, double> > keywords;
		text_ranker.key_words(input, keywords, 10);

		std::vector< std::pair< std::wstring, double> > key_sentences;
		text_ranker.key_sentences(input, key_sentences, 3);

		if (keywords.empty() && key_sentences.empty())
			return;
		std::locale::global(std::locale(""));
		std::ofstream out(dest_path);

		out << "[keywords]" << std::endl;
		for (auto& keyword : keywords)
		{
			try
			{
				out << to_utf8(keyword.first) << " : " << keyword.second << std::endl;
			}
			catch (const std::exception&)
			{}
		}

		out << std::endl << "[key sentences]" << std::endl;
		for (auto& key_sentence : key_sentences)
		{
			try
			{
				out << to_utf8(key_sentence.first) << " : " << key_sentence.second << std::endl;
			}
			catch (const std::exception&)
			{}
		}

		out << std::endl << "[original texts]" << std::endl;
		out << to_utf8(normalize_texts(input)) << std::endl;
		out.close();
	}
	catch (const std::exception& e)
	{
		std::wcout << L"[error] " << src_path << L" " << e.what();
	}
}

void test_summary_directory(const std::wstring& src, const std::wstring& dest_root)
{
	std::wcout.imbue(std::locale(""));
	std::vector<std::wstring> src_pathes;
	std::vector<std::wstring> dest_pathes;
	for (auto& path : std::filesystem::recursive_directory_iterator(src))
	{
		if (!path.is_directory())
		{
			src_pathes.push_back(std::filesystem::absolute(path).wstring());
			dest_pathes.push_back(dest_root + std::filesystem::path(path).filename().wstring() + L".txt");
		}	
	}

	std::vector<std::wstring> todo_pathes;
	long long total = 0;
	long long max = 0;
	for (int i = 0; i < src_pathes.size(); ++i)
	{
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
		test_summary_file(src_pathes[i], dest_pathes[i]);
		std::chrono::system_clock::time_point end = std::chrono::system_clock::now();

		if (std::wcout.bad())
			std::wcout.clear();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		max = std::max(ms, max);
		total += ms;
		if (ms > 900)
			todo_pathes.push_back(src_pathes[i]);

		std::wcout << L"[" << i+1 << "/" << src_pathes.size() << L"][" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" <<
			src_pathes[i] << std::endl;
	}
	std::wcout << L"total : " << total << " ms , average : " << total / src_pathes.size() << L" ms"<< " , max : "<< max << " ms" <<std::endl;
	for (auto& path : todo_pathes)
		std::wcout << path << std::endl;
}

void test_summary()
{
	std::locale::global(std::locale(""));
	//std::wstring src_path = L"d:/ci/hwp/2011충북대_정시모집요강.hwp";
	//std::wstring src_path = L"f:/sombra/33차유네스코총회참가보고서_국문.hwp";
	//std::wstring src_path = L"f:/sombra/english1.hwp";
	std::wstring src_path = L"f:/sombra/article1.hwp";
	//std::wstring src_path = L"f:/sombra/article4.hwp";
	//std::wstring src_path = L"d:/ci/hwp/(2) 무단방치 자전거 이동보관 현장 사진.hwp";
	//std::wstring src_path = L"f:/sombra/4.1 Medicine_Diseaseas of the Esophagus_2014A.docx";
	//std::wstring src_path = L"f:/sombra/중앙대_2011수시_100802.hwp";
	//std::wstring src_path = L"f:/sombra/GPS측량(2003).hwp";
	//std::wstring src_path = L"f:/sombra/2012_독해연습1_변형문제(1-19강).hwp";
	//std::wstring src_path = L"f:/sombra/Forging_Cold1.hwp";
	//std::wstring src_path = L"f:/sombra/제1차 국가교통조사계획.hwp";
	//std::wstring src_path = L"f:/sombra/교육과정내용.hwp";
	//std::wstring src_path = L"f:/sombra/2011 중앙대 수시 모집요강.hwp";
	//std::wstring src_path = L"d:/ci/hwp/HSK_시험에_잘나오는_的의_세가지_용법.hwp";
	//std::wstring src_path = L"f:/sombra/다이나믹 북 99호 - 카페를 100년간 이어가기 위해.hwp";
	//std::wstring src_path = L"d:/ci/hwp/홍콩심천(06.09).hwp";
	//std::wstring src_path = L"F:/sombra/confidential/2016년/임시 보관함/문서 임시 보관함/PM Team.docx";
	//std::wstring src_path = L"F:/sombra/confidential/2016년/광고 부분.pptx";
	//std::wstring src_path = L"F:/sombra/xlsx.xlsx";
	//std::wstring src_path = L"F:/sombra/오피스 2017 요금 및 혜택.xlsx";
	//std::wstring src_path = L"F:/sombra/오피스 2017 요금 및 혜택.xlsx";
	//std::wstring src_path = L"F:/sombra/프로젝트별 매출 현황_161013.xlsx";
	//std::wstring src_path = L"f:/sombra/article5.hwp";
	//std::wstring src_path = L"f:/sombra/2007년 친환경상품 구매지침_40704.hwp";
	//std::wstring src_path = L"d:/ci/hwp/07_14_2010_down01_01.hwp";

	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

	auto rules = filter::signature::builder_t::build_string_rules();
	auto spec = rules->scan(to_utf8(src_path));

	std::wstring input;
	if (spec == "hwp30")
	{
		exract_text<filter::hwp30::filter_t>(src_path, input);
	}
	else if (spec == "hwp50")
	{
		exract_text<filter::hwp50::filter_t>(src_path, input);
	}
	else if (spec == "hwpx")
	{
		exract_text<filter::hwpx::filter_t>(src_path, input);
	}
	else if (spec == "hwpml")
	{
		exract_text<filter::hml::filter_t>(src_path, input);
	}
	else if (spec == "docx")
	{
		exract_text<filter::docx::filter_t>(src_path, input);
	}
	else if (spec == "pptx")
	{
		exract_text<filter::pptx::filter_t>(src_path, input);
	}
	else if (spec == "xlsx")
	{
		exract_text<filter::xlsx::filter_t>(src_path, input);
	}
	else
		return;


	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();

	if (std::wcout.bad())
		std::wcout.clear();
	std::wcout << L"open 7 extract : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;
	/*
	start = std::chrono::system_clock::now();
	auto sections = filter.extract_all_texts(src);
	//filter.save(to_utf8(L"f:/sombra/result-save.hwp"), src);
	std::wstring input;
	for (auto& section : sections)
	{
		for (auto& para : section)
		{
			if (!para.empty())
			{
				input += para;
			}
		}
	}
	end = std::chrono::system_clock::now();
	std::wcout << L"extract : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;
	*/

	start = std::chrono::system_clock::now();
	nlp::text_ranker text_ranker;
	std::vector<std::string> stop_words_pathes;
	for (auto& path : std::filesystem::directory_iterator("dictionary/stopwords"))
		stop_words_pathes.push_back(std::filesystem::absolute(path).string());
	text_ranker.load_stop_words(stop_words_pathes);

	end = std::chrono::system_clock::now();
	std::wcout << L"load stop words : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;


	start = std::chrono::system_clock::now();
	std::vector< std::pair< std::wstring, double> > keywords;
	text_ranker.key_words(input, keywords, 10);
	end = std::chrono::system_clock::now();
	std::wcout << L"key words : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;

	start = std::chrono::system_clock::now();
	std::vector< std::pair< std::wstring, double> > key_sentences;
	text_ranker.key_sentences(input, key_sentences, 3);
	end = std::chrono::system_clock::now();
	std::wcout << L"key sentences : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;
	//std::vector<std::wstring> key_sentences;
	//text_ranker.key_sentences(input, key_sentences, 3);

	std::ofstream out(L"f:/sombra/result.txt");

	out << "[keywords]" << std::endl;
	for (auto& keyword : keywords)
	{
		try
		{
			out << to_utf8(keyword.first) << " : " << keyword.second << std::endl;
		}
		catch (const std::exception&)
		{}	
	}

	out << "[key sentences]" << std::endl;
	for (auto& key_sentence : key_sentences)
	{
		try
		{
			//out << to_utf8(key_sentence) << std::endl;
			out << to_utf8(key_sentence.first) << " : " << key_sentence.second << std::endl;
		}
		catch (const std::exception&)
		{}	
	}

	out << std::endl << "[original texts]" << std::endl;
	out << to_utf8(normalize_texts(input)) << std::endl;

	out.close();
}

void test_docx()
{
	std::wcout.imbue(std::locale(""));
	using namespace filter::docx;
	filter_t filter;
	auto src = filter.open(to_utf8(L"F:/sombra/0212.docx"));
	auto sections = filter.extract_all_texts(src);
}

void cmd_summary(const std::wstring& src_path, const std::wstring& dest_path, const std::wstring& stop_words_path)
{
	try
	{
		std::locale::global(std::locale(""));
		std::wcout << src_path << std::endl;
		std::wcout << dest_path << std::endl;
		std::filesystem::remove(dest_path);
		auto rules = filter::signature::builder_t::build_string_rules();
		auto spec = rules->scan(to_utf8(src_path));

		std::wstring input;
		if (spec == "hwp30")
		{
			exract_text<filter::hwp30::filter_t>(src_path, input);
		}
		else if (spec == "hwp50")
		{
			exract_text<filter::hwp50::filter_t>(src_path, input);
		}
		else if (spec == "hwpx")
		{
			exract_text<filter::hwpx::filter_t>(src_path, input);
		}
		else if (spec == "hwpml")
		{
			exract_text<filter::hml::filter_t>(src_path, input);
		}
		else if (spec == "docx")
		{
			exract_text<filter::docx::filter_t>(src_path, input);
		}
		else if (spec == "pptx")
		{
			exract_text<filter::pptx::filter_t>(src_path, input);
		}
		else if (spec == "xlsx")
		{
			exract_text<filter::xlsx::filter_t>(src_path, input);
		}
		else
			return;

		nlp::text_ranker text_ranker;
		std::vector<std::string> stop_words_pathes;
		for (auto& path : std::filesystem::directory_iterator(stop_words_path))
			stop_words_pathes.push_back(std::filesystem::absolute(path).string());
		text_ranker.load_stop_words(stop_words_pathes);

		std::vector< std::pair< std::wstring, double> > key_sentences;
		text_ranker.key_sentences(input, key_sentences, 3);
		if (key_sentences.empty())
			return ;

		std::vector< std::pair< std::wstring, double> > keywords;
		text_ranker.key_words(input, keywords, 10);

		std::ofstream out(dest_path);

		out << to_utf8(L"[키워드]") << std::endl;
		for (auto& keyword : keywords)
		{
			try
			{
				out << to_utf8(keyword.first) << " ";
			}
			catch (const std::exception&)
			{}		
		}

		out << std::endl << std::endl << to_utf8(L"[3줄 요약]") << std::endl;
		for (auto& key_sentence : key_sentences)
		{
			try
			{
				out << to_utf8(key_sentence.first) << std::endl;
			}
			catch (const std::exception&)
			{}
		}
		out.close();
	}
	catch (const std::exception& e)
	{
		std::wcout << L"[error] " << src_path << L" " << e.what();
		return;
	}
}

#include <atlstr.h>

int main(int argc, char* argv[])
{
	try
	{
		//test_hwpx();
		//test_summary_directory(L"D:/ci/docx/", L"F:/sombra/docx/");
		//test_summary_directory(L"D:/ci/hwp/", L"F:/sombra/result/");
		//test_summary_directory(L"F:/sombra/confidential/", L"F:/sombra/result_confidential/");
		test_summary();
		//test_docx();
		return 0;

		if (argc < 2)
			return 0;

		CString arg_path = (argv[1]);
		USES_CONVERSION;
		WCHAR* w_arg_path = T2W(arg_path.GetBuffer());

		std::ifstream arg(w_arg_path);
		if(!arg.is_open())
			return 0;

		std::string exe;
		std::getline(arg, exe);
		std::string src;
		std::getline(arg, src);
		std::string result;
		std::getline(arg, result);
		std::string stop_words;
		std::getline(arg, stop_words);
		arg.close();

		cmd_summary(to_wchar(src), to_wchar(result), to_wchar(stop_words));
		//cmd_summary((wchar_t*)(argv[1]), (wchar_t*)(argv[2]), (wchar_t*)(argv[3]));
		//cmd_summary(to_wchar(argv[1]), to_wchar(argv[2]), to_wchar(argv[3]));
		//return 0 ;
		
		//CString a = ConvertMultibyteToUnicode(argv[1]);
		//CString b = ConvertMultibyteToUnicode(argv[2]);
		//CString c = ConvertMultibyteToUnicode(argv[3]);
		/*
		CString a = (argv[1]);
		CString b = (argv[2]);
		CString c = (argv[3]);
		//USES_CONVERSION;
		WCHAR* wa = T2W(a.GetBuffer());
		WCHAR* wb = T2W(b.GetBuffer());
		WCHAR* wc = T2W(c.GetBuffer());

		cmd_summary(wa, wb, wc);
		*/
		
		//test_summary_directory(L"D:/ci/hwp/", L"F:/sombra/result/");
		//test_summary_directory(L"D:/ci/docx/", L"F:/sombra/docx/");
		//test_docx();
		//test_summary();
		//test_doc();
		//test_signature();
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