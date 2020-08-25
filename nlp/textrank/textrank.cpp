#include "nlp_pch.h"
#include "textrank/textrank.h"

#include <iostream>
#include "locale/charset_encoder.h"

#include <boost/algorithm/string.hpp> 
#include <boost/algorithm/string/case_conv.hpp>

#include <string>
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <cmath>
#include <set>
#include <map>
#include <sstream>

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

	template <class value_type>
	class text_ranker_impl
	{
	public:
		explicit text_ranker_impl(stop_words_t* stop_words)
			: m_stop_words(stop_words), m_damping_factor(0.85), m_max_iterations(100), m_tolerance(1.0e-5)
		{}
		explicit text_ranker_impl(stop_words_t* stop_words, double damping_factor, int max_iterations, double tolerance)
			: m_stop_words(stop_words), m_damping_factor(damping_factor), m_max_iterations(max_iterations), m_tolerance(tolerance)
		{}

		~text_ranker_impl() { }

		bool make_key_sentences(const value_type& src, std::vector<value_type>& dest, int topK);

	private:
		bool make_sentences(const value_type& input, std::vector<value_type>& output);
		bool remove_duplicates(const std::vector<value_type>& src, std::vector<value_type>& dest);
		bool build_graph(const std::vector<value_type>& sentences);
		double get_similarity(int a, int b);
		bool rank_sentences();
		bool make_wordsets(const std::vector<value_type>& sentences);

	private:
		double m_damping_factor;
		int m_max_iterations;
		double m_tolerance;
		std::vector<value_type> m_sentences;
		std::vector< std::vector<double> > m_similarity_matrix;
		std::vector<double> m_out_weight_sum;
		std::vector<double> m_scores;
		std::vector< std::set<value_type> > m_wordsets;
		std::vector<int> m_wordsizes;

		stop_words_t* m_stop_words;
	};

	template <class value_type>
	bool text_ranker_impl<value_type>::make_key_sentences(const value_type& input, std::vector<value_type>& outputs, int topK)
	{
		outputs.clear();
		if (input.empty() || topK < 1) {
			return false;
		}

		bool ret = true;
		ret &= make_sentences(input, m_sentences);
		ret &= make_wordsets(m_sentences);
		ret &= build_graph(m_sentences);
		ret &= rank_sentences();

		if (!ret) {
			return false;
		}

		int kDim = m_sentences.size();
		std::vector< std::pair<int, double> > visitPairs;  // (id, score)
		for (int i = 0; i < kDim; ++i) {
			visitPairs.push_back(std::pair<int, double>(i, m_scores[i]));
		}

		std::sort(visitPairs.begin(), visitPairs.end(), [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
			return a.second > b.second;
			});

		for (int i = 0; i < topK && i < kDim; ++i) {
			int id = visitPairs[i].first;
			outputs.push_back(m_sentences[id]);
		}
		return true;
	}

	template <class value_type>
	bool text_ranker_impl<value_type>::make_sentences(const value_type& input, std::vector<value_type>& outputs)
	{
		outputs.clear();
		if (input.empty()) {
			outputs.push_back(L"\n");
			return false;
		}

		static const int maxTextLen = 100000;
		value_type tempInput;
		if ((int)input.size() > maxTextLen) {
			tempInput = input.substr(0, maxTextLen);
		}
		else {
			tempInput = input;
		}

		boost::algorithm::to_lower(tempInput);

		static const int minSentenceLen = 30;
		vector<value_type> tempOutput;

		boost::split(tempOutput, tempInput, boost::is_any_of(L"?!.;£¿£¡¡££»¡¦¡¦¡¦\n"));
		vector<value_type> tempOutput2;
		for (int i = 0; i < (int)tempOutput.size(); ++i)
		{
			if (!tempOutput[i].empty() && (int)tempOutput[i].size() > minSentenceLen)
			{
				tempOutput2.push_back(tempOutput[i]);
			}
		}

		remove_duplicates(tempOutput2, outputs);

		static const int maxSentencesNum = 500;
		if ((int)outputs.size() > maxSentencesNum) {
			outputs.resize(maxSentencesNum);
		}

		return true;
	}

	template <class value_type>
	bool text_ranker_impl<value_type>::remove_duplicates(const std::vector<value_type>& src, std::vector<value_type>& dest)
	{
		dest.clear();
		std::unordered_set<value_type> set(src.begin(), src.end());
		dest = std::vector<value_type>(set.begin(), set.end());
		return true;
	}

	template <class value_type>
	bool text_ranker_impl<value_type>::build_graph(const std::vector<value_type>& sentences)
	{
		if (sentences.empty()) { return false; }

		int kDim = sentences.size();

		m_similarity_matrix.clear();
		m_similarity_matrix.resize(kDim, std::vector<double>(kDim, 0.0));

		for (int i = 0; i < kDim - 1; i++)
		{
			for (int j = i + 1; j < kDim; j++)
			{
				double similarity = get_similarity(i, j);
				// the similarity matrix is symmetrical, so transposes are filled in with the same similarity
				m_similarity_matrix[i][j] = similarity;
				m_similarity_matrix[j][i] = similarity;
			}
		}

		m_out_weight_sum.clear();
		m_out_weight_sum.resize(kDim, 0.0);

		for (int i = 0; i < kDim; ++i) {
			for (int j = 0; j < kDim; ++j) {
				if (i == j) { continue; }
				m_out_weight_sum[i] += m_similarity_matrix[i][j];
			}
		}

		return true;
	}

	template <class value_type>
	bool text_ranker_impl<value_type>::make_wordsets(const std::vector<value_type>& sentences)
	{
		int kDim = sentences.size();
		if (sentences.empty()) {
			return false;
		}

		m_wordsets.clear();
		m_wordsets.resize(kDim, std::set<value_type>());
		m_wordsizes.clear();
		m_wordsizes.resize(kDim, 0);

		for (int i = 0; i < kDim; ++i)
		{
			std::vector<value_type> tokens;
			boost::split(tokens, sentences[i], boost::is_any_of(L"\t ¡¢,"));
			if(m_stop_words)
				m_stop_words->remove_stop_words(tokens, 2);
			m_wordsizes[i] = tokens.size();
			std::set<value_type> wordset(tokens.begin(), tokens.end());
			std::swap(m_wordsets[i], wordset);
		}

		return true;
	}

	template <class value_type>
	double text_ranker_impl<value_type>::get_similarity(int a, int b)
	{
		if ((int)m_wordsets.size() <= a || (int)m_wordsets.size() <= b || m_wordsets.size() != m_wordsizes.size())
		{
			return 0.0;
		}
		std::vector<value_type> commonWords;
		const string_similarity<value_type>::similarity_t same = 0.75;
		for (const auto& first : m_wordsets[a])
		{
			for (const auto& second : m_wordsets[b])
			{
				auto similarity = string_similarity<value_type>::similarity(first, second);
				if (similarity > same)
				{
					if (second.size() > 1)
						commonWords.push_back(second);
				}
			}
		}

		double denominator = std::log(double(m_wordsizes[a])) + std::log(double(m_wordsizes[b]));
		if (std::fabs(denominator) < 1e-6) {
			return 0.0;
		}
		return 1.0 * commonWords.size() / denominator;
	}

	template <class value_type>
	bool text_ranker_impl<value_type>::rank_sentences()
	{
		if (m_similarity_matrix.empty() || m_similarity_matrix[0].empty() || m_out_weight_sum.empty()) {
			return false;
		}

		int kDim = m_sentences.size();

		m_scores.clear();
		m_scores.resize(kDim, 1.0);

		// iterate
		int iterNum = 0;
		for (; iterNum < m_max_iterations; ++iterNum)
		{
			double maxDelta = 0.0;
			vector<double> newScores(kDim, 0.0); // current iteration score

			for (int i = 0; i < kDim; ++i)
			{
				double sum_weight = 0.0;
				for (int j = 0; j < kDim; ++j)
				{
					if (i == j || m_out_weight_sum[j] < 1e-6)
						continue;
					double weight = m_similarity_matrix[j][i];
					sum_weight += weight / m_out_weight_sum[j] * m_scores[j];
				}
				double newScore = 1.0 - m_damping_factor + m_damping_factor * sum_weight;
				newScores[i] = newScore;

				double delta = fabs(newScore - m_scores[i]);
				maxDelta = max(maxDelta, delta);
			}

			m_scores = newScores;
			if (maxDelta < m_tolerance) {
				break;
			}
		}
		return true;
	}

	struct value_great
	{
		template <typename T>
		bool operator()(const std::pair<T, double>& x, const std::pair<T, double>& y) const
		{
			return x.second > y.second;
		}
	};

	template <class value_type>
	class TextRank
	{
	public:
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
		void ExtractKeyword(const std::vector<value_type>& token_vec, std::vector<std::pair<value_type, double> >& keywords, const size_t topN = 10);

	private:
		int BuildWordRelation(const std::vector<value_type>& token_vec, std::map<size_t, std::set<size_t> >& word_neighbors);
		void CalcWordScore(const std::map<size_t, std::set<size_t> >& word_neighbors, std::map<size_t, double>& score_map);
		void UpdateWeightMap(size_t i, size_t j);
		double GetWeight(size_t i, size_t j) const;
		void Clear();

	private:
		size_t m_window_length;
		size_t m_max_iter_num;
		double m_d;
		double m_least_delta;

		std::vector<value_type> m_word_vec;
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
		sort(word_tf_vec.begin(), word_tf_vec.end(), value_great());
		for (size_t i = 0; i < word_tf_vec.size(); ++i)
		{
			if (i == topN)
				break;
			keywords.push_back(word_tf_vec[i]);
		}
	}

	template <class value_type>
	void TextRank<value_type>::ExtractKeyword(const vector<value_type>& token_vec, vector<pair<value_type, double> >& keywords, const size_t topN)
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
		sort(score_vec.begin(), score_vec.end(), value_great());

		for (size_t i = 0; i < score_vec.size(); ++i)
		{
			if (i == topN)
				break;
			const value_type& word = m_word_vec[score_vec[i].first];
			keywords.push_back(make_pair(word, score_vec[i].second));
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
	int TextRank<value_type>::BuildWordRelation(const vector<value_type>& token_vec, map<size_t, set<size_t> >& word_neighbors)
	{
		m_weight_map.clear();
		word_neighbors.clear();

		const size_t n = token_vec.size();

		// use word index to replace string word 
		for (size_t i = 0; i < n; ++i)
		{
			const value_type& word = token_vec[i];
			if (m_word_index.find(word) == m_word_index.end())
			{
				m_word_vec.push_back(word);
				m_word_index.insert(make_pair(word, m_word_vec.size() - 1));
			}
		}

		for (size_t i = 0; i < n; ++i)
		{
			const value_type& word = token_vec[i];
			size_t id1 = m_word_index.at(word);

			if (word_neighbors.find(id1) == word_neighbors.end())
				word_neighbors.insert(make_pair(id1, set<size_t>()));

			for (size_t j = 1; j < m_window_length; ++j)
			{
				if (i + j >= n)
					break;
				size_t id2 = m_word_index.at(token_vec[i + j]);
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

	void ExtractNgram(const wstring& text, size_t n, vector<wstring>& res)
	{
		res.clear();

		if (text.size() < n)
		{
			cout << "text is too short" << endl;
			return;
		}

		wstringstream ss;
		size_t char_num = text.size();
		for (size_t i = 0; i < char_num; ++i)
		{
			for (size_t j = 0; j < n; ++j)
			{
				if (i + j >= char_num)
					return;
				ss << text[i + j];
			}
			res.push_back(ss.str());
			ss.str(L"");
			ss.clear();
		}
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
		std::vector<std::wstring> tokens;

		static const int maxTextLen = 100000;
		std::wstring input;
		if ((int)texts.size() > maxTextLen) {
			input = texts.substr(0, maxTextLen);
		}
		else {
			input = texts;
		}

		boost::split(tokens, input, boost::is_any_of(L"?!.;£¿£¡¡££»¡¦¡¦¡¦\n\t ¡¢,"));
		for (auto& token : tokens)
			boost::algorithm::to_lower(token);
		stop_words->remove_stop_words(tokens, 2);		
		TextRank<std::wstring> ranker;
		ranker.ExtractKeyword(tokens, keywords, topK);
		//ranker.ExtractHighTfWords(tokens, keywords, topK);
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

		boost::split(raws, input, boost::is_any_of(L"?!.;£¿£¡¡££»¡¦¡¦¡¦\n"));

		static const int maxSentencesNum = 500;
		std::vector<std::wstring> sentences;
		for (int i = 0; i < (int)raws.size(); ++i)
		{
			if ((int)i > maxSentencesNum)
				break;
			if (!raws[i].empty() && (int)raws[i].size() > 30)
			{
				sentences.push_back(raws[i]);
			}
		}

		vector<wstring> bigram_vec;
		map<wstring, vector<wstring> > sentence_token_map;
		for (size_t i = 0; i < sentences.size(); ++i)
		{
			ExtractNgram(sentences[i], 3, bigram_vec);
			if (bigram_vec.empty())
				continue;

			for (auto& token : bigram_vec)
				boost::algorithm::to_lower(token);
			
			stop_words->remove_stop_words(bigram_vec, 2);
			sentence_token_map.insert(make_pair(sentences[i], bigram_vec));
		}

		SentenceRank<std::wstring> ranker;
		ranker.ExtractKeySentence(sentence_token_map, key_sentences, topK);
		return true;
	}

	bool text_ranker::key_sentences(const std::wstring& texts, std::vector<std::wstring>& sentences, int topK)
	{
		text_ranker_impl<std::wstring> ranker(stop_words.get());
		return ranker.make_key_sentences(texts, sentences, topK);
	}
}

