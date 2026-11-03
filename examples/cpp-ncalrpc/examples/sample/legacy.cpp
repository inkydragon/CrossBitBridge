#include "legacy.h"
#include <limits>
static_assert(sizeof(void*) == 4, "This example DLL must be x86");
int32_t __cdecl legacy_add(int32_t a, int32_t b, int32_t* result) {
    if (!result) return 2;
    const int64_t sum = static_cast<int64_t>(a) + b;
    if (sum < std::numeric_limits<int32_t>::min() || sum > std::numeric_limits<int32_t>::max())
        return 1;
    *result = static_cast<int32_t>(sum);
    return 0;
}
int32_t __cdecl legacy_scale(uint32_t count, const float* input, float factor, float* output) {
    if (count && (!input || !output)) return 2;
    for (uint32_t i = 0; i < count; ++i) output[i] = input[i] * factor;
    return 0;
}
