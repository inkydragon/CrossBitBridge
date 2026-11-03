#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
// status: 0 success, 1 integer overflow, 2 invalid buffer. cdecl; x86 only.
int32_t __cdecl legacy_add(int32_t a, int32_t b, int32_t* result);
int32_t __cdecl legacy_scale(uint32_t count, const float* input, float factor, float* output);
#ifdef __cplusplus
}
#endif
