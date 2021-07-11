#pragma once
#include <Windows.h>
#include <stdint.h>


void InstallHook(void* func2hook, void* payloadFunc);
void InstallDX11Hooks();

__declspec(noinline) void PopAddress(uint64_t trampolinePtr);

inline void** get_vtable_ptr(void* obj)
{
	return *reinterpret_cast<void***>(obj);
}

