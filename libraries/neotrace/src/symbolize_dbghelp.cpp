#include "neotrace_backend.h"

#if NEOTRACE_SYMBOLIZE_DBGHELP

#include <neotrace/neotrace.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <dbghelp.h> // requires windows.h first

#include <mutex>

namespace neotrace
{

namespace
{

// the uintptr_t hop dodges gcc's -Wcast-function-type on FARPROC conversions
template <typename Fn>
Fn proc_cast(FARPROC proc) noexcept
{
	return reinterpret_cast<Fn>(reinterpret_cast<uintptr_t>(proc));
}

using sym_set_options_fn = DWORD(WINAPI *)(DWORD);
using sym_initialize_fn = BOOL(WINAPI *)(HANDLE, PCWSTR, BOOL);
using sym_from_addr_fn = BOOL(WINAPI *)(HANDLE, DWORD64, PDWORD64, PSYMBOL_INFO);
using sym_get_line_fn = BOOL(WINAPI *)(HANDLE, DWORD64, PDWORD, PIMAGEHLP_LINEW64);

struct dbghelp_api
{
	sym_from_addr_fn sym_from_addr = nullptr;
	sym_get_line_fn sym_get_line_from_addr = nullptr;
	bool ready = false;
};

// dbghelp is not thread-safe in any part, so one mutex serializes everything,
// including the one-time load + SymInitialize below
std::mutex dbghelp_mutex;

// call with dbghelp_mutex held
const dbghelp_api &dbghelp() noexcept
{
	static const dbghelp_api api = [] {
		dbghelp_api a;
		// loaded at runtime from system32 so no import library is needed and a broken
		// installation degrades instead of failing the build or the process start
		const HMODULE lib = LoadLibraryExW(L"dbghelp.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
		if (lib == nullptr)
			return a;
		const auto set_options = proc_cast<sym_set_options_fn>(GetProcAddress(lib, "SymSetOptions"));
		const auto initialize = proc_cast<sym_initialize_fn>(GetProcAddress(lib, "SymInitializeW"));
		a.sym_from_addr = proc_cast<sym_from_addr_fn>(GetProcAddress(lib, "SymFromAddr"));
		a.sym_get_line_from_addr = proc_cast<sym_get_line_fn>(GetProcAddress(lib, "SymGetLineFromAddrW64"));
		if (set_options == nullptr || initialize == nullptr || a.sym_from_addr == nullptr)
			return a;

		set_options(SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_LOAD_LINES | SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_NO_PROMPTS);
		if (initialize(GetCurrentProcess(), nullptr, TRUE) != 0)
			a.ready = true;
		else
			a.ready = GetLastError() == ERROR_INVALID_PARAMETER; // the consumer already initialized dbghelp; its session works for us too
		return a;
	}();
	return api;
}

void assign_utf8(std::string &out, const wchar_t *wide) noexcept
{
	const int needed = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
	if (needed <= 1)
		return;
	out.assign(static_cast<size_t>(needed), '\0');
	WideCharToMultiByte(CP_UTF8, 0, wide, -1, out.data(), needed, nullptr, nullptr);
	out.pop_back(); // drop the converted terminator
}

void assign_symbol(frame_info &info, const char *name) noexcept
{
	// mingw and clang-built pdbs carry raw itanium names dbghelp's UNDNAME cannot decode;
	// msvc names arrive already undecorated, so only itanium-mangled names need demangling
	info.symbol = (name[0] == '_' && name[1] == 'Z') ? demangle(name) : name;
}

} // namespace

frame_info symbolize(void *address) noexcept
{
	frame_info info;
	info.address = address;

	// module via the loader rather than dbghelp, so module+offset survives a total
	// absence of symbol information
	HMODULE module = nullptr;
	if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, static_cast<LPCWSTR>(address), &module) != 0 &&
	    module != nullptr)
	{
		wchar_t path[MAX_PATH];
		const DWORD len = GetModuleFileNameW(module, path, MAX_PATH);
		if (len > 0 && len < MAX_PATH)
			assign_utf8(info.module, path);
		info.offset = reinterpret_cast<uintptr_t>(address) - reinterpret_cast<uintptr_t>(module);
	}

	const std::scoped_lock lock(dbghelp_mutex);
	const dbghelp_api &api = dbghelp();
	if (!api.ready)
		return info;

	const HANDLE process = GetCurrentProcess();
	const DWORD64 addr64 = static_cast<DWORD64>(reinterpret_cast<uintptr_t>(address));

	alignas(SYMBOL_INFO) char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME] = {};
	auto *sym = reinterpret_cast<SYMBOL_INFO *>(buffer);
	sym->SizeOfStruct = sizeof(SYMBOL_INFO);
	sym->MaxNameLen = MAX_SYM_NAME;
	DWORD64 displacement = 0;
	if (api.sym_from_addr(process, addr64, &displacement, sym) != 0 && sym->Name[0] != '\0')
	{
		assign_symbol(info, sym->Name);
		info.offset = static_cast<uintptr_t>(displacement);
	}

	if (api.sym_get_line_from_addr != nullptr)
	{
		IMAGEHLP_LINEW64 line{};
		line.SizeOfStruct = sizeof(line);
		DWORD line_displacement = 0;
		if (api.sym_get_line_from_addr(process, addr64, &line_displacement, &line) != 0 && line.FileName != nullptr)
		{
			assign_utf8(info.file, line.FileName);
			info.line = line.LineNumber;
		}
	}

	return info;
}

} // namespace neotrace

#endif // NEOTRACE_SYMBOLIZE_DBGHELP
