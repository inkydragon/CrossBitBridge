#include <windows.h>
#include <cstdio>
#include <cstdint>
#include <vector>
#include "client.h"
#include "binding.h"
static_assert(sizeof(void*) == 8, "Probe must be x64");
#define CHECK(condition)                                                                           \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            fwprintf(stderr, L"FAIL line %d\n", __LINE__);                                         \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)

// Raise a synthetic fatal exception: verify it escapes rpc_try to this outer handler.
static bool fatal_exception_propagates() {
    __try {
        rpc_try([] { RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, nullptr); });
    } __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER
                                                                 : EXCEPTION_CONTINUE_SEARCH) {
        return true;
    }
    return false;
}

int wmain(int argc, wchar_t** argv) {
    if (argc != 2)
        return 2;
    CHECK(rpc_try([] { RpcRaiseException(RPC_S_SERVER_UNAVAILABLE); }) == RPC_S_SERVER_UNAVAILABLE);
    CHECK(fatal_exception_propagates());
    const auto endpoint = argv[1];
    uint32_t bits = 0;
    uint32_t status = RPC_S_SERVER_UNAVAILABLE;
    for (int attempt = 0; attempt < 100; ++attempt) {
        status = ncal_ping(endpoint, &bits);
        if (!status)
            break;
        Sleep(50);
    }
    CHECK(status == 0 && bits == 32);
    int32_t value = 0, app = -1;
    CHECK(ncal_add(endpoint, 19, 23, &value, &app) == 0 && app == 0 && value == 42);
    CHECK(ncal_add(endpoint, INT32_MAX, 1, &value, &app) == 0 && app == 1 && value == 0);
    float input[] = {1.0f, -2.0f, 0.0f, 3.5f};
    float output[4] = {};
    CHECK(ncal_scale(endpoint, 4, input, 2.0f, output, &app) == 0 && app == 0);
    CHECK(output[0] == 2 && output[1] == -4 && output[2] == 0 && output[3] == 7);
    CHECK(ncal_scale(endpoint, 0, nullptr, 2, nullptr, &app) == 0 && app == 0);
    std::vector<float> large_in(1048576, 1.25f), large_out(1048576);
    CHECK(ncal_scale(endpoint, 1048576, large_in.data(), 2, large_out.data(), &app) == 0 &&
          app == 0);
    CHECK(large_out.front() == 2.5f && large_out.back() == 2.5f);
    CHECK(ncal_scale(endpoint, 1048577, nullptr, 1, nullptr, &app) == ERROR_INVALID_PARAMETER);
    CHECK(ncal_scale(endpoint, 1, nullptr, 1, output, &app) == ERROR_INVALID_PARAMETER);
    CHECK(ncal_ping(nullptr, &bits) == ERROR_INVALID_PARAMETER);
    CHECK(ncal_ping(L"", &bits) == ERROR_INVALID_PARAMETER && bits == 0);
    CHECK(ncal_add(endpoint, 1, 2, nullptr, &app) == ERROR_INVALID_PARAMETER);
    CHECK(ncal_scale(endpoint, 1, input, 1, nullptr, &app) == ERROR_INVALID_PARAMETER);
    CHECK(ncal_scale(endpoint, 0, nullptr, 1, nullptr, nullptr) == ERROR_INVALID_PARAMETER);
    // Exercise cleanup after each RPC exception, then verify a healthy call still works.
    for (int attempt = 0; attempt < 16; ++attempt) {
        bits = 99;
        value = 99;
        app = 99;
        CHECK(ncal_ping(L"NcalRpc-no-such-endpoint", &bits) != 0 && bits == 0);
        CHECK(ncal_add(L"NcalRpc-no-such-endpoint", 1, 2, &value, &app) != 0);
        CHECK(value == 0 && app == -1);
        CHECK(ncal_scale(L"NcalRpc-no-such-endpoint", 0, nullptr, 1, nullptr, &app) != 0);
        CHECK(app == -1);
    }
    CHECK(ncal_add(endpoint, 19, 23, &value, &app) == 0 && app == 0 && value == 42);
    wprintf(L"PASS: x64 client -> ncalrpc -> x86 DLL; scalar, overflow, arrays, empty, bounds, "
            L"missing endpoint\n");
    return 0;
}
