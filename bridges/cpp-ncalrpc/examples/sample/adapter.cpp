#include "bridge.gen.h"
#include "adapter.h"
#include "legacy.h"
#include <cstdio>
#include <mutex>
#include <algorithm>
#include <filesystem>
#include <span>
#include <utility>
#include "resources.h"
static_assert(sizeof(void*) == 4, "The host must be x86");
static unique_library vendor;
static decltype(&legacy_add) add_fn = nullptr;
static decltype(&legacy_scale) scale_fn = nullptr;
// RPC may dispatch concurrent calls; serialize entry to the legacy library.
static std::mutex call_mutex;
bool adapter_initialize(const std::filesystem::path& path) {
    // Caller supplies an absolute path; dependencies may live alongside the DLL.
    if (!path.is_absolute()) {
        fwprintf(stderr, L"Vendor DLL path must be absolute\n"); return false;
    }
    unique_library module(LoadLibraryExW(path.c_str(), nullptr,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS));
    if (!module) {
        fwprintf(stderr, L"Cannot load DLL: %lu\n", GetLastError());
        return false;
    }
    const auto add = reinterpret_cast<decltype(add_fn)>(GetProcAddress(module.get(), "legacy_add"));
    // Preserve the first failed lookup's error code.
    const auto scale = add
        ? reinterpret_cast<decltype(scale_fn)>(GetProcAddress(module.get(), "legacy_scale"))
        : nullptr;
    if (!add || !scale) {
        fwprintf(stderr, L"Cannot load exports: %lu\n", GetLastError());
        return false;
    }
    vendor = std::move(module);
    add_fn = add;
    scale_fn = scale;
    return true;
}
void adapter_finalize() {
    add_fn = nullptr; scale_fn = nullptr;
    vendor.reset();
}
extern "C" unsigned long Impl_Ping(handle_t) { return sizeof(void*) * 8; }
extern "C" long Impl_Add(handle_t, long a, long b, long* result) {
    std::lock_guard guard(call_mutex);
    *result = 0;
    int32_t value = 0;
    const auto status = add_fn(a, b, &value);
    *result = value;
    return status;
}
extern "C" long Impl_Scale(handle_t, unsigned long count, const float* input,
                           float factor, float* output) {
    std::lock_guard guard(call_mutex);
    if (count && output) std::ranges::fill(std::span(output, count), 0.0f);
    if (count && (!input || !output)) return 2;
    return scale_fn(count, input, factor, output);
}
