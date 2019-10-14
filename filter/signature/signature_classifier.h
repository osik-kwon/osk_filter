#pragma once
#include <signature/signature_storage.h>
#include <stdexcept>
#include <map>
#include <algorithm>
#include <functional>
#include <trie/trie.h>

namespace filter
{
namespace signature
{

	template <class name_t>
	class deterministic_classifier_t
	{
	public:
		typedef int trie_key_t;
		typedef trie_impl trie_t;
		typedef trie_impl::result_t trie_result_t;
		typedef std::function<bool(storage_t&)> algorithm_t;
		typedef std::map<trie_key_t, algorithm_t> algorithms_t;
		deterministic_classifier_t() : unique_key(0)
		{}

		// type interfaces
		void insert_type(trie_key_t trie_key, const name_t& name)
		{
			if (types.find(trie_key) != types.end())
				throw std::logic_error("has already registered format");
			types.insert({ trie_key, name });
		}

		bool exist_type(trie_key_t trie_key) const
		{
			return types.find(trie_key) != types.end();
		}

		name_t find_type(trie_key_t trie_key) const
		{
			if (types.find(trie_key) == types.end())
				throw std::logic_error("trie id is not exist");
			return types.find(trie_key)->second;
		}

		// trie interfaces
		trie_key_t get_unique_key() const {
			return unique_key;
		}

		trie_key_t make_unique_key() {
			return ++unique_key;
		}

		bool exact_match(const std::string& key) const {
			return binary_classifiers.lookup(key) != -1;
		}

		trie_result_t longest_prefix(const std::string& key) const
		{
			return binary_classifiers.longest_prefix(key);
		}

		trie_key_t insert_key_at_trie(const std::string& key) {
			++unique_key;
			if (binary_classifiers.insert(key, unique_key) == -1)
				throw std::logic_error("trie insert fail");
			return unique_key;
		}

		// algorithm interfaces
		bool has_algorithms(const std::string& key) const {
			return classifiers.find(key) != classifiers.end();
		}

		const algorithms_t& get_algorithms(const std::string& key) const {
			if (classifiers.find(key) == classifiers.end())
				throw std::logic_error("algorithm is not exist");
			return classifiers.find(key)->second;
		}

		void insert_algorithm(trie_key_t id, const std::string& key, algorithm_t algorithm) {
			if (classifiers.find(key) == classifiers.end())
			{
				classifiers.insert({ key, algorithms_t{} });
			}
			classifiers[key].insert({ id, algorithm });
		}
	private:
		trie_key_t unique_key;
		trie_t binary_classifiers;
		std::map<trie_key_t, name_t> types;
		std::map<std::string, algorithms_t > classifiers;
	};

	template <class name_t>
	class nondeterministic_classifier_t
	{
	public:
		typedef std::function<bool(storage_t&)> algorithm_t;
		nondeterministic_classifier_t()
		{}

		// algorithm interfaces
		bool has_algorithm(const name_t& name) const {
			return classifiers.find(name) != classifiers.end();
		}

		algorithm_t get_algorithm(const name_t& name) const {
			if (classifiers.find(name) == classifiers.end())
				throw std::logic_error("algorithm is not exist");
			return classifiers.find(name)->second;
		}

		name_t classify_all(storage_t& storage, const name_t& default_name)
		{
			for (auto& algorithm : classifiers)
			{
				try
				{
					if (algorithm.second(storage))
						return algorithm.first;
				}
				catch (const std::exception&)
				{
				}
			}
			return default_name;
		}

		void insert_algorithm(const name_t& name, algorithm_t algorithm) {
			if (classifiers.find(name) != classifiers.end())
				throw std::logic_error("has already inserted algorithm");
			classifiers.insert({ name, algorithm });
		}
	private:
		std::map< name_t, algorithm_t > classifiers;
	};
}
}