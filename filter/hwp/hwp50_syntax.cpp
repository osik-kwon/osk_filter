#include "filter_pch.h"
#include <regex>
#include "hwp/hwp50_syntax.h"
#include "io/compound_file_binary.h"
#include "io/zlib.h"

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

		auto& cryptor = record.cryptor;
		cryptor.reset( binary_io::read_uint32(body_stream) );
		cryptor.decrypt_hwp50_distribution_key(record.body);
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
		cryptor_t& cryptor = const_cast<cryptor_t&>(record.cryptor);
		auto data = cryptor.encrypt_hwp50_distribution_key();
		stream << record.header;
		binary_io::write(stream, data);
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

	consumer_t::consumer_t() :
		compress_rule("(/BodyText/|/ViewText/|/BinData/|/DocHistory/).+"),
		crypt_rule("/ViewText/.+"),
		paragraph_rule("/BodyText/.+"),
		crypt_paragraph_rule("/ViewText/.+")
		/*
		// TODO: verify paragraph_rule, crypt_paragraph_rule
		/Scripts/JScriptVersion
		/Scripts/DefaultJScript
		/DocHistory/HistoryLastDoc
		/DocHistory/VersionLog0 ... N
		*/
	{}

	std::string consumer_t::file_header_entry() const {
		return std::string("/FileHeader");
	}

	file_header_t consumer_t::read_file_header(std::unique_ptr<storage_t>& storage) const
	{
		buffer_t buffer = cfb_t::extract_stream(storage, "/FileHeader");
		bufferstream stream(&buffer[0], buffer.size());

		file_header_t file_header;
		stream >> file_header;
		return file_header;
	}

	file_header_t consumer_t::read_file_header() const
	{
		auto entry = streams.find(file_header_entry());
		if (entry == streams.end())
			throw std::runtime_error("file header not exist");
		bufferstream stream(const_cast<char*>(&entry->second->at(0)), entry->second->size());
		file_header_t header;
		stream >> header;
		return header;
	}

	bool consumer_t::can_compress(const std::string& entry) const
	{
		return std::regex_match(entry, compress_rule);
	}

	bool consumer_t::can_crypt(const std::string& entry) const
	{
		return std::regex_match(entry, crypt_rule);
	}

	bool consumer_t::has_paragraph(const std::string& entry) const
	{
		if (header.options[file_header_t::distribution])
		{
			return std::regex_match(entry, crypt_paragraph_rule);
		}
		return std::regex_match(entry, paragraph_rule);
	}

	std::vector<record_t> consumer_t::read_records(bufferstream& stream) const
	{
		std::vector<record_t> records;
		stream.seekg(0);
		try
		{
			do
			{
				record_t record;
				stream >> record;
				records.push_back(std::move(record));
			} while (!stream.eof());
		}
		catch (const std::exception&)
		{
			if (stream.eof())
				return records;
			throw std::runtime_error("read records error");
		}
		return records;
	}

	void consumer_t::write_records(bufferstream& stream, const std::vector<record_t>& records) const
	{
		for (auto& record : records)
		{
			stream << record;
		}
	}

	void consumer_t::open(const std::string& path)
	{
		try
		{
			auto storage = cfb_t::make_read_only_storage(path);
			auto entries = cfb_t::make_full_entries(storage, "/");
			header = read_file_header(storage);
			for (auto& entry : entries)
			{
				auto plain = cfb_t::extract_stream(storage, entry);
				if (header.options[file_header_t::distribution] && can_crypt(entry))
				{
					bufferstream stream(&plain[0], plain.size());
					distribute_doc_data_record_t record;
					stream >> record;
					distribute_doc_data_records.insert(
						{ entry, std::make_unique<distribute_doc_data_record_t>(record) }
					);				
					plain = std::move(record.body);
				}
				if (header.options[file_header_t::compressed] && can_compress(entry))
					plain = hwp_zip::decompress(plain);

				if (streams.find(entry) != streams.end())
					throw std::runtime_error(entry + " stream already exist");
				if(!plain.empty())
					streams.emplace(std::move(entry), std::make_unique<buffer_t>(std::move(plain)));
			}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	void producer_t::encrypt_to_distribute_stream(const std::string& name, std::unique_ptr<consumer_t>& consumer, buffer_t& src)
	{
		auto& distribute_doc_data_records = consumer->get_distribute_doc_data_records();
		if (distribute_doc_data_records.find(name) == distribute_doc_data_records.end())
			throw std::runtime_error(name + " distribute_doc_data_record is not exist");

		distribute_doc_data_record_t record = *distribute_doc_data_records.find(name)->second;
		if (src.size() % 16 != 0) // 16 byte padding for AES128 ECB encrypt
		{
			int upper = src.size() + (16 - src.size() % 16);
			int mod = upper - src.size();
			for (int i = 0; i < mod; ++i)
				src.push_back(0);
		}
		auto cipher_text = record.cryptor.encrypt_aes128_ecb_nopadding(src);
		buffer_t buffer;
		buffer.resize(4 + 256 + cipher_text.size());

		bufferstream stream(&buffer[0], buffer.size());
		stream << record;
		binary_io::write(stream, cipher_text);
		src = std::move(buffer);
	}

	void producer_t::save(const std::string& path, std::unique_ptr<consumer_t>& consumer)
	{
		try
		{
			auto storage = cfb_t::make_writable_storage(path);
			file_header_t header = consumer->read_file_header();

			for (auto& entry : consumer->get_streams())
			{
				auto& name = entry.first;
				auto& data = entry.second;
				if ( header.options[file_header_t::distribution] && consumer->can_crypt(name) )
				{
					buffer_t compressed = *data;
					if (header.options[file_header_t::compressed] && consumer->can_compress(name))
						compressed = hwp_zip::compress_noexcept(*data);
					if (header.options[file_header_t::distribution])
						encrypt_to_distribute_stream(name, consumer, compressed);
					cfb_t::make_stream(storage, name, compressed);
				}
				else
				{
					cfb_t::make_stream(storage, name,
						(header.options[file_header_t::compressed] && consumer->can_compress(name)) ?
						hwp_zip::compress_noexcept(*data) : *data
					);
				}		
			}
			storage->close();
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}
}
}