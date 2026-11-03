#include "binding.h"

RPC_STATUS rpc_binding::connect(const wchar_t* endpoint) {
    if (!endpoint || !*endpoint || handle_)
        return ERROR_INVALID_PARAMETER;
    // RPC's string parameters are mutable in the API, but are not modified.
    wchar_t protocol[] = L"ncalrpc";
    RPC_WSTR text = nullptr;
    auto status = RpcStringBindingComposeW(
        nullptr, reinterpret_cast<RPC_WSTR>(protocol), nullptr,
        reinterpret_cast<RPC_WSTR>(const_cast<wchar_t*>(endpoint)), nullptr, &text);
    if (status != RPC_S_OK)
        return status;
    status = RpcBindingFromStringBindingW(text, &handle_);
    RpcStringFreeW(&text);
    if (status != RPC_S_OK)
        return status;
    status = RpcBindingSetAuthInfoW(handle_, nullptr, RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                    RPC_C_AUTHN_WINNT, nullptr, RPC_C_AUTHZ_NONE);
    if (status == RPC_S_OK)
        status = RpcMgmtSetComTimeout(handle_, RPC_C_BINDING_MIN_TIMEOUT);
    return status;
}
