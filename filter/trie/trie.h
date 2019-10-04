#pragma once
#include <vector>
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/iterator/iterator_facade.hpp>
# ifndef BOOST_NO_SFINAE
#  include <boost/type_traits/is_convertible.hpp>
#  include <boost/utility/enable_if.hpp>
# endif

//#define USE_PREFIX_TRIE
#ifdef USE_PREFIX_TRIE
#include <trie/cedarpp.h>
#else
#include <trie/cedar.h>
#endif

namespace filter
{
	// interface for script languages; you may want to extend this
	class trie_impl : public boost::noncopyable
	{
	public:
#ifdef USE_PREFIX_TRIE
		typedef cedar::npos_t npos_t;
#else
		typedef size_t npos_t;
#endif
		typedef ::cedar::da <int>  trie_t;

		trie_impl  () : _t (new ::cedar::da <int> ()), _num_keys (0) {}
		~trie_impl () { delete _t; }
		typedef std::pair<std::string, int> value_type;
		// iterator
		template <class value_type>
		class iterator_impl
			: public boost::iterator_facade<
			iterator_impl<value_type>
			, value_type
			, std::forward_iterator_tag
			>
		{
		private:
			struct enabler {};  // a private type avoids misuse
		public:
			explicit iterator_impl(trie_t* t): _trie(t), from(0), length(0), pos(trie_t::CEDAR_NO_PATH)
			{}
			explicit iterator_impl(trie_t* t, size_t _from, size_t _length, int _pos)
				: _trie(t), from(_from), length(_length), pos(_pos)
			{}

			template <class other_value>
			iterator_impl(
				iterator_impl<other_value> const& other
# ifndef BOOST_NO_SFINAE
				, typename boost::enable_if<
				boost::is_convertible<other_value*,value_type*>
				, enabler
				>::type = enabler()
# endif 
				) : _trie(other._trie), from(other.from), length(other.length), pos(other.pos) {}

			typedef typename boost::iterator_facade<iterator_impl<value_type>, value_type, std::forward_iterator_tag>::difference_type difference_type;
			typedef typename boost::iterator_facade<iterator_impl<value_type>, value_type, std::forward_iterator_tag>::reference reference;
		private:
			friend class boost::iterator_core_access;

			void increment() {
				pos = _trie->next(from, length);
			}

			template <class other_value>
			bool equal(iterator_impl<other_value> const& other) const
			{
				return this->_trie == other._trie && this->from == other.from && 
					this->length == other.length && this->pos == other.pos;
			}

			reference dereference() const {
				result.first.resize(0);
				result.first.resize(length);
				_trie->suffix(&result.first[0], length, from);

				size_t pos_from = 0;
				size_t value_from = 0;
				result.second = _trie->traverse (result.first.c_str(), value_from, pos_from);
				return result;
			}

# ifdef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
		public:
# else
		private:
			template <class> friend class iterator_impl;
# endif 
			trie_t* _trie;
			size_t from;
			size_t length;
			int pos;
			typedef typename std::remove_cv<value_type>::type result_type;
			mutable result_type result;
		};

		typedef iterator_impl<value_type> iterator;
		typedef iterator_impl<value_type const> const_iterator;

		iterator begin()
		{
			size_t from = 0;
			size_t length = 0;
			int pos = _t->begin(from, length);
			return iterator(_t, from, length, pos);
		}
		iterator end() { return iterator(_t); }
		const_iterator begin() const
		{
			size_t from = 0;
			size_t length = 0;
			int pos = _t->begin(from, length);
			return const_iterator(_t, from, length, pos);
		}
		const_iterator end() const { return const_iterator(_t); }

		// return type for prefix ()
		class result_t
		{
		public:
			result_t(const result_t& r) : _t (r._t), _id (r._id), _len (r._len), _val (r._val), _key (r._key)
			{}
			result_t(trie_t* t = 0, const npos_t id = 0, const size_t len = 0, const int val = 0) : _t (t), _id (id),  _len (len), _val (val), _key ()
			{}
			~result_t()
			{}
			void reset(const npos_t id, const size_t len, const int val)
			{
				_id = id;
				_len = len;
				_val = val;
				_key.clear ();
			}
			const std::string& key() const
			{
				if (_key.empty())
				{
					_key.resize(_len);
					_t->suffix (&_key[0], _len, _id);
				}
				return _key;
			}
			int value () const { return _val; }
		protected:
			trie_t*  _t;
			npos_t  _id;
			size_t  _len;
			int     _val;
		private:
			mutable std::string  _key;
		};

		// return type (iterator) for predict ()
		class trie_iterator : public result_t {
		private:
			const npos_t _root;
			result_t     _ret;
		public:
			trie_iterator (const trie_iterator& r) : result_t (r), _root (r._root), _ret (r._ret) {}
			trie_iterator (trie_t* t, const npos_t root = 0, const npos_t id = 0, const size_t len = 0, const int val = 0) : 
				result_t (t, id, len, val), _root (root), _ret (t) {}
			~trie_iterator () {}
			const result_t* next () {
				if (_val == trie_t::CEDAR_NO_PATH) return 0;
				_ret.reset (_id, _len, _val);
				_val = _t->next (_id, _len, _root);
				return &_ret;
			}
		};

		friend class longest_matcher;
		class longest_matcher
		{
		public:
			longest_matcher(trie_t* t) : m_trie(t), m_from(0), m_pos(0)
			{
				m_lrm = result_t(m_trie, 0, 0, trie_t::CEDAR_NO_VALUE);
			}

			void reset(trie_t* t)
			{
				m_trie = t;
				m_from = m_pos = 0;
				m_lrm = result_t(m_trie, 0, 0, trie_t::CEDAR_NO_VALUE);
			}

			bool lookup(char key) 
			{
				if(key == 0)
					return false;
				size_t pos = 0;
				const int id = m_trie->traverse (&key, m_from, pos, pos+1);
				if(id == trie_t::CEDAR_NO_PATH) // mismatch
					return false;
				// match
				m_pos++;
				if (id != trie_t::CEDAR_NO_VALUE)
					m_lrm.reset (m_from, m_pos, id);

				return true;
			}

			bool lookup(const std::string& key) 
			{
				if(key.empty())
					return false;
				for (auto code : key)
					if(!lookup(code))
						return false;
				return true;
			}

			result_t result() const
			{
				return m_lrm;
			}

			int value() const
			{
				return m_lrm.value();
			}

		private:
			npos_t m_from;	// current node id
			size_t m_pos;	// least-recently-matched key's length
			result_t m_lrm; // least-recently-matched
			trie_t* m_trie;
		};

		longest_matcher make_longest_matcher() const { return longest_matcher(_t); }

		// read/write
		bool open (const std::string& path)
		{
			return _t->open (path.c_str(), "rb") == 0;
		}
		bool save (const std::string& path)
		{
			return _t->save (path.c_str(), "wb") == 0;
		}
		// get statistics
		size_t num_keys () const { return _num_keys; } // O(1)

		// low-level predicates
		int  insert(const std::string& key, int n = 0) {
			return insert(key.c_str(), n);
		}

		int  insert(const char* key, int n = 0)
		{
			npos_t from = 0;
			size_t pos (0), len (std::strlen (key));
			const int n_ = _t->traverse (key, from, pos, len);
			bool flag = n_ == trie_t::CEDAR_NO_VALUE || n_ == trie_t::CEDAR_NO_PATH;
			if (flag) ++_num_keys;
			_t->update (key, from, pos, len) = n;
			return flag ? 0 : -1;
		}

		int  erase(const std::string& key) {
			return erase(key.c_str());
		}

		int  erase(const char* key)
		{
			if (_t->erase (key))
				return -1;
			else
			{
				--_num_keys;
				return 0;
			}
		}

		int  lookup(const std::string& key) const {
			return lookup(key.c_str());
		}

		int lookup(const char* key) const
		{
			return _t->exactMatchSearch <trie_t::result_type> (key);
		}

		// high-level (trie-specific) predicates
		std::vector <result_t> prefix (const char* key) const
		{
			std::vector <result_t> result;
			if (!key)
				return result;
			std::vector <trie_t::result_triple_type> result_;
			const size_t len = std::strlen (key);
			result_.resize (len);
			result.resize  (_t->commonPrefixSearch (key, &result_[0], len, len), _t);
			for (size_t i (0), n (result.size ()); i != n; ++i)
				result[i].reset (result_[i].id, result_[i].length, result_[i].value);
			return result;
		}

		result_t longest_prefix(const std::string& key) const
		{
			return longest_prefix(key.c_str());
		}

		result_t longest_prefix (const char* key) const
		{
			result_t r (_t, 0, 0, trie_t::CEDAR_NO_VALUE); // result for not found
			if (!key)
				return r;
			npos_t from = 0;
			for (size_t pos (0), len (std::strlen (key)); pos < len; ) {
				const int n = _t->traverse (key, from, pos, pos + 1);
				if (n == trie_t::CEDAR_NO_PATH)  break;
				if (n != trie_t::CEDAR_NO_VALUE) r.reset (from, pos, n);
			}
			return r;
		}

		trie_iterator predict (const char* key)
		{
			npos_t from = 0;
			size_t pos (0), len (0);
			int n = _t->traverse (key, from, pos);
			npos_t root = from;
			if (n != trie_t::CEDAR_NO_PATH)
				n = _t->begin (from, len);
			return trie_iterator (_t, root, from, len, n);
		}
	private:
		trie_t* _t;
		size_t  _num_keys;
	};
}