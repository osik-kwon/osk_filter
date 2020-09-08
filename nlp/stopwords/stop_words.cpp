#include "nlp_pch.h"
#include "stopwords/stop_words.h"
#include <exception>
#include <iostream>
#include <algorithm>
#include <io/file_stream.h>
#include <locale/charset_encoder.h>

#include <boost/algorithm/string.hpp> 
#include <boost/algorithm/string/case_conv.hpp>

namespace nlp
{
	stop_words_t::stop_words_t()
	{}

	stop_words_t::~stop_words_t()
	{}

	void stop_words_t::load_dictionary(const std::string& path)
	{
		try
		{
			std::ifstream dict(filter::to_fstream_path(path));
			dict.exceptions(std::ifstream::badbit | std::ifstream::failbit);
			std::string line;
			while (!dict.eof())
			{
				std::getline(dict, line);
				stop_words.insert(to_wchar(line));
			}
			dict.close();
		}
		catch (const std::exception& e)
		{
			std::cout << e.what();
		}
	}

	void stop_words_t::remove_stop_words(std::map<std::wstring, int>& tokens, size_t min)
	{
		std::map < std::wstring, int> norms;

		for (auto& token : tokens)
		{
			if (token.first.size() < min)
				continue;
			auto unigram = token.first;
			/*
			auto bigram = unigram;
			auto trigram = unigram;
			if (i + 1 < tokens.size())
			{
				bigram += L" ";
				bigram += tokens[i + 1].first;
			}
			if (i + 2 < tokens.size())
			{
				trigram = bigram;
				trigram += L" ";
				trigram += tokens[i + 2].first;
			}
			*/
			if (stop_words.find(unigram) == stop_words.end())
				//	if (stop_words.find(bigram) == stop_words.end())
				//		if (stop_words.find(trigram) == stop_words.end())
			{			
				if(!std::all_of(unigram.begin(), unigram.end(), boost::algorithm::is_digit()))
					norms.insert(std::make_pair(unigram, token.second));
			}
		}
		std::swap(tokens, norms);
	}
	void stop_words_t::remove_stop_words(std::vector<std::wstring>& tokens, size_t min)
	{
		std::vector<std::wstring> norms;
		norms.reserve(tokens.size());
		for (int i = 0; i < tokens.size(); ++i)
		{
			if (tokens[i].size() < min)
				continue;
			auto unigram = tokens[i];
			auto bigram = unigram;
			auto trigram = unigram;
			if (i + 1 < tokens.size())
			{
				bigram += L" ";
				bigram += tokens[i + 1];
			}
			if (i + 2 < tokens.size())
			{
				trigram = bigram;
				trigram += L" ";
				trigram += tokens[i + 2];
			}
			if (stop_words.find(unigram) == stop_words.end())
				if (stop_words.find(bigram) == stop_words.end())
					if (stop_words.find(trigram) == stop_words.end())
						norms.push_back(unigram);
		}
		std::swap(tokens, norms);
	}

}