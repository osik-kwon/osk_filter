#include "filter_pch.h"
#include "hwp/hwp30_syntax.h"
#include "locale/hchar_converter.h"
#include "locale/charset_encoder.h"

namespace filter
{
namespace hwp30
{
	struct control_builder
	{
		typedef control_code_t::control_t control_t;
		enum control_ids : control_t
		{
			table = 10,
			para_break = 13
		};
		typedef paragraph_t::controls_t controls_t;
		control_builder(){}
		static uint16_t push_back(control_t code, controls_t& controls)
		{
			switch (code)
			{
			case table:
				controls.push_back(std::make_unique<table_t>(code));
				return 4;
			case para_break:
			default:
				controls.push_back(std::make_unique<hchar_t>(code));
				return 1;
			}
			return 1;
		}
	};

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
		typedef face_name_list_t::face_name_t face_name_t;
		typedef face_name_list_t::face_names_t face_names_t;
		typedef face_name_list_t::family_list_t family_list_t;

		for (size_t family = 0; family < 7; ++family)
		{
			int16_t face_count = binary_io::read_int16(stream);
			face_names_t face_names;
			for (uint16_t id = 0; id < face_count; ++id)
				face_names.push_back( binary_io::read(stream, 40) );
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
				binary_io::write(stream, face_name);
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

	bufferstream& operator >> (bufferstream& stream, para_header_t& data)
	{
		data.prev_para_shape_id = binary_io::read_uint8(stream);
		data.char_count = binary_io::read_uint16(stream);
		data.line_count = binary_io::read_uint16(stream);
		data.char_shape_id = binary_io::read_uint8(stream);
		data.etc_flag = binary_io::read_uint8(stream);
		data.control_code = binary_io::read_uint32(stream);
		data.style_id = binary_io::read_uint8(stream);

		stream >> data.char_shape;
		if (data.prev_para_shape_id == 0 &&
			data.char_count > 0 )
			stream >> data.para_shape;
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const para_header_t& data)
	{
		binary_io::write_uint8(stream, data.prev_para_shape_id);
		binary_io::write_uint16(stream, data.char_count);
		binary_io::write_uint16(stream, data.line_count);
		binary_io::write_uint8(stream, data.char_shape_id);
		binary_io::write_uint8(stream, (uint8_t)data.etc_flag.to_ulong());
		binary_io::write_uint32(stream, data.control_code);
		binary_io::write_uint8(stream, data.style_id);

		stream << data.char_shape;
		if (data.prev_para_shape_id == 0 &&
			data.char_count > 0)
			stream << data.para_shape;
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, line_segment_list_t& data)
	{
		data.body = binary_io::read(stream, data.size());
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const line_segment_list_t& data)
	{
		binary_io::write(stream, data.body);
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, char_shape_info_list_t& data)
	{
		for (uint16_t id = 0; id < data.char_count; ++id)
		{
			char_shape_info_t char_shape_info;
			char_shape_info.flag = binary_io::read_uint8(stream);
			if (char_shape_info.flag != 1)
				stream >> char_shape_info.char_shape;
			data.char_shape_info_list.push_back(std::move(char_shape_info));
		}
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const char_shape_info_list_t& data)
	{
		for (auto& char_shape_info : data.char_shape_info_list)
		{
			binary_io::write_uint8(stream, char_shape_info.flag);
			if (char_shape_info.flag != 1)
				stream << char_shape_info.char_shape;
		}
		return stream;
	}

	// control codes
	bufferstream& hchar_t::read(bufferstream& stream)
	{
		utf32 = charset::hchar_converter::hchar_to_wchar(code);
		return stream;
	}

	bufferstream& hchar_t::write(bufferstream& stream)
	{
		binary_io::write_uint16(stream, code);
		return stream;
	}

	bufferstream& table_t::read(bufferstream& stream)
	{
		reserved = binary_io::read_uint32(stream);
		end_code = binary_io::read_uint16(stream);

		data = binary_io::read(stream, 80);
		cell_count = binary_io::read_uint16(stream);
		protect = binary_io::read_uint16(stream);

		for (uint16_t i = 0; i < cell_count; ++i)
		{
			cell_t cell;
			cell.data = binary_io::read(stream, 27);
			cells.push_back(std::move(cell));
		}

		para_lists.resize(cell_count);
		for (uint16_t i = 0; i < cell_count; ++i)
		{
			stream >> para_lists[i];
		}
		stream >> caption;
		return stream;
	}

	bufferstream& table_t::write(bufferstream& stream)
	{
		binary_io::write_uint16(stream, code);
		binary_io::write_uint32(stream, reserved);
		binary_io::write_uint16(stream, end_code);
		binary_io::write(stream, data);
		binary_io::write_uint16(stream, cell_count);
		binary_io::write_uint16(stream, protect);
		for (auto& cell : cells)
		{
			binary_io::write(stream, cell.data);
		}

		for (auto& para_list : para_lists)
		{
			stream << para_list;
		}
		stream << caption;
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, paragraph_t& data)
	{
		stream >> data.para_header;
		if (data.para_header.empty())
			return stream; // IMPORTANT!
		data.line_segment_list.line_count = data.para_header.line_count; // IMPORTANT!
		data.char_shape_info_list.char_count = data.para_header.char_count; // IMPORTANT!
		stream >> data.line_segment_list;
		if(data.para_header.char_shape_id != 0)
			stream >> data.char_shape_info_list;
		
		uint16_t count = data.para_header.char_count;
		for (uint16_t offset = 0; offset < count;)
		{
			uint16_t code = binary_io::read_uint16(stream);
			offset += control_builder::push_back(code, data.controls);
			data.controls.back()->read(stream);
		}

		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const paragraph_t& data)
	{
		stream << data.para_header;
		if (data.para_header.empty())
			return stream; // IMPORTANT!
		stream << data.line_segment_list;
		stream << data.char_shape_info_list;
		for (auto& control : data.controls)
			control->write(stream);
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, paragraph_list_t& data)
	{
		bool end_of_para = false;
		do {
			paragraph_t para;
			stream >> para;
			end_of_para = para.para_header.empty();
			data.para_list.push_back(std::move(para));
		} while (!end_of_para);
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const paragraph_list_t& data)
	{
		for (auto& para : data.para_list)
		{
			stream << para;
		}
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, document_header_t& data)
	{
		data.signature = binary_io::read(stream, 30);
		stream >> data.doc_info;
		stream >> data.doc_summary;
		if (data.doc_info.info_block_length != 0)
		{
			data.info_block.info_block_length = data.doc_info.info_block_length;
			stream >> data.info_block;
		}
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const document_header_t& data)
	{
		binary_io::write(stream, data.signature);
		stream << data.doc_info;
		stream << data.doc_summary;
		if (data.doc_info.info_block_length != 0)
			stream << data.info_block;
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, document_body_t& data)
	{
		stream >> data.face_name_list;
		stream >> data.style_list;
		stream >> data.sections;
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const document_body_t& data)
	{
		stream << data.face_name_list;
		stream << data.style_list;
		stream << data.sections;
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, document_tail_t& data)
	{
		// TODO: implement
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const document_tail_t& data)
	{
		binary_io::write(stream, data.trailer);
		return stream;
	}
}
}