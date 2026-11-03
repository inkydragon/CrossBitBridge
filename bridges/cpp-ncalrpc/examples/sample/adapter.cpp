#include "bridge.h"
#include "adapter.h"
#include "legacy.h"
#include <cstdio>
#include <mutex>
#include <algorithm>
#include <cwchar>
static_assert(sizeof(void*) == 4, "The host must be x86");
static HMODULE vendor = nullptr;
static decltype(&legacy_add) add_fn = nullptr;
static decltype(&legacy_scale) scale_fn = nullptr;
// RPC may dispatch concurrent calls; serialize entry to the legacy library.
static std::mutex call_mutex;
bool adapter_initialize(const wchar_t* path) {
    // Caller supplies an absolute path; dependencies may live alongside the DLL.
    if (!path || !((wcslen(path) >= 3 && path[1] == L':' &&
                    (path[2] == L'\\' || path[2] == L'/')) ||
                   (path[0] == L'\\' && path[1] == L'\\'))) {
        fwprintf(stderr, L"Vendor DLL path must be absolute\n"); return false;
    }
    vendor = LoadLibraryExW(path, nullptr,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (vendor) {
        add_fn = reinterpret_cast<decltype(add_fn)>(GetProcAddress(vendor, "legacy_add"));
        scale_fn = reinterpret_cast<decltype(scale_fn)>(GetProcAddress(vendor, "legacy_scale"));
    }
    if (!vendor || !add_fn || !scale_fn) {
        fwprintf(stderr, L"Cannot load DLL/exports: %lu\n", GetLastError());
        adapter_finalize(); return false;
    }
    return true;
}
void adapter_finalize() {
    add_fn = nullptr; scale_fn = nullptr;
    if (vendor) FreeLibrary(vendor);
    vendor = nullptr;
}
extern "C" unsigned long Impl_Ping(handle_t) { return sizeof(void*) * 8; }
extern "C" long Impl_Add(handle_t, long a, long b, long* result) {
    std::lock_guard<std::mutex> guard(call_mutex);
    *result = 0;
    int32_t value = 0;
    const auto status = add_fn(a, b, &value);
    *result = value;
    return status;
}
extern "C" long Impl_Scale(handle_t, unsigned long count, const float* input,
                           float factor, float* output) {
    std::lock_guard<std::mutex> guard(call_mutex);
    if (count && output) std::fill_n(output, count, 0.0f);
    if (count && (!input || !output)) return 2;
    return scale_fn(count, input, factor, output);
}
