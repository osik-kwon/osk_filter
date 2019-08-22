#include "filter_pch.h"
#include "hwp/hwp30_syntax.h"

namespace filter
{
namespace hwp30
{
	// serializers
	bufferstream& operator >> (bufferstream& stream, doc_info_t& data)
	{
		data.body = binary_io::read(stream, data.size() - sizeof(data.info_block_length)); // IMPORTANT!

		data.crypted = data.body[96];
		data.compressed = data.body[124];
		data.info_block_length = binary_io::read_uint16(stream); // IMPORTANT!
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const doc_info_t& data)
	{
		binary_io::write(stream, data.body);
		binary_io::write_uint16(stream, data.info_block_length);
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, doc_summary_t& data)
	{
		/*
		const std::streamsize length = 112 / 2;
		auto title = binary_io::read_u16string(stream, length);
		auto subject = binary_io::read_u16string(stream, length);
		auto authur = binary_io::read_u16string(stream, length);
		auto date = binary_io::read_u16string(stream, length);
		auto keyword = binary_io::read_u16string(stream, length * 2);
		auto etc = binary_io::read_u16string(stream, length * 3);
		*/
		data.body = binary_io::read(stream, data.size());
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const doc_summary_t& data)
	{
		binary_io::write(stream, data.body);
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, info_block_t& data)
	{
		data.body = binary_io::read(stream, data.size());
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const info_block_t& data)
	{
		binary_io::write(stream, data.body);
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, face_name_list_t& data)
	{
		typedef std::string face_name_t;
		typedef std::vector<face_name_t> face_names_t;
		typedef std::vector<face_names_t> family_list_t;

		for (size_t family = 0; family < 7; ++family)
		{
			int16_t face_count = binary_io::read_int16(stream);
			face_names_t face_names;
			for (uint16_t id = 0; id < face_count; ++id)
				face_names.push_back( binary_io::read_string(stream, 40) );
			data.family_list.push_back(std::move(face_names));
		}
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const face_name_list_t& data)
	{
		for ( auto& face_names : data.family_list )
		{
			binary_io::write_uint16(stream, (uint16_t)face_names.size());
			for ( auto& face_name : face_names )
				binary_io::write_string(stream, face_name);
		}
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, char_shape_t& data)
	{
		data.body = binary_io::read(stream, data.size());
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const char_shape_t& data)
	{
		binary_io::write(stream, data.body);
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, para_shape_t& data)
	{
		data.body = binary_io::read(stream, data.size());
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const para_shape_t& data)
	{
		binary_io::write(stream, data.body);
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, style_list_t& data)
	{
		int16_t style_count = binary_io::read_int16(stream);
		for (uint16_t id = 0; id < style_count; ++id)
		{
			style_t style;
			style.name = binary_io::read_string(stream, 20);
			stream >> style.char_shape;
			stream >> style.para_shape;
			data.styles.push_back(std::move(style));
		}
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const style_list_t& data)
	{
		binary_io::write_uint16(stream, (uint16_t)data.styles.size());
		for (auto& style : data.styles)
		{
			binary_io::write_string(stream, style.name);
			stream << style.char_shape;
			stream << style.para_shape;
		}
		return stream;
	}
}
}