#pragma once
#include <string>
#include <vector>
#include <set>
namespace nlp
{
	class TextRanker {
	public:
		explicit TextRanker()
			: m_d(0.85), mMaxIter(100), mTol(1.0e-5) { }
		explicit TextRanker(double d, int maxIter, double tol)
			: m_d(d), mMaxIter(maxIter), mTol(tol) { }

		~TextRanker() { }

		bool ExtractKeySentences(const std::string& input, std::vector<std::string>& outputs, int topK);

	private:
		bool ExtractSentences(const std::string& input, std::vector<std::string>& output);
		bool RemoveDuplicates(const std::vector<std::string>& input, std::vector<std::string>& output);
		bool BuildGraph(const std::vector<std::string>& sentences);
		double GetSimilarity(int a, int b);
		bool CalcSentenceScores();
		bool InitWordSet(const std::vector<std::string>& sentences);
		void StringReplaceAll(std::string& str, const std::string& from, const std::string& to);

	private:
		double m_d;
		int mMaxIter;
		double mTol;
		std::vector<std::string> mSentences;
		std::vector< std::vector<double> > mAdjacencyMatrix;
		std::vector<double> mOutWeightSum;
		std::vector<double> mScores;
		std::vector< std::set<std::string> > mWordSets;
		std::vector<int> mWordSizes;
	};
}