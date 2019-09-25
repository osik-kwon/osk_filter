#include "hwp/hwp50_filter.h"
#include "hwp/hwp30_filter.h"
#include "hwp/hwpx_filter.h"
#include "hwp/hwpml_filter.h"
#include "txt/txt_filter.h"
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

void test_txt()
{
	std::cout << "===== txt test =====" << std::endl;
	typedef filter::txt::filter_t filter_t;
	{
		std::cout << "===== txt open/save test =====" << std::endl;
		filter_t filter;
		std::cout << filter.detect_charset(to_utf8(u"d:/filter/utf8_LF.txt")) << std::endl;
		auto src = filter.open(to_utf8(u"d:/filter/utf8_LF.txt"));
		print(filter.extract_all_texts(src));
		filter.save(to_utf8(u"d:/filter/utf8_LF.export.txt"), src);
		std::cout << filter.detect_charset(to_utf8(u"d:/filter/utf8_LF.export.txt")) << std::endl;
		auto dest = filter.open(to_utf8(u"d:/filter/utf8_LF.export.txt"));
		print(filter.extract_all_texts(dest));
	}
	{
		std::cout << "===== txt open/save test =====" << std::endl;
		filter_t filter;
		std::cout << filter.detect_charset(to_utf8(u"d:/filter/utf8_CR.txt")) << std::endl;
		auto src = filter.open(to_utf8(u"d:/filter/utf8_CR.txt"));
		print(filter.extract_all_texts(src));
		filter.save(to_utf8(u"d:/filter/utf8_CR.export.txt"), src);
		std::cout << filter.detect_charset(to_utf8(u"d:/filter/utf8_CR.export.txt")) << std::endl;
		auto dest = filter.open(to_utf8(u"d:/filter/utf8_CR.export.txt"));
		print(filter.extract_all_texts(dest));
	}
	{
		std::cout << "===== txt open/save test =====" << std::endl;
		filter_t filter;
		std::cout << filter.detect_charset(to_utf8(u"d:/filter/utf8_LFCR.txt")) << std::endl;
		auto src = filter.open(to_utf8(u"d:/filter/utf8_LFCR.txt"));
		print(filter.extract_all_texts(src));
		filter.save(to_utf8(u"d:/filter/utf8_LFCR.export.txt"), src);
		std::cout << filter.detect_charset(to_utf8(u"d:/filter/utf8_LFCR.export.txt")) << std::endl;
		auto dest = filter.open(to_utf8(u"d:/filter/utf8_LFCR.export.txt"));
		print(filter.extract_all_texts(dest));
	}
	{
		std::cout << "===== txt open/save test =====" << std::endl;
		filter_t filter;
		std::cout << filter.detect_charset(to_utf8(u"d:/filter/euckr.txt")) << std::endl;
		auto src = filter.open(to_utf8(u"d:/filter/euckr.txt"));
		print(filter.extract_all_texts(src));
		filter.save(to_utf8(u"d:/filter/euckr.export.txt"), src);
		std::cout << filter.detect_charset(to_utf8(u"d:/filter/euckr.export.txt")) << std::endl;
		auto dest = filter.open(to_utf8(u"d:/filter/euckr.export.txt"));
		print(filter.extract_all_texts(dest));
	}
	{
		std::cout << "===== txt open/save test =====" << std::endl;
		filter_t filter;
		std::cout << filter.detect_charset(to_utf8(u"d:/filter/utf16_be.txt")) << std::endl;
		auto src = filter.open(to_utf8(u"d:/filter/utf16_be.txt"));
		print(filter.extract_all_texts(src));
		filter.save(to_utf8(u"d:/filter/utf16_be.export.txt"), src);
		std::cout << filter.detect_charset(to_utf8(u"d:/filter/utf16_be.export.txt")) << std::endl;
		auto dest = filter.open(to_utf8(u"d:/filter/utf16_be.export.txt"));
		print(filter.extract_all_texts(dest));
	}
	{
		std::cout << "===== txt open/save test =====" << std::endl;
		filter_t filter;
		std::cout << filter.detect_charset(to_utf8(u"d:/filter/utf16_le.txt")) << std::endl;
		auto src = filter.open(to_utf8(u"d:/filter/utf16_le.txt"));
		print(filter.extract_all_texts(src));
		filter.save(to_utf8(u"d:/filter/utf16_le.export.txt"), src);
		std::cout << filter.detect_charset(to_utf8(u"d:/filter/utf16_le.export.txt")) << std::endl;
		auto dest = filter.open(to_utf8(u"d:/filter/utf16_le.export.txt"));
		print(filter.extract_all_texts(dest));
	}
	/*
	{
		std::cout << "===== txt open/save test =====" << std::endl;
		filter_t filter;
		std::cout << filter.detect_charset(to_utf8(u"d:/filter/ascii.txt")) << std::endl;
		auto src = filter.open(to_utf8(u"d:/filter/ascii.txt"));
		print(filter.extract_all_texts(src));
		filter.save(to_utf8(u"d:/filter/ascii.export.txt"), src);
		std::cout << filter.detect_charset(to_utf8(u"d:/filter/ascii.export.txt")) << std::endl;
		auto dest = filter.open(to_utf8(u"d:/filter/ascii.export.txt"));
		print(filter.extract_all_texts(dest));
	}
	{
		std::cout << "===== txt open/save test =====" << std::endl;
		filter_t filter;
		std::cout << filter.detect_charset(to_utf8(u"d:/filter/utf8_bom.txt")) << std::endl;
		auto src = filter.open(to_utf8(u"d:/filter/utf8_bom.txt"));
		print(filter.extract_all_texts(src));
		filter.save(to_utf8(u"d:/filter/utf8_bom.export.txt"), src);
		std::cout << filter.detect_charset(to_utf8(u"d:/filter/utf8_bom.export.txt")) << std::endl;
		auto dest = filter.open(to_utf8(u"d:/filter/utf8_bom.export.txt"));
		print(filter.extract_all_texts(dest));
	}
	*/
	/*
	{
		std::cout << "===== txt open/save test =====" << std::endl;
		filter_t filter;
		std::cout << filter.detect_charset(to_utf8(u"d:/filter/iso-8859-8.txt")) << std::endl;
		auto src = filter.open(to_utf8(u"d:/filter/iso-8859-8.txt"));
		print(filter.extract_all_texts(src));
		filter.save(to_utf8(u"d:/filter/iso-8859-8.export.txt"), src);
		std::cout << filter.detect_charset(to_utf8(u"d:/filter/iso-8859-8.export.txt")) << std::endl;
		auto dest = filter.open(to_utf8(u"d:/filter/iso-8859-8.export.txt"));
		print(filter.extract_all_texts(dest));
	}
	*/
	/*
	{
		std::cout << "===== txt open/save test =====" << std::endl;
		filter_t filter;
		std::cout << filter.detect_charset(to_utf8(u"d:/filter/euckr_한글경로.txt")) << std::endl;
		auto src = filter.open(to_utf8(u"d:/filter/euckr_한글경로.txt"));
		print(filter.extract_all_texts(src));
		filter.save(to_utf8(u"d:/filter/euckr_한글경로.export.txt"), src);
		std::cout << filter.detect_charset(to_utf8(u"d:/filter/euckr_한글경로.export.txt")) << std::endl;
		auto dest = filter.open(to_utf8(u"d:/filter/euckr_한글경로.export.txt"));
		print(filter.extract_all_texts(dest));
	}
	{
		std::cout << "===== txt search privacy test =====" << std::endl;
		filter_t filter;
		auto src = filter.open(to_utf8(u"d:/filter/privacy.txt"));
		print(filter.extract_all_texts(src));
		std::wregex resident_registration_number(L"(?:[0-9]{2}(?:0[1-9]|1[0-2])(?:0[1-9]|[1,2][0-9]|3[0,1]))-[1-4][0-9]{6}");
		print(filter.search_privacy({ resident_registration_number }, src));
	}
	{
		std::cout << "===== hwpx replace privacy test =====" << std::endl;
		std::wregex resident_registration_number(L"(?:[0-9]{2}(?:0[1-9]|1[0-2])(?:0[1-9]|[1,2][0-9]|3[0,1]))-[1-4][0-9]{6}");
		filter_t filter;

		auto src = filter.open(to_utf8(u"d:/filter/privacy.txt"));
		print(filter.extract_all_texts(src));

		filter.replace_privacy({ resident_registration_number }, u'@', src);
		filter.save(to_utf8(u"d:/filter/privacy.export.txt"), src);

		auto dest = filter.open(to_utf8(u"d:/filter/privacy.export.txt"));
		print(filter.extract_all_texts(dest));
	}
	*/
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