#include "filter_pch.h"
#include "hwp/hwp50_syntax.h"
#include "cryptor/cryptor.h"

namespace filter
{
namespace hwp50
{
	// serializers
	bufferstream& operator >> (bufferstream& stream, file_header_t& file_header)
	{
		file_header.signature = binary_io::read_string(stream, file_header.signature_size);
		file_header.version = binary_io::read_uint32(stream);
		file_header.options = binary_io::read_uint32(stream);
		file_header.extended_options = binary_io::read_uint32(stream);
		file_header.kogl = binary_io::read_uint8(stream);
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const file_header_t& file_header)
	{
		binary_io::write_string(stream, file_header.signature);
		binary_io::write_uint32(stream, file_header.version);
		binary_io::write_uint32(stream, file_header.options.to_ulong());
		binary_io::write_uint32(stream, file_header.extended_options.to_ulong());
		binary_io::write_uint8(stream, file_header.kogl);
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, header_t& header)
	{
		uint32_t plain = binary_io::read_uint32(stream);
		header.tag = plain & 0x3FF;
		header.level = (plain >> 10) & 0x3FF;
		header.body_size = (plain >> 20) & 0xFFF;

		if (header.body_size == 0xFFF)
		{
			header.body_size = binary_io::read_uint32(stream);
		}

		/*
		// TODO: remove DEBUG
		if (syntax_t::is_para_text(header.tag))
		{
			std::cout << "[is_para_text] ";
		}
		else if (syntax_t::is_para_header(header.tag))
		{
			std::cout << "[is_para_header] ";
		}
		else if (syntax_t::is_list_header(header.tag))
		{
			std::cout << "[is_list_header] ";
		}
		std::cout << "tag is "  << header.tag << " , level is " << header.level << std::endl;
		*/

		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const header_t& header)
	{
		uint32_t plain = header.tag;
		plain += (header.level << 10);
		if (header.body_size >= 0xFFF)
		{
			plain += (0xFFF << 20);
			binary_io::write_uint32(stream, plain);
			binary_io::write_uint32(stream, header.body_size);
		}
		else
		{
			plain += (header.body_size << 20);
			binary_io::write_uint32(stream, plain);
		}
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, distribute_doc_data_record_t& record)
	{
		stream >> record.header;
		if( record.header.tag != syntax_t::HWPTAG_DISTRIBUTE_DOC_DATA )
			throw std::runtime_error("record tag is NOT HWPTAG_DISTRIBUTE_DOC_DATA");

		record.body = binary_io::read(stream, record.header.body_size);
		bufferstream body_stream(&record.body[0], record.body.size());	

		cryptor_t cryptor( binary_io::read_uint32(body_stream) );
		cryptor.make_hwp50_distribution_key(record.body);
		if(!cryptor.options.empty())
			record.options = cryptor.options[0];

		// read cipher text
		stream.seekg(0, stream.end);
		streamsize length = stream.tellg();
		if (length <= 260)
			throw std::runtime_error("decrypt error");
		length -= 260;
		stream.seekg(260, stream.beg);
		std::vector<uint8_t> cipher_text = binary_io::read_u8vector(stream, length);

		// decrypt
		record.body = std::move( cryptor.decrypt_aes128_ecb_nopadding(std::move(cipher_text)) );
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const distribute_doc_data_record_t& record)
	{
		// TODO: implement
		stream << record.header;
		binary_io::write(stream, record.body);
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, record_t& record)
	{
		stream >> record.header;
		record.body = binary_io::read(stream, record.header.body_size);
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const record_t& record)
	{
		stream << record.header;
		binary_io::write(stream, record.body);
		return stream;
	}

	bufferstream& operator >> (bufferstream& stream, para_text_t& para_text)
	{
		const streamsize size_of_control = sizeof(syntax_t::control_t);
		const streamsize size_of_inline_control = syntax_t::sizeof_inline_control();
		for (streamsize offset = 0; offset < para_text.body_size; offset += size_of_control)
		{
			syntax_t::control_t code = binary_io::read_uint16(stream);
			if (syntax_t::is_extend_control(code))
			{
				para_text_t::control_t control;
				control.body.push_back(code);
				control.type = para_text_t::control_is_t::is_extend_control;
				for (size_t i = 0; i < size_of_inline_control; i += size_of_control)
					control.body.push_back(binary_io::read_uint16(stream));
				offset += size_of_inline_control;
				para_text.controls.push_back(std::move(control));
			}
			else if (syntax_t::is_inline_control(code))
			{
				para_text_t::control_t control;
				control.body.push_back(code);
				control.type = para_text_t::control_is_t::is_inline_control;
				for (size_t i = 0; i < size_of_inline_control; i += size_of_control)
					control.body.push_back(binary_io::read_uint16(stream));
				offset += size_of_inline_control;
				para_text.controls.push_back(std::move(control));
			}
			else // char control
			{
				if (!para_text.controls.empty() &&
					para_text.controls.back().type == para_text_t::control_is_t::is_char_control)
				{
					para_text.controls.back().body.push_back(code);
				}
				else
				{
					para_text_t::control_t control;
					control.type = para_text_t::control_is_t::is_char_control;
					control.body.push_back(code);
					para_text.controls.push_back(std::move(control));
				}
			}
		}
		return stream;
	}

	bufferstream& operator << (bufferstream& stream, const para_text_t& para_text)
	{
		for (auto& control : para_text.controls)
		{
			for (auto& code : control.body)
			{
				binary_io::write_uint16(stream, code);
			}
		}
		return stream;
	}
}
}