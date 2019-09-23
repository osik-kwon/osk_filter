#include "filter_pch.h"
#include "io/file_stream.h"
#include "locale/charset_encoder.h"
namespace filter
{
#ifdef _MSC_VER
	std::wstring to_fstream_path(const std::string& path)
	{
		return to_wchar(path);
	}
#else
	std::string to_fstream_path(const std::string& path)
	{
		return path;
	}
#endif
}