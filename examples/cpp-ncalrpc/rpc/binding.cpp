#include "binding.h"
RPC_STATUS ncal_bind(const wchar_t* endpoint, RPC_BINDING_HANDLE* binding) {
    *binding = nullptr;
    RPC_WSTR text = nullptr;
    RPC_STATUS status = RpcStringBindingComposeW(nullptr,
        reinterpret_cast<RPC_WSTR>(const_cast<wchar_t*>(L"ncalrpc")), nullptr,
        reinterpret_cast<RPC_WSTR>(const_cast<wchar_t*>(endpoint)), nullptr, &text);
    if (status != RPC_S_OK) return status;
    status = RpcBindingFromStringBindingW(text, binding);
    RpcStringFreeW(&text);
    if (status != RPC_S_OK) return status;
    status = RpcBindingSetAuthInfoW(*binding, nullptr, RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                   RPC_C_AUTHN_WINNT, nullptr, RPC_C_AUTHZ_NONE);
    if (status == RPC_S_OK)
        status = RpcMgmtSetComTimeout(*binding, RPC_C_BINDING_MIN_TIMEOUT);
    if (status != RPC_S_OK) RpcBindingFree(binding);
    return status;
}
