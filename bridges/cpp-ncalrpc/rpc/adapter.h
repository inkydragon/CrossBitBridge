#pragma once
#include <filesystem>
// A project adapter implements these lifecycle hooks plus Impl_* from bridge.gen.h.
// Initialization runs before RPC starts; finalization runs after calls drain.
bool adapter_initialize(const std::filesystem::path& absolute_dll_path);
void adapter_finalize();
