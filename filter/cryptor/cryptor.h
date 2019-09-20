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
		seed_t make_seed();
	private:
		seed_t random_seed;
	};

	class cryptor_t
	{
	public:
		typedef binary_traits::byte_t byte_t;
		typedef binary_traits::buffer_t buffer_t;
		typedef binary_traits::bufferstream bufferstream;
		typedef binary_traits::streamsize streamsize;
		cryptor_t(uint32_t that) : seed(that)
		{}
		cryptor_t() : seed(0)
		{}

		void reset(uint32_t that) {
			seed = hwp50_distribution_srand_t(that);
		}

		void decrypt_hwp50_distribution_key(const buffer_t& hint);
		buffer_t encrypt_hwp50_distribution_key();

		buffer_t decrypt_aes128_ecb_nopadding(std::vector<uint8_t>&& cipher_text);
		buffer_t encrypt_aes128_ecb_nopadding(std::vector<uint8_t>&& plain);
		buffer_t encrypt_aes128_ecb_nopadding(buffer_t& plain);

		hwp50_distribution_srand_t seed;
		buffer_t symmetric_key;
		buffer_t hash_key;
		buffer_t options;
	};
}