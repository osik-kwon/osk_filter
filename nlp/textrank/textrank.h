#pragma once
#include <string>
#include <vector>
#include <memory>
#include <stopwords/stop_words.h>

namespace nlp
{
	class text_ranker
	{
	public:
		text_ranker();
		void load_stop_words(const std::vector<std::string>& stop_words_pathes);
		bool key_sentences(const std::wstring& texts, std::vector< std::pair< std::wstring, double> >& key_sentences, size_t topK);
		bool key_words(const std::wstring& texts, std::vector< std::pair< std::wstring, double> >& keywords, size_t topK);
	private:
		std::unique_ptr<stop_words_t> stop_words;
		std::vector<std::wstring> n_grams;
	};
}