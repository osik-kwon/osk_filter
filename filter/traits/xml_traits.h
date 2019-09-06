#pragma once
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
	};
}