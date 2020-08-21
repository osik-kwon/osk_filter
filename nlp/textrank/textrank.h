#pragma once
#include <string>
#include <vector>

namespace nlp
{
	class text_ranker
	{
	public:
		static bool key_sentences(const std::wstring& texts, std::vector<std::wstring>& sentences, int topK);
	};
}