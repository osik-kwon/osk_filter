#pragma once
#include <string>
#include <bitset>
#include <memory>
#include "traits/binary_traits.h"
#include "traits/compound_file_binary_traits.h"
#include "io/binary_iostream.h"

namespace filter
{
namespace doc
{
	typedef binary_traits::byte_t byte_t;
	typedef binary_traits::buffer_t buffer_t;
	typedef binary_traits::bufferstream bufferstream;
	typedef binary_traits::streamsize streamsize;

	struct FontSignature
	{
		uint32_t UnicodeSubsetBitfield0;
		uint32_t UnicodeSubsetBitfield1;
		uint32_t UnicodeSubsetBitfield2;
		uint32_t UnicodeSubsetBitfield3;
		uint32_t CodePageBitfield0;
		uint32_t CodePageBitfield1;
	};

	class FontFamilyName
	{
	public:
		FontFamilyName(streamsize length);

		uint8_t ffid;

		// TODO: implement
		/// <summary>
		/// When true, font is a TrueType font
		/// </summary>
		//bool fTrueType;

		/// <summary>
		/// Font family id
		/// </summary>
		//uint8_t ff;

		/// <summary>
		/// Pitch request
		/// </summary>
		//uint8_t prq;

		/// <summary>
		/// Base weight of font
		/// </summary>
		int16_t wWeight;

		/// <summary>
		/// Character set identifier
		/// </summary>
		uint8_t chs;

		uint8_t unknown1;

		/// <summary>
		/// Panose
		/// </summary>
		buffer_t panose;

		/// <summary>
		/// Font sinature
		/// </summary>
		FontSignature fs;

		/// <summary>
		/// Name of font
		/// </summary>
		buffer_t xszFtn;

		/// <summary>
		/// Alternative name of the font
		/// </summary>
		buffer_t xszAlt;
		
		// length of FontFamilyName
		streamsize length;
	};

	// serializers
	bufferstream& operator >> (bufferstream& stream, FontFamilyName& data);
	bufferstream& operator << (bufferstream& stream, const FontFamilyName& data);
}
}