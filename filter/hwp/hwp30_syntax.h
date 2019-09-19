#pragma once
#include <bitset>
#include <string>
#include <memory>
#include "traits/binary_traits.h"
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
		size_t size() const {
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
		size_t size() const {
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
		size_t size() const {
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

		size_t size() const
		{
			size_t offset = 0;
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
		size_t size() const {
			return 31;
		}
		buffer_t body;
	};

	struct para_shape_t
	{
		para_shape_t() = default;
		DECLARE_BINARY_SERIALIZER(para_shape_t);
		size_t size() const {
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

		size_t size() const
		{
			size_t offset = 0;
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

		size_t size() const {
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
		size_t size() const {
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

		size_t size() const
		{
			size_t offset = 0;
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

	struct paragraph_list_t;
	struct control_code_t
	{
		typedef uint16_t control_t;
		control_code_t() {}
		virtual ~control_code_t(){}
		virtual size_t size() const = 0;
		virtual bufferstream& read(bufferstream& stream) = 0;
		virtual bufferstream & write(bufferstream & stream) = 0;
		virtual control_t get_code() const = 0;
		virtual bool is_control_code() const {
			return true;
		}
		virtual bool has_drawing_object() const {
			return false;
		}
		virtual bool has_para_list() const{
			return false;
		}
		virtual bool has_caption() const {
			return false;
		}
		virtual std::vector<paragraph_list_t>* get_para_lists() {
			return nullptr;
		}
		virtual paragraph_list_t* get_caption() {
			return nullptr;
		}
	};

	struct paragraph_t
	{
		typedef std::vector<std::unique_ptr<control_code_t>> controls_t;
		paragraph_t() = default;
		DECLARE_BINARY_SERIALIZER(paragraph_t);
		size_t size() const
		{
			size_t offset = 0;
			offset += para_header.size();
			if (para_header.empty())
				return offset;

			offset += line_segment_list.size();
			offset += char_shape_info_list.size();

			for (auto& control : controls)
			{
				offset += control->size();
			}
			return offset;
		}

		para_header_t para_header;
		line_segment_list_t line_segment_list;
		char_shape_info_list_t char_shape_info_list;

		// control codes
		controls_t controls;
	};

	struct paragraph_list_t
	{
		typedef std::string name_t;
		paragraph_list_t() = default;
		DECLARE_BINARY_SERIALIZER(paragraph_list_t);

		size_t size() const {
			return std::accumulate(para_list.begin(), para_list.end(), 0, [](size_t size, auto& paragraph) {
				return size + paragraph.size(); });
		}
		std::vector<paragraph_t> para_list;
	};
#pragma region definition of control codes
	struct hchar_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		hchar_t(control_t code) : code(code), utf32(0)
		{}
		~hchar_t(){}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return code;
		}
		virtual bool is_control_code() const {
			return false;
		}
		virtual size_t size() const {
			return 2;
		}
		control_t code;
		char32_t utf32;
	};

	struct reserved_control_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		reserved_control_t(control_t code) : code(code), data_length(0), end_code(0)
		{}
		~reserved_control_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return code;
		}
		size_t size() const {
			return 2 + 4 + data_length + 2;
		}
		control_t code;
		uint32_t data_length;
		buffer_t data;
		control_t end_code;
	};

	struct tab_control_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		tab_control_t(control_t that) : code(that), width(0), leader(0), end_code(0)
		{}
		~tab_control_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return end_code;
		}
		virtual bool is_control_code() const {
			return false;
		}
		size_t size() const {
			return 8;
		}
		hchar_t code;
		uint16_t width;
		uint16_t leader;
		control_t end_code;
	};

	struct field_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		field_t(control_t code) : code(code), data_length(0), end_code(0)
		{}
		~field_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return code;
		}
		size_t size() const {
			return 2 + 4 + data_length + 2;
		}
		control_t code;
		uint32_t data_length;
		buffer_t data;
		control_t end_code;
	};

	struct bookmark_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		bookmark_t(control_t code) : code(code)
		{}
		~bookmark_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return code;
		}
		size_t size() const {
			return 42;
		}
		control_t code;
		buffer_t data;
	};

	struct date_format_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		date_format_t(control_t code) : code(code)
		{}
		~date_format_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return code;
		}
		size_t size() const {
			return 84;
		}
		control_t code;
		buffer_t data;
	};

	struct date_code_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		date_code_t(control_t code) : code(code)
		{}
		~date_code_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return code;
		}
		size_t size() const {
			return 96;
		}
		control_t code;
		buffer_t data;
	};

	struct line_shape_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		line_shape_t(control_t code) : code(code)
		{}
		~line_shape_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return code;
		}
		size_t size() const {
			return 92;
		}
		control_t code;
		buffer_t data;
	};

	struct hidden_text_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		hidden_text_t(control_t code) : code(code), reserved(0), end_code(0), reserved2(0)
		{}
		~hidden_text_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return code;
		}
		virtual bool has_para_list() const {
			return true;
		}
		virtual std::vector<paragraph_list_t>* get_para_lists() {
			return &para_lists;
		}
		size_t size() const {
			size_t offset = 0;
			offset += 8;
			offset += 8;
			offset += std::accumulate(para_lists.begin(), para_lists.end(), 0, [](size_t size, auto& para_list) {
				return size + para_list.size(); });
			return offset;
		}
		control_t code;
		uint32_t reserved;
		control_t end_code;
		uint64_t reserved2;
		std::vector<paragraph_list_t> para_lists;
	};

	struct header_footer_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		header_footer_t(control_t code) : code(code), reserved(0), end_code(0)
		{}
		~header_footer_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return code;
		}
		virtual bool has_para_list() const {
			return true;
		}
		virtual std::vector<paragraph_list_t>* get_para_lists() {
			return &para_lists;
		}
		size_t size() const {
			size_t offset = 0;
			offset += 8;
			offset += data.size();
			offset += std::accumulate(para_lists.begin(), para_lists.end(), 0, [](size_t size, auto& para_list) {
				return size + para_list.size(); });
			return offset;
		}
		control_t code;
		uint32_t reserved;
		control_t end_code;
		buffer_t data;
		std::vector<paragraph_list_t> para_lists;
	};

	struct note_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		note_t(control_t code) : code(code), reserved(0), end_code(0)
		{}
		~note_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return code;
		}
		virtual bool has_para_list() const {
			return true;
		}
		virtual std::vector<paragraph_list_t>* get_para_lists() {
			return &para_lists;
		}
		size_t size() const {
			size_t offset = 0;
			offset += 8;
			offset += data.size();
			offset += std::accumulate(para_lists.begin(), para_lists.end(), 0, [](size_t size, auto& para_list) {
				return size + para_list.size(); });
			return offset;
		}
		control_t code;
		uint32_t reserved;
		control_t end_code;
		buffer_t data;
		std::vector<paragraph_list_t> para_lists;
	};

	struct number_code_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		number_code_t(control_t code) : code(code), type(0), number(0), end_code(0)
		{}
		~number_code_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return code;
		}
		size_t size() const {
			return 2 + 2 + 2 + 2;
		}
		control_t code;
		uint16_t type;
		uint16_t number;
		control_t end_code;
	};

	struct start_new_number_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		start_new_number_t(control_t code) : code(code), type(0), number(0), end_code(0)
		{}
		~start_new_number_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return code;
		}
		size_t size() const {
			return 2 + 2 + 2 + 2;
		}
		control_t code;
		uint16_t type;
		uint16_t number;
		control_t end_code;
	};

	struct page_number_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		page_number_t(control_t code) : code(code), type(0), number(0), end_code(0)
		{}
		~page_number_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return code;
		}
		size_t size() const {
			return 2 + 2 + 2 + 2;
		}
		control_t code;
		uint16_t type;
		uint16_t number;
		control_t end_code;
	};

	struct start_odd_hide_number_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		start_odd_hide_number_t(control_t code) : code(code), type(0), number(0), end_code(0)
		{}
		~start_odd_hide_number_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return code;
		}
		size_t size() const {
			return 2 + 2 + 2 + 2;
		}
		control_t code;
		uint16_t type;
		uint16_t number;
		control_t end_code;
	};

	struct mail_merge_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		mail_merge_t(control_t code) : code(code)
		{}
		~mail_merge_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return code;
		}
		size_t size() const {
			return 24;
		}
		control_t code;
		buffer_t data;
	};

	struct text_overlap_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		text_overlap_t(control_t code) : code(code)
		{}
		~text_overlap_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return code;
		}
		size_t size() const {
			return 10;
		}
		control_t code;
		buffer_t data;
	};

	struct hypen_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		hypen_t(control_t that) : code(that), width(0), end_code(0)
		{}
		~hypen_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return end_code;
		}
		virtual bool is_control_code() const {
			return false;
		}
		size_t size() const {
			return 6;
		}
		hchar_t code;
		uint16_t width;
		control_t end_code;
	};

	struct outline_mark_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		outline_mark_t(control_t that) : code(that), type(0), end_code(0)
		{}
		~outline_mark_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return code;
		}
		size_t size() const {
			return 6;
		}
		control_t code;
		uint16_t type;
		control_t end_code;
	};

	struct find_mark_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		find_mark_t(control_t code) : code(code)
		{}
		~find_mark_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return code;
		}
		size_t size() const {
			return 246;
		}
		control_t code;
		buffer_t data;
	};

	struct outline_shape_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		outline_shape_t(control_t code) : code(code)
		{}
		~outline_shape_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return code;
		}
		size_t size() const {
			return 64;
		}
		control_t code;
		buffer_t data;
	};

	struct cross_reference_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		cross_reference_t(control_t code) : code(code), length(0), reserved(0)
		{}
		~cross_reference_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return code;
		}
		size_t size() const {
			return 2 + data.size() + 2 + 4 + contents.size();
		}
		uint16_t code;
		buffer_t data; // 46
		uint16_t length;
		uint32_t reserved;
		buffer_t contents; // if(length > 0) variable
	};

	struct blank_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		blank_t(control_t that) : code(that), end_code(0)
		{}
		~blank_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return end_code;
		}
		virtual bool is_control_code() const {
			return false;
		}
		size_t size() const {
			return 4;
		}
		hchar_t code;
		control_t end_code;
	};

	struct fixed_space_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		fixed_space_t(control_t that) : code(that), end_code(0)
		{}
		~fixed_space_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return end_code;
		}
		virtual bool is_control_code() const {
			return false;
		}
		size_t size() const {
			return 4;
		}
		hchar_t code;
		control_t end_code;
	};

	struct cell_t
	{
		cell_t() = default;
		size_t size() const {
			return 27;
		}
		buffer_t data;
	};

	struct table_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		table_t(control_t code) : code(code), reserved(0), end_code(0), cell_count(0), protect(0)
		{}
		~table_t(){}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return code;
		}
		virtual bool has_para_list() const {
			return true;
		}
		virtual bool has_caption() const {
			return true;
		}
		virtual std::vector<paragraph_list_t>* get_para_lists() {
			return &para_lists;
		}

		virtual paragraph_list_t* get_caption() {
			return &caption;
		}

		size_t size() const{
			size_t offset = 0;
			offset += 84;
			offset += std::accumulate(cells.begin(), cells.end(), 0, [](size_t size, auto& cell) {
				return size + cell.size(); });
			offset += std::accumulate(para_lists.begin(), para_lists.end(), 0, [](size_t size, auto& para_list) {
				return size + para_list.size(); });
			offset += caption.size();
			return offset;
		}
		control_t code;
		uint32_t reserved;
		control_t end_code;

		buffer_t data;
		uint16_t cell_count;
		uint16_t protect;
		std::vector<cell_t> cells;

		std::vector<paragraph_list_t> para_lists;
		paragraph_list_t caption;
	};

	struct frame_header_t
	{
		frame_header_t() : length(0)
		{}
		~frame_header_t() {}
		DECLARE_BINARY_SERIALIZER(frame_header_t);
		size_t size() const {
			return sizeof(length) + data.size();
		}
		uint32_t length;
		buffer_t data;
	};

	struct common_header_t
	{
		common_header_t() : length(0), object_type(0), link(0)
		{}
		~common_header_t() {}
		DECLARE_BINARY_SERIALIZER(common_header_t);
		size_t size() const {
			return 4 + 2 + 2 + basic_info.size() + basic_attr.size() + 4
				+ rotation.size() + gradation.size() + bitmap_pattern.size() + watermark.size();
		}
		bool has_child() {
			return link[1];
		}
		bool has_sibling() {
			return link[0];
		}
		uint32_t length;
		/*
		0 = 컨테이너
		1 = 선
		2 = 사각형
		3 = 타원
		4 = 호
		5 = 다각형
		6 = 글상자
		7 = 곡선
		8 = 변형된 타원 (회전되거나 호로 편집된 타원)
		9 = 변형된 호 (회전된 호)
		10 = 선을 그릴 수 있도록 확장된 곡선
		*/
		uint16_t object_type;
		std::bitset<16> link; // bit 0 = sibling이 존재하는지 여부, bit 1 = child가 존재하는지 여부
		buffer_t basic_info; // 40
		buffer_t basic_attr; // 40
		std::bitset<32> option;
		// optional
		buffer_t rotation; // 32
		buffer_t gradation; // 28
		buffer_t bitmap_pattern; // 278
		buffer_t watermark; // 3
	};

	struct detail_info_t
	{
		detail_info_t() : first_detail_length(0), second_detail_length(0)
		{}
		~detail_info_t() {}
		DECLARE_BINARY_SERIALIZER(detail_info_t);
		size_t size() const {
			return 4 + first_detail.size() + 4 + second_detail.size();
		}
		uint32_t first_detail_length;
		buffer_t first_detail;
		uint32_t second_detail_length;
		buffer_t second_detail;
	};

	struct object_t
	{
		object_t(const common_header_t& common_header) : common_header(common_header)
		{}
		virtual ~object_t(){}
		virtual size_t size() const = 0;
		virtual bufferstream& read(bufferstream& stream) = 0;
		virtual bufferstream& write(bufferstream& stream) = 0;

		bool has_child() {
			return common_header.has_child();
		}
		bool has_sibling() {
			return common_header.has_sibling();
		}
		virtual bool has_para_list() const {
			return false;
		}
		virtual bool has_caption() const {
			return false;
		}
		virtual std::vector<paragraph_list_t>* get_para_lists() {
			return nullptr;
		}
		virtual paragraph_list_t* get_caption() {
			return nullptr;
		}

		common_header_t common_header;
	};

	struct drawing_object_t
	{
		drawing_object_t()
		{}
		~drawing_object_t() {}
		DECLARE_BINARY_SERIALIZER(drawing_object_t);
		size_t size() const {
			size_t offset = frame_header.size();
			for (auto& object : objects)
				offset += object->size();
			return offset;
		}
		void read_by_preorder(bufferstream& stream);

		frame_header_t frame_header;
		std::vector< std::unique_ptr<object_t> > objects;
	};

	struct container_t : object_t
	{
		container_t(const common_header_t& common_header) : object_t(common_header)
		{}
		~container_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual size_t size() const {
			size_t offset = 0;
			offset += common_header.size();
			offset += detail_info.size();
			return offset;
		}
		detail_info_t detail_info;
	};

	struct textbox_t : object_t
	{
		textbox_t(const common_header_t& common_header) : object_t(common_header),
			first_info_length(0), second_info_length(0)
		{}
		~textbox_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual bool has_para_list() const {
			return true;
		}
		virtual std::vector<paragraph_list_t>* get_para_lists() {
			return &para_lists;
		}
		virtual size_t size() const {
			size_t offset = 0;
			offset += common_header.size();
			offset += 8;
			offset += std::accumulate(para_lists.begin(), para_lists.end(), 0, [](size_t size, auto& para_list) {
				return size + para_list.size(); });
			offset += detail_info.size();
			return offset;
		}
		uint32_t first_info_length;
		uint32_t second_info_length;
		std::vector<paragraph_list_t> para_lists;
		detail_info_t detail_info;
	};

	struct picture_t : control_code_t
	{
		typedef control_code_t::control_t control_t;
		picture_t(control_t code) : code(code), reserved(0), end_code(0), length(0), type(0)
		{}
		~picture_t() {}
		virtual bufferstream& read(bufferstream& stream);
		virtual bufferstream& write(bufferstream& stream);
		virtual control_t get_code() const {
			return code;
		}
		virtual bool has_drawing_object() const {
			return drawing_object.objects.size() > 0;
		}
		virtual bool has_caption() const {
			return true;
		}
		virtual paragraph_list_t* get_caption() {
			return &caption;
		}
		size_t size() const {
			size_t offset = 8 + 4 + data.size() + 1 + data2.size() + data3.size() + caption.size();
			if (has_drawing_object())
				offset += drawing_object.size();
			return offset;
		}
		control_t code;
		uint32_t reserved;
		control_t end_code;

		uint32_t length;
		buffer_t data; // 70
		uint8_t type; // 1: 외부파일, 2 : OLE, 2 : 포함 그림, 3 : Drawing Object
		buffer_t data2; // 273

		// variable data
		buffer_t data3; // length
		paragraph_list_t caption;

		drawing_object_t drawing_object;
	};

#pragma endregion definition of control codes

	struct document_header_t
	{
		document_header_t() = default;
		DECLARE_BINARY_SERIALIZER(document_header_t);
		size_t size() const {
			size_t offset = 0;
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
		size_t size() const {
			size_t offset = 0;
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

	struct extra_info_block_t
	{
		extra_info_block_t() : id(0), length(0)
		{}
		DECLARE_BINARY_SERIALIZER(extra_info_block_t);
		size_t size() const {
			return 4 + 4 + length;
		}
		bool is_last() const {
			return id == 0 && length == 0;
		}
		bool is_second_block() const {
			return ((id & 0x80000000) >> 31) == 1;
		}
		// extra_info_block 2 : id & 0x80000000 == 1;
		uint32_t id; // 1: picture, 2: OLE, 3: hypertext, 6: BG image, 4 : presentation
		uint32_t length;
		buffer_t data;
	};

	struct extra_info_blocks_t
	{
		extra_info_blocks_t()
		{}
		DECLARE_BINARY_SERIALIZER(extra_info_blocks_t);
		size_t size() const {
			return std::accumulate(extra_info_blocks.begin(), extra_info_blocks.end(), 0, [](size_t size, auto& extra_info_block) {
				return size + extra_info_block.size(); });
		}
		std::vector<extra_info_block_t> extra_info_blocks;
	};

	struct second_extra_info_blocks_t
	{
		second_extra_info_blocks_t()
		{}
		DECLARE_BINARY_SERIALIZER(second_extra_info_blocks_t);
		size_t size() const {
			return std::accumulate(second_extra_info_blocks.begin(), second_extra_info_blocks.end(), 0, [](size_t size, auto& extra_info_block) {
				return size + extra_info_block.size(); });
		}
		std::vector<extra_info_block_t> second_extra_info_blocks;
	};

	struct document_tail_t
	{
		document_tail_t()
		{}
		DECLARE_BINARY_SERIALIZER(document_tail_t);
		size_t size() const {
			return first.size() + second.size();
		}

		bool has_second_extra_block() const {
			return second.size() > 0;
		}

		extra_info_blocks_t first;
		second_extra_info_blocks_t second;
	};

	struct document_t
	{
		document_t() = default;
		size_t size() const {
			return header.size() + body.size() + tail.size();
		}

		document_header_t header;
		document_body_t body;
		document_tail_t tail;
	};

	struct syntax_t
	{
		typedef control_code_t::control_t control_t;
		enum control_ids : control_t
		{
			field = 5,
			bookmark = 6,
			date_format = 7,
			date_code = 8,
			tab = 9,
			table = 10,
			picture = 11,
			para_break = 13,
			line_shape = 14,
			hidden_text = 15,
			header_footer = 16,
			note = 17,
			number_code = 18,
			start_new_number = 19,
			page_number = 20,
			start_odd_hide_number = 21,
			mail_merge = 22,
			text_overlap = 23,
			hypen = 24,
			outline_mark = 25,
			find_mark = 26,
			outline_shape = 28,
			cross_reference = 29,
			blank = 30,
			fixed_space = 31
		};
		syntax_t() = default;
	};

	struct control_builder
	{
		typedef control_code_t::control_t control_t;
		typedef paragraph_t::controls_t controls_t;
		control_builder() {}
		static uint16_t push_back(control_t code, controls_t& controls);
	};
}
}