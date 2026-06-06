#include "neotrace_backend.h"

#if NEOTRACE_CAPTURE_UNWIND

#include <neotrace/neotrace.h>

#include <unwind.h>

namespace neotrace
{

namespace
{

struct unwind_state
{
	std::span<void *> out;
	size_t skip;
	size_t count;
};

_Unwind_Reason_Code on_frame(_Unwind_Context *ctx, void *arg) noexcept
{
	auto &state = *static_cast<unwind_state *>(arg);

	int ip_before_insn = 0;
	const uintptr_t ip = _Unwind_GetIPInfo(ctx, &ip_before_insn);
	if (ip == 0)
		return _URC_END_OF_STACK;

	if (state.skip > 0)
	{
		--state.skip;
		return _URC_NO_REASON;
	}
	if (state.count == state.out.size())
		return _URC_END_OF_STACK;

	// return addresses point after their call; nudge them back inside the call
	// instruction so symbolization cannot slide into the next function or line
	state.out[state.count++] = reinterpret_cast<void *>(ip_before_insn ? ip : ip - 1); // NOLINT(performance-no-int-to-ptr)
	return _URC_NO_REASON;
}

} // namespace

NEOTRACE_NOINLINE size_t capture(std::span<void *> out, size_t skip) noexcept
{
	unwind_state state{out, skip + 1, 0}; // + 1 drops capture() itself, the first frame reported
	_Unwind_Backtrace(&on_frame, &state);
	return state.count;
}

} // namespace neotrace

#endif // NEOTRACE_CAPTURE_UNWIND
