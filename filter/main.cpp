#include "filter_pch.h"
#include "hwp/hwp50_filter.h"
#include "hwp/hwp30_filter.h"
#include "hwp/hwp30_syntax.h"
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

void test_decompress_save()
{
	filter::hwp50::filter_t filter;
	filter.decompress_save("d:/filter/sample_compress.hwp", "d:/filter/sample_compress.hwp.hwp");
	filter.decompress_save("d:/filter/text.hwp", "d:/filter/text.hwp.hwp");
	filter.decompress_save("d:/filter/sample2.hwp", "d:/filter/sample2.hwp.hwp");
}

void test_extract_all_texts()
{
	filter::hwp50::filter_t filter;
	//print(filter.extract_all_texts(to_utf8(u"d:/filter/resume.hwp")));
	//print(filter.extract_all_texts(to_utf8(u"d:/filter/tab.hwp")));
	//print(filter.extract_all_texts(to_utf8(u"d:/filter/table_nested.hwp")));
	print(filter.extract_all_texts(to_utf8(u"d:/filter/table_cr.hwp")));
	//print( filter.extract_all_texts(to_utf8(u"d:/filter/[여성부]김현진_우리아이지키기_종합대책(08.5.27).hwp")) );
	//print( filter.extract_all_texts(to_utf8(u"d:/filter/[국립공원관리공단]조선희_북한산독립유공자묘역정비[이미지].hwp")) );
	//print( filter.extract_all_texts("d:/filter/sample2.hwp") );
	//print( filter.extract_all_texts("d:/filter/sample_compress.hwp") );
	//print( filter.extract_all_texts("d:/filter/text.hwp") );
}

void test_replace_privacy()
{
	std::wregex resident_registration_number(L"(?:[0-9]{2}(?:0[1-9]|1[0-2])(?:0[1-9]|[1,2][0-9]|3[0,1]))-[1-4][0-9]{6}");
	filter::hwp50::filter_t filter;

	print( filter.extract_all_texts(to_utf8(u"d:/filter/privacy.hwp")) );
	filter.replace_privacy(to_utf8(u"d:/filter/privacy.hwp"), to_utf8(u"d:/filter/privacy.hwp.hwp"), resident_registration_number, u'*');
	print(filter.extract_all_texts(to_utf8(u"d:/filter/privacy.hwp.hwp")));
}

void test_hwp30()
{
	typedef filter::hwp30::filter_t filter_t;
	filter_t filter;
	print(filter.extract_all_texts(to_utf8(u"d:/filter/hwp30/tab.hwp")));
	auto document = filter.open(to_utf8(u"d:/filter/hwp30/tab.hwp"));
	filter.save(document, to_utf8(u"d:/filter/hwp30/tab.hwp.hwp"));
	//print(filter.extract_all_texts(to_utf8(u"d:/filter/hwp30/table2.hwp")));
	//auto document = filter.open(to_utf8(u"d:/filter/hwp30/table2.hwp"));
	//filter.save(document, to_utf8(u"d:/filter/hwp30/table2.hwp.hwp"));
	
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

int main()
{
	//test_decompress_save();
	test_extract_all_texts();
	//test_replace_privacy();
	//test_hwp30();
	return 0;
}