#pragma once
#include <string>
#include <locale>
#include <codecvt>

namespace filter
{
	class charset
	{
	public:
		charset(){}
		static std::u16string u8_to_u16(const std::string& source)
		{
			std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
			return convert.from_bytes(source);
		}

		static std::string u16_to_u8(const std::u16string& source)
		{
			std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
			return convert.to_bytes(source);
		}
	};
}

extern std::u16string u8_to_u16(const std::string& source);
extern std::string u16_to_u8(const std::u16string& source);