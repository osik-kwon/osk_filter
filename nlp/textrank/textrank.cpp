#include "nlp_pch.h"
#include "textrank/textrank.h"

#include <iostream>
#include "locale/charset_encoder.h"

#include <boost/algorithm/string.hpp> 
#include <boost/algorithm/string/case_conv.hpp>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>

#include <boost/algorithm/searching/boyer_moore.hpp>
#include <boost/algorithm/searching/boyer_moore_horspool.hpp>
#include <boost/algorithm/searching/knuth_morris_pratt.hpp>

#include <regex>
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
	static const std::wstring para_delimiter = L"?!.;？！。；………\n";
	static const std::wstring word_delimiter = L"?!.;？！。；………\n\t 、,\"“”()[]‘’'";

	template <class value_type>
	class text_rank_impl_t
	{
	public:
		typedef typename value_type::iterator iterator;
		text_rank_impl_t() :
			m_window_length(3),
			m_max_iter_num(100),
			m_d(0.85),
			m_least_delta(1e-6)
		{}

		text_rank_impl_t(int window_length, int max_iter_num, double d, double least_delta) :
			m_window_length(window_length),
			m_max_iter_num(max_iter_num),
			m_d(d),
			m_least_delta(least_delta)
		{}

		~text_rank_impl_t() {}
		void extract_keywords(const std::vector< std::pair<value_type, iterator> >& token_vec,
			std::vector<std::pair<std::pair<value_type, iterator>, double> >& keywords, const size_t topN = 10);

	private:
		int build_relations(const std::vector< std::pair<value_type, iterator> >& token_vec, std::map<size_t, std::set<size_t> >& word_neighbors);
		void calculate_score(const std::map<size_t, std::set<size_t> >& word_neighbors, std::map<size_t, double>& score_map);
		void update_weight_map(size_t i, size_t j);
		double get_weight(size_t i, size_t j) const;
		void clear();

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
	void text_rank_impl_t<value_type>::extract_keywords(const vector< std::pair<value_type, iterator> >& token_vec, 
		vector<pair<std::pair<value_type, iterator>, double> >& keywords, const size_t topN)
	{
		keywords.clear();
		if (token_vec.empty())
			return;

		clear();

		map<size_t, set<size_t> > word_neighbors;
		map<size_t, double> score_map;

		build_relations(token_vec, word_neighbors);
		calculate_score(word_neighbors, score_map);

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
	void text_rank_impl_t<value_type>::update_weight_map(size_t i, size_t j)
	{
		if (i > j)
		{
			std::swap(i, j);
		}
		pair<size_t, size_t> key(i, j);
		if (m_weight_map.find(key) != m_weight_map.end())
			m_weight_map[key] += 1;
		else
			m_weight_map.insert(make_pair(key, 1.0));
	}

	template <class value_type>
	double text_rank_impl_t<value_type>::get_weight(size_t i, size_t j) const
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
	int text_rank_impl_t<value_type>::build_relations(const vector< std::pair<value_type, iterator> >& token_vec, map<size_t, set<size_t> >& word_neighbors)
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

				update_weight_map(id1, id2);

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
				sum += get_weight(id1, id2);
			}
			m_out_sum_map.insert(make_pair(id1, sum));
		}
		return 0;
	}

	template <class value_type>
	void text_rank_impl_t<value_type>::calculate_score(const map<size_t, set<size_t> >& word_neighbors, map<size_t, double>& score_map)
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
					double weight = get_weight(id2, id1);
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
	void text_rank_impl_t<value_type>::clear()
	{
		m_word_vec.clear();
		m_word_index.clear();
		m_weight_map.clear();
		m_out_sum_map.clear();
	}

	template <class value_type>
	class sentence_rank_impl_t
	{
	public:
		sentence_rank_impl_t() :
			m_window_length(3),
			m_max_iter_num(100),
			m_d(0.85),
			m_least_delta(1e-6)
		{}
		sentence_rank_impl_t(int window_length, int max_iter_num, double d, double least_delta) :
			m_window_length(window_length),
			m_max_iter_num(max_iter_num),
			m_d(d),
			m_least_delta(least_delta) {}

		~sentence_rank_impl_t() {}
		void extract_key_sentences(const std::map<value_type, std::vector<value_type> >& sentence_token_map, std::vector<std::pair<value_type, double> >& key_sentences, const size_t topN = 5);

	private:
		double get_similarity(const std::vector<value_type>& token_vec1, const std::vector<value_type>& token_vec2);
		void build_relations(const std::map<value_type, std::vector<value_type> >& sentence_token_map);
		void calculate_score(std::map<size_t, double>& score_map);
		double get_weight(size_t i, size_t j) const;
		void clear();

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
	void sentence_rank_impl_t<value_type>::extract_key_sentences(const map<value_type, vector<value_type> >& sentence_token_map, vector<pair<value_type, double> >& key_sentences, const size_t topN)
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

		clear();

		build_relations(sentence_token_map);

		map<size_t, double> score_map;
		calculate_score(score_map);
		vector<pair<size_t, double> > score_vec(score_map.begin(), score_map.end());
		sort(score_vec.begin(), score_vec.end(), [](const std::pair<size_t, double>& x, const std::pair<size_t, double>& y) {
			return x.second > y.second;
			});

		for (size_t i = 0; i < score_vec.size(); ++i)
		{
			if (i == topN)
				break;
			const value_type& sent = m_sentence_vec[score_vec[i].first];
			key_sentences.push_back(make_pair(sent, score_vec[i].second));
		}

	}

	template <class value_type>
	double sentence_rank_impl_t<value_type>::get_similarity(const vector<value_type>& token_vec1, const vector<value_type>& token_vec2)
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
		double similarity = both_num / (log(n1) + log(n2));
		return similarity;
	}

	template <class value_type>
	double sentence_rank_impl_t<value_type>::get_weight(size_t i, size_t j) const
	{
		if (i > j)
		{
			std::swap(i, j);
		}
		pair<size_t, size_t> key(i, j);
		if (m_weight_map.find(key) != m_weight_map.end())
			return m_weight_map.at(key);
		return 0;
	}

	template <class value_type>
	void sentence_rank_impl_t<value_type>::build_relations(const map<value_type, vector<value_type> >& sentence_token_map)
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
				double similarity = get_similarity(iter->second, iter2->second);
				pair<size_t, size_t> key(i, j);
				m_weight_map.insert(make_pair(key, similarity));
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
					sum += get_weight(i, j);
			}
			m_out_sum_map.insert(make_pair(i, sum));
		}

	}

	template <class value_type>
	void  sentence_rank_impl_t<value_type>::calculate_score(map<size_t, double>& score_map)
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
					double weight = get_weight(id2, id1);
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
	void sentence_rank_impl_t<value_type>::clear()
	{
		m_sentence_vec.clear();
		m_sentence_index.clear();
		m_weight_map.clear();
		m_out_sum_map.clear();
	}

	text_ranker::text_ranker() : stop_words(std::make_unique<stop_words_t>()), tagger(nullptr)
	{}

	void text_ranker::load_stop_words(const std::vector<std::string>& stop_words_pathes)
	{
		for (auto& path : stop_words_pathes)
		{
			stop_words->load_dictionary(path);
		}
	}

	void text_ranker::load_morphological_analyzer(const std::string& rc_path, const std::string& dic_path)
	{
		std::string command = "mecab";
		command += " -r ";
		command += rc_path;
		command += " -d ";
		command += dic_path;
		tagger.reset(::MeCab::createTagger(command.c_str()));
		if (!tagger)
		{
			const char* e = tagger ? tagger->what() : MeCab::getTaggerError();
			std::cout << e << std::endl;
		}
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

		inline container_t expand_paragraph(const container_t& corpus, locator_t loc) const
		{
			iterator left = loc.first;
			iterator right = --loc.second;
			return container_t(to_left(corpus, left), to_right(corpus, right));
		}

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
		void ngram_to_keywords(std::map<value_type, double>& keywords, value_type& corpus,
			std::vector< std::pair< std::pair<value_type, typename value_type::iterator>, double> >& patterns,
			std::unique_ptr<::MeCab::Tagger>& tagger);
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
	void ngram_transform_t<value_type>::ngram_to_keywords(std::map<value_type, double>& keywords, value_type& corpus,
		std::vector< std::pair< std::pair<value_type, typename value_type::iterator>, double> >& patterns, std::unique_ptr<::MeCab::Tagger>& tagger)
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

	bool text_ranker::key_words(const std::wstring& texts, std::vector< std::pair< std::wstring, double> >& tf_keywords, size_t topK)
	{
		const size_t text_limit = 10000;
		std::wstring input;
		if (texts.size() > text_limit)
			input = texts.substr(0, text_limit);
		else
			input = texts;

		const size_t token_limit = 3000;
		std::vector<std::pair<std::wstring, std::wstring::iterator> > tokens;
		ngram_transform_t<std::wstring>::to_ngram(input, 3, tokens, token_limit);

		std::vector< std::pair< std::pair<std::wstring, std::wstring::iterator>,  double> > keywords;
		text_rank_impl_t<std::wstring> ranker;
		ranker.extract_keywords(tokens, keywords, tokens.size());

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

		ngram_transform.ngram_to_keywords(ngram_keywords, input, keywords, tagger);
		stop_words->remove_stop_words(ngram_keywords, 2);

		if (tagger)
		{
			std::string src = to_utf8(input);
			const MeCab::Node* root = tagger->parseToNode(src.c_str());
			bool has_global_noun = false;
			std::map < std::wstring, double> norms;
			for (auto& ukeyword : ngram_keywords)
			{
				std::string keyword = to_utf8(ukeyword.first);

				auto offset_begin = std::distance(src.begin(), src.end());
				auto offset_end = std::distance(src.begin(), src.end());
				auto search = boost::algorithm::knuth_morris_pratt_search(src.begin(), src.end(), keyword.begin(), keyword.end());
				if (search.first != src.end())
				{
					offset_begin = std::distance(src.begin(), search.first);
					offset_end = std::distance(src.begin(), search.second);
				}
				else
				{
					throw std::runtime_error("kmp search failed");
				}

				std::wstring norm_keyword;
				const MeCab::Node* node = root;
				bool has_noun = false;
				for (; node; node = node->next)
				{
					auto begin = (int)(node->surface - src.c_str());
					if (begin >= offset_end)
						break;
					auto feature = to_wchar(node->feature);
					std::vector<std::wstring> tokens;
					boost::split(tokens, feature, boost::is_any_of(L","));
					if (
						begin >= offset_begin && begin < offset_end &&
						!tokens.empty() && !tokens[0].empty()
						)
					{
						//bool is_noun = tokens[0][0] == L'N';
						bool is_etc = tokens[0] == L"SL" || tokens[0] == L"SH";
						bool is_noun = tokens[0] == L"NNG"
							|| tokens[0] == L"NNP"
							|| tokens[0] == L"NNG"
							|| tokens[0] == L"NNB"
							|| tokens[0] == L"NNBC"
							|| tokens[0] == L"NR"
							|| tokens[0] == L"NP"
							|| tokens[0] == L"XPN"; // 체언 접두사
						bool is_numerals = tokens[0] == L"SN";
						bool is_noun_suffix = tokens[0] == L"XSN";
						//bool is_noun_prefix = tokens[0] == L"ETN";
						if (is_noun)
							has_noun = true;
						if(is_noun || is_numerals || is_noun_suffix || is_etc)
							norm_keyword += to_wchar(std::string(node->surface, node->length));
					}
				}

				if (norm_keyword.size() > 1 && has_noun)
				{
					has_global_noun = true;
					auto cur = norms.find(norm_keyword);
					if (cur == norms.end())
						norms.insert(std::make_pair(norm_keyword, ukeyword.second));
					else
						cur->second += ukeyword.second;
				}
			}
			if(!norms.empty() && has_global_noun)
				std::swap(norms, ngram_keywords);
		}
		
		std::multimap<double, std::wstring> sort_ngram_keywords;
		flip_map(sort_ngram_keywords, ngram_keywords);
		
		for (auto i = sort_ngram_keywords.rbegin(); i != sort_ngram_keywords.rend(); ++i)
		{
			if (topK <= tf_keywords.size())
				break;
			tf_keywords.push_back(std::make_pair(i->second, i->first));
		}

		return true;
	}


	bool text_ranker::key_sentences(const std::wstring& texts, std::vector< std::pair< std::wstring, double> >& key_sentences, size_t topK)
	{
		const size_t text_limit = 100000;
		std::wstring input;
		if (texts.size() > text_limit)
			input = texts.substr(0, text_limit);
		else
			input = texts;

		std::vector<std::wstring> raw_sentences;
		boost::split(raw_sentences, input, boost::is_any_of(para_delimiter));
		
		const size_t sentences_limit = 500;
		std::vector<std::wstring> sentences;
		for (size_t i = 0; i < raw_sentences.size(); ++i)
		{
			if (sentences.size() >= sentences_limit)
				break;
			if (!raw_sentences[i].empty() && raw_sentences[i].size() > 3)
			{
				sentences.push_back(raw_sentences[i]);
			}
		}
		
		const size_t ngram_limit = 250000;
		size_t complexity = 0;
		size_t count_of_tokens = 0;
		vector<wstring> bigram_vec;
		map<wstring, vector<wstring> > sentence_token_map;
		for (size_t i = 0; i < sentences.size(); ++i)
		{
			ngram_transform_t<std::wstring>::to_ngram(sentences[i], 3, bigram_vec);
			count_of_tokens += bigram_vec.size();
			complexity = sentence_token_map.size() * count_of_tokens;
			if (complexity > ngram_limit)
				break;
			if (bigram_vec.empty())
				continue;

			for (auto& token : bigram_vec)
				boost::algorithm::to_lower(token);
			
			stop_words->remove_stop_words(bigram_vec, 2);
			sentence_token_map.insert(make_pair(sentences[i], bigram_vec));
		}

		sentence_rank_impl_t<std::wstring> ranker;
		ranker.extract_key_sentences(sentence_token_map, key_sentences, topK);
		return true;
	}
}

