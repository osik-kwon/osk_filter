#pragma once
#include <functional>
#include <bitset>
#include <numeric>
#include <regex>

#include "io/binary_iostream.h"
#include "io/zlib.h"
#include "io/compound_file_binary.h"
#include "locale/charset_encoder.h"

namespace filter
{
namespace hwp50
{
	class filter_t
	{
	public:
		typedef cfb_traits::storage_t storage_t;
		typedef cfb_traits::stream_t stream_t;
		typedef binary_traits::byte_t byte_t;
		typedef binary_traits::buffer_t buffer_t;
		typedef binary_traits::bufferstream bufferstream;
		typedef binary_traits::streamsize streamsize;
		typedef syntax_traits::texts_t texts_t;
		typedef syntax_traits::text_t text_t;
		filter_t() {}

		struct file_header_t
		{
			file_header_t() : version(0), kogl(0)
			{
				std::fill(std::begin(reserved), std::end(reserved), 0);
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

		bool replace_privacy(const std::string& import_path, const std::string& export_path, const std::wregex& pattern, char16_t replace_dest)
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
						auto para_texts = read_para_text(para_text_stream, record.get().header.body_size);
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
						write_para_text(para_text_export_stream, para_texts);
						record.get().body = std::move(write_record_buffer);
					}

					auto write_size = std::accumulate(records.begin(), records.end(), 0, [](std::size_t size, auto& record) {
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
			catch (...)
			{
			}
			return false;
		}

		bool replace_all_texts(const std::string& import_path, const std::string& export_path, wchar_t replace_dest)
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
						auto para_texts = read_para_text(para_text_stream, record.get().header.body_size);
						for (auto& para_text : para_texts.controls)
						{
							if (para_text.type == para_text_t::is_char_control)
							{
								std::for_each(para_text.body.begin(), para_text.body.end(), [&replace_dest](auto& code) {
									typedef std::remove_reference< std::remove_cv<decltype(code)>::type >::type code_t;
									if (!syntax_t::is_carriage_return(code))
										code = static_cast<code_t>(replace_dest);
								});
							}
						}
						record.get().header.body_size = para_texts.size();
						buffer_t write_record_buffer;
						write_record_buffer.resize(record.get().header.body_size);
						bufferstream para_text_export_stream(&write_record_buffer[0], write_record_buffer.size());
						write_para_text(para_text_export_stream, para_texts);
						record.get().body = std::move(write_record_buffer);
					}

					auto write_size = std::accumulate(records.begin(), records.end(), 0, [](std::size_t size, auto& record) {
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
			catch (...)
			{
			}
			return false;
		}

		std::vector<texts_t> extract_all_texts(const std::string& import_path)
		{
			try
			{
				std::vector<texts_t> sections;
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
					auto section_texts = extract_texts(records);
					sections.push_back(std::move(section_texts));
				}
				return sections;
			}
			catch (const std::exception& e)
			{
				std::cout << e.what() << std::endl;
			}
			catch (...)
			{
			}
			return std::vector<texts_t>();
		}

		bool decompress_save(const std::string& import_path, const std::string& export_path)
		{
			try
			{
				std::unique_ptr<storage_t> import_storage = cfb_t::make_read_only_storage(import_path);
				std::unique_ptr<storage_t> export_storage = cfb_t::make_writable_storage(export_path);

				auto all_streams = cfb_t::make_full_entries(import_storage, "/");
				auto file_header_name = std::find_if(all_streams.begin(), all_streams.end(), [](const std::string& name) {
					return name == "/FileHeader"; });
				if (file_header_name != all_streams.end())
					all_streams.erase(file_header_name);

				auto import_header = read_file_header(import_storage);
				auto export_header = import_header;
				export_header.set_compressed(false);
				write_file_header(export_storage, export_header);
				for (auto& stream : all_streams)
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
			catch (...)
			{
			}
			return false;
		}
	private:
		file_header_t read_file_header(std::unique_ptr<storage_t>& storage)
		{
			buffer_t buffer = cfb_t::extract_stream(storage, "/FileHeader");
			bufferstream stream(&buffer[0], buffer.size());
			file_header_t file_header;
			file_header.signature = binary_stream_t::read_string(stream, file_header.signature_size);
			file_header.version = binary_stream_t::read_uint32(stream);
			file_header.options = binary_stream_t::read_uint32(stream);
			file_header.extended_options = binary_stream_t::read_uint32(stream);
			file_header.kogl = binary_stream_t::read_uint8(stream);
			return file_header;
		}

		void write_file_header(std::unique_ptr<storage_t>& storage, const file_header_t& file_header)
		{
			buffer_t buffer;
			buffer.resize(256);
			bufferstream stream(&buffer[0], buffer.size());
			binary_stream_t::write_string(stream, file_header.signature);
			binary_stream_t::write_uint32(stream, file_header.version);
			binary_stream_t::write_uint32(stream, file_header.options.to_ulong());
			binary_stream_t::write_uint32(stream, file_header.extended_options.to_ulong());
			binary_stream_t::write_uint8(stream, file_header.kogl);
			cfb_t::make_stream(storage, "/FileHeader", buffer);
		}

		header_t read_header(bufferstream& stream)
		{
			header_t header;
			uint32_t plain = binary_stream_t::read_uint32(stream);
			header.tag = plain & 0x3FF;
			header.level = (plain >> 10) & 0x3FF;
			header.body_size = (plain >> 20) & 0xFFF;

			if (header.body_size == 0xFFF)
			{
				header.body_size = binary_stream_t::read_uint32(stream);
			}
			return header;
		}

		void write_header(bufferstream& stream, const header_t& header)
		{
			uint32_t plain = header.tag;
			plain += (header.level << 10);
			if (header.body_size >= 0xFFF)
			{
				plain += (0xFFF << 20);
				binary_stream_t::write_uint32(stream, plain);
				binary_stream_t::write_uint32(stream, header.body_size);
			}
			else
			{
				plain += (header.body_size << 20);
				binary_stream_t::write_uint32(stream, plain);
			}
		}

		buffer_t read_body(bufferstream& stream, streamsize size)
		{
			return binary_stream_t::read(stream, size);
		}

		void write_body(bufferstream& stream, const buffer_t& buffer)
		{
			return binary_stream_t::write(stream, buffer);
		}

		std::vector<record_t> read_records(bufferstream& stream)
		{
			std::vector<record_t> records;
			stream.seekg(0);
			try
			{
				do
				{
					record_t record;
					record.header = read_header(stream);
					record.body = read_body(stream, record.header.body_size);
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

		void write_records(bufferstream& stream, const std::vector<record_t>& records)
		{
			for (auto& record : records)
			{
				write_header(stream, record.header);
				write_body(stream, record.body);
			}
		}

		para_text_t read_para_text(bufferstream& stream, streamsize size)
		{
			const streamsize size_of_control = sizeof(syntax_t::control_t);
			const streamsize size_of_inline_control = 7 * sizeof(syntax_t::control_t); // TODO: abstract 7
			para_text_t para_text;
			for (streamsize offset = 0; offset < size; offset += size_of_control)
			{
				syntax_t::control_t code = binary_stream_t::read_uint16(stream);
				if (syntax_t::is_extend_control(code))
				{
					para_text_t::control_t control;
					control.body.push_back(code);
					control.type = para_text_t::control_is_t::is_extend_control;
					for (size_t i = 0; i < size_of_inline_control; i += size_of_control)
						control.body.push_back(binary_stream_t::read_uint16(stream));
					offset += size_of_inline_control;
					para_text.controls.push_back(std::move(control));
				}
				else if (syntax_t::is_inline_control(code))
				{
					para_text_t::control_t control;
					control.body.push_back(code);
					control.type = para_text_t::control_is_t::is_inline_control;
					for (size_t i = 0; i < size_of_inline_control; i += size_of_control)
						control.body.push_back(binary_stream_t::read_uint16(stream));
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
			return para_text;
		}

		void write_para_text(bufferstream& stream, const para_text_t& records)
		{
			for (auto& control : records.controls)
			{
				for (auto& code : control.body)
				{
					binary_stream_t::write_uint16(stream, code);
				}
			}
		}

		text_t extract_para_text(bufferstream& stream, streamsize size)
		{
			text_t texts;
			const streamsize size_of_control = sizeof(syntax_t::control_t);
			const streamsize size_of_inline_control = 7 * sizeof(syntax_t::control_t); // TODO: abstract 7

			for (streamsize offset = 0; offset < size; offset += size_of_control)
			{
				syntax_t::control_t code = binary_stream_t::read_uint16(stream);
				if (syntax_t::is_char_control(code))
				{
					if (syntax_t::is_carriage_return(code))
						texts.push_back(L'\n'); // TODO: normalize
					else
						texts.push_back(static_cast<text_t::value_type>(code));
				}
				else
				{
					// TODO: implement tab
					auto inline_contol = binary_stream_t::read(stream, size_of_inline_control);
					offset += size_of_inline_control;
				}
			}
			return texts;
		}

		texts_t extract_texts(std::vector<record_t>& records)
		{
			texts_t texts;
			for (auto& record : records)
			{
				if (syntax_t::is_para_text(record.header.tag))
				{
					bufferstream stream(&record.body[0], record.header.body_size);
					auto text = extract_para_text(stream, record.header.body_size);
					if (!text.empty())
						texts.push_back(text);
				}
			}
			return texts;
		}
	};
}
}