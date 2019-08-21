#include "locale/charset_encoder.h"

std::u16string u8_to_u16(const std::string& src)
{
	return filter::charset::u8_to_u16(src);
}

std::string u16_to_u8(const std::u16string& src)
{
	return filter::charset::u16_to_u8(src);
}