#include "hwp/hwp50_filter.h"
#include "hwp/hwp30_filter.h"
#include "hwp/hwpx_filter.h"
#include "hwp/hwpml_filter.h"
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
		filter_t filter;
		print(filter.extract_all_texts(to_utf8(u"d:/filter/hwp30_group3.hwp")));
		auto document = filter.open(to_utf8(u"d:/filter/hwp30_group3.hwp"));
		filter.save(document, to_utf8(u"d:/filter/hwp30_group3.export.hwp"));
	}
	{
		filter_t filter;
		print(filter.extract_all_texts(to_utf8(u"d:/filter/hwp30.hwp")));
		auto document = filter.open(to_utf8(u"d:/filter/hwp30.hwp"));
		filter.save(document, to_utf8(u"d:/filter/hwp30.export.hwp"));
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

int main()
{
	try
	{
		//test_hwp50();
		//test_hwpml();
		//test_hwpx();
		test_hwp30();
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
	return 0;
}