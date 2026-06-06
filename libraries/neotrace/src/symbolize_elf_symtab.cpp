// dl_iterate_phdr on glibc is guarded by _GNU_SOURCE; define it before any libc header
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "neotrace_backend.h"

#if NEOTRACE_SYMBOLIZE_ELF_SYMTAB

#include <elf.h>
#include <fcntl.h>
#include <link.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <map>
#include <mutex>
#include <vector>

namespace neotrace::detail
{

namespace
{

struct func_sym
{
	uintptr_t value; // file-relative; add the module load bias for the runtime address
	uintptr_t size;
	std::string name; // mangled
};

struct module_syms
{
	std::vector<func_sym> funcs; // sorted by value; empty when the module has no usable .symtab
};

bool read_at(int fd, off_t pos, void *dst, size_t len) noexcept
{
	return pread(fd, dst, len, pos) == static_cast<ssize_t>(len);
}

module_syms parse_symtab(const char *path) noexcept
{
	// .symtab is not mapped at runtime (only .dynsym is), so it has to come off the disk
	module_syms result;
	constexpr uintptr_t sane_section_size = uintptr_t{1} << 30;

	const int fd = open(path, O_RDONLY | O_CLOEXEC);
	if (fd < 0)
		return result;

	ElfW(Ehdr) ehdr;
	bool ok = read_at(fd, 0, &ehdr, sizeof(ehdr)) && std::memcmp(ehdr.e_ident, ELFMAG, SELFMAG) == 0 &&
	          ehdr.e_ident[EI_CLASS] == (sizeof(void *) == 8 ? ELFCLASS64 : ELFCLASS32) && ehdr.e_shentsize == sizeof(ElfW(Shdr)) && ehdr.e_shnum > 0;

	std::vector<ElfW(Shdr)> shdrs;
	if (ok)
	{
		shdrs.resize(ehdr.e_shnum);
		ok = read_at(fd, static_cast<off_t>(ehdr.e_shoff), shdrs.data(), shdrs.size() * sizeof(ElfW(Shdr)));
	}

	const ElfW(Shdr) *symtab = nullptr;
	if (ok)
	{
		for (const auto &sh : shdrs)
		{
			if (sh.sh_type == SHT_SYMTAB)
			{
				symtab = &sh;
				break;
			}
		}
	}

	std::vector<ElfW(Sym)> syms;
	std::vector<char> strtab;
	if (symtab != nullptr && symtab->sh_entsize == sizeof(ElfW(Sym)) && symtab->sh_link < shdrs.size())
	{
		const ElfW(Shdr) &str = shdrs[symtab->sh_link];
		if (symtab->sh_size < sane_section_size && str.sh_size < sane_section_size)
		{
			syms.resize(symtab->sh_size / sizeof(ElfW(Sym)));
			strtab.resize(str.sh_size);
			if (!read_at(fd, static_cast<off_t>(symtab->sh_offset), syms.data(), syms.size() * sizeof(ElfW(Sym))) ||
			    !read_at(fd, static_cast<off_t>(str.sh_offset), strtab.data(), strtab.size()))
			{
				syms.clear();
			}
		}
	}
	close(fd);

	for (const auto &sym : syms)
	{
		if ((sym.st_info & 0xf) != STT_FUNC || sym.st_shndx == SHN_UNDEF || sym.st_value == 0)
			continue;
		if (sym.st_name >= strtab.size() || strtab[sym.st_name] == '\0')
			continue;
		result.funcs.push_back({static_cast<uintptr_t>(sym.st_value), static_cast<uintptr_t>(sym.st_size), std::string(&strtab[sym.st_name])});
	}
	std::sort(result.funcs.begin(), result.funcs.end(), [](const func_sym &a, const func_sym &b) { return a.value < b.value; });
	return result;
}

struct find_module_state
{
	uintptr_t address;
	uintptr_t bias;
	const char *name;
	bool found;
};

int on_module(dl_phdr_info *info, size_t, void *arg) noexcept
{
	auto &state = *static_cast<find_module_state *>(arg);
	for (ElfW(Half) i = 0; i < info->dlpi_phnum; ++i)
	{
		const auto &ph = info->dlpi_phdr[i];
		const uintptr_t lo = info->dlpi_addr + ph.p_vaddr;
		if (ph.p_type == PT_LOAD && state.address >= lo && state.address < lo + ph.p_memsz)
		{
			state.bias = info->dlpi_addr;
			state.name = info->dlpi_name;
			state.found = true;
			return 1;
		}
	}
	return 0;
}

} // namespace

bool elf_symtab_lookup(const void *address, std::string &name, uintptr_t &offset) noexcept
{
	find_module_state state{reinterpret_cast<uintptr_t>(address), 0, nullptr, false};
	dl_iterate_phdr(&on_module, &state);
	if (!state.found)
		return false;

	static std::mutex mutex;
	static std::map<uintptr_t, module_syms> cache; // keyed by load bias
	const std::scoped_lock lock(mutex);

	auto it = cache.find(state.bias);
	if (it == cache.end())
	{
		// the main executable's dlpi_name is empty (glibc); /proc/self/exe also dodges
		// relative-path trouble when the working directory changed after exec
		const char *path = (state.name != nullptr && state.name[0] != '\0') ? state.name : "/proc/self/exe";
		it = cache.emplace(state.bias, parse_symtab(path)).first;
	}

	const auto &funcs = it->second.funcs;
	if (funcs.empty())
		return false;

	const uintptr_t target = reinterpret_cast<uintptr_t>(address) - state.bias;
	auto next = std::upper_bound(funcs.begin(), funcs.end(), target, [](uintptr_t t, const func_sym &s) { return t < s.value; });
	if (next == funcs.begin())
		return false;
	const func_sym &sym = *std::prev(next);
	if (sym.size != 0 && target >= sym.value + sym.size)
		return false;
	if (sym.size == 0 && target - sym.value > (uintptr_t{1} << 20)) // unsized symbols: arbitrary sanity cap
		return false;

	name = sym.name;
	offset = target - sym.value;
	return true;
}

} // namespace neotrace::detail

#endif // NEOTRACE_SYMBOLIZE_ELF_SYMTAB
