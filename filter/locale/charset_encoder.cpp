#include "filter_pch.h"
#include "locale/charset_encoder.h"
#include "locale/utf_converters.hpp"

std::string to_utf8(const std::u16string& str)
{
	return charset::utf::conv<char>(str);
}

std::string to_utf8(const std::u32string& str)
{
	return charset::utf::conv<char>(str);
}

std::string to_utf8(const std::wstring& str)
{
	return charset::utf::conv<char>(str);
}

std::u16string to_utf16(const std::string& str)
{
	return charset::utf::conv<char16_t>(str);
}

std::u16string to_utf16(const std::u32string& str)
{
	return charset::utf::conv<char16_t>(str);
}

std::u16string to_utf16(char32_t utf32)
{
	std::u32string str = { utf32 };
	return charset::utf::conv<char16_t>(str);
}

std::u16string to_utf16(const std::wstring& str)
{
	return charset::utf::conv<char16_t>(str);
}

std::u32string to_utf32(const std::string& str)
{
	return charset::utf::conv<char32_t>(str);
}

std::u32string to_utf32(const std::u16string& str)
{
	return charset::utf::conv<char32_t>(str);
}

std::u32string to_utf32(char16_t utf16)
{
	std::u16string str = { utf16 };
	return charset::utf::conv<char32_t>(str);
}

std::u32string to_utf32(const std::wstring& str)
{
	return charset::utf::conv<char32_t>(str);
}

std::wstring to_wchar(const std::string& str)
{
	return charset::utf::conv<wchar_t>(str);
}

std::wstring to_wchar(const std::u16string& str)
{
	return charset::utf::conv<wchar_t>(str);
}

std::wstring to_wchar(const std::u32string& str)
{
	return charset::utf::conv<wchar_t>(str);
}