#pragma once

// internal backend selection; not part of the public api. exactly one capture and one
// symbolize backend is chosen per platform, and each backend .cpp is wrapped in a
// whole-file #if on its macro, so consumers compile all of src/ on every platform and
// unselected sources compile to nothing. platforms with no match degrade to the
// fallbacks in neotrace.cpp.

// capture. darwin uses backtrace() instead of _Unwind_Backtrace because apple clang
// emits no unwind tables under -fno-exceptions, which leaves the unwinder with nothing
// to walk; backtrace() follows frame pointers, which the darwin abi mandates.
// emscripten gets one combined backend (backend_emscripten.cpp): the platform only
// offers pre-symbolized callstack text, so capture and symbolize share parsed state.
#if defined(_WIN32)
#define NEOTRACE_CAPTURE_WINDOWS 1
#elif defined(__EMSCRIPTEN__) && __has_include(<emscripten/emscripten.h>)
#define NEOTRACE_CAPTURE_EMSCRIPTEN 1
#elif defined(__EMSCRIPTEN__)
#define NEOTRACE_CAPTURE_NONE 1
#elif defined(__APPLE__) && __has_include(<execinfo.h>)
#define NEOTRACE_CAPTURE_EXECINFO 1
#elif __has_include(<unwind.h>)
#define NEOTRACE_CAPTURE_UNWIND 1
#else
#define NEOTRACE_CAPTURE_NONE 1
#endif

// symbolize
#if defined(_WIN32) && __has_include(<dbghelp.h>)
#define NEOTRACE_SYMBOLIZE_DBGHELP 1
#elif NEOTRACE_CAPTURE_EMSCRIPTEN
#define NEOTRACE_SYMBOLIZE_EMSCRIPTEN 1
#elif !defined(_WIN32) && !defined(__EMSCRIPTEN__) && __has_include(<dlfcn.h>) && __has_include(<cxxabi.h>)
#define NEOTRACE_SYMBOLIZE_POSIX 1
#else
#define NEOTRACE_SYMBOLIZE_NONE 1
#endif

// linux extra: .symtab read off the binary sees local symbols .dynsym lacks, so names
// work without -rdynamic; implemented in symbolize_elf_symtab.cpp, used by the posix
// symbolizer as an override
#if NEOTRACE_SYMBOLIZE_POSIX && defined(__linux__) && __has_include(<link.h>) && __has_include(<elf.h>)
#define NEOTRACE_SYMBOLIZE_ELF_SYMTAB 1

#include <cstdint>
#include <string>

namespace neotrace::detail
{
// fills name (mangled) and offset when address falls inside a .symtab function symbol
bool elf_symtab_lookup(const void *address, std::string &name, uintptr_t &offset) noexcept;
} // namespace neotrace::detail
#endif

// capture's skip accounting assumes capture() and trace::current() keep their own frames
#if defined(_MSC_VER) && !defined(__clang__)
#define NEOTRACE_NOINLINE __declspec(noinline)
#else
#define NEOTRACE_NOINLINE [[gnu::noinline]]
#endif
