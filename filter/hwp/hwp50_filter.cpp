#include "filter_pch.h"
#include "hwp/hwp50_filter.h"
#include "locale/charset_encoder.h"
#include "io/compound_file_binary.h"
#include "io/zlib.h"

namespace filter
{
namespace hwp50
{
	bool filter_t::replace_privacy(const std::string& import_path, const std::string& export_path, const std::wregex& pattern, char16_t replace_dest)
	{
		try
		{
			std::unique_ptr<storage_t> import_storage = cfb_t::make_read_only_storage(import_path);
			std::unique_ptr<storage_t> export_storage = cfb_t::make_writable_storage(export_path);
			auto section_streams = cfb_t::make_full_entries(import_storage, syntax_t::section_root());
			auto all_streams_except_sections = cfb_t::make_all_streams_except(import_storage, section_streams);
			auto import_header = read_file_header(import_storage);
			cfb_t::copy_streams(import_storage, export_storage, all_streams_except_sections);
			for (auto& section_stream : section_streams)
			{
				auto section = cfb_t::extract_stream(import_storage, section_stream);
				if (import_header.is_compressed())
					section = hwp_zip::decompress_noexcept(section);

				bufferstream records_stream(&section[0], section.size());
				auto records = read_records(records_stream);
				std::vector<std::reference_wrapper<record_t>> para_text_record;
				std::for_each(records.begin(), records.end(), [&para_text_record](record_t& record) {
					if (syntax_t::is_para_text(record.header.tag))
						para_text_record.push_back(record);
				});

				for (auto record : para_text_record)
				{
					bufferstream para_text_stream(&record.get().body[0], record.get().header.body_size);
					para_text_t para_texts(record.get().header.body_size);
					para_text_stream >> para_texts;
					for (auto& para_text : para_texts.controls)
					{
						if (para_text.type == para_text_t::is_char_control)
						{
							std::wstring texts;
							std::copy(para_text.body.begin(), para_text.body.end(), std::back_inserter(texts));
							std::match_results<std::wstring::iterator> results;
							auto begin = texts.begin();
							while (std::regex_search(begin, texts.end(), results, pattern))
							{
								for (auto i = results[0].first; i != results[0].second; ++i)
								{
									*i = replace_dest;
								}
								begin += results.position() + results.length();
							}

							para_text.body.clear();
							std::copy(texts.begin(), texts.end(), std::back_inserter(para_text.body));
						}
					}
					record.get().header.body_size = para_texts.size();
					buffer_t write_record_buffer;
					write_record_buffer.resize(record.get().header.body_size);
					bufferstream para_text_export_stream(&write_record_buffer[0], write_record_buffer.size());
					para_text_export_stream << para_texts;
					record.get().body = std::move(write_record_buffer);
				}

				auto write_size = std::accumulate(records.begin(), records.end(), 0, [](size_t size, auto& record) {
					return size + record.size(); });

				buffer_t write_buffer;
				write_buffer.resize(write_size);
				bufferstream write_records_stream(&write_buffer[0], write_buffer.size());
				write_records(write_records_stream, records);

				if (!write_buffer.empty())
				{
					if (import_header.is_compressed())
						write_buffer = hwp_zip::compress_noexcept(write_buffer);
					cfb_t::make_stream(export_storage, section_stream, write_buffer);
				}
			}
			export_storage->close();
			return true;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return false;
	}

	filter_t::sections_t filter_t::extract_all_texts(const std::string& import_path)
	{
		try
		{
			sections_t sections;
			std::unique_ptr<storage_t> import_storage = cfb_t::make_read_only_storage(import_path);
			auto section_streams = cfb_t::make_full_entries(import_storage, syntax_t::section_root());
			auto header = read_file_header(import_storage);

			for (auto& section_stream : section_streams)
			{
				auto section = cfb_t::extract_stream(import_storage, section_stream);
				if (header.is_compressed())
				{
					section = hwp_zip::decompress(section);
				}

				bufferstream stream(&section[0], section.size());
				auto records = read_records(stream);
				auto section_text = extract_section_text(records);
				sections.push_back(std::move(section_text));
			}
			return sections;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return sections_t();
	}

	bool filter_t::decompress_save(const std::string& import_path, const std::string& export_path)
	{
		try
		{
			std::unique_ptr<storage_t> import_storage = cfb_t::make_read_only_storage(import_path);
			std::unique_ptr<storage_t> export_storage = cfb_t::make_writable_storage(export_path);
			auto all_streams_except_file_header = cfb_t::make_all_streams_except(import_storage, "/FileHeader");
			auto import_header = read_file_header(import_storage);
			auto export_header = import_header;
			export_header.set_compressed(false);
			write_file_header(export_storage, export_header);

			for (auto& stream : all_streams_except_file_header)
			{
				auto plain = cfb_t::extract_stream(import_storage, stream);
				if (import_header.is_compressed())
					plain = hwp_zip::decompress_noexcept(plain);
				if (!plain.empty())
					cfb_t::make_stream(export_storage, stream, plain);
			}
			export_storage->close();
			return true;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return false;
	}

	file_header_t filter_t::read_file_header(std::unique_ptr<storage_t>& storage)
	{
		buffer_t buffer = cfb_t::extract_stream(storage, "/FileHeader");
		bufferstream stream(&buffer[0], buffer.size());

		file_header_t file_header;
		stream >> file_header;
		return file_header;
	}

	void filter_t::write_file_header(std::unique_ptr<storage_t>& storage, const file_header_t& file_header)
	{
		buffer_t buffer;
		buffer.resize(file_header.size());
		bufferstream stream(&buffer[0], buffer.size());
		stream << file_header;
		cfb_t::make_stream(storage, "/FileHeader", buffer);
	}

	std::vector<record_t> filter_t::read_records(bufferstream& stream)
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

	void filter_t::write_records(bufferstream& stream, const std::vector<record_t>& records)
	{
		for (auto& record : records)
		{
			stream << record;
		}
	}

	filter_t::para_t filter_t::extract_para_text(bufferstream& stream, streamsize size)
	{
		para_t texts;
		const streamsize size_of_control = sizeof(syntax_t::control_t);
		for (streamsize offset = 0; offset < size; offset += size_of_control)
		{
			syntax_t::control_t code = binary_io::read_uint16(stream);
			if (syntax_t::is_char_control(code))
			{
				if (syntax_t::is_carriage_return(code))
					texts.push_back(L'\n'); // TODO: normalize
				else
					texts.push_back(static_cast<para_t::value_type>(code));
			}
			else
			{
				if (syntax_t::is_tab(code))
					texts.push_back(L'\t'); // TODO: normalize
				auto inline_contol = binary_io::read(stream, syntax_t::sizeof_inline_control());
				offset += syntax_t::sizeof_inline_control();
			}
		}
		return texts;
	}

	filter_t::section_t filter_t::extract_section_text(std::vector<record_t>& records)
	{
		section_t section;
		for (auto& record : records)
		{
			if (syntax_t::is_para_text(record.header.tag))
			{
				bufferstream stream(&record.body[0], record.header.body_size);
				auto para = extract_para_text(stream, record.header.body_size);
				if (!para.empty())
					section.push_back(para);
			}
		}
		return section;
	}
}
}