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
		explicit text_ranker_impl()
			: m_d(0.85), mMaxIter(100), mTol(1.0e-5) { }
		explicit text_ranker_impl(double d, int maxIter, double tol)
			: m_d(d), mMaxIter(maxIter), mTol(tol) { }

		~text_ranker_impl() { }

		bool ExtractKeySentences(const value_type& input, std::vector<value_type>& outputs, int topK);

	private:
		bool ExtractSentences(const value_type& input, std::vector<value_type>& output);
		bool RemoveDuplicates(const std::vector<value_type>& input, std::vector<value_type>& output);
		bool BuildGraph(const std::vector<value_type>& sentences);
		double GetSimilarity(int a, int b);
		bool CalcSentenceScores();
		bool InitWordSet(const std::vector<value_type>& sentences);

	private:
		double m_d;
		int mMaxIter;
		double mTol;
		std::vector<value_type> mSentences;
		std::vector< std::vector<double> > mAdjacencyMatrix;
		std::vector<double> mOutWeightSum;
		std::vector<double> mScores;
		std::vector< std::set<value_type> > mWordSets;
		std::vector<int> mWordSizes;
	};

	template <class value_type>
	bool text_ranker_impl<value_type>::ExtractKeySentences(const value_type& input, std::vector<value_type>& outputs, int topK)
	{
		outputs.clear();
		if (input.empty() || topK < 1) {
			return false;
		}

		bool ret = true;
		ret &= ExtractSentences(input, mSentences);
		ret &= BuildGraph(mSentences);
		ret &= CalcSentenceScores();

		if (!ret) {
			return false;
		}

		int kDim = mSentences.size();
		std::vector< std::pair<int, double> > visitPairs;  // (id, score)
		for (int i = 0; i < kDim; ++i) {
			visitPairs.push_back(std::pair<int, double>(i, mScores[i]));
		}

		std::sort(visitPairs.begin(), visitPairs.end(), [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
			return a.second > b.second;
			});

		for (int i = 0; i < topK && i < kDim; ++i) {
			int id = visitPairs[i].first;
			outputs.push_back(mSentences[id]);
		}
		return true;
	}

	template <class value_type>
	bool text_ranker_impl<value_type>::ExtractSentences(const value_type& input, std::vector<value_type>& outputs)
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
			if ((int)tempOutput[i].size() > minSentenceLen)
			{
				tempOutput2.push_back(tempOutput[i]);
			}
		}

		RemoveDuplicates(tempOutput2, outputs);

		static const int maxSentencesNum = 500;
		if ((int)outputs.size() > maxSentencesNum) {
			outputs.resize(maxSentencesNum);
		}

		return true;
	}

	template <class value_type>
	bool text_ranker_impl<value_type>::RemoveDuplicates(const std::vector<value_type>& inputs, std::vector<value_type>& outputs)
	{
		outputs.clear();

		std::unordered_set<value_type> s(inputs.begin(), inputs.end());
		outputs = std::vector<value_type>(s.begin(), s.end());

		return true;
	}

	template <class value_type>
	bool text_ranker_impl<value_type>::BuildGraph(const std::vector<value_type>& sentences)
	{
		if (sentences.empty()) { return false; }

		int kDim = sentences.size();

		mAdjacencyMatrix.clear();
		mAdjacencyMatrix.resize(kDim, std::vector<double>(kDim, 0.0));

		InitWordSet(sentences);

		for (int i = 0; i < kDim - 1; i++)
		{
			for (int j = i + 1; j < kDim; j++)
			{
				double similarity = GetSimilarity(i, j);
				// the similarity matrix is symmetrical, so transposes are filled in with the same similarity
				mAdjacencyMatrix[i][j] = similarity;
				mAdjacencyMatrix[j][i] = similarity;
			}
		}

		mOutWeightSum.clear();
		mOutWeightSum.resize(kDim, 0.0);

		for (int i = 0; i < kDim; ++i) {
			for (int j = 0; j < kDim; ++j) {
				if (i == j) { continue; }
				mOutWeightSum[i] += mAdjacencyMatrix[i][j];
			}
		}

		return true;
	}

	template <class value_type>
	bool text_ranker_impl<value_type>::InitWordSet(const std::vector<value_type>& sentences)
	{
		int kDim = sentences.size();
		if (sentences.empty()) {
			return false;
		}

		mWordSets.clear();
		mWordSets.resize(kDim, std::set<value_type>());
		mWordSizes.clear();
		mWordSizes.resize(kDim, 0);

		for (int i = 0; i < kDim; ++i) {
			std::vector<value_type> aWords;

			boost::split(aWords, sentences[i], boost::is_any_of(L"\t "));

			mWordSizes[i] = aWords.size();
			std::set<value_type> aWordSet(aWords.begin(), aWords.end());
			mWordSets[i] = aWordSet;
		}

		return true;
	}

	template <class value_type>
	double text_ranker_impl<value_type>::GetSimilarity(int a, int b)
	{
		if ((int)mWordSets.size() <= a || (int)mWordSets.size() <= b || mWordSets.size() != mWordSizes.size())
		{
			return 0.0;
		}
		std::vector<value_type> commonWords;
		const string_similarity<value_type>::similarity_t same = 0.85;
		for (const auto& first : mWordSets[a])
		{
			for (const auto& second : mWordSets[b])
			{
				auto similarity = string_similarity<value_type>::similarity(first, second);
				if (similarity > same)
				{
					if (second.size() > 1)
						commonWords.push_back(second);
				}
			}
		}

		double denominator = std::log(double(mWordSizes[a])) + std::log(double(mWordSizes[b]));
		if (std::fabs(denominator) < 1e-6) {
			return 0.0;
		}
		return 1.0 * commonWords.size() / denominator;
	}

	template <class value_type>
	bool text_ranker_impl<value_type>::CalcSentenceScores()
	{
		if (mAdjacencyMatrix.empty() || mAdjacencyMatrix[0].empty() || mOutWeightSum.empty()) {
			return false;
		}

		int kDim = mSentences.size();

		mScores.clear();
		mScores.resize(kDim, 1.0);

		// iterate
		int iterNum = 0;
		for (; iterNum < mMaxIter; ++iterNum)
		{
			double maxDelta = 0.0;
			vector<double> newScores(kDim, 0.0); // current iteration score

			for (int i = 0; i < kDim; ++i)
			{
				double sum_weight = 0.0;
				for (int j = 0; j < kDim; ++j) {
					if (i == j || mOutWeightSum[j] < 1e-6)
						continue;
					double weight = mAdjacencyMatrix[j][i];
					sum_weight += weight / mOutWeightSum[j] * mScores[j];
				}
				double newScore = 1.0 - m_d + m_d * sum_weight;
				newScores[i] = newScore;

				double delta = fabs(newScore - mScores[i]);
				maxDelta = max(maxDelta, delta);
			}

			mScores = newScores;
			if (maxDelta < mTol) {
				break;
			}
		}
		return true;
	}

	bool text_ranker::key_sentences(const std::wstring& texts, std::vector<std::wstring>& sentences, int topK)
	{
		text_ranker_impl<std::wstring> ranker;
		return ranker.ExtractKeySentences(texts, sentences, topK);
	}
}

