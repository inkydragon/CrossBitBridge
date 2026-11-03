#include <windows.h>
#include <rpc.h>
#include <sddl.h>
#include <cstdio>
#include <string>
#include <vector>
#include "bridge.h"
#include "adapter.h"

// Restrict this interface to the host's user. A random endpoint is not an ACL.
static PSECURITY_DESCRIPTOR current_user_descriptor() {
    HANDLE token = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) return nullptr;
    DWORD bytes = 0;
    GetTokenInformation(token, TokenUser, nullptr, 0, &bytes);
    std::vector<unsigned char> storage(bytes);
    if (!GetTokenInformation(token, TokenUser, storage.data(), bytes, &bytes)) {
        CloseHandle(token); return nullptr;
    }
    CloseHandle(token);
    LPWSTR sid = nullptr;
    if (!ConvertSidToStringSidW(reinterpret_cast<TOKEN_USER*>(storage.data())->User.Sid, &sid))
        return nullptr;
    std::wstring sddl = L"D:P(A;;GA;;;";
    sddl += sid;
    sddl += L")";
    LocalFree(sid);
    PSECURITY_DESCRIPTOR result = nullptr;
    if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(
            sddl.c_str(), SDDL_REVISION_1, &result, nullptr)) return nullptr;
    return result;
}
int wmain(int argc, wchar_t** argv) {
    if (argc != 3) {
        fwprintf(stderr, L"Usage: ncal_host.exe <endpoint> <absolute-vendor-dll>\n");
        return 2;
    }
    if (!adapter_initialize(argv[2])) return 3;
    auto sd = current_user_descriptor();
    if (!sd) { adapter_finalize(); return 4; }
    RPC_STATUS status = RpcServerUseProtseqEpW(
        reinterpret_cast<RPC_WSTR>(const_cast<wchar_t*>(L"ncalrpc")),
        RPC_C_PROTSEQ_MAX_REQS_DEFAULT, reinterpret_cast<RPC_WSTR>(argv[1]), nullptr);
    if (status == RPC_S_OK)
        status = RpcServerRegisterAuthInfoW(nullptr, RPC_C_AUTHN_WINNT, nullptr, nullptr);
    if (status == RPC_S_OK)
        status = RpcServerRegisterIf3(NCAL_SERVER_INTERFACE, nullptr, nullptr,
            RPC_IF_ALLOW_LOCAL_ONLY | RPC_IF_ALLOW_SECURE_ONLY,
            RPC_C_LISTEN_MAX_CALLS_DEFAULT, 16 * 1024 * 1024, nullptr, sd);
    if (status == RPC_S_OK) {
        wprintf(L"READY endpoint=%ls bits=%zu\n", argv[1], sizeof(void*) * 8);
        fflush(stdout);
        status = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, FALSE);
        RpcServerUnregisterIf(NCAL_SERVER_INTERFACE, nullptr, TRUE);
    }
    LocalFree(sd);
    adapter_finalize();
    if (status != RPC_S_OK) fwprintf(stderr, L"RPC server error: %lu\n", status);
    return status == RPC_S_OK ? 0 : 5;
}
