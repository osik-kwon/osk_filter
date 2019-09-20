#pragma once
#include <string>
#include <bitset>
#include <numeric>
#include <map>
#include <memory>
#include "traits/binary_traits.h"
#include "traits/compound_file_binary_traits.h"
#include "io/binary_iostream.h"
#include "cryptor/cryptor.h"

namespace filter
{
namespace hwp50
{
	typedef binary_traits::byte_t byte_t;
	typedef binary_traits::buffer_t buffer_t;
	typedef binary_traits::bufferstream bufferstream;
	typedef binary_traits::streamsize streamsize;

	struct file_header_t
	{
		file_header_t() : version(0), kogl(0)
		{
			std::fill(std::begin(reserved), std::end(reserved), 0);
		}

		DECLARE_BINARY_SERIALIZER(file_header_t);
		
		size_t size() const {
			return 256; // 256 bytes
		}

		enum options_t : uint32_t
		{
			compressed = 0,
			crypted = 1,
			distribution = 2
		};

		static const size_t signature_size = 32;
		std::string signature;
		uint32_t version;
		std::bitset<32> options;
		std::bitset<32> extended_options;
		binary_traits::byte_t kogl;
		binary_traits::byte_t reserved[207];
	};

	struct header_t
	{
		typedef uint32_t tag_t;
		typedef uint32_t size_t;
		typedef uint32_t level_t;
		header_t() : tag(0), level(0), body_size(0)
		{}

		DECLARE_BINARY_SERIALIZER(header_t);
		size_t size() const {
			return sizeof(uint32_t);
		}

		tag_t tag;
		level_t level;
		size_t body_size;
	};

	struct distribute_doc_data_record_t
	{
		typedef binary_traits::buffer_t buffer_t;
		distribute_doc_data_record_t()
		{}

		DECLARE_BINARY_SERIALIZER(distribute_doc_data_record_t);

		size_t size() const {
			return header.size() + body.size();
		}

		header_t header;
		buffer_t body;

		// edit
		std::bitset<8> options; // 0 : ���� ����, 1 : �μ� ����
		// cryptor
		cryptor_t cryptor;
	};

	struct record_t
	{
		typedef binary_traits::buffer_t buffer_t;
		record_t()
		{}

		DECLARE_BINARY_SERIALIZER(record_t);

		size_t size() const {
			return header.size() + body.size();
		}

		header_t header;
		buffer_t body;
	};

	struct para_text_t
	{
		enum control_is_t : uint16_t {
			is_char_control = 0,
			is_extend_control,
			is_inline_control
		};

		struct control_t
		{
			typedef uint16_t value_type;
			control_t() : type(is_char_control)
			{}

			size_t size() const {
				return body.size() * sizeof(value_type);
			}

			control_is_t type;
			std::vector<value_type> body;
		};

		para_text_t(size_t size) : body_size(size)
		{}

		DECLARE_BINARY_SERIALIZER(para_text_t);

		size_t size() const
		{
			return std::accumulate(controls.begin(), controls.end(), 0, [](size_t size, auto& control) {
				return size + control.size();
			});
		}

		size_t body_size;
		std::vector<control_t> controls;
	};

	class consumer_t
	{
	public:
		typedef cfb_traits::storage_t storage_t;
		typedef cfb_traits::stream_t stream_t;
		consumer_t();
		void open(const std::string& path);
		std::string file_header_entry() const;
		file_header_t read_file_header(std::unique_ptr<storage_t>& storage) const;
		file_header_t read_file_header() const;
		bool can_compress(const std::string& entry) const;
		bool can_crypt(const std::string& entry) const;
		bool has_paragraph(const std::string& entry) const;
		std::vector<record_t> read_records(bufferstream& stream) const;
		void write_records(bufferstream& stream, const std::vector<record_t>& records) const;
		std::map<std::string, std::unique_ptr<buffer_t> >& get_streams() {
			return streams;
		}
		std::map<std::string, std::unique_ptr<distribute_doc_data_record_t> >& get_distribute_doc_data_records() {
			return distribute_doc_data_records;
		}
	private:
		const std::regex paragraph_rule;
		const std::regex compress_rule;
		const std::regex crypt_rule;
		std::map<std::string, std::unique_ptr<buffer_t> >streams;
		std::map<std::string, std::unique_ptr<distribute_doc_data_record_t> > distribute_doc_data_records;
	};

	class producer_t
	{
	public:
		producer_t() = default;
		void save(const std::string& path, std::unique_ptr<consumer_t>& consumer);
	private:
	};

	struct syntax_t
	{
		typedef uint16_t control_t;
		enum tag_t : header_t::tag_t
		{
			// TODO: implement
			HWPTAG_BEGIN = 16,
			// docinfo
			HWPTAG_DOCUMENT_PROPERTIES			=	HWPTAG_BEGIN,		//�����Ӽ�
			HWPTAG_ID_MAPPINGS					=	HWPTAG_BEGIN + 1,	//ID Mapping �������
			HWPTAG_BIN_DATA						=	HWPTAG_BEGIN + 2,	//Bin Data
			HWPTAG_FACE_NAME					=	HWPTAG_BEGIN + 3,	//Typeface Name
			HWPTAG_BORDER_FILL					=	HWPTAG_BEGIN + 4,	//�׵θ�/���
			HWPTAG_CHAR_SHAPE					=	HWPTAG_BEGIN + 5,	//���� ���
			HWPTAG_TAB_DEF						=	HWPTAG_BEGIN + 6,	//������
			HWPTAG_NUMBERING					=	HWPTAG_BEGIN + 7,	//��ȣ ����
			HWPTAG_BULLET						=	HWPTAG_BEGIN + 8,	//�Ҹ� ����
			HWPTAG_PARA_SHAPE					=	HWPTAG_BEGIN + 9,	//���� ���
			HWPTAG_STYLE						=	HWPTAG_BEGIN + 10,	//��Ÿ��
			HWPTAG_DOC_DATA						=	HWPTAG_BEGIN + 11,	//���� ������ �������� parameter set archive
			HWPTAG_DISTRIBUTE_DOC_DATA			=	HWPTAG_BEGIN + 12,	//������ ���� ������
			HWPTAG_RESERVED						=	HWPTAG_BEGIN + 13,	//����
			HWPTAG_COMPATIBLE_DOCUMENT			=	HWPTAG_BEGIN + 14,	//ȣȯ ����
			HWPTAG_LAYOUT_COMPATIBILITY			=	HWPTAG_BEGIN + 15,	//���̾ƿ� ȣȯ��
			HWPTAG_TRACKCHANGE					=	HWPTAG_BEGIN + 16,	//���� ���� ����
			HWPTAG_MEMO_SHAPE					=	HWPTAG_BEGIN + 76,	//�޸� ���
			HWPTAG_FORBIDDEN_CHAR				=	HWPTAG_BEGIN + 78,	//��Ģó�� ����
			HWPTAG_TRACK_CHARNGE				=	HWPTAG_BEGIN + 80,	//���� ���� ���� �� ���
			HWPTAG_TRACK_CHARNGE_AUTHOR			=	HWPTAG_BEGIN + 81,	//���� ���� �ۼ���

			// body text
			HWPTAG_PARA_HEADER					=	HWPTAG_BEGIN + 50,	//���� ���
			HWPTAG_PARA_TEXT					=	HWPTAG_BEGIN + 51,	//������ �ؽ�Ʈ ����
			HWPTAG_PARA_CHAR_SHAPE				=	HWPTAG_BEGIN + 52,	//������ ���� ��� ����
			HWPTAG_PARA_LINE_SEG				=	HWPTAG_BEGIN + 53,	//������ �� �ٿ� ���� ALGIN ����
			HWPTAG_PARA_RANGE_TAG				=	HWPTAG_BEGIN + 54,	//������ RAGNE TAG ����
			HWPTAG_CTRL_HEADER					=	HWPTAG_BEGIN + 55,	//��Ʈ���� ��� ���
			HWPTAG_LIST_HEADER					=	HWPTAG_BEGIN + 56,	//���� ����Ʈ�� ��� ���
			HWPTAG_PAGE_DEF						=	HWPTAG_BEGIN + 57,	//���� ���� ��������
			HWPTAG_FOOTNOTE_SHAPE				=	HWPTAG_BEGIN + 58,	//���� ���� ����/���� ���
			HWPTAG_PAGE_BORDER_FILL				=	HWPTAG_BEGIN + 59,	//���� ���� �� �׵θ� ���
			HWPTAG_SHAPE_COMPONENT				=	HWPTAG_BEGIN + 60,	//shape object ���� shape component
			HWPTAG_TABLE						=	HWPTAG_BEGIN + 61,	//ǥ
			HWPTAG_SHAPE_COMPONENT_LINE			=	HWPTAG_BEGIN + 62,	//shape component �� ����, ���ἱ
			HWPTAG_SHAPE_COMPONENT_RECTANGLE	=	HWPTAG_BEGIN + 63,	//shape component �� �簢��
			HWPTAG_SHAPE_COMPONENT_ELLIPSE		=	HWPTAG_BEGIN + 64,	//shape component �� Ÿ��
			HWPTAG_SHAPE_COMPONENT_ARC			=	HWPTAG_BEGIN + 65,	//shape component �� ��ũ
			HWPTAG_SHAPE_COMPONENT_POLYGON		=	HWPTAG_BEGIN + 66,	//shape component �� �ٰ���
			HWPTAG_SHAPE_COMPONENT_CURVE		=	HWPTAG_BEGIN + 67,	//shape component �� �
			HWPTAG_SHAPE_COMPONENT_OLE			=	HWPTAG_BEGIN + 68,	//shape component �� OLE
			HWPTAG_SHAPE_COMPONENT_PICTURE		=	HWPTAG_BEGIN + 69,	//shape component �� �׸�
			HWPTAG_SHAPE_COMPONENT_CONTAINER	=	HWPTAG_BEGIN + 70,	//shape component �� �����̳�
			HWPTAG_CTRL_DATA					=	HWPTAG_BEGIN + 71,	//��Ʈ���� ������ �������� parameter set archive
			HWPTAG_EQEDIT						=	HWPTAG_BEGIN + 72,	//������ ��ũ��Ʈ ����
			HWPTAG_RESERVE						=	HWPTAG_BEGIN + 73,	//����
			HWPTAG_SHAPE_COMPONENT_TEXTART		=	HWPTAG_BEGIN + 74,	//shape component �� �۸ʽ�
			HWPTAG_SAMPLEOBJ					=	HWPTAG_BEGIN + 75,	//��� ��ü
			HWPTAG_MEMO							=	HWPTAG_BEGIN + 76,	//�޸� ���
			HWPTAG_MEMOLIST						=	HWPTAG_BEGIN + 77,	//�޸� ����Ʈ ���
			HWPTAG_CHARTDATA					=	HWPTAG_BEGIN + 79,	//��Ʈ ������
			HWPTAG_VIDEODATA					=	HWPTAG_BEGIN + 82,	//���� ������
		};

		syntax_t() = default;
		static bool is_carriage_return(control_t code)
		{
			return (code == 13);
		}

		static bool is_tab(control_t code)
		{
			return (code == 9);
		}

		static bool is_para_header(control_t code)
		{
			return (code == HWPTAG_PARA_HEADER);
		}

		static bool is_para_text(control_t code)
		{
			return (code == HWPTAG_PARA_TEXT);
		}

		static bool is_list_header(control_t code)
		{
			return (code == HWPTAG_LIST_HEADER);
		}

		static bool is_char_control(control_t code)
		{
			return !is_extend_control(code) && !is_inline_control(code);
		}

		static bool is_extend_control(control_t code)
		{
			return (
				(code >= 1 && code <= 3) || (code >= 11 && code <= 12) ||
				(code >= 14 && code <= 18) || (code >= 21 && code <= 23)
				);
		}

		static bool is_inline_control(control_t code)
		{
			return ((code >= 4 && code <= 9) || (code >= 19 && code <= 20));
		}

		static std::string section_root()
		{
			return std::string("/BodyText/");
		}

		static std::string viewtext_root()
		{
			return std::string("/ViewText/");
		}

		static size_t sizeof_inline_control() {
			return 7 * sizeof(syntax_t::control_t);
		}
	};
}
}