#pragma once
#include <bitset>
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
		typedef std::string face_name_t;
		typedef std::vector<face_name_t> face_names_t;
		typedef std::vector<face_names_t> family_list_t;

		face_name_list_t() = default;
		DECLARE_BINARY_SERIALIZER(face_name_list_t);

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

		std::vector<style_t> styles;
	};

	struct para_header_t
	{
		typedef std::string name_t;
		para_header_t() = default;
		DECLARE_BINARY_SERIALIZER(para_header_t);

		bool empty() const {
			return char_count == 0 || char_count == 0xffff;
		}

		uint8_t prev_para_shape_id; // 0 : new shape
		uint16_t char_count;
		uint16_t line_count;
		uint8_t char_shape_id; // 0: 대표 글자 모양
		std::bitset<8> etc_flag;
		uint32_t control_code;
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

		std::vector<char_shape_info_t> char_shape_info_list;
		uint16_t char_count;
	};

	struct hchar_t
	{
		hchar_t() : code(0)
		{}
		DECLARE_BINARY_SERIALIZER(hchar_t);
		uint16_t code;
	};
	

	struct paragraph_t
	{
		typedef std::string name_t;
		paragraph_t() = default;
		DECLARE_BINARY_SERIALIZER(paragraph_t);

		para_header_t para_header;
		line_segment_list_t line_segment_list;
		char_shape_info_list_t char_shape_info_list; // para_header_t.char_shape_id : 0이 아닐 때만 존재
		std::vector<hchar_t> hchars; // para_header_t.control_code : 0 일 때만 존재
		// TODO: implement control codes
	};

	struct document_t
	{
		document_t()
		{}

		buffer_t signature;
		doc_info_t doc_info;
		doc_summary_t doc_summary;
		info_block_t info_block;

		// compress data
		face_name_list_t face_name_list;
		style_list_t style_list;
		std::vector<paragraph_t> para_list;
		// TODO: implement
	};
}
}