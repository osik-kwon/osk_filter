#pragma once
#include <fstream>
#include <string>
namespace filter
{
#ifdef _MSC_VER
	extern std::wstring to_fstream_path(const std::string& path);
#else
	extern std::string to_fstream_path(const std::string& path);
#endif
}