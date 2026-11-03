#pragma once
#include <windows.h>
// A project adapter implements these lifecycle hooks plus Impl_* from bridge.h.
// Initialization runs before RPC starts; finalization runs after calls drain.
bool adapter_initialize(const wchar_t* absolute_dll_path);
void adapter_finalize();
