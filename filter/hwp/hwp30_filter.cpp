#include "filter_pch.h"
#include <fstream>
#include <functional>
#include "hwp/hwp30_filter.h"
#include "hwp/hwp30_syntax.h"
#include "locale/charset_encoder.h"

#include "io/binary_iostream.h"
#include "io/zlib.h"

namespace filter
{
namespace hwp30
{

	struct controller_t
	{
		typedef std::vector< std::reference_wrapper<hchar_t> > para_ref_t;
		typedef std::vector<para_ref_t> para_list_ref_t;
		controller_t() = default;
		static void extract_texts(const para_list_ref_t& para_list_ref, filter_t::section_t& para_texts);
		static void extract_para_list_ref(const paragraph_list_t& section, para_list_ref_t& para_list_ref);
	};

	void controller_t::extract_texts(const para_list_ref_t& para_list_ref, filter_t::section_t& para_texts)
	{
		for (auto& para_ref : para_list_ref)
		{
			filter_t::para_t para_text;
			for (auto code : para_ref)
			{
				auto utf16 = to_utf16(code.get().utf32);
				if (utf16.size() == 1)
				{
					if ( utf16[0] == syntax_t::para_break )
						para_text.push_back(L'\n'); // TODO: normalize
					else if (utf16[0] == syntax_t::tab )
						para_text.push_back(L'\t'); // TODO: normalize
					else if (utf16[0] == syntax_t::hypen)
						para_text.push_back(L'-'); // TODO: normalize
					else if (utf16[0] == syntax_t::hypen)
						para_text.push_back(L' '); // TODO: normalize
					else if (utf16[0] == syntax_t::fixed_space)
						para_text.push_back(L' '); // TODO: normalize
					else
						para_text.push_back(utf16[0]);
				}
			}
			if (!para_text.empty())
			{
				if (para_text.back() != L'\n') // TODO: normalize
					para_text.push_back(L'\n'); // TODO: normalize
				para_texts.push_back(std::move(para_text));
			}
		}
	}

	void controller_t::extract_para_list_ref(const paragraph_list_t& section, para_list_ref_t& para_list_ref)
	{
		for (auto& para : section.para_list)
		{
			para_ref_t para_ref;
			for (auto& control : para.controls)
			{
				if (!control->is_control_code())
				{
					switch (control->get_code())
					{
						case syntax_t::tab:
						{
							tab_control_t* tab = dynamic_cast<tab_control_t*>(control.get());
							if (tab)
								para_ref.push_back(dynamic_cast<hchar_t&>(tab->code));
						}
						break;
						case syntax_t::hypen:
						{
							hypen_t* hypen = dynamic_cast<hypen_t*>(control.get());
							if (hypen)
								para_ref.push_back(dynamic_cast<hchar_t&>(hypen->code));
						}
						break;
						case syntax_t::blank:
						{
							blank_t* blank = dynamic_cast<blank_t*>(control.get());
							if (blank)
								para_ref.push_back(dynamic_cast<hchar_t&>(blank->code));
						}
						break;
						case syntax_t::fixed_space:
						{
							fixed_space_t* fixed_space = dynamic_cast<fixed_space_t*>(control.get());
							if (fixed_space)
								para_ref.push_back(dynamic_cast<hchar_t&>(fixed_space->code));
						}
						break;
						default:
						{
							hchar_t* code = dynamic_cast<hchar_t*>(control.get());
							if (code)
								para_ref.push_back(dynamic_cast<hchar_t&>(*control.get()));
						}
						break;
					}
				}
				else if ( control->has_drawing_object() )
				{
					if( control->get_code() != syntax_t::picture )
						throw std::runtime_error("invalid syntax : drawing object should follow picture");

					picture_t* picture = dynamic_cast<picture_t*>(control.get());
					if (!picture)
						throw std::runtime_error("invalid syntax : drawing object should follow picture");

					if (!para_ref.empty())
						para_list_ref.push_back(std::move(para_ref));
					para_ref = para_ref_t();

					for (auto& object : picture->drawing_object.objects)
					{
						para_ref_t control_para_ref;
						if (object->get_para_lists())
						{
							auto& para_lists = *object->get_para_lists();
							for (auto& para_list : para_lists)
								extract_para_list_ref(para_list, para_list_ref);
						}
						if (!control_para_ref.empty())
							para_list_ref.push_back(std::move(control_para_ref));
					}
				}
				else if (control->has_para_list())
				{
					if (!para_ref.empty())
						para_list_ref.push_back(std::move(para_ref));
					para_ref = para_ref_t();

					para_ref_t control_para_ref;
					if (control->get_para_lists())
					{
						auto& para_lists = *control->get_para_lists();
						for (auto& para_list : para_lists)
							extract_para_list_ref(para_list, para_list_ref);
					}
					if (!control_para_ref.empty())
						para_list_ref.push_back(std::move(control_para_ref));
				}
				if (control->has_caption())
				{
					para_ref_t caption_para_ref;
					if (control->get_caption())
						extract_para_list_ref(*control->get_caption(), para_list_ref);
					if (!caption_para_ref.empty())
						para_list_ref.push_back(std::move(caption_para_ref));
				}
			}

			if (!para_ref.empty())
				para_list_ref.push_back(std::move(para_ref));
		}
	}

	filter_t::buffer_t filter_t::read_file(const std::string& path)
	{
		std::ifstream file(path, std::ios::binary);
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
	
	buffer_t filter_t::extract_body(buffer_t& buffer, bufferstream& stream, std::unique_ptr<document_t>& document)
	{
		streamsize stream_cur = stream.tellg();
		streamsize stream_end = buffer.size();
		streamsize body_size = stream_end - stream_cur;
		buffer_t body = binary_io::read(stream, body_size);
		if (document->header.doc_info.compressed != 0) // decompress : 0
			return hwp_zip::decompress_noexcept(body);
		return body;
	}

	std::unique_ptr<document_t> filter_t::parse(buffer_t& buffer)
	{
		std::unique_ptr<document_t> document(std::make_unique<document_t>());
		bufferstream header_stream(&buffer[0], buffer.size());
		header_stream >> document->header;

		buffer_t body_tail = extract_body(buffer, header_stream, document);
		if(body_tail.empty())
			throw std::runtime_error("hwp30 parse body error");
		bufferstream body_tail_stream(&body_tail[0], body_tail.size());
		body_tail_stream >> document->body;
		body_tail_stream >> document->tail;
		return document;
	}

	std::unique_ptr<document_t> filter_t::open(const std::string& open_path)
	{
		try
		{
			auto import_buffer = read_file(open_path);
			return parse(import_buffer);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return std::unique_ptr<document_t>(std::make_unique<document_t>());
	}

	filter_t::sections_t filter_t::extract_all_texts(const std::string& import_path)
	{
		try
		{
			sections_t sections;
			sections.resize(1);
			auto document = open(import_path);
			controller_t::para_list_ref_t para_list_ref;
			controller_t::extract_para_list_ref(document->body.sections, para_list_ref);
			controller_t::extract_texts(para_list_ref, sections[0]);
			return sections;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return sections_t();
	}

	bool filter_t::save(const std::unique_ptr<document_t>& document, const std::string& save_path)
	{
		try
		{
			buffer_t header_buffer;
			header_buffer.resize(document->header.size());
			if (header_buffer.size() == 0)
				throw std::runtime_error("save buffer size error");
			bufferstream header_stream(&header_buffer[0], header_buffer.size());
			header_stream << document->header;

			buffer_t body_tail_buffer;
			body_tail_buffer.resize(document->body.size() + document->tail.first.size() );
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

			std::ofstream fout(save_path, std::ios::out | std::ios::binary);
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
			return true;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return false;
	}
}
}