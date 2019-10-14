#include <filter_pch.h>
#include <signature/signature_builder.h>
#include <signature/signature_storage.h>

namespace filter
{
namespace signature
{
	std::unique_ptr< filter::signature::analyzer_t<std::string> > builder_t::build_string_rules()
	{
		using filter::signature::storage_t;
		auto make = std::make_unique< filter::signature::analyzer_t<std::string> >("txt");

		make->deterministic("pdf", "\x25\x50\x44\x46");
		make->deterministic("PKZIP archive_1", "\x50\x4B\x03\x04");
		make->deterministic("hwp30", "HWP Document File", [](storage_t& storage) {
			return storage.range(0, 30).match("HWP Document File V[1-3]\\.[0-9]{2} \x1a\x1\x2\x3\x4\x5");
			});
		make->deterministic("hwp50", "\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1", [](storage_t& storage) {
			return storage.compound("/FileHeader").range(0, 32).match("HWP Document File.*");
			});
		make->deterministic("hwpx", "\x50\x4B\x03\x04", [](storage_t& storage) {
			return storage.package("META-INF/container.xml").
				sequence(3).element("rootfile").attribute("media-type").equal("application/hwpml-package+xml");
			});

		make->nondeterministic("hwpml", [](storage_t& storage) {
			return storage.sequence(1).element("HWPML").exist();
			});
		return make;
	}
}
}