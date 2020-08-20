#include "nlp_pch.h"
#include "textrank/textrank.h"

#include <iostream>
#include "locale/charset_encoder.h"

#include <string>
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <cmath>
#include <set>

using namespace std;

namespace nlp
{
	class string_similarity
	{
	public:
		string_similarity() {}
		typedef double similarity_t;

		static similarity_t similarity(const std::string& longer, const std::string& shorter)
		{
			if (longer.size() < shorter.size())
			{
				return similarity_impl(shorter, longer);
			}
			return similarity_impl(longer, shorter);
		}

		static similarity_t average_of_similarities(const std::string& source, const std::vector<std::string>& words)
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

		static similarity_t similarity_impl(const std::string& longer, const std::string& shorter)
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

	static bool PairComp(std::pair<int, double> a, std::pair<int, double> b)
	{
		return a.second > b.second;
	}

	template <class Container>
	static void split(const std::string& str, Container& cont, char delimiter)
	{
		size_t current, previous = 0;
		current = str.find_first_of(delimiter);
		while (current != std::string::npos)
		{
			cont.push_back(str.substr(previous, current - previous));
			previous = current + 1;
			current = str.find_first_of(delimiter, previous);
		}
		cont.push_back(str.substr(previous, current - previous));
	}

	bool TextRanker::ExtractKeySentences(const std::string& input, std::vector<string>& outputs, int topK)
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

		std::sort(visitPairs.begin(), visitPairs.end(), PairComp);

		for (int i = 0; i < topK && i < kDim; ++i) {
			int id = visitPairs[i].first;
			outputs.push_back(mSentences[id]);
		}
		return true;
	}

	bool TextRanker::ExtractSentences(const std::string& input, std::vector<std::string>& outputs)
	{
		outputs.clear();
		if (input.empty()) {
			outputs.push_back("");
			return false;
		}

		static const int maxTextLen = 100000;
		std::string tempInput;
		if ((int)input.size() > maxTextLen) {
			tempInput = input.substr(0, maxTextLen);
		}
		else {
		tempInput = input;
		}

		// TODO: normalize
		// xxxx::toLowerCase(tempInput);

		/*
		std::wstring ucs2 = to_wchar(tempInput);
		for (auto& code : ucs2)
		{
			if ( !ptk::unicode::is_basic_latin_letter_or_digit(code)
				&& !ptk::unicode::is_modern_hangul(code)
				&& (code > 255)
				)
			{
				code = L'.';
			}
		}
		tempInput = to_utf8(ucs2);
		*/

		static const std::string punctuations[] = { "?", "!", ".", ";", "£¿", "£¡", "¡£", "£»", "¡¦¡¦", "¡¦", "\n" };
		for (int i = 0; i < (int)(sizeof(punctuations) / sizeof(punctuations[0])); ++i) {
			std::string punc = punctuations[i];
			StringReplaceAll(tempInput, punc, ".");
		}

		static const int minSentenceLen = 30;
		vector<std::string> tempOutput;
		split(tempInput, tempOutput, '.');
		vector<std::string> tempOutput2;
		for (int i = 0; i < (int)tempOutput.size(); ++i)
		{
			if ((int)tempOutput[i].size() > minSentenceLen)
			{
				//auto para = to_wchar(tempOutput[i]);
				//std::wcout << para;

				tempOutput2.push_back(tempOutput[i]);
			}
		}
		/*
		for (auto& lines : tempOutput2)
		{
			auto para = to_wchar(lines);
			std::wcout << para;
		}
		*/
		RemoveDuplicates(tempOutput2, outputs);

		static const int maxSentencesNum = 500;
		if ((int)outputs.size() > maxSentencesNum) {
			outputs.resize(maxSentencesNum);
		}

		return true;
	}

	bool TextRanker::RemoveDuplicates(const std::vector<std::string>& inputs, std::vector<std::string>& outputs)
	{
		outputs.clear();

		std::unordered_set<std::string> s(inputs.begin(), inputs.end());
		outputs = std::vector<std::string>(s.begin(), s.end());

		return true;
	}

	bool TextRanker::BuildGraph(const std::vector<std::string>& sentences)
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

	bool TextRanker::InitWordSet(const std::vector<std::string>& sentences)
	{
		int kDim = sentences.size();
		if (sentences.empty()) {
			return false;
		}

		mWordSets.clear();
		mWordSets.resize(kDim, std::set<std::string>());
		mWordSizes.clear();
		mWordSizes.resize(kDim, 0);

		for (int i = 0; i < kDim; ++i) {
			std::vector<std::string> aWords;
			//split(sentences[i], aWords, '\t');
			split(sentences[i], aWords, ' ');
			/*{
				auto para = to_wchar(sentences[i]);
				std::wcout << para;
			}


			for (auto& lines : aWords)
			{
				auto para = to_wchar(lines);
				std::wcout << para;
			}
			*/

			mWordSizes[i] = aWords.size();
			std::set<std::string> aWordSet(aWords.begin(), aWords.end());
			mWordSets[i] = aWordSet;
		}

		return true;
	}

	double TextRanker::GetSimilarity(int a, int b)
	{
		if ((int)mWordSets.size() <= a || (int)mWordSets.size() <= b || mWordSets.size() != mWordSizes.size()) {
			return 0.0;
		}
		/*
		std::wcout << "====================\n";

		for (auto word : mWordSets[a])
		{
			std::wcout << to_wchar(word) << "\t";
		}
		std::wcout << "\n";

		for (auto word : mWordSets[b])
		{
			std::wcout << to_wchar(word) << "\t";
		}

		std::wcout << "\n";
		*/
		std::vector<std::string> commonWords;
		/*
		std::set_intersection
		(
			mWordSets[a].begin(),
			mWordSets[a].end(),
			mWordSets[b].begin(),
			mWordSets[b].end(),
			std::back_inserter(commonWords)
		);
		*/
		// TODO:
		const string_similarity::similarity_t same = 0.85;
		for (const auto& first : mWordSets[a])
		{
			for (const auto& second : mWordSets[b])
			{
				string_similarity::similarity_t similarity = string_similarity::similarity(first, second);
				if (similarity > same)
				{
					if( to_wchar(second).size() > 1 )
						commonWords.push_back(second);
				}
			}
		}

		/*
		std::wcout << "====================\n";

		for (auto word : commonWords)
		{
			std::wcout << to_wchar(word) << "\t";
		}
		std::wcout << "\n";
		*/
		double denominator = std::log(double(mWordSizes[a])) + std::log(double(mWordSizes[b]));
		if (std::fabs(denominator) < 1e-6) {
			return 0.0;
		}
		return 1.0 * commonWords.size() / denominator;
	}

	bool TextRanker::CalcSentenceScores()
	{
		if (mAdjacencyMatrix.empty() || mAdjacencyMatrix[0].empty() || mOutWeightSum.empty()) {
			return false;
		}

		int kDim = mSentences.size();

		mScores.clear();
		mScores.resize(kDim, 1.0);

		// iterate
		int iterNum = 0;
		for (; iterNum < mMaxIter; ++iterNum) {
			double maxDelta = 0.0;
			vector<double> newScores(kDim, 0.0); // current iteration score

			for (int i = 0; i < kDim; ++i) {
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

		// std::cout << "iterNum: " << iterNum << "\n";
		return true;
	}

	void TextRanker::StringReplaceAll(std::string& str, const std::string& from, const std::string& to) {
		if (from.empty()) { return; }

		bool isToContainFrom = false;   // In case 'to' contains 'from', like replacing 'x' with 'yx'
		if (to.find(from, 0) != std::string::npos) {
			isToContainFrom = true;
		}

		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			if (isToContainFrom) {
				start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
			}
		}
	}
}

