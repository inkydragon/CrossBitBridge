#pragma once
#include <rpc.h>

// One synchronous call owns one binding. Only the Windows API receives its address.
class rpc_binding {
public:
    rpc_binding() = default;

    ~rpc_binding() {
        if (handle_)
            RpcBindingFree(&handle_);
    }

    rpc_binding(const rpc_binding&) = delete;
    rpc_binding& operator=(const rpc_binding&) = delete;
    // Borrows a null-terminated string for this call; no endpoint pointer is retained.
    RPC_STATUS connect(const wchar_t* endpoint);

    RPC_BINDING_HANDLE get() const noexcept {
        return handle_;
    }

private:
    RPC_BINDING_HANDLE handle_ = nullptr;
};

// Keep SEH in a function without C++ objects requiring unwinding (MSVC C2712).
// The caller owns the binding and callable outside this SEH frame.
// Fatal exceptions continue searching; only recoverable exceptions become status codes.
template <class Call> RPC_STATUS rpc_try(const Call& call) {
    RpcTryExcept {
        call();
    }
    RpcExcept(RpcExceptionFilter(RpcExceptionCode())) {
        return RpcExceptionCode();
    }
    RpcEndExcept
    return RPC_S_OK;
}

template <class Call> RPC_STATUS ncal_call(const wchar_t* endpoint, const Call& call) {
    rpc_binding binding;
    const auto status = binding.connect(endpoint);
    if (status != RPC_S_OK)
        return status;
    return rpc_try([&] { call(binding.get()); });
}
