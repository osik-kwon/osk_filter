#include "hwp/hwp50_filter.h"
#include "hwp/hwp30_filter.h"
#include "hwp/hwp30_syntax.h"
#include "hwp/hwpx_filter.h"
#include "hwp/hwpml_filter.h"
#include "io/open_package_conventions.h"
#include "locale/charset_encoder.h"

void print(const filter::hwp50::filter_t::sections_t& sections)
{
	setlocale(LC_ALL, "korean");
	for (auto& section : sections)
	{
		for (auto& para : section)
		{
			if (!para.empty() && para.size() != 1)
			{
				std::wcout << para;
				if (std::wcout.bad())
					std::wcout.clear();
			}
		}
	}
}

void print(const filter::hwp50::filter_t::section_t& section)
{
	setlocale(LC_ALL, "korean");
	for (auto& para : section)
	{
		if (!para.empty() && para.size() != 1)
		{
			std::wcout << para;
			if (std::wcout.bad())
				std::wcout.clear();
		}
	}
}

void test_hwp30()
{
	typedef filter::hwp30::filter_t filter_t;
	{
		//filter_t filter;
		//print(filter.extract_all_texts(to_utf8(u"d:/filter/hwp30/CSP-3852.hwp")));
		//auto document = filter.open(to_utf8(u"d:/filter/hwp30/CSP-3852.hwp"));
		//filter.save(document, to_utf8(u"d:/filter/hwp30/CSP-3852.hwp.hwp"));
	}
	{
		filter_t filter;
		print(filter.extract_all_texts(to_utf8(u"d:/filter/hwp30/pic.hwp")));
		auto document = filter.open(to_utf8(u"d:/filter/hwp30/pic.hwp"));
		filter.save(document, to_utf8(u"d:/filter/hwp30/pic.hwp.hwp"));
	}
	
	{
		filter_t filter;
		print(filter.extract_all_texts(to_utf8(u"d:/filter/hwp30/tab.hwp")));
		auto document = filter.open(to_utf8(u"d:/filter/hwp30/tab.hwp"));
		filter.save(document, to_utf8(u"d:/filter/hwp30/tab.hwp.hwp"));
	}
	
	{
		filter_t filter;
		print(filter.extract_all_texts(to_utf8(u"d:/filter/hwp30/table2.hwp")));
		auto document = filter.open(to_utf8(u"d:/filter/hwp30/table2.hwp"));
		filter.save(document, to_utf8(u"d:/filter/hwp30/table2.hwp.hwp"));
	}
	
	//print(filter.extract_all_texts(to_utf8(u"d:/filter/hwp30/table3.hwp")));
	//print(filter.extract_all_texts(to_utf8(u"d:/filter/hwp30/hwp97_hangul.hwp")));
	//print(filter.extract_all_texts(to_utf8(u"d:/filter/hwp30/table.hwp")));
	//auto document = filter.open(to_utf8(u"d:/filter/hwp30/hwp97_all.hwp"));
	//filter.save(document, to_utf8(u"d:/filter/hwp30/hwp97_all.hwp.hwp"));

	//filter.save(to_utf8(u"d:/filter/hwp30/0F0034.hwp"), to_utf8(u"d:/filter/hwp30/0F0034.hwp.hwp"));
	//filter.save(to_utf8(u"d:/filter/hwp30/hwp97_hangul.hwp"), to_utf8(u"d:/filter/hwp30/hwp97_hangul.hwp.hwp"));
	//print(filter.extract_all_texts(to_utf8(u"d:/filter/hwp30/hwp97_hangul.hwp")));
	//print(filter.extract_all_texts(to_utf8(u"d:/filter/hwp30/hwp97_1.hwp")));
}

#include <iostream>
#include <xlnt/xlnt.hpp>

#include <xlnt/detail/serialization/open_stream.hpp>
#include <xlnt/detail/serialization/zstream.hpp>
#include <xlnt/utils/path.hpp>
#include <xlnt/utils/exceptions.hpp>

#include <xml/pugixml.hpp>

void test_opc(const xlnt::path& open_path, const xlnt::path& save_path)
{
	using namespace xlnt::detail;
	std::ifstream source;
	open_stream(source, open_path.string());
	if (!source.good())
		throw xlnt::exception("file not found " + open_path.string());

	std::map< std::string, pugi::xml_document > src_parts;
	std::unique_ptr<izstream> iarchive;
	iarchive.reset(new izstream(source));
	auto files = iarchive->files();
	for (auto file : files)
	{
		auto stream_buf = iarchive->open(file);
		std::istream stream(stream_buf.get());

		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load(stream, pugi::parse_default, pugi::xml_encoding::encoding_auto);
		if (!result)
			continue;
		//const std::string query = "//w:t[text()]"; // docx
		const std::string query = "//hp:t[text()]"; // hwpx
		pugi::xpath_node_set texts = doc.select_nodes(query.c_str());
		for (pugi::xpath_node_set::const_iterator it = texts.begin(); it != texts.end(); ++it)
		{
			std::string text(it->node().first_child().value());
			if (!text.empty())
				std::cout << text << "\n";
		}
		src_parts.emplace(std::move(file.string()), std::move(doc));
	}
	source.close();

	std::ofstream ostream;
	open_stream(ostream, save_path.string());
	std::unique_ptr<ozstream> oarchive;
	oarchive.reset(new ozstream(ostream));
	for (auto file : files)
	{
		std::unique_ptr<std::streambuf> part_buf;
		part_buf.reset();
		part_buf = oarchive->open(file);

		std::ostream part_stream(nullptr);
		part_stream.rdbuf(part_buf.get());

		auto part_doc = src_parts.find(file.string());
		if (part_doc == src_parts.end())
			continue;
		pugi::xml_document& doc = part_doc->second;
		doc.save(part_stream, PUGIXML_TEXT("\t"), pugi::parse_default, pugi::xml_encoding::encoding_auto);
	}
	oarchive.reset(nullptr);
	ostream.close();
}


void test_hwpml()
{
	typedef filter::hml::filter_t filter_t;
	{
		filter_t filter;
		auto src = filter.open(to_utf8(u"d:/filter/privacy.hml"));
		print(filter.extract_all_texts(src));

		std::wregex resident_registration_number(L"(?:[0-9]{2}(?:0[1-9]|1[0-2])(?:0[1-9]|[1,2][0-9]|3[0,1]))-[1-4][0-9]{6}");
		print(filter.search_privacy({ resident_registration_number }, src));
	}
	{
		filter_t filter;
		auto src = filter.open(to_utf8(u"d:/filter/hml.hml"));
		print(filter.extract_all_texts(src));
		filter.save(to_utf8(u"d:/filter/hml.export.hml"), src);
		auto dest = filter.open(to_utf8(u"d:/filter/hml.export.hml"));
		print(filter.extract_all_texts(dest));
	}
	{
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
	typedef filter::hwpx::filter_t filter_t;
	{
		filter_t filter;
		auto src = filter.open(to_utf8(u"d:/filter/hwpx.hwpx"));
		print(filter.extract_all_texts(src));
		filter.save(to_utf8(u"d:/filter/hwpx.export.hwpx"), src);
		auto dest = filter.open(to_utf8(u"d:/filter/hwpx.export.hwpx"));
		print(filter.extract_all_texts(dest));
	}
	{
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

void test_hwp50()
{
	typedef filter::hwp50::filter_t filter_t;
	{
		filter_t filter;
		auto src = filter.open(to_utf8(u"d:/filter/hwp50.hwp"));
		print(filter.extract_all_texts(src));
		filter.save(to_utf8(u"d:/filter/hwp50.export.hwp"), src);
		auto dest = filter.open(to_utf8(u"d:/filter/hwp50.export.hwp"));
		print(filter.extract_all_texts(dest));
	}
	{
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

int main()
{
	try
	{
		//test_opc(xlnt::path("d:/filter/simple.docx"), xlnt::path("d:/filter/simple.docx.docx"));
		//test_opc(xlnt::path("d:/filter/simple.hwpx"), xlnt::path("d:/filter/simple.hwpx.hwpx"));

		//xlnt::workbook wb(xlnt::path("d:/filter/sample.xlsx"));
		//xlnt::worksheet ws = wb.active_sheet();
		//ws.cell("A1").value(5);
		//ws.cell("B2").value("string data");
		//ws.cell("C3").formula("=RAND()");
		//ws.merge_cells("C3:C4");
		//ws.freeze_panes("B2");
		//wb.save("d:/filter/sample.export.xlsx");
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	test_hwp50();
	//test_hwpml();
	//test_hwpx();
	//test_decompress_save();
	//test_extract_all_texts();
	//test_replace_privacy();
	//test_hwp30();
	return 0;
}