# Inline fixes against harmonytf/discord-rpc, applied via `cmake -DSRC=<src-dir> -P`.
# Used by both build systems to avoid depending on `patch` (esp. on Windows)

if(NOT DEFINED SRC)
    message(FATAL_ERROR "discord-rpc-fixes.cmake: SRC must be set to the discord-rpc source dir")
endif()

set(_register "${SRC}/src/discord_register_win.cpp")
file(READ "${_register}" _content)
string(REPLACE "#undefine RegSetKeyValueW" "#undef RegSetKeyValueW" _content "${_content}")
file(WRITE "${_register}" "${_content}")

set(_cmakelists "${SRC}/src/CMakeLists.txt")
file(READ "${_cmakelists}" _content)
string(REPLACE
    "target_compile_options(discord-rpc PRIVATE /EHsc"
    "target_compile_options(discord-rpc PRIVATE /EHs-c-"
    _content "${_content}")
file(WRITE "${_cmakelists}" "${_content}")
