#include <filter_pch.h>
#include <signature/signature_storage.h>
#include <regex>
#include <io/file_stream.h>
#include <io/compound_file_binary.h>

namespace filter
{
namespace signature
{
	range_t::range_t() : begin(0), end(0)
	{}

	range_t::range_t(const std::string& buffer, std::size_t begin, std::size_t end) :
		begin(begin), end(end), data(buffer.begin() + begin, buffer.begin() + end)
	{}

	bool range_t::match(const std::string& rule) const
	{
		if (data.size() < (begin + end))
			throw std::runtime_error("buffer overrun error");
		std::regex matcher(rule);
		return std::regex_match(data.begin(), data.begin() + end, matcher);
	}

	bool range_t::equal(const std::string& dest) const
	{
		return data == dest;
	}

	attribute_t::attribute_t(const std::unique_ptr<xml::parser>& parser, const std::string& name)
		: parser(parser), name(name)
	{}

	bool attribute_t::equal(const std::string& dest) const
	{
		auto& attribute_map = parser->attribute_map();
		if (attribute_map.find(name) == attribute_map.end())
			throw std::logic_error("attribute is not exist"); // TODO: custom exception
		return attribute_map.find(name)->second.value == dest;
	}

	bool attribute_t::match(const std::string& rule) const
	{
		auto& attribute_map = parser->attribute_map();
		if (attribute_map.find(name) == attribute_map.end())
			throw std::logic_error("attribute is not exist"); // TODO: custom exception

		std::regex matcher(rule);
		return std::regex_match(attribute_map.find(name)->second.value, matcher);
	}

	bool attribute_t::exist() const
	{
		auto& attribute_map = parser->attribute_map();
		return attribute_map.find(name) != attribute_map.end();
	}

	element_t::element_t(const std::unique_ptr<xml::parser>& parser) : parser(parser)
	{}

	bool element_t::equal(const std::string& dest) const
	{
		return parser->qname() == dest;
	}
	bool element_t::exist() const
	{
		return !parser->qname().empty();
	}

	attribute_t element_t::attribute(const std::string& name) const
	{
		return attribute_t(parser, name);
	}

	sequence_t::sequence_t()
	{}

	sequence_t::sequence_t(std::unique_ptr<std::streambuf>& buf, const std::string& path, size_t nth_element)
	{
		load(buf, path);
		visit(nth_element);
	}

	sequence_t::sequence_t(const std::string& path, size_t nth_element)
	{
		load(path);
		visit(nth_element);
	}

	void sequence_t::load(const std::string& path)
	{
		file = std::make_unique<std::ifstream>(to_fstream_path(path), std::ios::binary);
		file->exceptions(std::ifstream::badbit | std::ifstream::failbit);
		parser = std::make_unique<xml::parser>(*file, path);
	}

	void sequence_t::load(std::unique_ptr<std::streambuf>& buf, const std::string& path)
	{
		stream = std::make_unique<std::istream>(buf.get());
		parser = std::make_unique<xml::parser>(*stream, path);
	}

	void sequence_t::visit(size_t nth_element)
	{
		size_t id = 0;
		for (auto event(parser->next()); event != xml::parser::eof; event = parser->next())
		{
			if (event == xml::parser::start_element)
			{
				auto& attribute_map = parser->attribute_map(); // IMPORTANT! e->attr_unhandled_ = 0; // Assume all handled.
				++id;
				if (id == nth_element)
					return;
			}
		}
	}

	element_t sequence_t::element(const std::string& name) const
	{
		return element_t(parser);
	}

	attribute_t sequence_t::attribute(const std::string& name) const
	{
		return attribute_t(parser, name);
	}

	attributes_t::attributes_t(const pugi::xpath_node_set& nodes, const std::string& name) :
		nodes(nodes), name(name)
	{}

	bool attributes_t::equal(const std::string& dest) const
	{
		// TODO:
		return true;
	}

	bool attributes_t::match(const std::string& rule) const
	{
		// TODO:
		return true;
	}

	bool attributes_t::exist() const
	{
		// TODO:
		/*
		for (auto it = nodes.begin(); it != nodes.end(); ++it)
		{
			auto node = *it;
			node.node().attribute(name.c_str()).empty();
			std::cout << node.node().attribute("Filename").value() << "\n";
		}
		*/
		return true;
	}

	xpath_t::xpath_t()
	{}

	xpath_t::xpath_t(const std::string& path, const std::string& xpath)
	{
		std::ifstream src(to_fstream_path(path), std::ios::binary);
		src.exceptions(std::ofstream::badbit | std::ofstream::failbit);
		document = std::make_unique<xml_document_t>();
		pugi::xml_parse_result result = document->load(src, pugi::parse_default, pugi::xml_encoding::encoding_auto);
		if (!result)
			throw std::logic_error("invalid xml format"); // TODO: custom exception
	}

	xpath_t::xpath_t(std::unique_ptr<std::streambuf>& buf, const std::string& xpath) : xpath(xpath)
	{
		std::istream stream(buf.get());
		document = std::make_unique<xml_document_t>();
		pugi::xml_parse_result result = document->load(stream, pugi::parse_default, pugi::xml_encoding::encoding_auto);
		if (!result)
			throw std::logic_error("invalid xml format"); // TODO: custom exception
	}

	bool xpath_t::exist() const
	{
		auto nodes = document->select_nodes(xpath.c_str());
		return !nodes.empty();
	}

	attributes_t xpath_t::attributes(const std::string& name)
	{
		auto nodes = document->select_nodes(xpath.c_str());
		if(nodes.empty())
			throw std::logic_error("invalid xpath"); // TODO: custom exception
		return attributes_t(nodes, name);
	}

	package_t::package_t()
	{}

	package_t::package_t(const std::string& path, const std::string& part_name) : path(path), part_name(part_name)
	{
		open_package();
	}

	void package_t::open_package()
	{
		source = std::make_unique<std::ifstream>(to_fstream_path(path), std::ios::binary);
		source->exceptions(std::ifstream::badbit | std::ifstream::failbit);
		archive = std::make_unique<izstream_t>(*source);
		if (!archive->has_file(path_t(part_name)))
			throw std::logic_error("part is not exist"); // TODO: custom exception
	}

	sequence_t& package_t::sequence(size_t nth_element)
	{
		auto stream_buf = archive->open(path_t(part_name));
		sequence_buffer = sequence_t(stream_buf, part_name, nth_element);
		return sequence_buffer;
	}

	xpath_t& package_t::xpath(const std::string& path)
	{
		auto stream_buf = archive->open(path_t(part_name));
		xpath_buffer = xpath_t(stream_buf, path);
		return xpath_buffer;
	}

	compound_t::compound_t()
	{}

	compound_t::compound_t(const std::string& path, const std::string& stream_name) :
		path(path), stream_name(stream_name)
	{
		open_storage();
	}

	range_t& compound_t::range(size_t begin, size_t end)
	{
		range_buffer = std::make_unique<range_t>(
			cfb_t::extract_stream_by_string(storage, stream_name), begin, end
			);
		return *range_buffer;
	}

	void compound_t::open_storage()
	{
		storage = cfb_t::make_read_only_storage(path);
		if (!storage->exists(stream_name))
			throw std::logic_error("stream is not exist");
	}

	range_t& storage_t::range(size_t begin, size_t end)
	{
		range_buffer = std::make_unique<range_t>(header, begin, end);
		return *range_buffer;
	}

	sequence_t& storage_t::sequence(size_t nth_element)
	{
		sequence_buffer = std::make_unique<sequence_t>(path, nth_element);
		return *sequence_buffer;
	}

	package_t& storage_t::package(const std::string& part_name)
	{
		package_buffer = std::make_unique<package_t>(path, part_name);
		return *package_buffer;
	}

	compound_t& storage_t::compound(const std::string& stream_name)
	{
		compound_buffer = std::make_unique<compound_t>(path, stream_name);
		return *compound_buffer;
	}

	storage_t::storage_t(const std::string& path) : path(path)
	{
		header = read_header(64);
	}

	std::string storage_t::read_header(size_t minimal_length)
	{
		std::ifstream file(to_fstream_path(path), std::ios::binary);
		file.exceptions(std::ifstream::badbit | std::ifstream::failbit);

		file.unsetf(std::ios::skipws);
		file.seekg(0, std::ios::end);
		std::streampos size = file.tellg();
		file.seekg(0, std::ios::beg);

		size_t file_size = (size_t)size;
		size_t buffer_size = 0;
		if (file_size > minimal_length)
			buffer_size = minimal_length;
		else
			buffer_size = file_size;

		std::string buffer;
		if (buffer_size == 0)
			return buffer; // zero byte
		std::copy_n(std::istream_iterator<char>(file), buffer_size, std::back_inserter(buffer));
		return buffer;
	}

	bool storage_t::empty() const {
		return header.empty();
	}

	const std::string& storage_t::get_header() const {
		return header;
	}
}
}