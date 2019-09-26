#pragma once
#include <boost/iostreams/filter/newline.hpp>

namespace boost {
namespace iostreams {
	class wnewline_checker : public detail::newline_base {
	public:
		typedef wchar_t char_type;
		struct category
			: dual_use_filter_tag,
			closable_tag
		{ };
		explicit wnewline_checker(int target = newline::mixed)
			: detail::newline_base(0), target_(target), open_(false)
		{ }
		template<typename Source>
		int get(Source& src)
		{
			using newline::CR;
			using newline::LF;

			if (!open_) {
				open_ = true;
				source() = 0;
			}

			int c;
			if ((c = iostreams::get(src)) == WWOULD_BLOCK)
				return WWOULD_BLOCK;

			// Update source flags.
			if (c != WEOF)
				source() &= ~f_line_complete;
			if ((source() & f_has_CR) != 0) {
				if (c == LF) {
					source() |= newline::dos;
					source() |= f_line_complete;
				}
				else {
					source() |= newline::mac;
					if (c == WEOF)
						source() |= f_line_complete;
				}
			}
			else if (c == LF) {
				source() |= newline::posix;
				source() |= f_line_complete;
			}
			source() = (source() & ~f_has_CR) | (c == CR ? f_has_CR : 0);

			// Check for errors.
			if (c == WEOF &&
				(target_ & newline::final_newline) != 0 &&
				(source() & f_line_complete) == 0)
			{
				fail();
			}
			if ((target_ & newline::platform_mask) != 0 &&
				(source() & ~target_ & newline::platform_mask) != 0)
			{
				fail();
			}

			return c;
		}

		template<typename Sink>
		bool put(Sink& dest, int c)
		{
			using iostreams::newline::CR;
			using iostreams::newline::LF;

			if (!open_) {
				open_ = true;
				source() = 0;
			}

			if (!iostreams::put(dest, c))
				return false;

			// Update source flags.
			source() &= ~f_line_complete;
			if ((source() & f_has_CR) != 0) {
				if (c == LF) {
					source() |= newline::dos;
					source() |= f_line_complete;
				}
				else {
					source() |= newline::mac;
				}
			}
			else if (c == LF) {
				source() |= newline::posix;
				source() |= f_line_complete;
			}
			source() = (source() & ~f_has_CR) | (c == CR ? f_has_CR : 0);

			// Check for errors.
			if ((target_ & newline::platform_mask) != 0 &&
				(source() & ~target_ & newline::platform_mask) != 0)
			{
				fail();
			}

			return true;
		}

		template<typename Sink>
		void close(Sink&, BOOST_IOS::openmode)
		{
			using iostreams::newline::final_newline;

			// Update final_newline flag.
			if ((source() & f_has_CR) != 0 ||
				(source() & f_line_complete) != 0)
			{
				source() |= final_newline;
			}

			// Clear non-sticky flags.
			source() &= ~(f_has_CR | f_line_complete);

			// Check for errors.
			if ((target_ & final_newline) != 0 &&
				(source() & final_newline) == 0)
			{
				fail();
			}
		}
	private:
		void fail() { throw std::runtime_error("boost newline error"); }
		int& source() { return flags_; }
		int source() const { return flags_; }

		enum flags {
			f_has_CR = 32768,
			f_line_complete = f_has_CR << 1
		};

		int   target_;  // Represents expected input.
		bool  open_;
	};
	BOOST_IOSTREAMS_PIPABLE(wnewline_checker, 0)

		class wnewline_filter {
		public:
			typedef wchar_t char_type;
			struct category
				: dual_use,
				filter_tag,
				closable_tag
			{ };

			explicit wnewline_filter(int target) : flags_(target)
			{
				if (target != iostreams::newline::posix &&
					target != iostreams::newline::dos &&
					target != iostreams::newline::mac)
				{
					boost::throw_exception(std::logic_error("bad flags"));
				}
			}

			template<typename Source>
			int get(Source& src)
			{
				using iostreams::newline::CR;
				using iostreams::newline::LF;

				BOOST_ASSERT((flags_ & f_write) == 0);
				flags_ |= f_read;

				if (flags_ & (f_has_LF | f_has_EOF)) {
					if (flags_ & f_has_LF)
						return newline();
					else
						return WEOF;
				}

				int c =
					(flags_ & f_has_CR) == 0 ?
					iostreams::get(src) :
					CR;

				if (c == WWOULD_BLOCK)
					return WWOULD_BLOCK;

				if (c == CR) {
					flags_ |= f_has_CR;

					int d;
					if ((d = iostreams::get(src)) == WWOULD_BLOCK)
						return WWOULD_BLOCK;

					if (d == LF) {
						flags_ &= ~f_has_CR;
						return newline();
					}

					if (d == WEOF) {
						flags_ |= f_has_EOF;
					}
					else {
						iostreams::putback(src, d);
					}

					flags_ &= ~f_has_CR;
					return newline();
				}

				if (c == LF)
					return newline();

				return c;
			}

			template<typename Sink>
			bool put(Sink& dest, char_type c)
			{
				using iostreams::newline::CR;
				using iostreams::newline::LF;

				BOOST_ASSERT((flags_ & f_read) == 0);
				flags_ |= f_write;

				if ((flags_ & f_has_LF) != 0)
					return c == LF ?
					newline(dest) :
					newline(dest) && this->put(dest, c);

				if (c == LF)
					return newline(dest);

				if ((flags_ & f_has_CR) != 0)
					return newline(dest) ?
					this->put(dest, c) :
					false;

				if (c == CR) {
					flags_ |= f_has_CR;
					return true;
				}

				return iostreams::put(dest, c);
			}

			template<typename Sink>
			void close(Sink& dest, BOOST_IOS::openmode)
			{
				if ((flags_ & f_write) != 0 && (flags_ & f_has_CR) != 0)
					newline_if_sink(dest);
				flags_ &= ~f_has_LF; // Restore original flags.
			}
		private:

			// Returns the appropriate element of a newline sequence.
			int newline()
			{
				using iostreams::newline::CR;
				using iostreams::newline::LF;

				switch (flags_ & iostreams::newline::platform_mask) {
				case iostreams::newline::posix:
					return LF;
				case iostreams::newline::mac:
					return CR;
				case iostreams::newline::dos:
					if (flags_ & f_has_LF) {
						flags_ &= ~f_has_LF;
						return LF;
					}
					else {
						flags_ |= f_has_LF;
						return CR;
					}
				}
				return BOOST_IOSTREAMS_ASSERT_UNREACHABLE(0);
			}

			// Writes a newline sequence.
			template<typename Sink>
			bool newline(Sink& dest)
			{
				using iostreams::newline::CR;
				using iostreams::newline::LF;

				bool success = false;
				switch (flags_ & iostreams::newline::platform_mask) {
				case iostreams::newline::posix:
					success = boost::iostreams::put(dest, LF);
					break;
				case iostreams::newline::mac:
					success = boost::iostreams::put(dest, CR);
					break;
				case iostreams::newline::dos:
					if ((flags_ & f_has_LF) != 0) {
						if ((success = boost::iostreams::put(dest, LF)))
							flags_ &= ~f_has_LF;
					}
					else if (boost::iostreams::put(dest, CR)) {
						if (!(success = boost::iostreams::put(dest, LF)))
							flags_ |= f_has_LF;
					}
					break;
				}
				if (success)
					flags_ &= ~f_has_CR;
				return success;
			}

			// Writes a newline sequence if the given device is a Sink.
			template<typename Device>
			void newline_if_sink(Device& dest)
			{
				typedef typename iostreams::category_of<Device>::type category;
				newline_if_sink(dest, is_convertible<category, output>());
			}

			template<typename Sink>
			void newline_if_sink(Sink& dest, mpl::true_) { newline(dest); }

			template<typename Source>
			void newline_if_sink(Source&, mpl::false_) { }

			enum flags {
				f_has_LF = 32768,
				f_has_CR = f_has_LF << 1,
				f_has_newline = f_has_CR << 1,
				f_has_EOF = f_has_newline << 1,
				f_read = f_has_EOF << 1,
				f_write = f_read << 1
			};
			int       flags_;
	};
	BOOST_IOSTREAMS_PIPABLE(wnewline_filter, 0)
}
}