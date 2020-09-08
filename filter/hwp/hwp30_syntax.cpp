#include "filter_pch.h"
#include "hwp/hwp30_syntax.h"
#include "locale/hchar_converter.h"
#include "locale/charset_encoder.h"
#include "io/binary_iostream.h"
#include "io/zlib.h"
#include "io/file_stream.h"

namespace filter
{
namespace hwp30
{
	uint16_t control_builder::push_back(control_t code, controls_t& controls)
	{
		switch (code)
		{
		case 1:
		case 2:
		case 3:
		case 4:
		case 12:
		case 27:
			controls.push_back(std::make_unique<reserved_control_t>(code));
			return 4;
		case syntax_t::field:
			controls.push_back(std::make_unique<field_t>(code));
			return 4;
		case syntax_t::bookmark:
			controls.push_back(std::make_unique<bookmark_t>(code));
			return 4;
		case syntax_t::date_format:
			controls.push_back(std::make_unique<date_format_t>(code));
			return 42;
		case syntax_t::date_code:
			controls.push_back(std::make_unique<date_code_t>(code));
			return 48;
		case syntax_t::tab:
			controls.push_back(std::make_unique<tab_control_t>(code));
			return 4;
		case syntax_t::table:
			controls.push_back(std::make_unique<table_t>(code));
			return 4;
		case syntax_t::drawing_object:
			controls.push_back(std::make_unique<drawing_object_t>(code));
			return 4;
		case syntax_t::line_shape:
			controls.push_back(std::make_unique<line_shape_t>(code));
			return 4;
		case syntax_t::hidden_text:
			controls.push_back(std::make_unique<hidden_text_t>(code));
			return 4;
		case syntax_t::header_footer:
			controls.push_back(std::make_unique<header_footer_t>(code));
			return 4;
		case syntax_t::note:
			controls.push_back(std::make_unique<note_t>(code));
			return 4;
		case syntax_t::number_code:
			controls.push_back(std::make_unique<number_code_t>(code));
			return 4;
		case syntax_t::start_new_number:
			controls.push_back(std::make_unique<start_new_number_t>(code));
			return 4;
		case syntax_t::page_number:
			controls.push_back(std::make_unique<page_number_t>(code));
			return 4;
		case syntax_t::start_odd_hide_number:
			controls.push_back(std::make_unique<start_odd_hide_number_t>(code));
			return 4;
		case syntax_t::mail_merge:
			controls.push_back(std::make_unique<mail_merge_t>(code));
			return 12;
		case syntax_t::text_overlap:
			controls.push_back(std::make_unique<text_overlap_t>(code));
			return 5;
		case syntax_t::hypen:
			controls.push_back(std::make_unique<hypen_t>(code));
			return 3;
		case syntax_t::outline_mark:
			controls.push_back(std::make_unique<outline_mark_t>(code));
			return 3;
		case syntax_t::find_mark:
			controls.push_back(std::make_unique<find_mark_t>(code));
			return 123;
		case syntax_t::outline_shape:
			controls.push_back(std::make_unique<outline_shape_t>(code));
			return 32;
		case syntax_t::cross_reference:
			controls.push_back(std::make_unique<cross_reference_t>(code));
			return 4;
		case syntax_t::blank:
			controls.push_back(std::make_unique<blank_t>(code));
			return 2;
		case syntax_t::fixed_space:
			controls.push_back(std::make_unique<fixed_space_t>(code));
			return 2;
		case syntax_t::para_break:
		default:
			controls.push_back(std::make_unique<hchar_t>(code));
			return 1;
		}
		return 1;
	}

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

#pragma region definition of control codes

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

	bufferstream& reserved_control_t::read(bufferstream& stream)
	{
		data_length = binary_io::read_uint32(stream);
		data = binary_io::read(stream, data_length);
		end_code = binary_io::read_uint16(stream);
		return stream;
	}

	bufferstream& reserved_control_t::write(bufferstream& stream)
	{
		binary_io::write_uint16(stream, code);
		binary_io::write_uint16(stream, data_length);
		binary_io::write(stream, data);
		binary_io::write_uint16(stream, end_code);
		return stream;
	}

	bufferstream& tab_control_t::read(bufferstream& stream)
	{
		code.read(stream);
		width = binary_io::read_uint16(stream);
		leader = binary_io::read_uint16(stream);
		end_code = binary_io::read_uint16(stream);
		return stream;
	}

	bufferstream& tab_control_t::write(bufferstream& stream)
	{
		code.write(stream);
		binary_io::write_uint16(stream, width);
		binary_io::write_uint16(stream, leader);
		binary_io::write_uint16(stream, end_code);
		return stream;
	}

	bufferstream& field_t::read(bufferstream& stream)
	{
		data_length = binary_io::read_uint32(stream);
		data = binary_io::read(stream, data_length);
		end_code = binary_io::read_uint16(stream);
		return stream;
	}

	bufferstream& field_t::write(bufferstream& stream)
	{
		binary_io::write_uint16(stream, code);
		binary_io::write_uint16(stream, data_length);
		binary_io::write(stream, data);
		binary_io::write_uint16(stream, end_code);
		return stream;
	}

	bufferstream& bookmark_t::read(bufferstream& stream)
	{
		data = binary_io::read(stream, 40);
		return stream;
	}

	bufferstream& bookmark_t::write(bufferstream& stream)
	{
		binary_io::write_uint16(stream, code);
		binary_io::write(stream, data);
		return stream;
	}

	bufferstream& date_format_t::read(bufferstream& stream)
	{
		data = binary_io::read(stream, 82);
		return stream;
	}

	bufferstream& date_format_t::write(bufferstream& stream)
	{
		binary_io::write_uint16(stream, code);
		binary_io::write(stream, data);
		return stream;
	}

	bufferstream& date_code_t::read(bufferstream& stream)
	{
		data = binary_io::read(stream, 94);
		return stream;
	}

	bufferstream& date_code_t::write(bufferstream& stream)
	{
		binary_io::write_uint16(stream, code);
		binary_io::write(stream, data);
		return stream;
	}

	bufferstream& line_shape_t::read(bufferstream& stream)
	{
		data = binary_io::read(stream, 90);
		return stream;
	}

	bufferstream& line_shape_t::write(bufferstream& stream)
	{
		binary_io::write_uint16(stream, code);
		binary_io::write(stream, data);
		return stream;
	}

	bufferstream& hidden_text_t::read(bufferstream& stream)
	{
		reserved = binary_io::read_uint32(stream);
		end_code = binary_io::read_uint16(stream);
		reserved2 = binary_io::read_uint64(stream);

		para_lists.resize(1);
		stream >> para_lists[0];
		return stream;
	}

	bufferstream& hidden_text_t::write(bufferstream& stream)
	{
		binary_io::write_uint16(stream, code);
		binary_io::write_uint32(stream, reserved);
		binary_io::write_uint16(stream, end_code);
		binary_io::write_uint64(stream, reserved2);
		for (auto& para_list : para_lists)
		{
			stream << para_list;
		}
		return stream;
	}

	bufferstream& header_footer_t::read(bufferstream& stream)
	{
		reserved = binary_io::read_uint32(stream);
		end_code = binary_io::read_uint16(stream);
		data = binary_io::read(stream, 10);

		para_lists.resize(1);
		stream >> para_lists[0];
		return stream;
	}

	bufferstream& header_footer_t::write(bufferstream& stream)
	{
		binary_io::write_uint16(stream, code);
		binary_io::write_uint32(stream, reserved);
		binary_io::write_uint16(stream, end_code);
		binary_io::write(stream, data);

		for (auto& para_list : para_lists)
		{
			stream << para_list;
		}
		return stream;
	}

	bufferstream& note_t::read(bufferstream& stream)
	{
		reserved = binary_io::read_uint32(stream);
		end_code = binary_io::read_uint16(stream);
		data = binary_io::read(stream, 14);

		para_lists.resize(1);
		stream >> para_lists[0];
		return stream;
	}

	bufferstream& note_t::write(bufferstream& stream)
	{
		binary_io::write_uint16(stream, code);
		binary_io::write_uint32(stream, reserved);
		binary_io::write_uint16(stream, end_code);
		binary_io::write(stream, data);

		for (auto& para_list : para_lists)
		{
			stream << para_list;
		}
		return stream;
	}

	bufferstream& number_code_t::read(bufferstream& stream)
	{
		type = binary_io::read_uint16(stream);
		number = binary_io::read_uint16(stream);
		end_code = binary_io::read_uint16(stream);
		return stream;
	}

	bufferstream& number_code_t::write(bufferstream& stream)
	{
		binary_io::write_uint16(stream, code);
		binary_io::write_uint16(stream, type);
		binary_io::write_uint16(stream, number);
		binary_io::write_uint16(stream, end_code);
		return stream;
	}

	bufferstream& start_new_number_t::read(bufferstream& stream)
	{
		type = binary_io::read_uint16(stream);
		number = binary_io::read_uint16(stream);
		end_code = binary_io::read_uint16(stream);
		return stream;
	}

	bufferstream& start_new_number_t::write(bufferstream& stream)
	{
		binary_io::write_uint16(stream, code);
		binary_io::write_uint16(stream, type);
		binary_io::write_uint16(stream, number);
		binary_io::write_uint16(stream, end_code);
		return stream;
	}

	bufferstream& page_number_t::read(bufferstream& stream)
	{
		type = binary_io::read_uint16(stream);
		number = binary_io::read_uint16(stream);
		end_code = binary_io::read_uint16(stream);
		return stream;
	}

	bufferstream& page_number_t::write(bufferstream& stream)
	{
		binary_io::write_uint16(stream, code);
		binary_io::write_uint16(stream, type);
		binary_io::write_uint16(stream, number);
		binary_io::write_uint16(stream, end_code);
		return stream;
	}

	bufferstream& start_odd_hide_number_t::read(bufferstream& stream)
	{
		type = binary_io::read_uint16(stream);
		number = binary_io::read_uint16(stream);
		end_code = binary_io::read_uint16(stream);
		return stream;
	}

	bufferstream& start_odd_hide_number_t::write(bufferstream& stream)
	{
		binary_io::write_uint16(stream, code);
		binary_io::write_uint16(stream, type);
		binary_io::write_uint16(stream, number);
		binary_io::write_uint16(stream, end_code);
		return stream;
	}

	bufferstream& mail_merge_t::read(bufferstream& stream)
	{
		data = binary_io::read(stream, 22);
		return stream;
	}

	bufferstream& mail_merge_t::write(bufferstream& stream)
	{
		binary_io::write_uint16(stream, code);
		binary_io::write(stream, data);
		return stream;
	}

	bufferstream& text_overlap_t::read(bufferstream& stream)
	{
		data = binary_io::read(stream, 8);
		return stream;
	}

	bufferstream& text_overlap_t::write(bufferstream& stream)
	{
		binary_io::write_uint16(stream, code);
		binary_io::write(stream, data);
		return stream;
	}

	bufferstream& hypen_t::read(bufferstream& stream)
	{
		code.read(stream);
		width = binary_io::read_uint16(stream);
		end_code = binary_io::read_uint16(stream);
		return stream;
	}

	bufferstream& hypen_t::write(bufferstream& stream)
	{
		code.write(stream);
		binary_io::write_uint16(stream, width);
		binary_io::write_uint16(stream, end_code);
		return stream;
	}

	bufferstream& outline_mark_t::read(bufferstream& stream)
	{
		type = binary_io::read_uint16(stream);
		end_code = binary_io::read_uint16(stream);
		return stream;
	}

	bufferstream& outline_mark_t::write(bufferstream& stream)
	{
		binary_io::write_uint16(stream, code);
		binary_io::write_uint16(stream, type);
		binary_io::write_uint16(stream, end_code);
		return stream;
	}

	bufferstream& find_mark_t::read(bufferstream& stream)
	{
		data = binary_io::read(stream, 244);
		return stream;
	}

	bufferstream& find_mark_t::write(bufferstream& stream)
	{
		binary_io::write_uint16(stream, code);
		binary_io::write(stream, data);
		return stream;
	}

	bufferstream& outline_shape_t::read(bufferstream& stream)
	{
		data = binary_io::read(stream, 62);
		return stream;
	}

	bufferstream& outline_shape_t::write(bufferstream& stream)
	{
		binary_io::write_uint16(stream, code);
		binary_io::write(stream, data);
		return stream;
	}

	bufferstream& cross_reference_t::read(bufferstream& stream)
	{
		data = binary_io::read(stream, 46);
		length = binary_io::read_uint16(stream);
		reserved = binary_io::read_uint32(stream);
		if( length > 0 )
			contents = binary_io::read(stream, length);
		return stream;
	}

	bufferstream& cross_reference_t::write(bufferstream& stream)
	{
		binary_io::write_uint16(stream, code);
		binary_io::write(stream, data);
		binary_io::write_uint16(stream, length);
		binary_io::write_uint32(stream, reserved);
		if (contents.size() > 0)
			binary_io::write(stream, contents);
		return stream;
	}

	bufferstream& blank_t::read(bufferstream& stream)
	{
		code.read(stream);
		end_code = binary_io::read_uint16(stream);
		return stream;
	}

	bufferstream& blank_t::write(bufferstream& stream)
	{
		code.write(stream);
		binary_io::write_uint16(stream, end_code);
		return stream;
	}

	bufferstream& fixed_space_t::read(bufferstream& stream)
	{
		code.read(stream);
		end_code = binary_io::read_uint16(stream);
		return stream;
	}

	bufferstream& fixed_space_t::write(bufferstream& stream)
	{
		code.write(stream);
		binary_io::write_uint16(stream, end_code);
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

	bufferstream& operator >> (bufferstream& stream, frame_header_t& data)
	{
		data.length = binary_io::read_uint32(stream);
		if (data.length > 0)
			data.data = binary_io::read(stream, data.length);
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const frame_header_t& data)
	{
		binary_io::write_uint32(stream, data.length);
		if (data.length > 0)
			binary_io::write(stream, data.data);
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, common_header_t& data)
	{
		data.length = binary_io::read_uint32(stream);
		data.object_type = binary_io::read_uint16(stream);
		data.link = binary_io::read_uint16(stream);
		data.basic_info = binary_io::read(stream, 40);
		data.basic_attr = binary_io::read(stream, 40);
		data.option = binary_io::read_uint32(stream);

		if (data.length == 88)
			return stream;

		if (data.option[17])
			data.rotation = binary_io::read(stream, 32);
		if (data.option[16])
			data.gradation = binary_io::read(stream, 28);
		if (data.option[18])
			data.bitmap_pattern = binary_io::read(stream, 278);
		if (data.option[20])
			data.watermark = binary_io::read(stream, 3);
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const common_header_t& data)
	{
		binary_io::write_uint32(stream, data.length);
		binary_io::write_uint16(stream, data.object_type);
		binary_io::write_uint16(stream, (uint16_t)data.link.to_ulong());
		binary_io::write(stream, data.basic_info);
		binary_io::write(stream, data.basic_attr);
		binary_io::write_uint32(stream, (uint32_t)data.option.to_ulong());
		if (data.length == 88)
			return stream;

		if (data.option[17])
			binary_io::write(stream, data.rotation);
		if (data.option[16])
			binary_io::write(stream, data.gradation);
		if (data.option[18])
			binary_io::write(stream, data.bitmap_pattern);
		if (data.option[20])
			binary_io::write(stream, data.watermark);
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, detail_info_t& data)
	{
		data.first_detail_length = binary_io::read_uint32(stream);
		if( data.first_detail_length > 0 )
			data.first_detail = binary_io::read(stream, data.first_detail_length);
		data.second_detail_length = binary_io::read_uint32(stream);
		if (data.second_detail_length > 0)
			data.second_detail = binary_io::read(stream, data.second_detail_length);
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const detail_info_t& data)
	{
		binary_io::write_uint32(stream, data.first_detail_length);
		if (data.first_detail_length > 0)
			binary_io::write(stream, data.first_detail);
		binary_io::write_uint32(stream, data.second_detail_length);
		if (data.second_detail_length > 0)
			binary_io::write(stream, data.second_detail);
		return stream;
	}

	void drawing_object_impl_t::read_by_preorder(bufferstream& stream)
	{
		while (true)
		{
			common_header_t common_header;
			stream >> common_header;
			if (common_header.object_type == 0)
			{
				objects.emplace_back(std::make_unique<container_t>(common_header));
				objects.back()->read(stream);
				if (common_header.has_child())
					read_by_preorder(stream);
			}
			else
			{
				objects.emplace_back(std::make_unique<any_object_t>(common_header));
				objects.back()->read(stream);
			}
			if (!common_header.has_sibling())
				break;
		}
	}

	bufferstream& operator >> (bufferstream& stream, drawing_object_impl_t& data)
	{
		stream >> data.frame_header;
		data.read_by_preorder(stream);
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const drawing_object_impl_t& data)
	{
		stream << data.frame_header;
		for (auto& object : data.objects)
			object->write(stream);
		return stream;
	}

	bufferstream& container_t::read(bufferstream& stream)
	{
		stream >> detail_info;
		return stream;
	}

	bufferstream& container_t::write(bufferstream& stream)
	{
		stream << common_header;
		stream << detail_info;
		return stream;
	}

	bufferstream& any_object_t::read(bufferstream& stream)
	{
		if (common_header.option[19]) // bit 19 : 그리기를 글상자로 만들 것인지의 여부
		{
			first_info_length = binary_io::read_uint32(stream);
			second_info_length = binary_io::read_uint32(stream);
			para_lists.resize(1);
			stream >> para_lists[0];
		}
		stream >> detail_info;
		return stream;
	}

	bufferstream& any_object_t::write(bufferstream& stream)
	{
		stream << common_header;
		// bit 19 : 그리기를 글상자로 만들 것인지의 여부
		if ( common_header.option[19] && para_lists.size() > 0 )
		{
			binary_io::write_uint32(stream, first_info_length);
			binary_io::write_uint32(stream, second_info_length);
			for (auto& para_list : para_lists)
				stream << para_list;
		}
		stream << detail_info;
		return stream;
	}

	bufferstream& drawing_object_t::read(bufferstream& stream)
	{
		reserved = binary_io::read_uint32(stream);
		end_code = binary_io::read_uint16(stream);

		length = binary_io::read_uint32(stream);
		data = binary_io::read(stream, 70);
		type = binary_io::read_uint8(stream);
		data2 = binary_io::read(stream, 273);

		if (type != 3 && length > 0)
			data3 = binary_io::read(stream, length);
		else if (type == 3) // drawing object
			stream >> drawing_object;

		stream >> caption;
		return stream;
	}

	bufferstream& drawing_object_t::write(bufferstream& stream)
	{
		binary_io::write_uint16(stream, code);
		binary_io::write_uint32(stream, reserved);
		binary_io::write_uint16(stream, end_code);
		binary_io::write_uint32(stream, length);
		binary_io::write(stream, data);
		binary_io::write_uint8(stream, type);
		binary_io::write(stream, data2);
		if (type != 3 && length > 0)
			binary_io::write(stream, data3);
		else if (type == 3) // drawing object
			stream << drawing_object;

		stream << caption;
		return stream;
	}

#pragma endregion definition of control codes

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
			// TODO: remove
			if (data.para_list.size() >= 100)
				return stream;
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

	bufferstream& operator >> (bufferstream& stream, extra_info_block_t& data)
	{
		data.id = binary_io::read_uint32(stream);
		data.length = binary_io::read_uint32(stream);

		if ( data.is_last() )
			return stream;

		if( data.length > 0 )
			data.data = binary_io::read(stream, data.length);
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const extra_info_block_t& data)
	{
		binary_io::write_uint32(stream, data.id);
		binary_io::write_uint32(stream, data.length);
		if ( data.is_last() )
			return stream;
		if (data.length > 0)
			binary_io::write(stream, data.data);
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, extra_info_blocks_t& data)
	{
		bool end_of_block = false;
		do {
			extra_info_block_t extra_info_block;
			stream >> extra_info_block;
			end_of_block = extra_info_block.is_last() || extra_info_block.is_second_block();
			data.extra_info_blocks.push_back(std::move(extra_info_block));
		} while (!end_of_block);
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const extra_info_blocks_t& data)
	{
		for (auto& extra_info_block : data.extra_info_blocks)
		{
			stream << extra_info_block;
		}
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, second_extra_info_blocks_t& data)
	{
		bool end_of_block = false;
		do {
			extra_info_block_t extra_info_block;
			stream >> extra_info_block;
			end_of_block = extra_info_block.is_last();
			data.second_extra_info_blocks.push_back(std::move(extra_info_block));
		} while (!end_of_block);
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const second_extra_info_blocks_t& data)
	{
		for (auto& extra_info_block : data.second_extra_info_blocks)
		{
			stream << extra_info_block;
		}
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, document_tail_t& data)
	{
		stream >> data.first;
		if( !data.first.extra_info_blocks.empty() && !data.first.extra_info_blocks.back().is_last() )
			stream >> data.second;
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const document_tail_t& data)
	{
		// NOTE: DO NOT USE - second block is NOT zip
		stream << data.first;
		stream << data.second;
		return stream;
	}

	consumer_t::consumer_t()
	{}

	consumer_t::buffer_t consumer_t::read_file(const std::string& path)
	{
		std::ifstream file(to_fstream_path(path), std::ios::binary);
		if (file.fail())
			throw std::runtime_error("file I/O error");
		file.unsetf(std::ios::skipws);
		file.seekg(0, std::ios::end);
		std::streampos size = file.tellg();
		file.seekg(0, std::ios::beg);

		buffer_t buffer;
		buffer.reserve(size_t(size));
		std::copy(std::istream_iterator<byte_t>(file), std::istream_iterator<byte_t>(), std::back_inserter(buffer));
		return buffer;
	}

	std::unique_ptr<document_t> consumer_t::parse(buffer_t& buffer)
	{
		std::unique_ptr<document_t> document(std::make_unique<document_t>());
		bufferstream header_stream(&buffer[0], buffer.size());
		header_stream >> document->header;

		buffer_t body_tail = extract_body(buffer, header_stream, document);
		if (body_tail.empty())
			throw std::runtime_error("hwp30 parse body error");
		bufferstream body_tail_stream(&body_tail[0], body_tail.size());
		body_tail_stream >> document->body;
		// TODO: remove
		if (document->body.sections.para_list.size() >= 100)
			return document;
		body_tail_stream >> document->tail;
		return document;
	}

	buffer_t consumer_t::extract_body(buffer_t& buffer, bufferstream& stream, std::unique_ptr<document_t>& document)
	{
		streamsize stream_cur = stream.tellg();
		streamsize stream_end = buffer.size();
		streamsize body_size = stream_end - stream_cur;
		buffer_t body = binary_io::read(stream, body_size);
		if (document->header.doc_info.compressed != 0) // decompress : 0
			return hwp_zip::decompress_noexcept(body);
		return body;
	}

	void consumer_t::open(const std::string& path)
	{
		try
		{
			auto import_buffer = read_file(path);
			document = parse(import_buffer);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	producer_t::producer_t()
	{}

	void producer_t::save(const std::string& path, std::unique_ptr<consumer_t>& consumer)
	{
		try
		{
			auto& document = consumer->get_document();
			if(!document)
				throw std::runtime_error("document is not exist");
			buffer_t header_buffer;
			header_buffer.resize(document->header.size());
			if (header_buffer.size() == 0)
				throw std::runtime_error("save buffer size error");
			bufferstream header_stream(&header_buffer[0], header_buffer.size());
			header_stream << document->header;

			buffer_t body_tail_buffer;
			body_tail_buffer.resize(document->body.size() + document->tail.first.size());
			if (body_tail_buffer.size() == 0)
				throw std::runtime_error("save buffer size error");
			bufferstream body_tail_stream(&body_tail_buffer[0], body_tail_buffer.size());
			body_tail_stream << document->body;
			body_tail_stream << document->tail.first;

			// compress
			if (document->header.doc_info.compressed != 0)
			{
				streamsize buffer_size = body_tail_stream.tellp(); // IMPORTANT!
				body_tail_buffer = hwp_zip::compress_noexcept((char*)& body_tail_buffer[0], (size_t)buffer_size);
			}

			std::ofstream fout(to_fstream_path(path), std::ios::out | std::ios::binary);
			fout.write((char*)& header_buffer[0], header_buffer.size());
			fout.write((char*)& body_tail_buffer[0], body_tail_buffer.size());
			if (document->tail.has_second_extra_block())
			{
				buffer_t second_extra_block_buffer;
				second_extra_block_buffer.resize(document->tail.second.size());
				if (second_extra_block_buffer.size() == 0)
					throw std::runtime_error("save buffer size error");
				bufferstream second_extra_block_stream(&second_extra_block_buffer[0], second_extra_block_buffer.size());
				second_extra_block_stream << document->tail.second;

				fout.write((char*)& second_extra_block_buffer[0], second_extra_block_buffer.size());
			}
			fout.close();
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}
}
}