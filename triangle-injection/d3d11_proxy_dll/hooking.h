#pragma once
#include <Windows.h>
#include <stdint.h>


void InstallHook(void* func2hook, void* payloadFunc);
__declspec(noinline) void PopAddress(uint64_t trampolinePtr);

