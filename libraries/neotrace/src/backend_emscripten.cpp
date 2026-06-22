#include "neotrace_backend.h"

#if NEOTRACE_CAPTURE_EMSCRIPTEN

// emscripten 5.x removed the per-frame introspection apis (emscripten_return_address,
// emscripten_pc_get_*), leaving emscripten_get_callstack's pre-symbolized text as the
// only primitive, so capture and symbolize live in one backend: capture parses real
// wasm code offsets out of the callstack text (still meaningful for offline tools) and
// interns each frame's name and module into an offset-keyed registry that symbolize
// reads back later. the registry is bounded by distinct call sites, not by runtime.
// the price of the text round trip: capture allocates here, unlike every other backend.

#include <neotrace/neotrace.h>

#include <emscripten/emscripten.h>

#include <charconv>
#include <map>
#include <mutex>
#include <string_view>

namespace neotrace
{

namespace
{

struct frame_record
{
	std::string symbol; // usually pre-demangled by emscripten's name section, sometimes raw mangled
	std::string module;
};

std::mutex registry_mutex;
std::map<uintptr_t, frame_record> registry; // wasm code offset -> parsed frame info
char callstack_buffer[16384];               // guarded by registry_mutex

// node/v8 wasm frames look like
//   "    at demo.wasm.demo::middle(int) (wasm://wasm/demo.wasm-000100c6:wasm-function[67]:0x2af1)"
// or, with no name section, "    at wasm://wasm/demo.wasm-000100c6:wasm-function[67]:0x2af1";
// js-only frames carry no wasm-function chunk and are not c++ frames at all
bool parse_wasm_line(std::string_view line, uintptr_t &offset, std::string_view &name, std::string_view &module)
{
	if (line.find("wasm-function[") == std::string_view::npos)
		return false;

	const auto hex_pos = line.rfind(":0x");
	if (hex_pos == std::string_view::npos)
		return false;
	const char *hex_begin = line.data() + hex_pos + 3;
	const auto [parse_end, ec] = std::from_chars(hex_begin, line.data() + line.size(), offset, 16);
	if (ec != std::errc{} || parse_end == hex_begin)
		return false;

	const auto at_pos = line.find("at ");
	const auto location_pos = line.find(" (wasm://");
	if (at_pos != std::string_view::npos && location_pos != std::string_view::npos && location_pos > at_pos + 3)
	{
		name = line.substr(at_pos + 3, location_pos - (at_pos + 3));
		// v8 prefixes the name section entry with "<module>." ; strip it from the symbol
		if (const auto wasm_pos = name.find(".wasm."); wasm_pos != std::string_view::npos)
			name.remove_prefix(wasm_pos + 6);
	}

	const auto url_pos = line.find("wasm://wasm/");
	if (url_pos != std::string_view::npos)
	{
		module = line.substr(url_pos + 12);
		if (const auto colon = module.find(':'); colon != std::string_view::npos)
			module = module.substr(0, colon);
		if (const auto dash = module.rfind('-'); dash != std::string_view::npos)
			module = module.substr(0, dash); // drop v8's content-hash suffix
		if (module.find(".wasm") == std::string_view::npos)
			module = {}; // nameless modules show only v8's content hash; not a module name
	}

	return true;
}

} // namespace

NEOTRACE_NOINLINE size_t capture(std::span<void *> out, size_t skip) noexcept
{
	if (out.empty())
		return 0;

	const std::scoped_lock lock(registry_mutex);
	// v8 caps Error().stack at 10 frames by default, which the engine-side helpers and
	// our own two frames already half-consume; raise it before every walk
	EM_ASM({ Error.stackTraceLimit = $0; }, static_cast<int>(out.size() + 16));
	emscripten_get_callstack(EM_LOG_JS_STACK, callstack_buffer, sizeof(callstack_buffer));

	std::string_view stack(callstack_buffer);
	size_t to_drop = skip + 1; // the first wasm frame is capture() itself
	size_t count = 0;
	while (!stack.empty() && count < out.size())
	{
		const auto newline = stack.find('\n');
		const std::string_view line = stack.substr(0, newline);
		stack.remove_prefix(newline == std::string_view::npos ? stack.size() : newline + 1);

		uintptr_t offset = 0;
		std::string_view name;
		std::string_view module;
		if (!parse_wasm_line(line, offset, name, module))
			continue;
		if (to_drop > 0)
		{
			--to_drop;
			continue;
		}
		out[count++] = reinterpret_cast<void *>(offset);
		registry.try_emplace(offset, frame_record{std::string(name), std::string(module)});
	}
	return count;
}

frame_info symbolize(void *address) noexcept
{
	frame_info info;
	info.address = address;

	const std::scoped_lock lock(registry_mutex);
	const auto it = registry.find(reinterpret_cast<uintptr_t>(address));
	if (it == registry.end())
		return info;

	info.module = it->second.module;
	const std::string &raw = it->second.symbol;
	// emscripten usually pre-demangles the wasm name section; older toolchains leave it mangled
	info.symbol = (raw.size() >= 2 && raw[0] == '_' && raw[1] == 'Z') ? demangle(raw.c_str()) : raw;
	return info;
}

} // namespace neotrace

#endif // NEOTRACE_CAPTURE_EMSCRIPTEN
