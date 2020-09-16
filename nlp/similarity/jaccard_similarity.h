#pragma once
#include <algorithm>
#include <string>
#include <vector>
#include <set>

namespace nlp
{
	template <typename value_type>
	class jaccard_similarity_t
	{
	public:
		typedef double similarity_t;
		static similarity_t index(const std::vector<value_type>& s1, const std::vector<value_type>& s2)
		{
			std::set<value_type> a;
			for (auto word : s1)
				a.insert(word);

			std::set<value_type> b;
			for (auto word : s2)
				b.insert(word);

			std::vector<value_type> intersect;
			std::set_intersection
			(
				a.begin(),
				a.end(),
				b.begin(),
				b.end(),
				std::back_inserter(intersect)
			);

			size_t sum = a.size() + b.size();
			size_t size_in = intersect.size();

			similarity_t _intersect = static_cast<similarity_t>(size_in);
			similarity_t _union = 1.0;
			if(sum - size_in  != 0)
				_union = static_cast<similarity_t>(sum - size_in);
			return _intersect / _union;
		}

		static similarity_t distance(const std::vector<value_type>& s1, const std::vector<value_type>& s2)
		{
			return 1.0 - index(s1, s2);
		}
	private:

	};
}