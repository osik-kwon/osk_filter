#include "filter_pch.h"
#include "hwp/hwp50_filter.h"
#include "locale/charset_encoder.h"

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
	//auto sections = filter.extract_all_texts(u16_to_u8(u"d:/filter/[������]������_�츮������Ű��_���մ�å(08.5.27).hwp"));
	//auto sections = filter.extract_all_texts(u16_to_u8(u"d:/filter/[����������������]������_���ѻ굶�������ڹ�������[�̹���].hwp"));
	auto sections = filter.extract_all_texts("d:/filter/sample2.hwp");
	//auto sections = filter.extract_all_texts("d:/filter/sample_compress.hwp");
	//auto sections = filter.extract_all_texts("d:/filter/text.hwp");
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

void test_replace_all_texts()
{
	filter::hwp50::filter_t filter;
	filter.replace_all_texts("d:/filter/sample_compress.hwp", "d:/filter/sample_compress.hwp.hwp", L'*');
	filter.replace_all_texts("d:/filter/text.hwp", "d:/filter/text.hwp.hwp", L'*');
	filter.replace_all_texts("d:/filter/sample2.hwp", "d:/filter/sample2.hwp.hwp", L'*');
}


void test_replace_privacy()
{
	std::wregex resident_registration_number(L"(?:[0-9]{2}(?:0[1-9]|1[0-2])(?:0[1-9]|[1,2][0-9]|3[0,1]))-[1-4][0-9]{6}");
	filter::hwp50::filter_t filter;
	filter.replace_privacy(u16_to_u8(u"d:/filter/privacy.hwp"), u16_to_u8(u"d:/filter/privacy.hwp.hwp"), resident_registration_number, u'*');
	filter.replace_privacy(u16_to_u8(u"d:/filter/��������.hwp"), u16_to_u8(u"d:/filter/��������.hwp.hwp"), resident_registration_number, u'*');
}

int main()
{
	test_decompress_save();
	test_extract_all_texts();
	//test_replace_all_texts();
	//test_replace_privacy();
	return 0;
}