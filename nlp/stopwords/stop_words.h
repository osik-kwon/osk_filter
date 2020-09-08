#pragma once
#include <string>
#include <map>
#include <set>
#include <vector>

namespace nlp
{
	class stop_words_t
	{
	public:
		stop_words_t();
		~stop_words_t();
		void load_dictionary(const std::string& path);
		void remove_stop_words(std::vector<std::wstring>& src, size_t min = 0);
		void remove_stop_words(std::map<std::wstring, int>& tokens, size_t min = 0);
	private:
		std::set<std::wstring> stop_words;
	};
}