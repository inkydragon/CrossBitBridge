#include "bridge.h"
#include "binding.h"
#include "client.h"
static_assert(sizeof(void*) == 8, "The client must be x64");
uint32_t __cdecl ncal_ping(const wchar_t* endpoint, uint32_t* bits) {
    if (!endpoint || !bits) return ERROR_INVALID_PARAMETER;
    *bits = 0;
    RPC_BINDING_HANDLE binding = nullptr;
    RPC_STATUS status = ncal_bind(endpoint, &binding);
    if (status) return status;
    RpcTryExcept { *bits = Wire_Ping(binding); }
    RpcExcept(1) { status = RpcExceptionCode(); }
    RpcEndExcept
    RpcBindingFree(&binding);
    return status;
}
uint32_t __cdecl ncal_add(const wchar_t* endpoint, int32_t a, int32_t b,
                          int32_t* result, int32_t* app_status) {
    if (!endpoint || !result || !app_status) return ERROR_INVALID_PARAMETER;
    *result = 0; *app_status = -1;
    RPC_BINDING_HANDLE binding = nullptr;
    RPC_STATUS status = ncal_bind(endpoint, &binding);
    if (status) return status;
    long value = 0;
    RpcTryExcept { *app_status = Wire_Add(binding, a, b, &value); *result = value; }
    RpcExcept(1) { status = RpcExceptionCode(); }
    RpcEndExcept
    RpcBindingFree(&binding);
    return status;
}
uint32_t __cdecl ncal_scale(const wchar_t* endpoint, uint32_t count,
    const float* input, float factor, float* output, int32_t* app_status) {
    if (!endpoint || !app_status || count > 1048576 || (count && (!input || !output)))
        return ERROR_INVALID_PARAMETER;
    *app_status = -1;
    RPC_BINDING_HANDLE binding = nullptr;
    RPC_STATUS status = ncal_bind(endpoint, &binding);
    if (status) return status;
    float empty_output = 0;
    RpcTryExcept { *app_status = Wire_Scale(binding, count, input, factor, count ? output : &empty_output); }
    RpcExcept(1) { status = RpcExceptionCode(); }
    RpcEndExcept
    RpcBindingFree(&binding);
    return status;
}
