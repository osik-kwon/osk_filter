#include "filter_pch.h"
#include "word/doc_font_family_name.h"

namespace filter
{
namespace doc
{
	FontFamilyName::FontFamilyName(streamsize length) : length(length)
	{}

	streamsize searchTerminationZero(bufferstream& stream)
	{
		streamsize begin = stream.tellg();
		while (binary_io::read_uint16(stream) != 0);
		streamsize end = stream.tellg();
		stream.seekg(begin, stream.beg);
		return end;
	}

	// serializers
	bufferstream& operator >> (bufferstream& stream, FontFamilyName& data)
	{
		streamsize startPos = stream.tellg();
		data.ffid = binary_io::read_uint8(stream);

		// TODO: implement
		//FFID
		/*
		int ffid = (int)_reader.ReadByte();

		int req = ffid;
		req = req << 6;
		req = req >> 6;
		this.prq = (byte)req;

		this.fTrueType = Utils.BitmaskToBool(ffid, 0x04);

		int family = ffid;
		family = family << 1;
		family = family >> 4;
		this.ff = (byte)family;
		*/
		data.wWeight = binary_io::read_int16(stream);

		data.chs = binary_io::read_uint8(stream);

		//skip byte 5
		data.unknown1 = binary_io::read_uint8(stream);

		//read the 10 bytes panose
		data.panose = binary_io::read(stream, 10);

		//read the 24 bytes FontSignature
		data.fs.UnicodeSubsetBitfield0 = binary_io::read_uint32(stream);
		data.fs.UnicodeSubsetBitfield1 = binary_io::read_uint32(stream);
		data.fs.UnicodeSubsetBitfield2 = binary_io::read_uint32(stream);
		data.fs.UnicodeSubsetBitfield3 = binary_io::read_uint32(stream);
		data.fs.CodePageBitfield0 = binary_io::read_uint32(stream);
		data.fs.CodePageBitfield1 = binary_io::read_uint32(stream);

		//read the next \0 terminated string
		streamsize strStart = stream.tellg();
		streamsize strEnd = searchTerminationZero(stream);
		data.xszFtn = binary_io::read(stream, strEnd - strStart);

		streamsize readBytes = stream.tellg() - startPos;
		if (readBytes < data.length)
		{
			//read the next \0 terminated string

			streamsize strStart = stream.tellg();
			streamsize strEnd = searchTerminationZero(stream);
			data.xszAlt = binary_io::read(stream, strEnd - strStart);
		}
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const FontFamilyName& data)
	{
		streamsize startPos = stream.tellg();
		binary_io::write_uint8(stream, data.ffid);
		binary_io::write_int16(stream, data.wWeight);

		binary_io::write_uint8(stream, data.chs);

		//skip byte 5
		binary_io::write_uint8(stream, data.unknown1);

		//read the 10 bytes panose
		binary_io::write(stream, data.panose);

		//read the 24 bytes FontSignature
		binary_io::write_uint32(stream, data.fs.UnicodeSubsetBitfield0);
		binary_io::write_uint32(stream, data.fs.UnicodeSubsetBitfield1);
		binary_io::write_uint32(stream, data.fs.UnicodeSubsetBitfield2);
		binary_io::write_uint32(stream, data.fs.UnicodeSubsetBitfield3);
		binary_io::write_uint32(stream, data.fs.CodePageBitfield0);
		binary_io::write_uint32(stream, data.fs.CodePageBitfield1);

		//read the next \0 terminated string
		binary_io::write(stream, data.xszFtn);
		//read the next \0 terminated string
		if( data.xszAlt.size() > 0 )
			binary_io::write(stream, data.xszAlt);
		return stream;
	}
}
}