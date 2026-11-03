#include "bridge.gen.h"
#include "binding.h"
#include "client.h"
static_assert(sizeof(void*) == 8, "The client must be x64");
uint32_t __cdecl ncal_ping(const wchar_t* endpoint, uint32_t* bits) {
    if (!endpoint || !bits) return ERROR_INVALID_PARAMETER;
    *bits = 0;
    return ncal_call(endpoint, [&](RPC_BINDING_HANDLE binding) {
        *bits = Wire_Ping(binding);
    });
}
uint32_t __cdecl ncal_add(const wchar_t* endpoint, int32_t a, int32_t b,
                          int32_t* result, int32_t* app_status) {
    if (!endpoint || !result || !app_status) return ERROR_INVALID_PARAMETER;
    *result = 0; *app_status = -1;
    long value = 0;
    return ncal_call(endpoint, [&](RPC_BINDING_HANDLE binding) {
        *app_status = Wire_Add(binding, a, b, &value);
        *result = value;
    });
}
uint32_t __cdecl ncal_scale(const wchar_t* endpoint, uint32_t count,
    const float* input, float factor, float* output, int32_t* app_status) {
    if (!endpoint || !app_status || count > 1048576 || (count && (!input || !output)))
        return ERROR_INVALID_PARAMETER;
    *app_status = -1;
    float empty_output = 0;
    return ncal_call(endpoint, [&](RPC_BINDING_HANDLE binding) {
        *app_status = Wire_Scale(binding, count, input, factor,
                                 count ? output : &empty_output);
    });
}
