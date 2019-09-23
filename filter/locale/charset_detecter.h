#pragma once
#include <string>
#include <vector>

namespace filter
{
	class charset_detecter
	{
	public:
		charset_detecter() = default;
		static std::string detect(const std::string& data);
		static std::string detect(const std::vector<uint8_t>& data);
		static std::string detect(const std::vector<char>& data);
	};
}