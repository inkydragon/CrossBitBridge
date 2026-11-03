#pragma once
#include <stdint.h>
#ifdef NCAL_CLIENT_BUILD
#define NCAL_API __declspec(dllexport)
#else
#define NCAL_API __declspec(dllimport)
#endif
#ifdef __cplusplus
extern "C" {
#endif
// Return value is transport status (0 success), app_status is the vendor status.
// Calls are synchronous; endpoint is a null-terminated UTF-16 string.
// Strings and buffers are borrowed from the caller and must remain valid until return.
NCAL_API uint32_t __cdecl ncal_ping(const wchar_t* endpoint, uint32_t* server_bits);
NCAL_API uint32_t __cdecl ncal_add(const wchar_t* endpoint, int32_t a, int32_t b,
                                  int32_t* result, int32_t* app_status);
NCAL_API uint32_t __cdecl ncal_scale(const wchar_t* endpoint, uint32_t count,
    const float* input, float factor, float* output, int32_t* app_status);
#ifdef __cplusplus
}
#endif
