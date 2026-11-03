#pragma once
#include <windows.h>
#include <memory>
#include <type_traits>

struct close_handle {
    void operator()(void* value) const noexcept { CloseHandle(value); }
};
struct free_local {
    void operator()(void* value) const noexcept { LocalFree(value); }
};
struct free_library {
    void operator()(HMODULE value) const noexcept { FreeLibrary(value); }
};
// Token handles use nullptr as their invalid value (not INVALID_HANDLE_VALUE).
using unique_token = std::unique_ptr<void, close_handle>;
template<class T>
using unique_local = std::unique_ptr<T, free_local>;
using unique_library = std::unique_ptr<std::remove_pointer_t<HMODULE>, free_library>;
