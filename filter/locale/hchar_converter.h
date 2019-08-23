#pragma once
#include <cstdint>

namespace charset
{
	class hchar_converter
	{
	public:
		hchar_converter() = default;

		static bool is_pua(uint32_t code);
		static uint32_t	hchar_to_wchar(uint16_t hchar);
		static uint16_t	kschar_to_hchar(uint16_t code);
		static uint16_t	wchar_to_hchar(uint16_t code);
		static uint16_t	wspecial_to_hspecial(uint16_t code);
	private:
		static uint32_t	general_hchar_to_wchar(uint16_t hchar);
		static uint16_t	hanja_hchar_to_wchar(uint16_t hchar);
		static uint16_t	hangul_hchar_to_wchar(uint16_t hchar);
		static uint16_t	distance(uint16_t hchar, uint16_t wchar_start, uint16_t hchar_start);
	};
}