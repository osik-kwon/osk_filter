#pragma once
#include <bitset>
#include <string>
#include "define/binary_traits.h"
#include "io/binary_iostream.h"

namespace filter
{
namespace hwp30
{
	typedef binary_traits::byte_t byte_t;
	typedef binary_traits::buffer_t buffer_t;
	typedef binary_traits::bufferstream bufferstream;
	typedef binary_traits::streamsize streamsize;

	struct doc_info_t
	{
		doc_info_t() = default;

		DECLARE_BINARY_SERIALIZER(doc_info_t);
		std::size_t size() const {
			return 128;
		}
		buffer_t body;

		// semantics
		uint16_t crypted;
		uint8_t compressed;
		uint16_t info_block_length;
	};

	struct doc_summary_t
	{
		doc_summary_t() = default;
		DECLARE_BINARY_SERIALIZER(doc_summary_t);
		std::size_t size() const {
			return 1008;
		}
		buffer_t body;
	};


	struct info_block_t
	{
		info_block_t() : info_block_length(0)
		{}
		info_block_t(uint16_t len) : info_block_length(len)
		{}
		DECLARE_BINARY_SERIALIZER(info_block_t);
		std::size_t size() const {
			return info_block_length;
		}
		buffer_t body;
		uint16_t info_block_length;
	};

	// compress data
	struct face_name_list_t
	{
		typedef buffer_t face_name_t;
		typedef std::vector<face_name_t> face_names_t;
		typedef std::vector<face_names_t> family_list_t;

		face_name_list_t() = default;
		DECLARE_BINARY_SERIALIZER(face_name_list_t);

		std::size_t size() const
		{
			std::size_t offset = 0;
			for (auto& face_names : family_list) {
				offset += 2;
				offset += ( face_names.size() * 40 );
			}
			return offset;
		}

		family_list_t family_list;
	};

	struct char_shape_t
	{
		char_shape_t() = default;
		DECLARE_BINARY_SERIALIZER(char_shape_t);
		std::size_t size() const {
			return 31;
		}
		buffer_t body;
	};

	struct para_shape_t
	{
		para_shape_t() = default;
		DECLARE_BINARY_SERIALIZER(para_shape_t);
		std::size_t size() const {
			return 187;
		}
		buffer_t body;
	};

	struct style_t
	{
		typedef std::string name_t;
		style_t() = default;

		name_t name; // 20 bytes
		char_shape_t char_shape;
		para_shape_t para_shape;
	};

	struct style_list_t
	{
		style_list_t() = default;
		DECLARE_BINARY_SERIALIZER(style_list_t);

		std::size_t size() const
		{
			std::size_t offset = 0;
			offset += 2;
			for (auto& style : styles)
			{
				offset += style.name.size();
				offset += style.char_shape.size();
				offset += style.para_shape.size();
			}
			return offset;
		}

		std::vector<style_t> styles;
	};

	struct para_header_t
	{
		typedef std::string name_t;
		para_header_t() : prev_para_shape_id(0), char_count(0), line_count(0), char_shape_id(0), style_id(0)
		{}
		DECLARE_BINARY_SERIALIZER(para_header_t);

		std::size_t size() const {
			return 12 + char_shape.size() + para_shape.size();
		}

		bool empty() const {
			return char_count == 0 || char_count == 0xffff;
		}

		uint8_t prev_para_shape_id; // 0 : new shape
		uint16_t char_count;
		uint16_t line_count;
		uint8_t char_shape_id; // 0: 대표 글자 모양
		std::bitset<8> etc_flag;
		uint32_t control_code; // TODO: remove
		uint8_t style_id;
		char_shape_t char_shape;
		para_shape_t para_shape; // prev_para_shape_id 가 0, char_count 가 1개 이상 일 때 존재
	};

	struct line_segment_list_t
	{
		line_segment_list_t() : line_count(0)
		{}
		line_segment_list_t(uint16_t count) : line_count(count)
		{}
		DECLARE_BINARY_SERIALIZER(line_segment_list_t);
		std::size_t size() const {
			return 14 * line_count;
		}
		buffer_t body;
		uint16_t line_count;
	};

	struct char_shape_info_t
	{
		char_shape_info_t() : flag(0)
		{}
		char_shape_t char_shape;
		uint8_t flag;
	};

	struct char_shape_info_list_t
	{
		char_shape_info_list_t() : char_count(0)
		{}
		char_shape_info_list_t(uint16_t count) : char_count(count)
		{}
		DECLARE_BINARY_SERIALIZER(char_shape_info_list_t);

		std::size_t size() const
		{
			std::size_t offset = 0;
			offset += 2;
			for (auto& char_shape_info : char_shape_info_list)
			{
				offset += 1;
				if (char_shape_info.flag != 1)
					offset += char_shape_info.char_shape.size();
			}
			return offset;
		}

		std::vector<char_shape_info_t> char_shape_info_list;
		uint16_t char_count;
	};

	struct hchar_t
	{
		hchar_t() : code(0), utf32(0)
		{}
		DECLARE_BINARY_SERIALIZER(hchar_t);
		std::size_t size() const {
			return 2;
		}
		uint16_t code;
		char32_t utf32;
	};
	
	struct control_code_t
	{
		control_code_t() : begin_code(0), reserved(0), end_code(0)
		{}
		DECLARE_BINARY_SERIALIZER(control_code_t);
		std::size_t size() const {
			return 8;
		}
		bool is_control_code() const {
			return begin_code > 0 && begin_code < 32;
		}
		uint16_t begin_code;
		uint32_t reserved;
		uint16_t end_code;
	};

	struct cell_t
	{
		cell_t() = default;
		std::size_t size() const {
			return 27;
		}
		buffer_t data;
	};

	struct table_t
	{
		table_t() : cell_count(0), protect(0)
		{}
		DECLARE_BINARY_SERIALIZER(table_t);
		std::size_t size() const {
			return 80 + 2 + 2 + 
				std::accumulate(cells.begin(), cells.end(), 0, [](std::size_t size, auto& cell) {
				return size + cell.size(); });
		}
		buffer_t data;
		uint16_t cell_count;
		uint16_t protect;
		std::vector<cell_t> cells;

	};

	struct paragraph_list_t;
	struct paragraph_t
	{
		typedef std::string name_t;
		paragraph_t() = default;
		DECLARE_BINARY_SERIALIZER(paragraph_t);
		std::size_t size() const;

		para_header_t para_header;
		line_segment_list_t line_segment_list;
		char_shape_info_list_t char_shape_info_list; // para_header_t.char_shape_id : 0이 아닐 때만 존재
		std::vector<hchar_t> hchars; // para_header_t.control_code : 0 일 때만 존재
		control_code_t control_code;
		table_t table;
		// TODO: implement control codes
		std::vector<paragraph_list_t> childs;
	};

	struct paragraph_list_t
	{
		typedef std::string name_t;
		paragraph_list_t() = default;
		DECLARE_BINARY_SERIALIZER(paragraph_list_t);

		std::size_t size() const {
			return std::accumulate(para_list.begin(), para_list.end(), 0, [](std::size_t size, auto& paragraph) {
				return size + paragraph.size(); });
		}

		std::vector<paragraph_t> para_list;
		// TODO: implement
	};

	struct document_header_t
	{
		document_header_t() = default;
		DECLARE_BINARY_SERIALIZER(document_header_t);
		std::size_t size() const {
			std::size_t offset = 0;
			offset += signature.size();
			offset += doc_info.size();
			offset += doc_summary.size();

			// variable
			offset += info_block.body.size();
			return offset;
		}

		buffer_t signature;
		doc_info_t doc_info;
		doc_summary_t doc_summary;
		info_block_t info_block;
	};

	struct document_body_t
	{
		document_body_t() = default;
		DECLARE_BINARY_SERIALIZER(document_body_t);
		std::size_t size() const {
			std::size_t offset = 0;
			offset += face_name_list.size();
			offset += style_list.size();
			offset += sections.size();
			return offset;
		}
		// compressd data
		face_name_list_t face_name_list;
		style_list_t style_list;
		paragraph_list_t sections;
		// TODO: implement
	};

	struct document_tail_t
	{
		document_tail_t()
		{
			trailer.resize( 8 );
		}
		DECLARE_BINARY_SERIALIZER(document_tail_t);
		std::size_t size() const {
			return 8;
		}

		// TODO: implement
		buffer_t trailer;
	};

	struct document_t
	{
		document_t() = default;
		std::size_t size() const {
			return header.size() + body.size() + tail.size();
		}

		document_header_t header;
		document_body_t body;
		document_tail_t tail;
	};
}
}