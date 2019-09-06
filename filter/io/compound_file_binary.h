#pragma once
#include <memory>
#include "traits/binary_traits.h"
#include "traits/compound_file_binary_traits.h"

namespace filter
{
	class cfb_t
	{
	public:
		typedef cfb_traits::storage_t storage_t;
		typedef cfb_traits::stream_t stream_t;
		typedef binary_traits::byte_t byte_t;
		typedef binary_traits::buffer_t buffer_t;
		cfb_t() = default;
		static buffer_t extract_stream(std::unique_ptr<storage_t>& storage, const std::string& name);
		static void make_stream(std::unique_ptr<storage_t>& storage, const std::string& name, const buffer_t& buffer);
		static void copy_streams(std::unique_ptr<storage_t>& import_storage, std::unique_ptr<storage_t>& export_storage, const std::vector<std::string>& all_streams_except_sections);

		static std::vector<std::string> make_all_streams_except(std::unique_ptr<storage_t>& import_storage, const std::vector<std::string>& others);
		static std::vector<std::string> make_all_streams_except(std::unique_ptr<storage_t>& import_storage, const std::string& other);
		static std::vector<std::string> make_full_entries(std::unique_ptr<storage_t>& storage, const std::string root);

		static std::unique_ptr<storage_t> make_read_only_storage(const std::string& path) {
			return make_storage(path, false, false);
		}
		static std::unique_ptr<storage_t> make_writable_storage(const std::string& path) {
			return make_storage(path, true, true);
		}
	private:
		static std::unique_ptr<storage_t> make_storage(const std::string& path, bool write_access, bool create);
	};
}