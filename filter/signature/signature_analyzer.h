#pragma once
#include <string>
#include <signature/signature_classifier.h>
namespace filter
{
namespace signature
{
	template <class name_t>
	class analyzer_t
	{
	public:
		typedef typename deterministic_classifier_t<name_t>::algorithm_t deterministic_algorithm_t;
		typedef typename nondeterministic_classifier_t<name_t>::algorithm_t nondeterministic_algorithm_t;
		analyzer_t(name_t default_name = name_t{});
		name_t scan(const std::string& path);
		analyzer_t& deterministic(const name_t& name, const std::string& key);
		analyzer_t& deterministic(const name_t& name, const std::string& key, deterministic_algorithm_t algorithm);
		analyzer_t& nondeterministic(const name_t& name, nondeterministic_algorithm_t algorithm);
	private:
		deterministic_classifier_t<name_t> deterministic_classifiers;
		nondeterministic_classifier_t<name_t> nondeterministic_classifiers;
		name_t default_name;
	};

	template <class name_t>
	analyzer_t<name_t>::analyzer_t(name_t default_name) : default_name(default_name)
	{}

	template <class name_t>
	name_t analyzer_t<name_t>::scan(const std::string& path)
	{
		try
		{
			// TODO: zero byte
			storage_t storage(path);
			auto result = deterministic_classifiers.longest_prefix(storage.get_header());
			auto id = result.value();
			auto key = result.key();
			if (deterministic_classifiers.has_algorithms(key))
			{
				const auto& algorithms = deterministic_classifiers.get_algorithms(key);
				for (auto& algorithm : algorithms)
				{
					try
					{
						if (algorithm.second(storage))
							return deterministic_classifiers.find_type(algorithm.first);
					}
					catch (const std::exception&) // IMPORTANT!
					{
					}
				}
			}

			if (deterministic_classifiers.exist_type(id))
				return deterministic_classifiers.find_type(id);

			auto name = nondeterministic_classifiers.classify_all(storage, default_name);
			const name_t empty{};
			if (name != empty)
				return name;

			return default_name;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return default_name;
	}

	template <class name_t>
	analyzer_t<name_t>& analyzer_t<name_t>::deterministic(const name_t& name, const std::string& key)
	{
		if (deterministic_classifiers.exact_match(key))
			throw std::logic_error("has already registered key in trie");
		auto trie_key = deterministic_classifiers.insert_key_at_trie(key);
		deterministic_classifiers.insert_type(trie_key, name);
		return *this;
	}

	template <class name_t>
	analyzer_t<name_t>& analyzer_t<name_t>::deterministic(const name_t& name, const std::string& key, deterministic_algorithm_t algorithm)
	{
		if (!deterministic_classifiers.exact_match(key))
		{
			auto trie_key = deterministic_classifiers.insert_key_at_trie(key);
			deterministic_classifiers.insert_type(trie_key, name);
		}
		else
		{
			// IMPORTANT!
			auto trie_key = deterministic_classifiers.make_unique_key();
			deterministic_classifiers.insert_type(trie_key, name);
		}

		deterministic_classifiers.insert_algorithm(
			deterministic_classifiers.get_unique_key(), key, algorithm);
		return *this;
	}

	template <class name_t>
	analyzer_t<name_t>& analyzer_t<name_t>::nondeterministic(const name_t& name, nondeterministic_algorithm_t algorithm)
	{
		nondeterministic_classifiers.insert_algorithm(name, algorithm);
		return *this;
	}
}
}