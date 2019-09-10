#pragma once
#include <string>
#include <vector>
#include <map>
#include <xml/pugixml.hpp>
#include <xlnt/utils/path.hpp>
#include <xlnt/detail/serialization/zstream.hpp>

namespace filter
{
	struct xml_traits
	{
		typedef pugi::xml_document xml_document_t;
		typedef xlnt::path path_t;
		typedef xlnt::detail::izstream izstream_t;
		typedef xlnt::detail::ozstream ozstream_t;

		typedef std::map< std::string, std::unique_ptr<xml_document_t> > part_documents_t;
		typedef std::vector< std::uint8_t > buffer_t;
		typedef std::map< std::string, std::vector< std::uint8_t > > part_buffer_t;
		typedef std::vector<path_t> part_names_t;
	};
}