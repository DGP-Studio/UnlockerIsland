#pragma once

#pragma comment(lib, "ntdll.lib")

#include <Windows.h>

#define LDR_ADDREF_DLL_PIN 0x00000001

EXTERN_C NTSYSAPI NTSTATUS NTAPI LdrAddRefDll(_In_ ULONG Flags, _In_ PVOID DllHandle);