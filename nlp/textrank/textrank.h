#pragma once
#include <string>
#include <vector>

namespace nlp
{
	class text_ranker
	{
	public:
		static bool key_sentences(const std::wstring& texts, std::vector<std::wstring>& sentences, int topK);
		static bool key_words(const std::wstring& texts, std::vector< std::pair< std::wstring, double> >& keywords, int topK);
	};
}