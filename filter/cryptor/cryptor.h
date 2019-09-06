#pragma once
#include "traits/binary_traits.h"

namespace filter
{
	class hwp50_distribution_srand_t
	{
	public:
		typedef uint32_t seed_t;
		hwp50_distribution_srand_t(seed_t seed) : random_seed(seed)
		{}
		seed_t rand();
	private:
		seed_t random_seed;
	};

	class cryptor_t
	{
	public:
		typedef binary_traits::buffer_t buffer_t;
		cryptor_t(uint32_t that) : seed(that)
		{}
		void make_hwp50_distribution_key(const buffer_t& hint);
		buffer_t decrypt_aes128_ecb_nopadding(std::vector<uint8_t>&& cipher_text);

		hwp50_distribution_srand_t seed;
		buffer_t symmetric_key;
		buffer_t hash_key;
		buffer_t options;
	};
}