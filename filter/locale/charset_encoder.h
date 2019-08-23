#pragma once
#include <string>

// thank you! https://github.com/ww898/utf-cpp/blob/master/README.md
// license : https://github.com/ww898/utf-cpp/blob/master/LICENSE.md

extern std::string to_utf8(const std::u16string& str);
extern std::string to_utf8(const std::u32string& str);
extern std::string to_utf8(const std::wstring& str);

extern std::u16string to_utf16(const std::string& str);
extern std::u16string to_utf16(const std::u32string& str);
extern std::u16string to_utf16(char32_t utf32);
extern std::u16string to_utf16(const std::wstring& str);

extern std::u32string to_utf32(const std::string& str);
extern std::u32string to_utf32(const std::u16string& str);
extern std::u32string to_utf32(char16_t utf16);
extern std::u32string to_utf32(const std::wstring& str);

extern std::wstring to_wchar(const std::string& str);
extern std::wstring to_wchar(const std::u16string& str);
extern std::wstring to_wchar(const std::u32string& str);