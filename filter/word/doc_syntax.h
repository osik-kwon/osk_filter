#pragma once
#include <string>
#include <memory>
#include "traits/binary_traits.h"
#include "traits/compound_file_binary_traits.h"
#include "io/binary_iostream.h"
#include "word/doc_file_information_block.h"

namespace filter
{
namespace doc
{
	typedef binary_traits::byte_t byte_t;
	typedef binary_traits::buffer_t buffer_t;
	typedef binary_traits::bufferstream bufferstream;
	typedef binary_traits::streamsize streamsize;

	class consumer_t
	{
	public:
		typedef cfb_traits::storage_t storage_t;
		typedef cfb_traits::stream_t stream_t;
		consumer_t();
		void open(const std::string& path);
	private:
	};

	class producer_t
	{
	public:
		producer_t();
		void save(const std::string& path, std::unique_ptr<consumer_t>& consumer);
	private:
	};
}
}