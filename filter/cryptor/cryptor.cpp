#include "filter_pch.h"
#include "cryptor/cryptor.h"
#include <algorithm>
#include <detail/cryptography/aes.hpp>

namespace filter
{
	hwp50_distribution_srand_t::seed_t hwp50_distribution_srand_t::rand()
	{
		random_seed = (random_seed * 214013 + 2531011) & 0xFFFFFFFF;
		return (random_seed >> 16) & 0x7FFF;
	}

	void cryptor_t::make_hwp50_distribution_key(const buffer_t& hint)
	{
		buffer_t data;
		std::copy(hint.begin(), hint.end(), std::back_inserter(data));
		uint8_t _xor = 0;
		for (int32_t i = 0, n = 0; i < 256; i++, n--)
		{
			if (n == 0)
			{
				_xor = seed.rand() & 0xFF;
				n = (seed.rand() & 0xF) + 1;
			}

			if (i >= 4)
				data[i] ^= _xor;
		}
		size_t start_offset = 4 + (data[0] & 0xF);
		std::copy(data.begin() + start_offset, data.begin() + (start_offset + 16), std::back_inserter(symmetric_key));
		std::copy(data.begin() + start_offset, data.begin() + (start_offset + 80), std::back_inserter(hash_key));
		std::copy(data.begin() + start_offset + 80, data.begin() + (start_offset + 80 + 1), std::back_inserter(options));
	}
	cryptor_t::buffer_t cryptor_t::decrypt_aes128_ecb_nopadding(std::vector<uint8_t>&& cipher_text)
	{
		try
		{
			std::vector<uint8_t> key;
			std::copy(symmetric_key.begin(), symmetric_key.end(), std::back_inserter(key));
			std::vector<uint8_t> plain;
			plain = xlnt::detail::aes_ecb_decrypt(cipher_text, key);
			buffer_t out;
			std::copy(plain.begin(), plain.end(), std::back_inserter(out));
			return out;
		}
		catch (const std::exception&)
		{
		}
		return buffer_t();
	}
}