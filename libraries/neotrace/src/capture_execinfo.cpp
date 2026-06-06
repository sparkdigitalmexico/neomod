#include "neotrace_backend.h"

#if NEOTRACE_CAPTURE_EXECINFO

#include <neotrace/neotrace.h>

#include <execinfo.h>

#include <algorithm>

namespace neotrace
{

// darwin mandates frame pointers and backtrace() walks them, so it works regardless of
// unwind tables; apple clang emits none under -fno-exceptions (observed with clang 21
// on arm64), which makes _Unwind_Backtrace return nothing there.
NEOTRACE_NOINLINE size_t capture(std::span<void *> out, size_t skip) noexcept
{
	constexpr size_t raw_cap = 256;
	void *raw[raw_cap];

	const size_t internal = 1 + skip; // frame 0 is capture() itself
	const size_t want = std::min(raw_cap, out.size() + internal);
	const size_t got = static_cast<size_t>(::backtrace(raw, static_cast<int>(want)));

	size_t count = 0;
	for (size_t i = internal; i < got && count < out.size(); ++i)
	{
		// entries are return addresses pointing after their call; nudge them back inside
		// the call instruction so symbolization cannot slide into the next function
		out[count++] = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(raw[i]) - 1); // NOLINT(performance-no-int-to-ptr)
	}
	return count;
}

} // namespace neotrace

#endif // NEOTRACE_CAPTURE_EXECINFO
