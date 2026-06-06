// dladdr and Dl_info are guarded by _GNU_SOURCE on glibc; define it before any libc
// header gets pulled in
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "neotrace_backend.h"

#if NEOTRACE_SYMBOLIZE_POSIX

#include <neotrace/neotrace.h>

#include <cxxabi.h>
#include <dlfcn.h>

#include <cstdlib>

namespace neotrace
{

namespace
{

void assign_demangled(frame_info &info, const char *mangled) noexcept
{
	int status = 0;
	char *demangled = abi::__cxa_demangle(mangled, nullptr, nullptr, &status);
	info.symbol = (status == 0 && demangled != nullptr) ? demangled : mangled; // non-c++ names don't demangle; keep them raw
	std::free(demangled);
}

} // namespace

frame_info symbolize(void *address) noexcept
{
	frame_info info;
	info.address = address;

	Dl_info dl{};
	if (dladdr(address, &dl) != 0)
	{
		if (dl.dli_fname != nullptr)
		{
			info.module = dl.dli_fname;
			if (dl.dli_fbase != nullptr)
				info.offset = reinterpret_cast<uintptr_t>(address) - reinterpret_cast<uintptr_t>(dl.dli_fbase);
		}

		if (dl.dli_sname != nullptr)
		{
			assign_demangled(info, dl.dli_sname);
			if (dl.dli_saddr != nullptr)
				info.offset = reinterpret_cast<uintptr_t>(address) - reinterpret_cast<uintptr_t>(dl.dli_saddr);
		}
	}

#if NEOTRACE_SYMBOLIZE_ELF_SYMTAB
	// prefer the .symtab answer whenever one exists: dladdr only sees .dynsym, which
	// misses local symbols entirely and misattributes them to the nearest preceding
	// exported symbol when one is close enough
	std::string symtab_name;
	uintptr_t symtab_offset = 0;
	if (detail::elf_symtab_lookup(address, symtab_name, symtab_offset))
	{
		assign_demangled(info, symtab_name.c_str());
		info.offset = symtab_offset;
	}
#endif

	return info;
}

} // namespace neotrace

#endif // NEOTRACE_SYMBOLIZE_POSIX
