#include "neotrace_backend.h"

#if NEOTRACE_CAPTURE_WINDOWS

#include <neotrace/neotrace.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace neotrace
{

namespace
{

using rtl_capture_fn = USHORT(WINAPI *)(ULONG frames_to_skip, ULONG frames_to_capture, PVOID *backtrace, PULONG hash);

// the uintptr_t hop dodges gcc's -Wcast-function-type on FARPROC conversions
rtl_capture_fn resolve_rtl_capture() noexcept
{
	const HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
	if (ntdll == nullptr)
		return nullptr;
	return reinterpret_cast<rtl_capture_fn>(reinterpret_cast<uintptr_t>(GetProcAddress(ntdll, "RtlCaptureStackBackTrace")));
}

} // namespace

NEOTRACE_NOINLINE size_t capture(std::span<void *> out, size_t skip) noexcept
{
	// resolved at runtime from the always-loaded ntdll so no import library is needed
	static const rtl_capture_fn rtl_capture = resolve_rtl_capture();
	if (rtl_capture == nullptr || out.empty())
		return 0;

	const ULONG want = static_cast<ULONG>(out.size() < 0xffff ? out.size() : 0xffff);
	const USHORT got = rtl_capture(static_cast<ULONG>(skip + 1), want, out.data(), nullptr); // + 1 drops capture() itself

	for (USHORT i = 0; i < got; ++i)
	{
		// entries are return addresses pointing after their call; nudge them back inside
		// the call instruction so symbolization cannot slide into the next function/line
		out[i] = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(out[i]) - 1);
	}
	return got;
}

} // namespace neotrace

#endif // NEOTRACE_CAPTURE_WINDOWS
