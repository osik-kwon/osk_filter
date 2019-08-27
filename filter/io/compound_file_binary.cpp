#include "filter_pch.h"
#include "io/compound_file_binary.h"

namespace filter
{
	cfb_t::buffer_t cfb_t::extract_stream(std::unique_ptr<storage_t>& storage, const std::string& name)
	{
		std::unique_ptr<stream_t> cfb_stream = std::make_unique<stream_t>(storage.get(), name);
		if (cfb_stream->fail())
			throw std::runtime_error("stream create error");
		buffer_t buffer;
		buffer.resize(static_cast<size_t>(cfb_stream->size()));
		if (cfb_stream->read(reinterpret_cast<byte_t*>(&buffer[0]), cfb_stream->size()) != cfb_stream->size())
			throw std::runtime_error("stream read error");
		return buffer;
	}

	void cfb_t::make_stream(std::unique_ptr<storage_t>& storage, const std::string& name, const buffer_t& buffer)
	{
		auto stream = std::make_unique<stream_t>(storage.get(), name, true, buffer.size());
		if (stream->fail())
			throw std::runtime_error("stream open error");
		stream->seek(0);
		stream->write(reinterpret_cast<byte_t*>(
			const_cast<char*>(&buffer[0])
			), buffer.size());
		stream->flush();
	}

	void cfb_t::copy_streams(std::unique_ptr<storage_t>& import_storage, std::unique_ptr<storage_t>& export_storage, const std::vector<std::string>& all_streams_except_sections)
	{
		for (auto& stream : all_streams_except_sections)
		{
			make_stream(export_storage, stream, extract_stream(import_storage, stream));
		}
	}

	std::vector<std::string> cfb_t::make_all_streams_except(std::unique_ptr<storage_t>& import_storage, const std::vector<std::string>& others)
	{
		auto all_streams_others = make_full_entries(import_storage, "/");
		for (const auto& other : others)
		{
			auto found_other = std::find_if(all_streams_others.begin(),
				all_streams_others.end(), [&other](const std::string& name) {
				return name == other; });
			if (found_other != all_streams_others.end())
				all_streams_others.erase(found_other);
		}
		return all_streams_others;
	}

	std::vector<std::string> cfb_t::make_all_streams_except(std::unique_ptr<storage_t>& import_storage, const std::string& other)
	{
		auto all_streams_others = make_full_entries(import_storage, "/");
		auto found_other = std::find_if(all_streams_others.begin(),
			all_streams_others.end(), [&other](const std::string& name) {
			return name == other; });
		if (found_other != all_streams_others.end())
			all_streams_others.erase(found_other);
		return all_streams_others;
	}

	std::unique_ptr<cfb_t::storage_t> cfb_t::make_storage(const std::string& path, bool write_access, bool create)
	{
		auto original_storage = std::make_unique<storage_t>(path.c_str());
		original_storage->open(write_access, create);
		if (original_storage->result() != POLE::Storage::Ok)
			throw std::runtime_error("storage open error");
		return original_storage;
	}

	std::vector<std::string> cfb_t::make_full_entries(std::unique_ptr<storage_t>& storage, const std::string root)
	{
		std::vector<std::string> full_entries;
		auto streams = storage->GetAllStreams(root);
		std::copy(streams.begin(), streams.end(), std::back_inserter(full_entries));
		return full_entries;
	}
}