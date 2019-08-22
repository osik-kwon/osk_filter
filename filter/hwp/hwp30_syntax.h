#pragma once
#include "define/binary_traits.h"
#include "io/binary_iostream.h"

namespace filter
{
namespace hwp30
{
	typedef binary_traits::byte_t byte_t;
	typedef binary_traits::buffer_t buffer_t;
	typedef binary_traits::bufferstream bufferstream;
	typedef binary_traits::streamsize streamsize;

	struct doc_info_t
	{
		doc_info_t() = default;

		DECLARE_BINARY_SERIALIZER(doc_info_t);
		std::size_t size() const {
			return 128;
		}
		buffer_t body;

		// semantics
		uint16_t crypted;
		uint8_t compressed;
		uint16_t info_block_length;
	};

	struct doc_summary_t
	{
		doc_summary_t() = default;
		DECLARE_BINARY_SERIALIZER(doc_summary_t);
		std::size_t size() const {
			return 1008;
		}
		buffer_t body;
	};


	struct info_block_t
	{
		info_block_t() : info_block_length(0)
		{}
		info_block_t(uint16_t len) : info_block_length(len)
		{}
		DECLARE_BINARY_SERIALIZER(info_block_t);
		std::size_t size() const {
			return info_block_length;
		}
		buffer_t body;
		uint16_t info_block_length;
	};

	// compress data
	struct face_name_list_t
	{
		typedef std::string face_name_t;
		typedef std::vector<face_name_t> face_names_t;
		typedef std::vector<face_names_t> family_list_t;

		face_name_list_t() = default;
		DECLARE_BINARY_SERIALIZER(face_name_list_t);

		family_list_t family_list;
	};

	struct char_shape_t
	{
		char_shape_t() = default;
		DECLARE_BINARY_SERIALIZER(char_shape_t);
		std::size_t size() const {
			return 31;
		}
		buffer_t body;
	};

	struct para_shape_t
	{
		para_shape_t() = default;
		DECLARE_BINARY_SERIALIZER(para_shape_t);
		std::size_t size() const {
			return 187;
		}
		buffer_t body;
	};

	struct style_t
	{
		typedef std::string name_t;
		style_t() = default;
		DECLARE_BINARY_SERIALIZER(style_t);

		name_t name; // 20 bytes
		char_shape_t char_shape;
		para_shape_t para_shape;
	};

	struct style_list_t
	{
		style_list_t() = default;
		DECLARE_BINARY_SERIALIZER(style_list_t);

		std::vector<style_t> styles;
	};

	struct document_t
	{
		document_t()
		{}

		buffer_t signature;
		doc_info_t doc_info;
		doc_summary_t doc_summary;
		info_block_t info_block;

		// compress data
		face_name_list_t face_name_list;
		style_list_t style_list;
		// TODO: implement
	};

}
}