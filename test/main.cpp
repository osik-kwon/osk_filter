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
#include "pdf/pdf_filter.h"
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

#include <mecab/mecab.h>
#include <similarity/jaccard_similarity.h>


#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>

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
void exract_text_impl(const std::wstring& src_path, std::wstring& dest)
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

bool extract_text(std::wstring& dest, const std::wstring& src_path)
{
	auto rules = filter::signature::builder_t::build_string_rules();
	auto spec = rules->scan(to_utf8(src_path));

	if (spec == "hwp30")
	{
		exract_text_impl<filter::hwp30::filter_t>(src_path, dest);
	}
	else if (spec == "hwp50")
	{
		exract_text_impl<filter::hwp50::filter_t>(src_path, dest);
	}
	else if (spec == "hwpx")
	{
		exract_text_impl<filter::hwpx::filter_t>(src_path, dest);
	}
	else if (spec == "hwpml")
	{
		exract_text_impl<filter::hml::filter_t>(src_path, dest);
	}
	else if (spec == "docx")
	{
		exract_text_impl<filter::docx::filter_t>(src_path, dest);
	}
	else if (spec == "pptx")
	{
		exract_text_impl<filter::pptx::filter_t>(src_path, dest);
	}
	else if (spec == "xlsx")
	{
		exract_text_impl<filter::xlsx::filter_t>(src_path, dest);
	}
	else if (spec == "pdf")
	{
		exract_text_impl<filter::pdf::filter_t>(src_path, dest);
	}
	else
	{
		if (std::filesystem::path(src_path).extension().wstring() == L".txt")
		{
			exract_text_impl<filter::txt::filter_t>(src_path, dest);
		}
		else
			return false;
	}
	return true;
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
		std::wstring input;
		if (!extract_text(input, src_path))
			return;

		nlp::text_ranker text_ranker;
		std::vector<std::string> stop_words_pathes;
		for (auto& path : std::filesystem::directory_iterator("dictionary/stopwords"))
			stop_words_pathes.push_back(std::filesystem::absolute(path).string());
		text_ranker.load_stop_words(stop_words_pathes);
		text_ranker.load_morphological_analyzer(std::filesystem::absolute("dictionary/mecabrc").string(),
			std::filesystem::absolute("dictionary/mecab-ko-dic").string());

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
			src_pathes.push_back(std::filesystem::absolute(path).generic_wstring());
			dest_pathes.push_back(dest_root + std::filesystem::path(path).filename().generic_wstring() + L".txt");
		}	
	}

	std::vector<std::wstring> todo_pathes;
	long long total = 0;
	long long max = 0;
	for (size_t i = 0; i < src_pathes.size(); ++i)
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
	//std::wstring src_path = L"d:/ci/docx/3GPP-Spec-Titles.docx";
	//std::wstring src_path = L"f:/notion/mindmap.hwp";
	//std::wstring src_path = L"f:/notion/인프라웨어_엔진(BWP)_구조품질_200721.pptx";
	//std::wstring src_path = L"f:/sombra/twitter.txt";
	//std::wstring src_path = L"f:/pdf/sample/방콕_가이드북_한쪽_v4.0_150306.pdf";
	std::wstring src_path = L"f:/pdf/sample/2.pdf";
	//std::wstring src_path = L"f:/sombra/legacy.hwp";
	//std::wstring src_path = L"d:/ci/docx/2주차.docx";
	//std::wstring src_path = L"f:/sombra/english1.hwp";
	//std::wstring src_path = L"f:/sombra/article1.hwp";
	//std::wstring src_path = L"f:/sombra/article5.hwp";
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

	std::wstring input;
	if (!extract_text(input, src_path))
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

	text_ranker.load_morphological_analyzer(std::filesystem::absolute("dictionary/mecabrc").string(),
		std::filesystem::absolute("dictionary/mecab-ko-dic").string());

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

void test_pdf()
{
	std::wcout.imbue(std::locale(""));
	using namespace filter::pdf;
	filter_t filter;
	auto sections = filter.extract_all_texts(to_utf8(L"F:/pdf/sample/simple2.pdf"));
}

void cmd_summary(const std::wstring& src_path, const std::wstring& dest_path, const std::wstring& stop_words_path,
	const std::wstring& mecab_rc, const std::wstring& mecab_dic)
{
	try
	{
		std::locale::global(std::locale(""));
		std::wcout << src_path << std::endl;
		std::wcout << dest_path << std::endl;
		std::filesystem::remove(dest_path);

		std::wstring input;
		if (!extract_text(input, src_path))
			return;

		nlp::text_ranker text_ranker;
		std::vector<std::string> stop_words_pathes;
		for (auto& path : std::filesystem::directory_iterator(stop_words_path))
			stop_words_pathes.push_back(std::filesystem::absolute(path).string());
		text_ranker.load_stop_words(stop_words_pathes);

		if (!mecab_rc.empty() && !mecab_dic.empty())
		{
			text_ranker.load_morphological_analyzer(to_utf8(mecab_rc), to_utf8(mecab_dic));
		}
		

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

void test_directory()
{
	std::locale::global(std::locale(""));
	std::wstring src_path = L"F:/주간보고/";

	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
	std::wstring input;
	input.reserve(100000);
	for (auto& path : std::filesystem::recursive_directory_iterator(src_path))
	{
		if (path.is_directory())
			continue;
		std::wcout << std::filesystem::absolute(path).generic_wstring() << std::endl;
		std::wstring document;
		if (!extract_text(document, std::filesystem::absolute(path).generic_wstring()))
			continue;
		if(!document.empty())
			input += document;
	}
	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();

	if (std::wcout.bad())
		std::wcout.clear();
	std::wcout << L"open & extract : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;

	start = std::chrono::system_clock::now();
	nlp::text_ranker text_ranker;
	std::vector<std::string> stop_words_pathes;
	for (auto& path : std::filesystem::directory_iterator("dictionary/stopwords"))
		stop_words_pathes.push_back(std::filesystem::absolute(path).string());
	text_ranker.load_stop_words(stop_words_pathes);
	text_ranker.load_morphological_analyzer(std::filesystem::absolute("dictionary/mecabrc").string(),
		std::filesystem::absolute("dictionary/mecab-ko-dic").string());

	end = std::chrono::system_clock::now();
	std::wcout << L"load stop words : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;

	start = std::chrono::system_clock::now();
	std::vector< std::pair< std::wstring, double> > keywords;
	text_ranker.key_words(input, keywords, 100);
	end = std::chrono::system_clock::now();
	std::wcout << L"key words : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;

	start = std::chrono::system_clock::now();
	std::vector< std::pair< std::wstring, double> > key_sentences;
	text_ranker.key_sentences(input, key_sentences, 30);
	end = std::chrono::system_clock::now();
	std::wcout << L"key sentences : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;

	std::ofstream out(L"f:/sombra/multiple_result.txt");

	out << "[keywords]" << std::endl;
	for (auto& keyword : keywords)
	{
		try
		{
			out << to_utf8(keyword.first) << " : " << keyword.second << std::endl;
		}
		catch (const std::exception&)
		{
		}
	}

	out << "[key sentences]" << std::endl;
	for (auto& key_sentence : key_sentences)
	{
		try
		{
			out << to_utf8(key_sentence.first) << " : " << key_sentence.second << std::endl;
		}
		catch (const std::exception&)
		{
		}
	}

	out << std::endl << "[original texts]" << std::endl;
	out << to_utf8(normalize_texts(input)) << std::endl;

	out.close();
}

void test_directory2()
{
	std::locale::global(std::locale(""));
	std::wstring src_path = L"F:/주간보고/";
	//std::wstring src_path = L"F:/Weekly/";

	std::ofstream out(L"f:/sombra/multiple_result_team1.txt");
	//std::ofstream out(L"f:/sombra/multiple_result_team2.txt");

	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
	std::vector<std::wstring> input;
	input.reserve(100000);
	for (auto& path : std::filesystem::recursive_directory_iterator(src_path))
	{
		if (path.is_directory())
			continue;
		std::wcout << std::filesystem::absolute(path).generic_wstring() << std::endl;
		std::wstring document;
		if (!extract_text(document, std::filesystem::absolute(path).generic_wstring()))
			continue;
		if (!document.empty())
			input.push_back(document);
	}
	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();

	if (std::wcout.bad())
		std::wcout.clear();
	std::wcout << L"open & extract : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;

	start = std::chrono::system_clock::now();
	nlp::text_ranker text_ranker;
	std::vector<std::string> stop_words_pathes;
	for (auto& path : std::filesystem::directory_iterator("dictionary/stopwords"))
		stop_words_pathes.push_back(std::filesystem::absolute(path).string());
	text_ranker.load_stop_words(stop_words_pathes);
	text_ranker.load_morphological_analyzer(std::filesystem::absolute("dictionary/mecabrc").string(),
		std::filesystem::absolute("dictionary/mecab-ko-dic").string());

	end = std::chrono::system_clock::now();
	std::wcout << L"load stop words : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;

	std::wcout << L"total : " << input.size() << std::endl;
	std::wstring input_keywords;
	size_t id = 0;
	for (auto& document : input)
	{
		++id;
		std::wcout << L"[" << id << L"]";
		start = std::chrono::system_clock::now();
		std::vector< std::pair< std::wstring, double> > keywords;
		text_ranker.key_words(document, keywords, 1000);
		for (auto& keyword : keywords)
		{
			input_keywords += L' ';
			input_keywords += keyword.first;
		}
		end = std::chrono::system_clock::now();
		std::wcout << L"key words : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;
	}
	
	//std::wcout << input_keywords << std::endl;

	start = std::chrono::system_clock::now();
	std::vector< std::pair< std::wstring, double> > keywords;
	text_ranker.key_words(input_keywords, keywords, 100);
	end = std::chrono::system_clock::now();
	std::wcout << L"key words : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;

	
	out << "[keywords]" << std::endl;
	for (auto& keyword : keywords)
	{
		try
		{
			out << to_utf8(keyword.first) << " : " << keyword.second << std::endl;
		}
		catch (const std::exception&)
		{
		}
	}
	out.close();
}

void test_directory3()
{
	std::locale::global(std::locale(""));
	//std::wstring src_path = L"F:/주간보고/";
	std::wstring src_path = L"F:/Weekly/";

	std::ofstream out(L"f:/sombra/multiple_result_team1.txt");
	//std::ofstream out(L"f:/sombra/multiple_result_team2.txt");
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
	std::vector<std::wstring> input;
	input.reserve(100000);
	for (auto& path : std::filesystem::recursive_directory_iterator(src_path))
	{
		if (path.is_directory())
			continue;
		std::wcout << std::filesystem::absolute(path).generic_wstring() << std::endl;
		std::wstring document;
		if (!extract_text(document, std::filesystem::absolute(path).generic_wstring()))
			continue;
		if (!document.empty())
			input.push_back(document);
	}
	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();

	if (std::wcout.bad())
		std::wcout.clear();
	std::wcout << L"open & extract : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;

	start = std::chrono::system_clock::now();
	nlp::text_ranker text_ranker;
	std::vector<std::string> stop_words_pathes;
	for (auto& path : std::filesystem::directory_iterator("dictionary/stopwords"))
		stop_words_pathes.push_back(std::filesystem::absolute(path).string());
	text_ranker.load_stop_words(stop_words_pathes);
	text_ranker.load_morphological_analyzer(std::filesystem::absolute("dictionary/mecabrc").string(),
		std::filesystem::absolute("dictionary/mecab-ko-dic").string());

	end = std::chrono::system_clock::now();
	std::wcout << L"load stop words : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;


	std::wcout << L"total : " << input.size() << std::endl;
	std::wstring input_keywords;
	size_t id = 0;
	for (auto& document : input)
	{
		++id;
		std::wcout << L"[" << id << L"]";
		start = std::chrono::system_clock::now();

		std::vector< std::pair< std::wstring, double> > key_sentences;
		text_ranker.key_sentences(document, key_sentences, 100);

		for (auto& sentence : key_sentences)
		{
			input_keywords += L'\n';
			input_keywords += sentence.first;
		}
		end = std::chrono::system_clock::now();
		std::wcout << L"key words : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;
	}

	//std::wcout << input_keywords << std::endl;

	start = std::chrono::system_clock::now();
	std::vector< std::pair< std::wstring, double> > key_sentences;
	text_ranker.key_sentences(input_keywords, key_sentences, 100);
	end = std::chrono::system_clock::now();
	std::wcout << L"key words : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;

	
	out << "[keywords]" << std::endl;
	for (auto& sentence : key_sentences)
	{
		try
		{
			out << to_utf8(sentence.first) << " : " << sentence.second << std::endl;
		}
		catch (const std::exception&)
		{
		}
	}
	out.close();
}

void test_directory4()
{
	std::locale::global(std::locale(""));
	//std::wstring src_path = L"F:/주간보고/";
	std::wstring src_path = L"F:/Weekly/";

	//std::ofstream out(L"f:/sombra/multiple_result_team1.txt");
	std::ofstream out(L"f:/sombra/multiple_result_team2.txt");

	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
	std::vector<std::wstring> input;
	input.reserve(100000);
	for (auto& path : std::filesystem::recursive_directory_iterator(src_path))
	{
		if (path.is_directory())
			continue;
		std::wcout << std::filesystem::absolute(path).generic_wstring() << std::endl;
		std::wstring document;
		if (!extract_text(document, std::filesystem::absolute(path).generic_wstring()))
			continue;
		if (!document.empty())
			input.push_back(document);
	}
	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();

	if (std::wcout.bad())
		std::wcout.clear();
	std::wcout << L"open & extract : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;

	start = std::chrono::system_clock::now();
	nlp::text_ranker text_ranker;
	std::vector<std::string> stop_words_pathes;
	for (auto& path : std::filesystem::directory_iterator("dictionary/stopwords"))
		stop_words_pathes.push_back(std::filesystem::absolute(path).string());
	text_ranker.load_stop_words(stop_words_pathes);
	text_ranker.load_morphological_analyzer(std::filesystem::absolute("dictionary/mecabrc").string(),
		std::filesystem::absolute("dictionary/mecab-ko-dic").string());

	end = std::chrono::system_clock::now();
	std::wcout << L"load stop words : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;

	std::wcout << L"total : " << input.size() << std::endl;
	std::wstring input_keywords;
	size_t id = 0;
	for (auto& document : input)
	{
		++id;
		std::wcout << L"[" << id << L"]";
		start = std::chrono::system_clock::now();
		std::vector< std::pair< std::wstring, double> > keywords;
		text_ranker.key_words(document, keywords, 1000);
		for (auto& keyword : keywords)
		{
			input_keywords += L' ';
			input_keywords += keyword.first;
		}
		end = std::chrono::system_clock::now();
		std::wcout << L"key words : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;
	}

	//std::wcout << input_keywords << std::endl;

	start = std::chrono::system_clock::now();
	std::vector< std::pair< std::wstring, double> > keywords;
	text_ranker.key_words(input_keywords, keywords, 100);
	end = std::chrono::system_clock::now();
	std::wcout << L"key words : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;


	out << "[keywords]" << std::endl;
	for (auto& keyword : keywords)
	{
		try
		{
			out << to_utf8(keyword.first) << " : " << keyword.second << std::endl;
		}
		catch (const std::exception&)
		{
		}
	}
	out.close();
}

#define CHECK(eval) if (! eval) { \
   const char *e = tagger ? tagger->what() : MeCab::getTaggerError(); \
   std::cerr << "Exception:" << e << std::endl; \
   delete tagger; \
   return ; }

void test_mecab()
{
	std::wcout.imbue(std::locale(""));
	//std::string input = to_utf8(L"트위터, 정보당국에 데이터 분석자료 팔지 않겠다. 트위터가 수많은 트윗을 분석해 정보를 판매하는 서비스를 미국 정보당국에는 제공하지 않기로 했다. 월스트리트저널은 미국 정보당국 관계자 등을 인용해 데이터마이너(Dataminer)가 정보당국에 대한 서비스는 중단하기로 했다고 9일(현지시간) 보도했다. 트위터가 5% 지분을 가진 데이터마이너는 소셜미디어상 자료를 분석해 고객이 의사결정을 하도록 정보를 제공하는 기업이다. 트위터에 올라오는 트윗에 실시간으로 접근해 분석한 자료를 고객에게 팔 수 있는 독점권을 갖고 있다. 정보당국은 이 회사로부터 구매한 자료로 테러나 정치적 불안정 등과 관련된 정보를 획득했다. 이 회사가 정보당국에 서비스를 판매하지 않기로 한 것은 트위터의 결정인 것으로 알려졌다. 데이터마이너 경영진은 최근 “트위터가 정보당국에 서비스하는 것을 원치 않는다”고 밝혔다고 이 신문은 전했다. 트위터도 성명을 내고 “정보당국 감시용으로 데이터를 팔지 않는 것은 트위터의 오래된 정책”이라며 “트위터 자료는 대체로 공개적이고 미국 정부도 다른 사용자처럼 공개된 어카운트를 살펴볼 수 있다”고 해명했다. 그러나 이는 이 회사가 2년 동안 정보당국에 서비스를 제공해 온 데 대해서는 타당한 설명이 되지 않는다. 트위터의 이번 결정은 미국의 정보기술(IT)기업과 정보당국 간 갈등의 연장 선상에서 이뤄진 것으로 여겨지고 있다. IT기업은 이용자 프라이버시에 무게 중심을 두는 데 비해 정보당국은 공공안전을 우선시해 차이가 있었다. 특히 애플은 캘리포니아 주 샌버너디노 총격범의 아이폰에 저장된 정보를 보겠다며 데이터 잠금장치 해제를 요구하는 미 연방수사국(FBI)과 소송까지 진행했다. 정보당국 고위 관계자도 “트위터가 정보당국과 너무 가까워 보이는 것을 우려하는 것 같다”고 말했다. 데이터마이너는 금융기관이나, 언론사 등 정보당국을 제외한 고객에 대한 서비스는 계속할 계획이다.");
	//std::string input = to_utf8(L"러스트는 갓언어입니다. 똥바는 똥언어입니다. 알버트는 귀요미!?");
	//std::string input = to_utf8(L"MeCabはC/C++のライブラリを提供しています。また, SWIGを通して Perl/Ruby/Python から利用することも可能です。");

	//std::string input = to_utf8(L"피고인은 소산입니다 없습니다 정치적·사회적·도덕적 도덕적으로 있는 대학생들이 두고 제출합니다 항소이유서");
	//std::string input = to_utf8(L"이것은 형태소 분석기 입니다 아버지가방에들어가신다");
	
	std::string input = to_utf8(L"마인드세트의 코로나의");

	//std::wcout << L"알버트는 누름틀 장인입니까? 에이먼은 전생에 프로토스 종족의 아비터였습니다. 마인드맵은 갓기능입니다." << std::endl;

	//std::string input = to_utf8(L"알버트는 누름틀 장인입니까? 에이먼은 전생에 프로토스 종족의 아비터였습니다. 마인드맵은 갓기능입니다.");

	//get_args("F:/project/hello_mecab/hello_mecab/lib/mecab-ko-dic");
	// "F:/project/mecab/mecabrc"
	//auto args = get_args("F:/project/mecab/mecab-ko-dic");
	// 

	MeCab::Tagger* tagger = MeCab::createTagger("mecab -r F:/project/mecab/mecabrc -d F:/project/mecab/mecab-ko-dic");
	//MeCab::Tagger* tagger = MeCab::createTagger("mecab -r F:/project/mecab/mecabrc -d F:/project/mecab/mecab-ja-dic");
	//MeCab::Tagger* tagger = MeCab::createTagger("mecab -r ./mecabrc -d F:/project/mecab/mecab-ko-dic");
	CHECK(tagger);

	
	// Gets tagged result in string format.
	const char* result = tagger->parse(input.c_str());
	CHECK(result);
	auto test_result = to_wchar(result);
	std::wcout << L"INPUT: " << to_wchar(input) << std::endl;
	std::wcout << L"RESULT: " << std::endl << to_wchar(result) << std::endl;

	// Gets N best results in string format.
	result = tagger->parseNBest(3, input.c_str());
	CHECK(result);
	std::wcout << "NBEST: " << std::endl << to_wchar(result);
	

	// Gets N best results in sequence.
   // CHECK(tagger->parseNBestInit(input));
	//for (int i = 0; i < 3; ++i) {
	//    std::cout << i << ":" << std::endl << tagger->next();
	//}

	// Gets Node object.
	const MeCab::Node* node = tagger->parseToNode(input.c_str());
	CHECK(node);
	for (; node; node = node->next) {
		//std::cout << node->id << ' ';
		//if (node->stat == MECAB_BOS_NODE)
		//	std::cout << "BOS";
		//else if (node->stat == MECAB_EOS_NODE)
		//	std::cout << "EOS";
		//else
		//{
		//	auto test = to_wchar(std::string(node->surface, node->length));
		//	std::wcout << to_wchar(std::string(node->surface, node->length));
		//}

		auto feature = to_wchar(node->feature);

		std::vector<std::wstring> tokens;
		boost::split(tokens, feature, boost::is_any_of(L","));
		if (!tokens.empty() && !tokens[0].empty() && tokens[0][0] == L'N')
		{
			std::cout << node->id << ' ';
			std::wcout << to_wchar(std::string(node->surface, node->length));
			std::wcout << L' ' << to_wchar(node->feature)
				<< ' ' << (int)(node->surface - input.c_str())
				<< ' ' << (int)(node->surface - input.c_str() + node->length)
				<< ' ' << node->rcAttr
				<< ' ' << node->lcAttr
				<< ' ' << node->posid
				<< ' ' << (int)node->char_type
				<< ' ' << (int)node->stat
				<< ' ' << (int)node->isbest
				<< ' ' << node->alpha
				<< ' ' << node->beta
				<< ' ' << node->prob
				<< ' ' << node->cost << std::endl;
		}
			
		
	}

	// Dictionary info.
	const MeCab::DictionaryInfo* d = tagger->dictionary_info();
	for (; d; d = d->next) {
		std::cout << "filename: " << d->filename << std::endl;
		std::cout << "charset: " << d->charset << std::endl;
		std::cout << "size: " << d->size << std::endl;
		std::cout << "type: " << d->type << std::endl;
		std::cout << "lsize: " << d->lsize << std::endl;
		std::cout << "rsize: " << d->rsize << std::endl;
		std::cout << "version: " << d->version << std::endl;
	}

	delete tagger;
}

void test_distance(const std::wstring& target, const std::wstring& root, const std::wstring& dest)
{
	std::locale::global(std::locale(""));

	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
	nlp::text_ranker text_ranker;
	std::vector<std::string> stop_words_pathes;
	for (auto& path : std::filesystem::directory_iterator("dictionary/stopwords"))
		stop_words_pathes.push_back(std::filesystem::absolute(path).string());
	text_ranker.load_stop_words(stop_words_pathes);
	text_ranker.load_morphological_analyzer(std::filesystem::absolute("dictionary/mecabrc").string(),
		std::filesystem::absolute("dictionary/mecab-ko-dic").string());

	std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
	std::wcout << L"load analyzer : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;

	start = std::chrono::system_clock::now();

	std::vector<std::wstring> pathes;
	for (auto& path : std::filesystem::recursive_directory_iterator(root))
	{
		if (path.is_directory())
			continue;
		auto absolute = std::filesystem::absolute(path).generic_wstring();
		if (absolute == target)
			continue;
		pathes.push_back(std::filesystem::absolute(path).generic_wstring());
	}

	size_t id = 0;
	std::vector< std::vector<std::wstring> > documents;
	std::vector<std::wstring> target_document;

	std::wcout << std::filesystem::absolute(target).generic_wstring() << std::endl;
	std::wstring document;
	if (!extract_text(document, std::filesystem::absolute(target).generic_wstring()))
		return;
	if (!document.empty())
	{
		auto local_start = std::chrono::system_clock::now();
		std::vector< std::pair< std::wstring, double> > keywords;
		std::vector<std::wstring> out_keywords;
		text_ranker.key_words(document, keywords, 1000);
		for (auto& keyword : keywords)
		{
			out_keywords.push_back(keyword.first);
		}
		target_document = out_keywords;
		auto local_end = std::chrono::system_clock::now();
		std::wcout << L"target : " << std::chrono::duration_cast<std::chrono::milliseconds>(local_end - local_start).count() << L"ms]" << std::endl;
	}

	for (auto& path : pathes)
	{
		std::wcout << std::filesystem::absolute(path).generic_wstring() << std::endl;
		std::wstring document;
		if (!extract_text(document, std::filesystem::absolute(path).generic_wstring()))
			continue;
		if (!document.empty())
		{
			++id;
			
			auto local_start = std::chrono::system_clock::now();
			std::vector< std::pair< std::wstring, double> > keywords;
			std::vector<std::wstring> out_keywords;
			text_ranker.key_words(document, keywords, 1000);
			for (auto& keyword : keywords)
			{
				out_keywords.push_back(keyword.first);
			}
			documents.emplace_back(std::move(out_keywords));
			auto local_end = std::chrono::system_clock::now();
			std::wcout << L"[" << id << "/" << pathes.size() << L"]";
			std::wcout << L"key words : " << std::chrono::duration_cast<std::chrono::milliseconds>(local_end - local_start).count() << L"ms]" << std::endl;			
		}
	}
	end = std::chrono::system_clock::now();

	if (std::wcout.bad())
		std::wcout.clear();
	std::wcout << L"open & extract & keywords : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << L"ms]" << std::endl;


	std::map<double, std::wstring, std::greater<double> > sorted_doc;
	for (size_t id = 0; id < documents.size(); ++id)
	{
		auto sim = nlp::jaccard_similarity_t<std::wstring>::index(target_document, documents[id]);
		sorted_doc.insert(std::make_pair(sim, pathes[id]));
	};
	
	std::ofstream out(dest);

	out << "[target]" << std::endl;
	out << to_utf8(target) << std::endl << std::endl;

	out << "[jaccard similarity]" << std::endl;
	for (auto& doc : sorted_doc)
	{
		try
		{
			out << doc.first << " : " << to_utf8(doc.second) << std::endl;
		}
		catch (const std::exception&)
		{
		}
	}

	out << std::endl;

	boost::accumulators::accumulator_set<double, boost::accumulators::features<boost::accumulators::tag::mean> > acc;
	for (auto& doc : sorted_doc)
		acc(doc.first);

	std::vector< std::pair< std::wstring, double> > means;
	double mean = boost::accumulators::mean(acc) - std::numeric_limits<double>::epsilon();

	out << "[extract means] : " << mean << std::endl;
	for (auto& doc : sorted_doc)
	{
		if (doc.first > mean)
		{
			means.push_back(std::make_pair(doc.second, doc.first));
			try
			{
				out << means.back().second << " : " << to_utf8(means.back().first) << std::endl;
			}
			catch (const std::exception&)
			{
			}
		}
	}
	out.close();
}

#include <atlstr.h>

int main(int argc, char* argv[])
{
	try
	{
		//test_pdf();
		//test_distance(L"F:/jaccard/머신러닝.hwp", L"F:/jaccard/", L"f:/sombra/jaccard.txt");
		//test_distance(L"d:/ci/hwp/자전거안전사고예방수칙.hwp", L"d:/ci/hwp/", L"f:/sombra/jaccard_hwp.txt");
		//test_mecab();
		//test_hwpx();
		//test_summary_directory(L"D:/ci/docx/", L"F:/sombra/docx/");
		//test_summary_directory(L"D:/ci/hwp/", L"F:/sombra/hwp/");
		//test_summary_directory(L"F:/sombra/confidential/", L"F:/sombra/result_confidential/");
		test_summary();
		//test_directory2();
		//test_directory3();
		//test_directory4();
		//test_docx();
		//return 0;

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
		std::string mecab_rc;
		std::getline(arg, mecab_rc);
		std::string mecab_dic;
		std::getline(arg, mecab_dic);
		arg.close();

		cmd_summary(to_wchar(src), to_wchar(result), to_wchar(stop_words), to_wchar(mecab_rc), to_wchar(mecab_dic));
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