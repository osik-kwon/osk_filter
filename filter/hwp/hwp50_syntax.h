#pragma once
#include <string>
#include <bitset>
#include <numeric>
#include "define/binary_traits.h"
#include "define/compound_file_binary_traits.h"
#include "io/binary_iostream.h"

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

		friend bufferstream& operator >> (bufferstream& stream, file_header_t& file_header)
		{
			file_header.signature = binary_stream_t::read_string(stream, file_header.signature_size);
			file_header.version = binary_stream_t::read_uint32(stream);
			file_header.options = binary_stream_t::read_uint32(stream);
			file_header.extended_options = binary_stream_t::read_uint32(stream);
			file_header.kogl = binary_stream_t::read_uint8(stream);
			return stream;
		}

		friend bufferstream& operator << (bufferstream& stream, const file_header_t& file_header)
		{
			binary_stream_t::write_string(stream, file_header.signature);
			binary_stream_t::write_uint32(stream, file_header.version);
			binary_stream_t::write_uint32(stream, file_header.options.to_ulong());
			binary_stream_t::write_uint32(stream, file_header.extended_options.to_ulong());
			binary_stream_t::write_uint8(stream, file_header.kogl);
			return stream;
		}

		bool is_compressed() const {
			return options[0];
		}

		void set_compressed(bool compress) {
			options[0] = compress;
		}

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

		std::size_t size() const {
			return sizeof(uint32_t);
		}

		tag_t tag;
		level_t level;
		size_t body_size;
	};

	struct record_t
	{
		typedef binary_traits::buffer_t buffer_t;
		record_t()
		{}

		std::size_t size() const {
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

			std::size_t size() const {
				return body.size() * sizeof(value_type);
			}

			control_is_t type;
			std::vector<value_type> body;
		};

		para_text_t()
		{}

		std::size_t size() const
		{
			return std::accumulate(controls.begin(), controls.end(), 0, [](std::size_t size, auto& control) {
				return size + control.size();
			});
		}

		std::vector<control_t> controls;
	};

	struct syntax_t
	{
		typedef uint16_t control_t;
		enum tag_t : header_t::tag_t
		{
			// TODO: implement
			HWPTAG_BEGIN = 16,
			// docinfo

			// body text
			HWPTAG_PARA_TEXT = HWPTAG_BEGIN + 51
		};

		static bool is_carriage_return(control_t code)
		{
			return (code == 13);
		}

		static bool is_para_text(control_t code)
		{
			return (code == HWPTAG_PARA_TEXT);
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
	};
}
}