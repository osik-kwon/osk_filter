#include "filter_pch.h"
#include "cryptor/cryptor.h"
#include <algorithm>
#include <detail/cryptography/aes.hpp>
#include "io/binary_iostream.h"

namespace filter
{
	hwp50_distribution_srand_t::seed_t hwp50_distribution_srand_t::rand()
	{
		random_seed = (random_seed * 214013 + 2531011) & 0xFFFFFFFF;
		return (random_seed >> 16) & 0x7FFF;
	}

	hwp50_distribution_srand_t::seed_t hwp50_distribution_srand_t::make_seed()
	{
		seed_t time = 0x76642a54;
		random_seed =
			(time & 0x000000ff) << 24 |
			(time & 0x0000ff00) << 8 |
			(time & 0x00ff0000) >> 8 |
			(time & 0xff000000) >> 24;
		return random_seed;
	}

	void cryptor_t::xor_merge(buffer_t& data)
	{
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
	}

	void cryptor_t::decrypt_hwp50_distribution_key(const buffer_t& hint)
	{
		buffer_t data;
		std::copy(hint.begin(), hint.end(), std::back_inserter(data));
		xor_merge(data);
		size_t start_offset = 4 + (data[0] & 0xF);
		std::copy(data.begin() + start_offset, data.begin() + (start_offset + 16), std::back_inserter(symmetric_key));
		std::copy(data.begin() + start_offset, data.begin() + (start_offset + 80), std::back_inserter(hash_key));
		std::copy(data.begin() + start_offset + 80, data.begin() + (start_offset + 80 + 1), std::back_inserter(options));
	}

	cryptor_t::buffer_t cryptor_t::encrypt_hwp50_distribution_key()
	{
		if(hash_key.size() != 80)
			throw std::runtime_error("hash_key should be 80 bytes");
		if (options.size() != 1)
			throw std::runtime_error("options should be 1 bytes");
		buffer_t data;
		data.resize(256);

		bufferstream stream(&data[0], data.size());
		binary_io::write_uint32(stream, seed.make_seed());
		size_t start_offset = 4 + (data[0] & 0xF);
		std::copy(hash_key.begin(), hash_key.end(), data.begin() + start_offset);
		std::copy(options.begin(), options.end(), data.begin() + start_offset + hash_key.size() );
		const uint8_t SHA_signature = 0x80;
		data[start_offset + hash_key.size() + options.size()] = SHA_signature;
		xor_merge(data);
		return data;
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
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return buffer_t();
	}

	cryptor_t::buffer_t cryptor_t::encrypt_aes128_ecb_nopadding(buffer_t& plain)
	{
		std::vector<uint8_t> in;
		std::copy(plain.begin(), plain.end(), std::back_inserter(in));
		return encrypt_aes128_ecb_nopadding(std::move(in));
	}

	cryptor_t::buffer_t cryptor_t::encrypt_aes128_ecb_nopadding(std::vector<uint8_t>&& plain)
	{
		try
		{
			std::vector<uint8_t> key;
			std::copy(symmetric_key.begin(), symmetric_key.end(), std::back_inserter(key));
			std::vector<uint8_t> cipher_text;
			cipher_text = xlnt::detail::aes_ecb_encrypt(plain, key);
			buffer_t out;
			std::copy(cipher_text.begin(), cipher_text.end(), std::back_inserter(out));
			return out;
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		return buffer_t();
	}
}