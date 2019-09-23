#include "filter_pch.h"
#include <memory>
#include "locale/charset_detecter.h"
#include "uchardet.h"

namespace filter
{
	template <class T>
	static std::string detect_charset_impl(const T& data)
	{
		typedef std::unique_ptr<struct uchardet, std::function<void(struct uchardet*)> > unique_uchardet_t;
		unique_uchardet_t handle = unique_uchardet_t(uchardet_new(), [](uchardet_t handle) {
			uchardet_delete(handle); });
		if (!handle)
			return std::string();

		int retval = uchardet_handle_data(handle.get(), (const char*)data.data(), data.size());
		if (retval != 0)
			return std::string();
		uchardet_data_end(handle.get());
		return std::string(uchardet_get_charset(handle.get()));
	}

	std::string charset_detecter::detect(const std::string& data) {
		return detect_charset_impl(data);
	}
	std::string charset_detecter::detect(const std::vector<uint8_t>& data) {
		return detect_charset_impl(data);
	}
	std::string charset_detecter::detect(const std::vector<char>& data) {
		return detect_charset_impl(data);
	}
}