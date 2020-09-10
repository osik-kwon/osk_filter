#include "nlp_pch.h"
#include "textrank/textrank.h"

#include <iostream>
#include "locale/charset_encoder.h"

#include <boost/algorithm/string.hpp> 
#include <boost/algorithm/string/case_conv.hpp>


#include <boost/algorithm/searching/boyer_moore.hpp>
#include <boost/algorithm/searching/boyer_moore_horspool.hpp>
#include <boost/algorithm/searching/knuth_morris_pratt.hpp>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>

#include <regex>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <cmath>
#include <set>
#include <map>
#include <sstream>
#include <chrono>

using namespace std;

namespace nlp
{
	template <class value_type>
	class string_similarity
	{
	public:
		string_similarity() {}
		typedef double similarity_t;

		static similarity_t similarity(const value_type& longer, const value_type& shorter)
		{
			if (longer.size() < shorter.size())
			{
				return similarity_impl(shorter, longer);
			}
			return similarity_impl(longer, shorter);
		}

		static similarity_t average_of_similarities(const value_type& source, const std::vector<value_type>& words)
		{
			if (words.size() == 0)
				return different();

			similarity_t length = static_cast<similarity_t>(words.size());
			similarity_t sum_of_similarity = static_cast<similarity_t>(0.0);
			for (auto& word : words)
				sum_of_similarity += similarity(source, word);
			return sum_of_similarity / length;
		}

		/*
		Levenshtein distance
		TODO: implement damerau modification (transpose cost)
		time complexity: O(m * n)
		space complexity : O(m * n)
		*/
		template<typename T>
		static typename T::size_type edit_distance(const T& source, const T& target,
			typename T::size_type insert_cost = 1,
			typename T::size_type delete_cost = 1,
			typename T::size_type replace_cost = 1)
		{
			if (source.size() > target.size())
			{
				return edit_distance(target, source, delete_cost, insert_cost, replace_cost);
			}

			typedef typename T::size_type size_type;
			const size_type min_size = source.size(), max_size = target.size();
			std::vector<size_type> lev_dist(min_size + 1);

			lev_dist[0] = 0;
			for (size_type i = 1; i <= min_size; ++i)
			{
				lev_dist[i] = lev_dist[i - 1] + delete_cost;
			}

			for (size_type j = 1; j <= max_size; ++j) {
				size_type previous_diagonal = lev_dist[0], previous_diagonal_save;
				lev_dist[0] += insert_cost;

				for (size_type i = 1; i <= min_size; ++i) {
					previous_diagonal_save = lev_dist[i];
					if (source[i - 1] == target[j - 1]) {
						lev_dist[i] = previous_diagonal;
					}
					else {
						lev_dist[i] = std::min(std::min(lev_dist[i - 1] + delete_cost, lev_dist[i] + insert_cost), previous_diagonal + replace_cost);
					}
					previous_diagonal = previous_diagonal_save;
				}
			}
			return lev_dist[min_size];
		}
	private:
		static similarity_t same() {
			return static_cast<similarity_t>(1.0);
		}

		static similarity_t different() {
			return static_cast<similarity_t>(0.0);
		}

		static similarity_t similarity_impl(const value_type& longer, const value_type& shorter)
		{
			if (longer.size() == 0)
				return same();
			similarity_t longer_length = static_cast<similarity_t>(longer.size());
			if (longer_length == 0)
				return same();
			return (longer_length - static_cast<similarity_t>(edit_distance(longer, shorter)))
				/ longer_length;
		}
	};

	static const std::wstring para_delimiter = L"?!.;£¿£¡¡££»¡¦¡¦¡¦\n";
	static const std::wstring word_delimiter = L"?!.;£¿£¡¡££»¡¦¡¦¡¦\n\t ¡¢,\"¡°¡±()[]¡®¡¯'";

	template <class value_type>
	class TextRank
	{
	public:
		typedef typename value_type::iterator iterator;
		TextRank() :
			m_window_length(3),
			m_max_iter_num(100),
			m_d(0.85),
			m_least_delta(1e-6)
		{}

		TextRank(int window_length, int max_iter_num, double d, double least_delta) :
			m_window_length(window_length),
			m_max_iter_num(max_iter_num),
			m_d(d),
			m_least_delta(least_delta)
		{}

		~TextRank() {}

		void ExtractHighTfWords(const std::vector<value_type>& token_vec, std::vector<std::pair<value_type, double> >& keywords, const size_t topN);
		void ExtractKeyword(const std::vector< std::pair<value_type, iterator> >& token_vec,
			std::vector<std::pair<std::pair<value_type, iterator>, double> >& keywords, const size_t topN = 10);

	private:
		int BuildWordRelation(const std::vector< std::pair<value_type, iterator> >& token_vec, std::map<size_t, std::set<size_t> >& word_neighbors);
		void CalcWordScore(const std::map<size_t, std::set<size_t> >& word_neighbors, std::map<size_t, double>& score_map);
		void UpdateWeightMap(size_t i, size_t j);
		double GetWeight(size_t i, size_t j) const;
		void Clear();

	private:
		size_t m_window_length;
		size_t m_max_iter_num;
		double m_d;
		double m_least_delta;
		
		std::vector< std::pair<value_type, iterator> > m_word_vec;
		std::map<value_type, size_t> m_word_index;                 // word -> word index of m_word_vec
		std::map<std::pair<size_t, size_t>, double> m_weight_map;   // edge weight  
		std::map<size_t, double> m_out_sum_map;                     // out edges weight sum of one node  
	};

	template <class value_type>
	void TextRank<value_type>::ExtractHighTfWords(const vector<value_type>& token_vec, vector<pair<value_type, double> >& keywords, const size_t topN)
	{
		keywords.clear();
		if (token_vec.empty())
			return;
		map<value_type, double> word_tf;
		for (size_t i = 0; i < token_vec.size(); ++i)
		{
			const value_type& word = token_vec[i];
			if (word_tf.find(word) == word_tf.end())
				word_tf[word] = 1;
			else
				word_tf[word] += 1;
		}
		vector<pair<value_type, double> > word_tf_vec(word_tf.begin(), word_tf.end());
		sort(word_tf_vec.begin(), word_tf_vec.end(), [](const std::pair<value_type, double> & x, const std::pair<value_type, double> & y)
		{
			return x.second > y.second;
		});
		for (size_t i = 0; i < word_tf_vec.size(); ++i)
		{
			if (i == topN)
				break;
			keywords.push_back(word_tf_vec[i]);
		}
	}

	template <class value_type>
	void TextRank<value_type>::ExtractKeyword(const vector< std::pair<value_type, iterator> >& token_vec, 
		vector<pair<std::pair<value_type, iterator>, double> >& keywords, const size_t topN)
	{
		keywords.clear();
		if (token_vec.empty())
			return;

		Clear();

		map<size_t, set<size_t> > word_neighbors;
		map<size_t, double> score_map;

		BuildWordRelation(token_vec, word_neighbors);
		CalcWordScore(word_neighbors, score_map);

		vector<pair<size_t, double> > score_vec(score_map.begin(), score_map.end());
		sort(score_vec.begin(), score_vec.end(), [](const std::pair<size_t, double>& x, const std::pair<size_t, double>& y) {
				return x.second > y.second;
			});

		for (size_t i = 0; i < score_vec.size(); ++i)
		{
			if (i == topN)
				break;
			//const value_type& word = m_word_vec[score_vec[i].first].first;
			keywords.push_back(make_pair(m_word_vec[score_vec[i].first], score_vec[i].second));
		}
	}

	template <class value_type>
	void TextRank<value_type>::UpdateWeightMap(size_t i, size_t j)
	{
		if (i > j)
		{
			size_t tmp = i;
			i = j;
			j = tmp;
		}
		pair<size_t, size_t> key(i, j);
		if (m_weight_map.find(key) != m_weight_map.end())
			m_weight_map[key] += 1;
		else
			m_weight_map.insert(make_pair(key, 1.0));
	}

	template <class value_type>
	double TextRank<value_type>::GetWeight(size_t i, size_t j) const
	{
		if (i > j)
		{
			size_t tmp = i;
			i = j;
			j = tmp;
		}
		pair<size_t, size_t> key(i, j);
		if (m_weight_map.find(key) != m_weight_map.end())
			return m_weight_map.at(key);
		return 0;
	}

	template <class value_type>
	int TextRank<value_type>::BuildWordRelation(const vector< std::pair<value_type, iterator> >& token_vec, map<size_t, set<size_t> >& word_neighbors)
	{
		m_weight_map.clear();
		word_neighbors.clear();

		const size_t n = token_vec.size(); 
		for (size_t i = 0; i < n; ++i)
		{
			const value_type& word = token_vec[i].first;
			if (m_word_index.find(word) == m_word_index.end())
			{
				m_word_vec.push_back(token_vec[i]);
				m_word_index.insert(make_pair(word, m_word_vec.size() - 1));
			}
		}

		for (size_t i = 0; i < n; ++i)
		{
			const value_type& word = token_vec[i].first;
			size_t id1 = m_word_index.at(word);

			if (word_neighbors.find(id1) == word_neighbors.end())
				word_neighbors.insert(make_pair(id1, set<size_t>()));

			for (size_t j = 1; j < m_window_length; ++j)
			{
				if (i + j >= n)
					break;
				size_t id2 = m_word_index.at(token_vec[i + j].first);
				if (id2 == id1)
					continue;

				UpdateWeightMap(id1, id2);

				word_neighbors[id1].insert(id2);

				// undirected graph
				if (word_neighbors.find(id2) == word_neighbors.end())
					word_neighbors.insert(make_pair(id2, set<size_t>()));
				word_neighbors[id2].insert(id1);
			}
		}

		// calc weight sum of out-edges
		map<size_t, set<size_t> >::iterator witer = word_neighbors.begin();
		for (; witer != word_neighbors.end(); ++witer)
		{
			size_t id1 = witer->first;
			set<size_t>& neighbors = witer->second;
			set<size_t>::iterator niter = neighbors.begin();
			double sum = 0;
			for (; niter != neighbors.end(); ++niter)
			{
				size_t id2 = *niter;
				sum += GetWeight(id1, id2);
			}
			m_out_sum_map.insert(make_pair(id1, sum));
		}
		return 0;
	}

	template <class value_type>
	void TextRank<value_type>::CalcWordScore(const map<size_t, set<size_t> >& word_neighbors, map<size_t, double>& score_map)
	{
		score_map.clear();

		// initialize
		map<size_t, set<size_t> >::const_iterator witer = word_neighbors.begin();
		for (; witer != word_neighbors.end(); ++witer)
			score_map.insert(make_pair(witer->first, 1.0));

		// iterate
		for (size_t i = 0; i < m_max_iter_num; ++i)
		{
			double max_delta = 0;
			map<size_t, double> new_score_map; // current iteration score

			for (witer = word_neighbors.begin(); witer != word_neighbors.end(); ++witer)
			{
				size_t id1 = witer->first;
				double new_score = 1 - m_d;
				double sum_weight = 0;
				const set<size_t>& neighbors = witer->second;
				set<size_t>::const_iterator niter = neighbors.begin();
				for (; niter != neighbors.end(); ++niter)
				{
					size_t id2 = *niter;
					double weight = GetWeight(id2, id1);
					sum_weight += weight / m_out_sum_map.at(id2) * score_map[id2];
					//sum_weight +=  1.0/word_neighbors[id2].size()*score_map[id2];
				}
				new_score += m_d * sum_weight;
				new_score_map.insert(make_pair(id1, new_score));

				double delta = fabs(new_score - score_map[id1]);
				max_delta = max(max_delta, delta);
			}
			score_map = new_score_map;
			if (max_delta < m_least_delta)
			{
				break;
			}
		}
	}

	template <class value_type>
	void TextRank<value_type>::Clear()
	{
		m_word_vec.clear();
		m_word_index.clear();
		m_weight_map.clear();
		m_out_sum_map.clear();
	}

	template <class value_type>
	class SentenceRank
	{
		struct value_great
		{
			template <typename T>
			bool operator()(const std::pair<T, double>& x, const std::pair<T, double>& y) const
			{
				return x.second > y.second;
			}
		};

	public:
		SentenceRank() :
			m_window_length(3),
			m_max_iter_num(100),
			m_d(0.85),
			m_least_delta(1e-6)
		{}
		SentenceRank(int window_length, int max_iter_num, double d, double least_delta) :
			m_window_length(window_length),
			m_max_iter_num(max_iter_num),
			m_d(d),
			m_least_delta(least_delta) {}

		~SentenceRank() {}
		void ExtractKeySentence(const std::map<value_type, std::vector<value_type> >& sentence_token_map, std::vector<std::pair<value_type, double> >& key_sentences, const size_t topN = 5);

	private:
		double CalcDist(const std::vector<value_type>& token_vec1, const std::vector<value_type>& token_vec2);
		void BuildSentenceRelation(const std::map<value_type, std::vector<value_type> >& sentence_token_map);
		void CalcSentenceScore(std::map<size_t, double>& score_map);
		double GetWeight(size_t i, size_t j) const;
		void Clear();

	private:
		size_t m_window_length;
		size_t m_max_iter_num;
		double m_d;
		double m_least_delta;

		std::vector<value_type> m_sentence_vec;
		std::map<value_type, size_t> m_sentence_index;
		std::map<std::pair<size_t, size_t>, double> m_weight_map;   // edge weight
		std::map<size_t, double> m_out_sum_map;                     // out edges weight sum of one node  
	};

	template <class value_type>
	void SentenceRank<value_type>::ExtractKeySentence(const map<value_type, vector<value_type> >& sentence_token_map, vector<pair<value_type, double> >& key_sentences, const size_t topN)
	{
		key_sentences.clear();

		size_t n = sentence_token_map.size();

		if (n == 0)
			return;

		if (n == 1)
		{
			key_sentences.push_back(make_pair(sentence_token_map.begin()->first, 1.0));
			return;
		}

		Clear();

		BuildSentenceRelation(sentence_token_map);

		map<size_t, double> score_map;
		CalcSentenceScore(score_map);
		/*
		for (auto score : score_map)
		{
			std::wcout << score.second << std::endl;
		}

		size_t id = 0;
		for (auto& sentence : m_sentence_vec)
		{
			std::wcout <<"["<<id<<"] "<< sentence << " : "<< score_map.find(id)->second << std::endl;
			for (auto token :  sentence_token_map.find(sentence)->second)
			{
				std::wcout << "(" << token << ") ";
			}
			std::wcout << std::endl;
			++id;
		}
		*/
		vector<pair<size_t, double> > score_vec(score_map.begin(), score_map.end());
		sort(score_vec.begin(), score_vec.end(), value_great());

		for (size_t i = 0; i < score_vec.size(); ++i)
		{
			if (i == topN)
				break;
			const value_type& sent = m_sentence_vec[score_vec[i].first];
			key_sentences.push_back(make_pair(sent, score_vec[i].second));
		}

	}

	template <class value_type>
	double SentenceRank<value_type>::CalcDist(const vector<value_type>& token_vec1, const vector<value_type>& token_vec2)
	{
		size_t both_num = 0;
		size_t n1 = token_vec1.size();
		size_t n2 = token_vec2.size();
		if (n1 < 2 || n2 < 2)
			return 0;
		for (size_t i = 0; i < n1; ++i)
		{
			const value_type& token = token_vec1[i];
			for (size_t j = 0; j < n2; ++j)
			{
				if (token == token_vec2[j])
				{
					both_num++;
					break;
				}
			}
		}
		double dist = both_num / (log(n1) + log(n2));
		return dist;
	}

	template <class value_type>
	double SentenceRank<value_type>::GetWeight(size_t i, size_t j) const
	{
		if (i > j)
		{
			size_t tmp = i;
			i = j;
			j = tmp;
		}
		pair<size_t, size_t> key(i, j);
		if (m_weight_map.find(key) != m_weight_map.end())
			return m_weight_map.at(key);
		return 0;
	}

	template <class value_type>
	void SentenceRank<value_type>::BuildSentenceRelation(const map<value_type, vector<value_type> >& sentence_token_map)
	{
		auto iter = sentence_token_map.begin();
		for (; iter != sentence_token_map.end(); ++iter)
		{
			m_sentence_vec.push_back(iter->first);
			size_t i = m_sentence_vec.size() - 1;
			m_sentence_index.insert(make_pair(iter->first, i));
			size_t j = i + 1;
			auto iter2 = iter;
			advance(iter2, 1);
			for (; iter2 != sentence_token_map.end(); ++iter2)
			{
				double dist = CalcDist(iter->second, iter2->second);
				pair<size_t, size_t> key(i, j);
				m_weight_map.insert(make_pair(key, dist));
				j++;
			}
		}
		// calc weight sum of out-edges
		size_t n = m_sentence_vec.size();
		for (size_t i = 0; i < n; ++i)
		{
			double sum = 0.0;
			for (size_t j = 0; j < n; ++j)
			{
				if (j != i)
					sum += GetWeight(i, j);
			}
			m_out_sum_map.insert(make_pair(i, sum));
		}

	}

	template <class value_type>
	void  SentenceRank<value_type>::CalcSentenceScore(map<size_t, double>& score_map)
	{
		score_map.clear();

		// initialize
		size_t n = m_sentence_vec.size();
		for (size_t id = 0; id < n; ++id)
			score_map.insert(make_pair(id, 1.0));

		// iterate
		for (size_t i = 0; i < m_max_iter_num; ++i)
		{
			double max_delta = 0;
			map<size_t, double> new_score_map; // current iteration score

			for (size_t id1 = 0; id1 < n; ++id1)
			{
				double new_score = 1 - m_d;
				double sum_weight = 0.0;
				for (size_t id2 = 0; id2 < n; ++id2)
				{
					if (id1 == id2 || m_out_sum_map[id2] < 1e-6)
						continue;
					double weight = GetWeight(id2, id1);
					sum_weight += weight / m_out_sum_map.at(id2) * score_map[id2];
				}
				new_score += m_d * sum_weight;
				new_score_map.insert(make_pair(id1, new_score));

				double delta = fabs(new_score - score_map[id1]);
				max_delta = max(max_delta, delta);
			}
			score_map = new_score_map;
			if (max_delta < m_least_delta)
			{
				break;
			}
		}
	}

	template <class value_type>
	void SentenceRank<value_type>::Clear()
	{
		m_sentence_vec.clear();
		m_sentence_index.clear();
		m_weight_map.clear();
		m_out_sum_map.clear();
	}

	text_ranker::text_ranker() : stop_words(std::make_unique<stop_words_t>())
	{}

	void text_ranker::load_stop_words(const std::vector<std::string>& stop_words_pathes)
	{
		for (auto& path : stop_words_pathes)
		{
			stop_words->load_dictionary(path);
		}
	}

	bool text_ranker::key_words(const std::wstring& texts, std::vector< std::pair< std::wstring, double> >& keywords, int topK)
	{
		/*
		std::vector<std::wstring> tokens;

		static const int maxTextLen = 10000;
		std::wstring input;
		if ((int)texts.size() > maxTextLen) {
			input = texts.substr(0, maxTextLen);
		}
		else {
			input = texts;
		}

		boost::split(tokens, input, boost::is_any_of(word_delimiter));
		static const int max_tokens = 500;
		if (tokens.size() > tokens.size())
			tokens.resize(max_tokens);

		for (auto& token : tokens)
			boost::algorithm::to_lower(token);
		stop_words->remove_stop_words(tokens, 2);


		std::vector<std::pair<std::wstring, std::wstring::iterator> > tokens2;
		TextRank<std::wstring> ranker;
		ranker.ExtractKeyword(tokens2, keywords, topK);
		//ranker.ExtractHighTfWords(tokens, keywords, topK);
		*/
		return true;
	}
	
	class expand_words_t
	{
	public:
		typedef std::wstring container_t;
		typedef container_t::iterator iterator;
		typedef container_t::reverse_iterator reverse_iterator;
		typedef std::pair<iterator, iterator> locator_t;
		typedef std::function<bool(wchar_t)> is_delimiter_t;
		expand_words_t(is_delimiter_t is_delimiter) : is_delimiter(is_delimiter)
		{}

		inline container_t expand(const container_t& corpus, locator_t loc) const
		{
			iterator left = loc.first;
			iterator right = --loc.second;
			//if (is_delimiter(*loc.first) && !is_delimiter(*loc.second))
			//	return container_t(to_right_concat(corpus, ++left), to_right(corpus, right));

			//if (!is_delimiter(*loc.first) && is_delimiter(*loc.second))
			//	return container_t(to_left(corpus, left), to_left_concat(corpus, --right));

			if (!is_delimiter(*left) && !is_delimiter(*right))
				return container_t(to_left(corpus, left), to_right(corpus, right));

			return container_t();
		}
	private:
		iterator make_forward(reverse_iterator rit) const
		{
			return --(rit.base());
		}

		inline iterator to_left(const container_t& corpus, iterator loc) const
		{		
			reverse_iterator left = std::make_reverse_iterator(++loc);
			for (; left != corpus.rend() && !is_delimiter(*left); ++left);
			if (left == corpus.rend())
				--left;
			return make_forward(left);
		}

		inline iterator to_left_concat(const container_t& corpus, iterator loc) const
		{
			reverse_iterator left = std::make_reverse_iterator(++loc);
			for (; left != corpus.rend() && is_delimiter(*left); ++left);
			return make_forward(left);
		}

		inline iterator to_right(const container_t& corpus, iterator loc) const
		{
			iterator right = loc;
			for (; right != corpus.end() && !is_delimiter(*right); ++right);
			return right;
		}

		inline iterator to_right_concat(const container_t& corpus, iterator loc) const
		{
			iterator right = loc;
			for (; right != corpus.end() && is_delimiter(*right); ++right);
			return right;
		}

		is_delimiter_t is_delimiter;
	};

	template<typename A, typename B>
	std::pair<B, A> flip_pair(const std::pair<A, B>& p)
	{
		return std::pair<B, A>(p.second, p.first);
	}

	template<typename A, typename B>
	void flip_map(std::multimap<B, A>& dst, const std::map<A, B>& src)
	{
		std::transform(src.begin(), src.end(), std::inserter(dst, dst.begin()),
			flip_pair<A, B>);
	}

	template <class value_type>
	class ngram_transform_t
	{
	public:
		ngram_transform_t(){}
		~ngram_transform_t(){}

		size_t knuth_morris_pratt_count(value_type& corpus, const value_type& pattern)
		{
			size_t count = 0;
			auto cur = corpus.begin();
			while (cur != corpus.end())
			{
				auto search = boost::algorithm::knuth_morris_pratt_search(cur, corpus.end(), pattern.begin(), pattern.end());
				if (search.first != corpus.end())
				{
					cur = search.second;
					++count;
				}
				else
				{
					cur = search.second;
				}
			}
			return count;
		}

		void ngram_to_keywords(std::map<value_type, double>& keywords, value_type& corpus,
			std::vector< std::pair< std::pair<value_type, typename value_type::iterator>, double> >& patterns)
		{
			if (patterns.empty())
				return;
			auto n = patterns[0].first.first.size();
			if (n < 2)
				return;

			for (size_t i = 0; i < corpus.size(); ++i)
			{
				if (i + n - 1 >= corpus.size())
					return;

				for (auto& pattern : patterns)
				{
					auto cur = pattern.first.second;

					bool equal = true;
					for (size_t j = 0; j < n; j++)
					{
						if (corpus[i + j] != *cur)
							equal = false;
						++cur;
					}

					if (equal)
					{
						expand_words_t expander(boost::is_any_of(word_delimiter));
						auto big_gram_range = expander.expand(corpus, std::make_pair(pattern.first.second, cur));
						std::vector<value_type> words;
						boost::split(words, big_gram_range, boost::is_any_of(word_delimiter));
						for (auto& word : words)
						{
							if (word.size() < 2)
								continue;
							auto keyword = keywords.find(word);
							if (keyword == keywords.end())
								keywords.insert(std::make_pair(word, pattern.second));
							else
								keyword->second += pattern.second;
						}
						
					}
				}
			}		
		}
		static void to_ngram(const value_type& text, size_t n, std::vector<value_type>& res);
		static void to_ngram(value_type& text, size_t n, std::vector<std::pair<value_type, typename value_type::iterator> >& res, size_t limits)
		{
			res.clear();

			if (text.size() < n)
			{
				cout << "text is too short" << endl;
				return;
			}

			typedef typename value_type::value_type char_t;
			basic_stringstream<char_t, char_traits<char_t>, allocator<char_t>> ss;
			size_t char_num = text.size();
			for (size_t i = 0; i < char_num; ++i)
			{
				size_t delimiter_count = 0;
				size_t same_count = 1;
				char_t prev = 0;
				for (size_t j = 0; j < n; ++j)
				{
					if (i + j >= char_num)
						return;
					for (auto demilier : word_delimiter)
					{
						if (demilier == text[i + j])
							++delimiter_count;
					}

					if (prev == text[i + j])
						++same_count;
					ss << text[i + j];
					prev = text[i + j];
				}

				if (res.size() >= limits)
					break;

				if (same_count < n && delimiter_count < (n - 1))
				{
					typename value_type::iterator begin = text.begin();
					std::advance(begin, i);
					res.push_back(std::make_pair(ss.str(), begin));
				}
				ss.str(value_type());
				ss.clear();
			}
		}
		

	private:
		typedef expand_words_t::locator_t locator_t;
	};

	template <class value_type>
	void ngram_transform_t<value_type>::to_ngram(const value_type& text, size_t n, std::vector<value_type>& res)
	{
		res.clear();

		if (text.size() < n)
		{
			cout << "text is too short" << endl;
			return;
		}

		typedef typename value_type::value_type char_t;
		basic_stringstream<char_t, char_traits<char_t>, allocator<char_t>> ss;
		size_t char_num = text.size();
		for (size_t i = 0; i < char_num; ++i)
		{
			size_t delimiter_count = 0;
			size_t same_count = 1;
			char_t prev = 0;
			for (size_t j = 0; j < n; ++j)
			{
				if (i + j >= char_num)
					return;
				for (auto demilier : word_delimiter)
				{
					if (demilier == text[i + j])
						++delimiter_count;
				}

				if (prev == text[i + j])
					++same_count;
				ss << text[i + j];
				prev = text[i + j];
			}

			if (same_count < n && delimiter_count < (n - 1))
				res.push_back(ss.str());
			ss.str(value_type());
			ss.clear();
		}
	}

	bool text_ranker::key_words_ngram(const std::wstring& texts, std::vector< std::pair< std::wstring, double> >& tf_keywords, int topK)
	{
		//std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
		std::vector<std::pair<std::wstring, std::wstring::iterator> > tokens;

		static const int maxTextLen = 10000;
		std::wstring input;
		if ((int)texts.size() > maxTextLen) {
			input = texts.substr(0, maxTextLen);
		}
		else {
			input = texts;
		}

		ngram_transform_t<std::wstring>::to_ngram(input, 3, tokens, 3000);

		std::vector< std::pair< std::pair<std::wstring, std::wstring::iterator>,  double> > keywords;
		TextRank<std::wstring> ranker;
		//ranker.ExtractKeyword(tokens, keywords, 1000);
		ranker.ExtractKeyword(tokens, keywords, tokens.size());

		boost::accumulators::accumulator_set<double, boost::accumulators::features<boost::accumulators::tag::mean> > acc;
		for (auto& keyword : keywords)
			acc(keyword.second);

		std::vector< std::pair< std::pair<std::wstring, std::wstring::iterator>, double> > means;
		double mean = boost::accumulators::mean(acc) - std::numeric_limits<double>::epsilon();
		for (size_t i = 0; i < keywords.size(); i++)
		{
			if (keywords[i].second > mean)
				means.push_back(keywords[i]);
		}
		std::swap(keywords, means);

		ngram_transform_t<std::wstring> ngram_transform;
		std::map<std::wstring, double> ngram_keywords;

		ngram_transform.ngram_to_keywords(ngram_keywords, input, keywords);

		stop_words->remove_stop_words(ngram_keywords, 2);

		std::multimap<double, std::wstring> sort_ngram_keywords;
		flip_map(sort_ngram_keywords, ngram_keywords);
		
		for (auto i = sort_ngram_keywords.rbegin(); i != sort_ngram_keywords.rend(); ++i)
		{
			if (topK <= tf_keywords.size())
				break;
			tf_keywords.push_back(std::make_pair(i->second, i->first));
		}
		
		//std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
		//auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		//std::cout << "tokens is " << keywords.size() << ", time : " << ms << " ms" << std::endl;

		return true;
	}


	bool text_ranker::key_sentences(const std::wstring& texts, std::vector< std::pair< std::wstring, double> >& key_sentences, int topK)
	{
		std::vector<std::wstring> raws;

		static const int maxTextLen = 100000;
		std::wstring input;
		if ((int)texts.size() > maxTextLen) {
			input = texts.substr(0, maxTextLen);
		}
		else {
			input = texts;
		}

		boost::split(raws, input, boost::is_any_of(para_delimiter));
		
		static const int maxSentencesNum = 500;
		std::vector<std::wstring> sentences;
		for (int i = 0; i < (int)raws.size(); ++i)
		{
			if (sentences.size() >= maxSentencesNum)
				break;
			if (!raws[i].empty() && (int)raws[i].size() > 3)
			{
				sentences.push_back(raws[i]);
			}
		}

		int complexity = 0;
		static const int ngram_limit = 250000;
		size_t count_of_tokens = 0;
		vector<wstring> bigram_vec;
		map<wstring, vector<wstring> > sentence_token_map;
		for (size_t i = 0; i < sentences.size(); ++i)
		{
			ngram_transform_t<std::wstring>::to_ngram(sentences[i], 3, bigram_vec);
			count_of_tokens += bigram_vec.size();
			complexity = sentence_token_map.size()* count_of_tokens;
			if (complexity > ngram_limit)
				break;
			if (bigram_vec.empty())
				continue;

			for (auto& token : bigram_vec)
				boost::algorithm::to_lower(token);
			
			stop_words->remove_stop_words(bigram_vec, 2);
			sentence_token_map.insert(make_pair(sentences[i], bigram_vec));
		}

		//std::cout << "sentences is " << sentence_token_map.size() << " , token is " << count_of_tokens << 
		//	" = " << sentence_token_map.size() * count_of_tokens <<std::endl;

		SentenceRank<std::wstring> ranker;
		ranker.ExtractKeySentence(sentence_token_map, key_sentences, topK);
		return true;
	}
}

