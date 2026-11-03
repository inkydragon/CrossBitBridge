#include <windows.h>
#include <rpc.h>
#include <sddl.h>
#include <cstdio>
#include <string>
#include <vector>
#include <exception>
#include "bridge.gen.h"
#include "adapter.h"
#include "resources.h"

static void report_security_error(const wchar_t* operation, DWORD error = GetLastError()) {
    fwprintf(stderr, L"Security setup: %ls failed: %lu\n", operation, error);
}

// Restrict this interface to the host's user. A random endpoint is not an ACL.
static unique_local<void> current_user_descriptor() {
    HANDLE token = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        report_security_error(L"OpenProcessToken");
        return nullptr;
    }
    unique_token token_owner(token);
    DWORD bytes = 0;
    const auto queried = GetTokenInformation(token, TokenUser, nullptr, 0, &bytes);
    const auto error = queried ? ERROR_INVALID_DATA : GetLastError();
    if (error != ERROR_INSUFFICIENT_BUFFER || bytes == 0) {
        report_security_error(L"GetTokenInformation(size)",
                              error == ERROR_INSUFFICIENT_BUFFER ? ERROR_INVALID_DATA : error);
        return nullptr;
    }
    std::vector<unsigned char> storage(bytes);
    if (!GetTokenInformation(token, TokenUser, storage.data(), bytes, &bytes)) {
        report_security_error(L"GetTokenInformation(TokenUser)");
        return nullptr;
    }
    LPWSTR sid = nullptr;
    if (!ConvertSidToStringSidW(reinterpret_cast<TOKEN_USER*>(storage.data())->User.Sid, &sid)) {
        report_security_error(L"ConvertSidToStringSidW");
        return nullptr;
    }
    unique_local<wchar_t> sid_owner(sid);
    std::wstring sddl = L"D:P(A;;GA;;;";
    sddl += sid_owner.get();
    sddl += L")";
    PSECURITY_DESCRIPTOR result = nullptr;
    if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(
            sddl.c_str(), SDDL_REVISION_1, &result, nullptr)) {
        report_security_error(L"ConvertStringSecurityDescriptorToSecurityDescriptorW");
        return nullptr;
    }
    return unique_local<void>(result);
}
int wmain(int argc, wchar_t** argv) try {
    if (argc != 3) {
        fwprintf(stderr, L"Usage: ncal_host.exe <endpoint> <absolute-vendor-dll>\n");
        return 2;
    }
    const std::filesystem::path dll_path(argv[2]);
    if (!adapter_initialize(dll_path)) return 3;
    struct adapter_lifetime { ~adapter_lifetime() { adapter_finalize(); } } adapter;
    auto sd = current_user_descriptor();
    if (!sd) return 4;
    wchar_t protocol[] = L"ncalrpc";
    RPC_STATUS status = RpcServerUseProtseqEpW(
        reinterpret_cast<RPC_WSTR>(protocol),
        RPC_C_PROTSEQ_MAX_REQS_DEFAULT, reinterpret_cast<RPC_WSTR>(argv[1]), nullptr);
    if (status == RPC_S_OK)
        status = RpcServerRegisterAuthInfoW(nullptr, RPC_C_AUTHN_WINNT, nullptr, nullptr);
    if (status == RPC_S_OK)
        status = RpcServerRegisterIf3(NCAL_SERVER_INTERFACE, nullptr, nullptr,
            RPC_IF_ALLOW_LOCAL_ONLY | RPC_IF_ALLOW_SECURE_ONLY,
            RPC_C_LISTEN_MAX_CALLS_DEFAULT, 16 * 1024 * 1024, nullptr, sd.get());
    if (status == RPC_S_OK) {
        wprintf(L"READY endpoint=%ls bits=%zu\n", argv[1], sizeof(void*) * 8);
        fflush(stdout);
        status = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, FALSE);
        RpcServerUnregisterIf(NCAL_SERVER_INTERFACE, nullptr, TRUE);
    }
    if (status != RPC_S_OK) fwprintf(stderr, L"RPC server error: %lu\n", status);
    return status == RPC_S_OK ? 0 : 5;
}
catch (const std::exception& error) {
    fprintf(stderr, "Host initialization failed: %s\n", error.what());
    return 5;
}
