#pragma once
#include <signature/signature_analyzer.h>

namespace filter
{
namespace signature
{
	class builder_t
	{
	public:
		builder_t() = default;
		static std::unique_ptr< filter::signature::analyzer_t<std::string> > build_string_rules();
	};
}
}