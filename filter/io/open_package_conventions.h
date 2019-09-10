#pragma once
#include <iostream>
#include <memory>
#include "traits/xml_traits.h"
namespace filter
{
namespace opc
{
	class consumer_t
	{
	public:
		typedef xml_traits::xml_document_t xml_document_t;
		typedef xml_traits::path_t path_t;
		typedef xml_traits::izstream_t izstream_t;
		typedef xml_traits::part_documents_t part_documents_t;
		typedef xml_traits::buffer_t buffer_t;
		typedef xml_traits::part_buffer_t part_buffer_t;
		typedef xml_traits::part_names_t part_names_t;
		consumer_t();
		~consumer_t();
		void open(const path_t& path);
		xml_document_t* get_part(const path_t& path)
		{
			auto part = parts.find(path.string());
			if (part != parts.end())
				return part->second.get();
			return nullptr;
		}
		part_documents_t& get_parts() {
			return parts;
		}
		part_buffer_t& get_buffers() {
			return buffers;
		}
		part_names_t& get_names() {
			return names;
		}
	private:
		std::unique_ptr<izstream_t> open_package(const path_t& path);
		void load_part(const path_t& path, std::unique_ptr<izstream_t>& izstream);

		std::ifstream source;
		part_documents_t parts;
		part_buffer_t buffers;
		part_names_t names;
	};

	class producer_t
	{
	public:
		typedef xml_traits::xml_document_t xml_document_t;
		typedef xml_traits::path_t path_t;
		typedef xml_traits::ozstream_t ozstream_t;
		producer_t();
		~producer_t();
		void save(const path_t& path, std::unique_ptr<consumer_t>& consumer);
	private:
		std::ofstream dest;
	};
}
}